/*
 * Copyright 2024 Nintendo
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

// clang-format off

#pragma once

template <typename T, uint32_t N>
constexpr uint32_t GetArrayLength(const T (&arr)[N]) {
    return N;
}

constexpr uint32_t CalculateRequiredGroupSize(int x, int group_size) { return (x + group_size - 1) / group_size; }
