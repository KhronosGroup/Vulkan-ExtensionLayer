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

#ifndef HASH_TABLE_H_
#define HASH_TABLE_H_

/**
 * @file hash_table.h
 *
 * @brief Contains a small hash table utility for C code
 */

#include <stdint.h>

struct hash_table;

struct hash_table *hash_table_new(void);
void hash_table_destroy(struct hash_table *table);

void hash_table_insert(struct hash_table *table, uint64_t key, void *data);
void *hash_table_search(struct hash_table *table, uint64_t key);
void hash_table_remove(struct hash_table *table, uint64_t key);

#endif /* HASH_TABLE_H_ */
