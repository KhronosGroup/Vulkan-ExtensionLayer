/* Copyright (c) 2015-2017, 2019-2021 The Khronos Group Inc.
 * Copyright (c) 2015-2017, 2019-2021 Valve Corporation
 * Copyright (c) 2015-2017, 2019-2021 LunarG, Inc.
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
 *
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Courtney Goeltzenleuchter <courtney@LunarG.com>
 * Author: Dave Houlton <daveh@lunarg.com>
 */

#pragma once
#include <vector>
#include <functional>
#include <unordered_map>

// Minimum Visual Studio 2015 Update 2, or libc++ with C++17
#if defined(_MSC_FULL_VER) && _MSC_FULL_VER >= 190023918 && NTDDI_VERSION > NTDDI_WIN10_RS2 && \
    (!defined(_LIBCPP_VERSION) || __cplusplus >= 201703)
#include <shared_mutex>
#else
#include <mutex>
#endif

class ReadWriteLock {
  private:
#if defined(_MSC_FULL_VER) && _MSC_FULL_VER >= 190023918 && NTDDI_VERSION > NTDDI_WIN10_RS2 && \
    (!defined(_LIBCPP_VERSION) || __cplusplus >= 201703)
    typedef std::shared_mutex lock_t;
#else
    typedef std::mutex lock_t;
#endif

  public:
    void lock() { m_lock.lock(); }
    bool try_lock() { return m_lock.try_lock(); }
    void unlock() { m_lock.unlock(); }
#if defined(_MSC_FULL_VER) && _MSC_FULL_VER >= 190023918 && NTDDI_VERSION > NTDDI_WIN10_RS2 && \
    (!defined(_LIBCPP_VERSION) || __cplusplus >= 201703)
    void lock_shared() { m_lock.lock_shared(); }
    bool try_lock_shared() { return m_lock.try_lock_shared(); }
    void unlock_shared() { m_lock.unlock_shared(); }
#else
    void lock_shared() { lock(); }
    bool try_lock_shared() { return try_lock(); }
    void unlock_shared() { unlock(); }
#endif
  private:
    lock_t m_lock;
};

#if defined(_MSC_FULL_VER) && _MSC_FULL_VER >= 190023918 && NTDDI_VERSION > NTDDI_WIN10_RS2 && \
    (!defined(_LIBCPP_VERSION) || __cplusplus >= 201703)
typedef std::shared_lock<ReadWriteLock> read_lock_guard_t;
typedef std::unique_lock<ReadWriteLock> write_lock_guard_t;
#else
typedef std::unique_lock<ReadWriteLock> read_lock_guard_t;
typedef std::unique_lock<ReadWriteLock> write_lock_guard_t;
#endif

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
class vk_concurrent_unordered_map {
  public:
    void insert_or_assign(const Key &key, const T &value) {
        uint32_t h = ConcurrentMapHashObject(key);
        write_lock_guard_t lock(locks[h].lock);
        maps[h][key] = value;
    }

    bool insert(const Key &key, const T &value) {
        uint32_t h = ConcurrentMapHashObject(key);
        write_lock_guard_t lock(locks[h].lock);
        auto ret = maps[h].insert(typename std::unordered_map<Key, T>::value_type(key, value));
        return ret.second;
    }

    // returns size_type
    size_t erase(const Key &key) {
        uint32_t h = ConcurrentMapHashObject(key);
        write_lock_guard_t lock(locks[h].lock);
        return maps[h].erase(key);
    }

    bool contains(const Key &key) const {
        uint32_t h = ConcurrentMapHashObject(key);
        read_lock_guard_t lock(locks[h].lock);
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

    FindResult find(const Key &key) const {
        uint32_t h = ConcurrentMapHashObject(key);
        read_lock_guard_t lock(locks[h].lock);

        auto itr = maps[h].find(key);
        bool found = itr != maps[h].end();

        if (found) {
            return FindResult(true, itr->second);
        } else {
            return end();
        }
    }

    FindResult pop(const Key &key) {
        uint32_t h = ConcurrentMapHashObject(key);
        write_lock_guard_t lock(locks[h].lock);

        auto itr = maps[h].find(key);
        bool found = itr != maps[h].end();

        if (found) {
            auto ret = std::move(FindResult(true, itr->second));
            maps[h].erase(itr);
            return ret;
        } else {
            return end();
        }
    }

    std::vector<std::pair<const Key, T>> snapshot(std::function<bool(T)> f = nullptr) const {
        std::vector<std::pair<const Key, T>> ret;
        for (int h = 0; h < BUCKETS; ++h) {
            read_lock_guard_t lock(locks[h].lock);
            for (auto j : maps[h]) {
                if (!f || f(j.second)) {
                    ret.push_back(j);
                }
            }
        }
        return ret;
    }

  private:
    static const int BUCKETS = (1 << BUCKETSLOG2);

    std::unordered_map<Key, T, Hash> maps[BUCKETS];
    struct {
        mutable ReadWriteLock lock;
        // Put each lock on its own cache line to avoid false cache line sharing.
        char padding[(-int(sizeof(ReadWriteLock))) & 63];
    } locks[BUCKETS];

    uint32_t ConcurrentMapHashObject(const Key &object) const {
        uint64_t u64 = (uint64_t)(uintptr_t)object;
        uint32_t hash = (uint32_t)(u64 >> 32) + (uint32_t)u64;
        hash ^= (hash >> BUCKETSLOG2) ^ (hash >> (2 * BUCKETSLOG2));
        hash &= (BUCKETS - 1);
        return hash;
    }
};
