#pragma once

#include "yacsl/compiler/attributes.hpp"
#include "yacsl/containers/hash.hpp"
#include "yacsl/containers/option.hpp"
#include "yacsl/containers/result.hpp"
#include "yacsl/containers/union.hpp"
#include "yacsl/containers/vector.hpp"
#include "yacsl/debug/panic.hpp"
#include "yacsl/memory/allocator_type.hpp"
#include "yacsl/memory/arena.hpp"
#include "yacsl/memory/lifetimes.hpp"
#include "yacsl/meta/qualifiers.hpp"
#include "yacsl/meta/tagged_type.hpp"
#include "yacsl/utils/nil.hpp"

namespace yacsl::containers {

template <typename T, usize N = 0,
          memory::AllocatorType A = memory::MainArenaAllocator,
          HashType<T>           H = Hash<T, usize>, typename SizeT = usize>
class HashSet {
public:
  constexpr explicit HashSet(SizeT capacity, A const& allocator = A()) noexcept
      : nodes_(capacity, utils::NIL, allocator) {}

  template <meta::MaybeCVRef<T> TF, typename... Args>
  constexpr Result<T&, T&> try_insert(this HashSet& self, TF&& key,
                                      Args&&... args) {
    auto existing = self.get_node(key);
    if (existing.is_value()) {
      return Result<T&, T&>::new_error(
          existing.value_unchecked().template get_unchecked<T>());
    }

    ++self.pair_count_;
    if (self.pair_count_ >= self.nodes_.len() * 2 / 3) {
      self.rehash(self.pair_count_ * 2);
    }
    auto const start_index =
        static_cast<SizeT>(self.hash_(key)) % self.nodes_.len();

    for (auto i = start_index; i < start_index + self.nodes_.len(); ++i) {
      Node& node = self.nodes_[i % self.nodes_.len()];
      if (!node.template is_a<T>()) {
        T& pair = node.template emplace<T>(memory::forward<TF>(key),
                                           memory::forward<Args>(args)...);
        return Result<T&, T&>::new_value(pair);
      }
    }
    debug::unreachable("");
  }

  template <meta::MaybeCVRef<T> TF, typename... Args>
  constexpr T& insert(this HashSet& self, TF&& key, Args&&... args) {
    return self
        .try_insert(memory::forward<TF>(key), memory::forward<Args>(args)...)
        .value();
  }
  template <meta::MaybeCVRef<T> TF, typename... Args>
  constexpr T& insert_unchecked(this HashSet& self, TF&& key, Args&&... args) {
    return self
        .try_insert(memory::forward<TF>(key), memory::forward<Args>(args)...)
        .value_unchecked();
  }

  constexpr bool try_remove(this HashSet& self, T const& key) noexcept {
    Option<Node&> node = self.get_node(key);
    if (node.is_nil()) {
      return false;
    }
    node.value_unchecked().template emplace<Tombstone>();
    --self.pair_count_;
    return true;
  }
  constexpr void remove(this HashSet& self, T const& key) noexcept {
    --self.pair_count_;
    self.get_node(key).value().template emplace<Tombstone>();
  }
  constexpr void remove_unchecked(this HashSet& self, T const& key) noexcept {
    --self.pair_count_;
    self.get_node(key).value_unchecked().template emplace<Tombstone>();
  }

  constexpr SizeT capacity(this HashSet const& self) noexcept {
    return self.nodes_.len();
  }

  constexpr void rehash(this HashSet& self, SizeT new_size) {
    auto nodes       = memory::move(self.nodes_);
    self.nodes_      = Nodes(new_size, utils::NIL);
    self.pair_count_ = 0;
    for (Node& node : nodes) {
      if (node.template is_a<T>()) {
        T& node_key = node.template get_unchecked<T>();
        self.insert(memory::move(node_key));
      }
    }
  }

  template <typename Self>
  // NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward)
  constexpr Option<meta::CopyCVRef<Self, T>> get_option(this Self&& self,
                                                        T const& key) noexcept {
    return memory::forward<Self>(self).get_node(key).transform(
        [](meta::CopyCVRef<Self, Node> node) -> meta::CopyCVRef<Self, T> {
          return memory::forward<meta::CopyCVRef<Self, T>>(
              node.template get_unchecked<T>());
        });
  }

  template <typename Self>
  constexpr meta::CopyCVRef<Self, T> get(this Self&& self,
                                         T const&    key) noexcept {
    return memory::forward<Self>(self).get_option(key).value();
  }

  template <typename Self>
  constexpr meta::CopyCVRef<Self, T> get_unchecked(this Self&& self,
                                                   T const&    key) noexcept {
    return memory::forward<Self>(self).get_option(key).value_unchecked();
  }

  template <typename Self> constexpr auto begin(this Self&& self) noexcept {
    for (auto& node : self.nodes_) {
      if (node.template is_a<T>()) {
        return Iterator<Self&&>(memory::forward<Self>(self), &node);
      }
    }
    return Iterator<Self&&>(memory::forward<Self>(self), self.nodes_.end());
  }

  template <typename Self> constexpr auto end(this Self&& self) noexcept {
    return Iterator<Self&&>(memory::forward<Self>(self), self.nodes_.end());
  }

private:
  using Tombstone = meta::TaggedType<"Tombstone">;
  using Node      = Union<T, Tombstone>;
  using Nodes     = Vector<Node, N, A, SizeT>;

  template <typename Self>
  constexpr Option<meta::CopyCVRef<Self, Node>>
  // NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward)
  get_node(this Self&& self, T const& key) noexcept {
    auto const start_index =
        static_cast<SizeT>(self.hash_(key)) % self.nodes_.len();
    for (auto i = start_index; i < start_index + self.nodes_.len(); ++i) {
      auto const index = i % self.nodes_.len();
      auto&&     node  = self.nodes_[index];
      if (node.template is_a<T>()) {
        auto&& node_key = node.template get_unchecked<T>();
        if (node_key == key) {
          return memory::forward<meta::CopyCVRef<Self, Node>>(node);
        }
      } else if (!node.template is_a<Tombstone>()) {
        return utils::NIL;
      }
    }
    return utils::NIL;
  }

public:
  template <typename Map> class Iterator {
  public:
    constexpr explicit Iterator(Map map, Node* ptr) noexcept
        : ptr_(ptr), end_(map.nodes_.end()) {}

    constexpr bool operator==(this Iterator, Iterator) noexcept = default;

    constexpr meta::CopyCVRef<Map, T> operator*(this Iterator self) noexcept {
      return self.ptr_->template get_unchecked<T>();
    }

    template <typename Self> Self&& operator++(this Self&& self) noexcept {
      ++self.ptr_;
      while (self.ptr_ != self.end_ && !self.ptr_->template is_a<T>()) {
        ++self.ptr_;
      }
      return memory::forward<Self>(self);
    }

  private:
    meta::CopyCV<Map, Node>* ptr_;
    meta::CopyCV<Map, Node>* end_;
  };

private:
  Nodes                     nodes_;
  SizeT                     pair_count_ = 0;
  YACSL_NO_UNIQUE_ADDRESS H hash_;
};

} // namespace yacsl::containers
