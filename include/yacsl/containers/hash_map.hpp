#pragma once

#include "yacsl/compiler/attributes.hpp"
#include "yacsl/containers/hash.hpp"
#include "yacsl/containers/union.hpp"
#include "yacsl/containers/vector.hpp"
#include "yacsl/memory/allocator_type.hpp"
#include "yacsl/memory/arena.hpp"
#include "yacsl/meta/tagged_type.hpp"

namespace yacsl::containers {

template <typename K, typename V, usize N = 0,
          memory::AllocatorType A = memory::MainArenaAllocator,
          HashType<K>           H = Hash<K, usize>, typename SizeT = usize>
class HashMap {
public:
private:
  struct Pair {
    K key;
    V value;
  };
  using NeverUsed = meta::TaggedType<"NeverUsed">;
  using Tombstone = meta::TaggedType<"Tombstone">;
  using Node      = Union<Pair, NeverUsed, Tombstone>;

  Vector<Node, N, A, SizeT> nodes_;
  YACSL_NO_UNIQUE_ADDRESS A allocator_;
  YACSL_NO_UNIQUE_ADDRESS H hash_;
};

} // namespace yacsl::containers
