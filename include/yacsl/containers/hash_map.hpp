#pragma once

#include "yacsl/compiler/attributes.hpp"
#include "yacsl/containers/hash.hpp"
#include "yacsl/containers/option.hpp"
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

template <typename K, typename V, usize N = 0,
          memory::AllocatorType A = memory::MainArenaAllocator,
          HashType<K>           H = Hash<K, usize>, typename SizeT = usize>
class HashMap {
public:
  constexpr explicit HashMap(SizeT capacity, A const& allocator = A()) noexcept
      : nodes_(capacity, utils::NIL, allocator) {}

  template <meta::MaybeCVRef<K> KF, typename... Args>
  constexpr V& insert(this HashMap& self, KF&& key, Args&&... args) {
    ++self.pair_count_;
    if (self.pair_count_ >= self.nodes_.len() * 2 / 3) {
      self.rehash(self.pair_count_ * 2);
    }
    auto const start_index =
        static_cast<SizeT>(self.hash_(key)) % self.nodes_.len();
    for (auto i = start_index; i < start_index + self.nodes_.len(); ++i) {
      Node& node = self.nodes_[i % self.nodes_.len()];
      if (!node.template is_a<Pair>()) {
        Pair& pair = node.template emplace<Pair>(
            memory::forward<KF>(key), memory::forward<Args>(args)...);
        return pair.value;
      }
    }
    debug::unreachable("");
  }

  constexpr SizeT capacity(this HashMap const& self) noexcept {
    return self.nodes_.len();
  }

  constexpr void rehash(this HashMap& self, SizeT new_size) {
    auto nodes  = memory::move(self.nodes_);
    self.nodes_ = Nodes(new_size, utils::NIL);
    for (Node& node : nodes) {
      if (node.template is_a<Pair>()) {
        Pair& pair = node.template get_unchecked<Pair>();
        self.insert(memory::move(pair.key), memory::move(pair.value));
      }
    }
  }

  template <typename Self>
  // NOLINTNEXTLINE(cppcoreguidelines-missing-std-forward)
  constexpr Option<meta::CopyCVRef<Self, V>> get_option(this Self&& self,
                                                        K const& key) noexcept {
    auto const start_index =
        static_cast<SizeT>(self.hash_(key)) % self.nodes_.len();
    for (auto i = start_index; i < start_index + self.nodes_.len(); ++i) {
      auto const index = i % self.nodes_.len();
      auto&&     node  = self.nodes_[index];
      if (node.template is_a<Pair>()) {
        auto&& pair = node.template get_unchecked<Pair>();
        if (pair.key == key) {
          return memory::forward<meta::CopyCVRef<Self, V>>(pair.value);
        }
      }
    }
    return utils::NIL;
  }

  template <typename Self>
  constexpr meta::CopyCVRef<Self, V> get(this Self&& self,
                                         K const&    key) noexcept {
    return memory::forward<Self>(self).get_option(key).value();
  }

  template <typename Self>
  constexpr meta::CopyCVRef<Self, V> get_unchecked(this Self&& self,
                                                   K const&    key) noexcept {
    return memory::forward<Self>(self).get_option(key).value_unchecked();
  }

  template <typename Self> constexpr auto begin(this Self&& self) noexcept {
    for (auto& node : self.nodes_) {
      if (node.template is_a<Pair>()) {
        return Iterator<Self&&>(memory::forward<Self>(self), &node);
      }
    }
    return Iterator<Self&&>(memory::forward<Self>(self), self.nodes_.end());
  }

  template <typename Self> constexpr auto end(this Self&& self) noexcept {
    return Iterator<Self&&>(memory::forward<Self>(self), self.nodes_.end());
  }

public:
  struct Pair {
    template <meta::MaybeCVRef<K> KF, typename... Args>
    constexpr Pair(KF&& key_ref, Args&&... args)
        : key(memory::forward<KF>(key_ref)),
          value(memory::forward<Args>(args)...) {}
    K key;
    V value;
  };

private:
  using Tombstone = meta::TaggedType<"Tombstone">;
  using Node      = Union<Pair, Tombstone>;
  using Nodes     = Vector<Node, N, A, SizeT>;

public:
  template <typename Map> class Iterator {
  public:
    constexpr explicit Iterator(Map map, Node* ptr) noexcept
        : ptr_(ptr), end_(map.nodes_.end()) {}

    constexpr bool operator==(this Iterator, Iterator) noexcept = default;

    constexpr meta::CopyCVRef<Map, Pair>
    operator*(this Iterator self) noexcept {
      return self.ptr_->template get_unchecked<Pair>();
    }

    template <typename Self> Self&& operator++(this Self&& self) noexcept {
      ++self.ptr_;
      while (self.ptr_ != self.end_ && !self.ptr_->template is_a<Pair>()) {
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
