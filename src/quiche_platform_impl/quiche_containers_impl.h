// Copyright 2017 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NET_THIRD_PARTY_QUICHE_OVERRIDES_QUICHE_PLATFORM_IMPL_QUICHE_CONTAINERS_IMPL_H_
#define NET_THIRD_PARTY_QUICHE_OVERRIDES_QUICHE_PLATFORM_IMPL_QUICHE_CONTAINERS_IMPL_H_

#include <vector>
#include <set>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <deque>
#include <string>

namespace quiche {

// 内联向量实现
template <typename T, size_t N, typename A = std::allocator<T>>
using QuicheInlinedVectorImpl = std::vector<T, A>;

// 小型有序集合实现
template <typename Key, typename Compare = std::less<Key>>
using QuicheSmallOrderedSetImpl = std::set<Key, Compare>;

// 小型映射实现
template <typename Key, typename Value, typename Compare = std::less<Key>>
using QuicheSmallMapImpl = std::map<Key, Value, Compare>;

// 无序映射实现
template <typename Key, typename Value, typename Hash = std::hash<Key>, typename Pred = std::equal_to<Key>>
using QuicheUnorderedMapImpl = std::unordered_map<Key, Value, Hash, Pred>;

// 无序集合实现
template <typename Key, typename Hash = std::hash<Key>, typename Pred = std::equal_to<Key>>
using QuicheUnorderedSetImpl = std::unordered_set<Key, Hash, Pred>;

// 双端队列实现
template <typename T>
using QuicheDequeImpl = std::deque<T>;

// 字符串实现
using QuicheStringImpl = std::string;

}  // namespace quiche

#endif  // NET_THIRD_PARTY_QUICHE_OVERRIDES_QUICHE_PLATFORM_IMPL_QUICHE_CONTAINERS_IMPL_H_
