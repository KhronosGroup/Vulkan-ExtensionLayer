/*
 * Copyright © 2019 Intel Corporation
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

#include <unordered_map>

extern "C"
{

#include "hash_table.h"

struct hash_table
{
    hash_table() {}
    ~hash_table() {}

    std::unordered_map<uint64_t, void*> map;
};

struct hash_table *hash_table_new(void)
{
    struct hash_table *table = new hash_table;

    return table;
}

void hash_table_destroy(struct hash_table *table)
{
    delete table;
}

void hash_table_insert(struct hash_table *table, uint64_t key, void *data)
{
    table->map.insert(std::make_pair(key, data));
}

void *hash_table_search(struct hash_table *table, uint64_t key)
{
    auto iter = table->map.find(key);

    if (iter == table->map.end())
        return nullptr;

    return iter->second;
}

void hash_table_remove(struct hash_table *table, uint64_t key)
{
    auto iter = table->map.find(key);

    if (iter == table->map.end())
        return;

    table->map.erase(iter);
}

} // extern "C"
