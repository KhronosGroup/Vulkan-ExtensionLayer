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

#pragma once

/**
 * @file list.h
 *
 * @brief Embedded list implementation
 */

#include <stdbool.h>

struct list_head
{
    struct list_head *prev;
    struct list_head *next;
};

static inline void list_inithead(struct list_head *head)
{
    head->prev = head;
    head->next = head;
}

static inline void list_add(struct list_head *item, struct list_head *list)
{
    list->next->prev = item;
    item->next = list->next;

    list->next = item;
    item->prev = list;
}

static inline void list_addtail(struct list_head *item, struct list_head *list)
{
    list->prev->next = item;
    item->prev = list->prev;

    list->prev = item;
    item->next = list;
}

static inline void list_del(struct list_head *item)
{
    item->next->prev = item->prev;
    item->prev->next = item->next;
}

static inline bool list_empty(struct list_head *list)
{
    return list->prev == list;
}

#define list_entry(__type, __item, __field)                     \
    ((__type *) (((char*)__item) - offsetof(__type, __field)))

#define list_for_each_entry_safe(__type, __name, __list, __field)       \
    for (__type *__name = list_entry(__type, (__list)->next, __field),  \
             *__tmp = list_entry(__type, (__list)->next->next, __field); \
         &(__name)->__field != (__list);                                \
         __name = __tmp, __tmp = list_entry(__type, __tmp->__field.next, __field))

#define list_for_each_entry(__type, __name, __list, __field)            \
    for (__type *__name = list_entry(__type, (__list)->next, __field);  \
         &(__name->__field) != (__list);                                \
         __name = list_entry(__type, __name->__field.next, __field))

#define list_for_each_entry_rev(__type, __name, __list, __field)        \
    for (__type *__name = list_entry(__type, (__list)->prev, __field);  \
         &(__name->__field) != (__list);                                \
         __name = list_entry(__type, __name->__field.prev, __field))

#define list_first_entry(__list, __type, __field)       \
    list_entry(__type, (__list)->next, __field)

