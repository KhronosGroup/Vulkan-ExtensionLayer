/* Copyright (c) 2015-2017, 2019-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2017, 2019-2023 Valve Corporation
 * Copyright (c) 2015-2017, 2019-2023 LunarG, Inc.
 * Modifications Copyright (C) 2022 RasterGrid Kft.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <cassert>
#include <cstddef>
#include <cstring>
#include <functional>
#include <unordered_map>
#include <string>
#include <vector>
#include <bitset>
#include <iomanip>
#include <array>
#include <mutex>
#include <memory>

#ifndef WIN32
#include <strings.h>  // For ffs()
#else
#include <intrin.h>  // For __lzcnt()
#endif

#define STRINGIFY(s) STRINGIFY_HELPER(s)
#define STRINGIFY_HELPER(s) #s

#if defined __PRETTY_FUNCTION__
#define VVL_PRETTY_FUNCTION __PRETTY_FUNCTION__
#else
// For MSVC
#if defined(__FUNCSIG__)
#define VVL_PRETTY_FUNCTION __FUNCSIG__
#else
#define VVL_PRETTY_FUNCTION __FILE__ ":" STRINGIFY(__LINE__)
#endif
#endif

#ifdef __cplusplus
#include <shared_mutex>

// Aliases to avoid excessive typing. We can't easily auto these away because
// there are virtual methods in ValidationObject which return lock guards
// and those cannot use return type deduction.
typedef std::shared_lock<std::shared_mutex> ReadLockGuard;
typedef std::unique_lock<std::shared_mutex> WriteLockGuard;

// helper class for the very common case of getting and then locking a command buffer (or other state object)
template <typename T, typename Guard>
class LockedSharedPtr : public std::shared_ptr<T> {
  public:
    LockedSharedPtr(std::shared_ptr<T> &&ptr, Guard &&guard) : std::shared_ptr<T>(std::move(ptr)), guard_(std::move(guard)) {}
    LockedSharedPtr() : std::shared_ptr<T>(), guard_() {}

  private:
    Guard guard_;
};

// https://en.cppreference.com/w/cpp/thread/hardware_destructive_interference_size
// https://en.wikipedia.org/wiki/False_sharing
// TODO use C++20 to check for std::hardware_destructive_interference_size feature support.
constexpr std::size_t get_hardware_destructive_interference_size() { return 64; }

// Limited concurrent_unordered_map that supports internally-synchronized
// insert/erase/access. Splits locking across N buckets and uses shared_mutex
// for read/write locking. Iterators are not supported. The following
// operations are supported:
//
// insert_or_assign: Insert a new element or update an existing element.
// insert: Insert a new element and return whether it was inserted.
// erase: Remove an element.
// contains: Returns true if the key is in the map.
// find: Returns != end() if found, value is in ret->second.
// pop: Erases and returns the erased value if found.
//
// find/end: find returns a vaguely iterator-like type that can be compared to
// end and can use iter->second to retrieve the reference. This is to ease porting
// for existing code that combines the existence check and lookup in a single
// operation (and thus a single lock). i.e.:
//
//      auto iter = map.find(key);
//      if (iter != map.end()) {
//          T t = iter->second;
//          ...
//
// snapshot: Return an array of elements (key, value pairs) that satisfy an optional
// predicate. This can be used as a substitute for iterators in exceptional cases.
template <typename Key, typename T, int BUCKETSLOG2 = 2, typename Hash = std::hash<Key>>
class vl_concurrent_unordered_map {
  public:
    template <typename... Args>
    void insert_or_assign(const Key &key, Args &&...args) {
        uint32_t h = ConcurrentMapHashObject(key);
        WriteLockGuard lock(locks[h].lock);
        maps[h][key] = {std::forward<Args>(args)...};
    }

    template <typename... Args>
    bool insert(const Key &key, Args &&...args) {
        uint32_t h = ConcurrentMapHashObject(key);
        WriteLockGuard lock(locks[h].lock);
        auto ret = maps[h].emplace(key, std::forward<Args>(args)...);
        return ret.second;
    }

    // returns size_type
    size_t erase(const Key &key) {
        uint32_t h = ConcurrentMapHashObject(key);
        WriteLockGuard lock(locks[h].lock);
        return maps[h].erase(key);
    }

    bool contains(const Key &key) const {
        uint32_t h = ConcurrentMapHashObject(key);
        ReadLockGuard lock(locks[h].lock);
        return maps[h].count(key) != 0;
    }

    // type returned by find() and end().
    class FindResult {
      public:
        FindResult(bool a, T b) : result(a, std::move(b)) {}

        // == and != only support comparing against end()
        bool operator==(const FindResult &other) const {
            if (result.first == false && other.result.first == false) {
                return true;
            }
            return false;
        }
        bool operator!=(const FindResult &other) const { return !(*this == other); }

        // Make -> act kind of like an iterator.
        std::pair<bool, T> *operator->() { return &result; }
        const std::pair<bool, T> *operator->() const { return &result; }

      private:
        // (found, reference to element)
        std::pair<bool, T> result;
    };

    // find()/end() return a FindResult containing a copy of the value. For end(),
    // return a default value.
    FindResult end() const { return FindResult(false, T()); }
    FindResult cend() const { return end(); }

    FindResult find(const Key &key) const {
        uint32_t h = ConcurrentMapHashObject(key);
        ReadLockGuard lock(locks[h].lock);

        auto itr = maps[h].find(key);
        const bool found = itr != maps[h].end();

        if (found) {
            return FindResult(true, itr->second);
        } else {
            return end();
        }
    }

    FindResult pop(const Key &key) {
        uint32_t h = ConcurrentMapHashObject(key);
        WriteLockGuard lock(locks[h].lock);

        auto itr = maps[h].find(key);
        const bool found = itr != maps[h].end();

        if (found) {
            auto ret = FindResult(true, itr->second);
            maps[h].erase(itr);
            return ret;
        } else {
            return end();
        }
    }

    std::vector<std::pair<const Key, T>> snapshot(std::function<bool(T)> f = nullptr) const {
        std::vector<std::pair<const Key, T>> ret;
        for (int h = 0; h < BUCKETS; ++h) {
            ReadLockGuard lock(locks[h].lock);
            for (const auto &j : maps[h]) {
                if (!f || f(j.second)) {
                    ret.emplace_back(j.first, j.second);
                }
            }
        }
        return ret;
    }

    void clear() {
        for (int h = 0; h < BUCKETS; ++h) {
            WriteLockGuard lock(locks[h].lock);
            maps[h].clear();
        }
    }

    size_t size() const {
        size_t result = 0;
        for (int h = 0; h < BUCKETS; ++h) {
            ReadLockGuard lock(locks[h].lock);
            result += maps[h].size();
        }
        return result;
    }

    bool empty() const {
        bool result = 0;
        for (int h = 0; h < BUCKETS; ++h) {
            ReadLockGuard lock(locks[h].lock);
            result |= maps[h].empty();
        }
        return result;
    }

  private:
    static const int BUCKETS = (1 << BUCKETSLOG2);

    std::unordered_map<Key, T, Hash> maps[BUCKETS];
    struct alignas(get_hardware_destructive_interference_size()) AlignedSharedMutex {
        std::shared_mutex lock;
    };
    mutable std::array<AlignedSharedMutex, BUCKETS> locks;

    uint32_t ConcurrentMapHashObject(const Key &object) const {
        uint64_t u64 = (uint64_t)(uintptr_t)object;
        uint32_t hash = (uint32_t)(u64 >> 32) + (uint32_t)u64;
        hash ^= (hash >> BUCKETSLOG2) ^ (hash >> (2 * BUCKETSLOG2));
        hash &= (BUCKETS - 1);
        return hash;
    }
};
#endif
