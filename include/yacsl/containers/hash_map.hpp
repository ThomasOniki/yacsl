#pragma once

#include "yacsl/compiler/attributes.hpp"
#include "yacsl/containers/hash.hpp"
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

public:
  struct Pair {
    template <meta::MaybeCVRef<K> KF, typename... Args>
    constexpr Pair(KF&& key_ref, Args&&... args)
        : key(memory::forward<KF>(key_ref)),
          value(memory::forward<Args>(args)...) {}
    K key;
    V value;
  };
  using Tombstone = meta::TaggedType<"Tombstone">;
  using Node      = Union<Pair, Tombstone>;
  using Nodes     = Vector<Node, N, A, SizeT>;

private:
  Nodes                     nodes_;
  SizeT                     pair_count_ = 0;
  YACSL_NO_UNIQUE_ADDRESS H hash_;
};

} // namespace yacsl::containers
