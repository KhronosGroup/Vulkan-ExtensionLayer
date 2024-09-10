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

#pragma once

// clang-format off

#include <type_traits>
#include <cassert>
#include <bitset>
#include <mutex>
#include <shared_mutex>
#include <functional>
#include <cstring>

#include <vulkan/vulkan.h>

#include "log.h"
#include "vk_api_hash.h"

namespace shader_object {

#define SHADER_OBJECT_DEBUG_UTILS_STR_LENGTH 128u
    
enum ShaderType {
    VERTEX_SHADER = 0,
    FRAGMENT_SHADER,
    TESSELLATION_CONTROL_SHADER,
    TESSELLATION_EVALUATION_SHADER,
    GEOMETRY_SHADER,
    MESH_SHADER,
    TASK_SHADER,
    NUM_SHADERS
};

static VKAPI_ATTR void* VKAPI_CALL DefaultAlloc(void*, size_t size, size_t alignment, VkSystemAllocationScope) {
    return std::malloc(size);
}

static VKAPI_ATTR void VKAPI_CALL DefaultFree(void*, void* pMem) {
    std::free(pMem);
}

static VKAPI_ATTR void* VKAPI_CALL DefaultRealloc(void*, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope) {
    return std::realloc(pOriginal, size);
}

static const VkAllocationCallbacks kDefaultAllocator = {
    nullptr, DefaultAlloc, DefaultRealloc, DefaultFree, nullptr, nullptr,
};

#include "generated/shader_object_constants.h"
#include "generated/shader_object_entry_points_x_macros.inl"

struct LayerDispatchDevice {
#define ENTRY_POINT(name) PFN_vk##name name = nullptr;
#define ENTRY_POINT_ALIAS(alias, canon)
    ENTRY_POINTS_DEVICE
    ADDITIONAL_DEVICE_FUNCTIONS
#undef ENTRY_POINT_ALIAS
#undef ENTRY_POINT

    void Initialize(VkDevice device, PFN_vkGetDeviceProcAddr get_proc_addr) {
#define ENTRY_POINT_ALIAS(alias, canon) if (canon == nullptr) { canon = (PFN_vk##canon)get_proc_addr(device, "vk" #alias); }
#define ENTRY_POINT(name) ENTRY_POINT_ALIAS(name, name)
        ENTRY_POINTS_DEVICE
        ADDITIONAL_DEVICE_FUNCTIONS
#undef ENTRY_POINT
#undef ENTRY_POINT_ALIAS
    }
};

inline bool operator==(VkStencilOpState const& a, VkStencilOpState const& b) { return memcmp(&a, &b, sizeof(a)) == 0; }

inline bool operator==(VkPipelineColorBlendAttachmentState const& a, VkPipelineColorBlendAttachmentState const& b) {
    return memcmp(&a, &b, sizeof(a)) == 0;
}

inline bool operator==(VkVertexInputAttributeDescription const& a, VkVertexInputAttributeDescription const& b) {
    return memcmp(&a, &b, sizeof(a)) == 0;
}

inline bool operator==(VkVertexInputBindingDescription const& a, VkVertexInputBindingDescription const& b) {
    return memcmp(&a, &b, sizeof(a)) == 0;
}

inline bool operator==(VkViewportSwizzleNV const& a, VkViewportSwizzleNV const& b) {
    return memcmp(&a, &b, sizeof(a)) == 0;
}

class AlignedMemory {
  public:
    AlignedMemory() : size_(0), max_alignment_(1), memory_write_ptr_(nullptr) {}

    // Reserve memory for T[count] respecting T's alignment
    template<typename T>
    constexpr void Add(size_t count = 1) {
        if (count > 0) {
            size_ += align(std::alignment_of<T>::value, size_);
            size_ += sizeof(T) * count;

            if (max_alignment_ < std::alignment_of<T>::value) {
                max_alignment_ = std::alignment_of<T>::value;
            }
        }
    }

    template<typename T>
    void SetMemoryWritePtr(T* ptr) {
        assert(memory_write_ptr_ == nullptr);
        memory_write_ptr_ = reinterpret_cast<uint8_t*>(ptr);
    }

    // Allocate memory required for storing all arrays added with Add
    void Allocate(VkAllocationCallbacks const& allocator, VkSystemAllocationScope scope) {
        assert(GetSize() > 0);
        SetMemoryWritePtr(allocator.pfnAllocation(allocator.pUserData, GetSize(), GetAlignment(), scope));
    }

    // Fetch a pointer to array T[count] that was reserved via Add
    // The order, types and counts in which GetNextAlignedPtr is called must exactly match those of calls to Add
    template<typename T>
    T* GetNextAlignedPtr(size_t count = 1) {
        T* result = nullptr;

        if (count > 0) {
            memory_write_ptr_ += align(std::alignment_of<T>::value, reinterpret_cast<size_t>(memory_write_ptr_));
            result = reinterpret_cast<T*>(memory_write_ptr_);
            memory_write_ptr_ += sizeof(T) * count;
        }

        return result;
    }

    // Fetches a pointer to A[src_size / sizeof(A)] array and copies data from src_ptr to it
    // src_size must be aligned to sizeof(A)
    template<typename A, typename T, typename P>
    void CopyBytes(P*& dst_ptr, T& dst_size, void const* src_ptr, T src_size) {
        void* data_ptr = static_cast<void*>(GetNextAlignedPtr<A>(src_size / sizeof(A)));

        if (src_size > 0) {
            memcpy(data_ptr, src_ptr, src_size);
        }

        dst_size = src_size;
        dst_ptr = static_cast<P*>(data_ptr);
    }

    // Fetches a pointer to P[src_count] array and copies data from src_ptr to it
    template<typename P, typename T>
    void CopyStruct(P*& dst_ptr, uint32_t& dst_count, T const* src_ptr, uint32_t src_count) {
        static_assert(std::is_same<const P, const T>::value);

        size_t dst_size;
        CopyBytes<T>(dst_ptr, dst_size, src_ptr, sizeof(T) * src_count);
        dst_count = src_count;
    }

    size_t GetSize() const { return size_ + align(max_alignment_, size_); }
    size_t GetAlignment() const { return max_alignment_; }

    uint8_t* GetMemoryWritePtr() const { return memory_write_ptr_; }

    explicit operator bool() const { return memory_write_ptr_ != nullptr; }

  private:
    size_t align(size_t alignment, size_t offset) const {
        return (alignment - (offset % alignment)) % alignment;
    }

    size_t size_;
    size_t max_alignment_;
    uint8_t* memory_write_ptr_;
};

template <typename T>
class ReaderWriterContainer {
  public:
    ReaderWriterContainer() = default;
    ReaderWriterContainer(T&& data) : data_(std::move(data)) {}

    T const& GetDataForReading(std::shared_lock<std::shared_mutex>& lock) {
        lock = std::shared_lock<std::shared_mutex>(mutex_);
        return data_;
    }

    T& GetDataForWriting(std::unique_lock<std::shared_mutex>& lock) {
        lock = std::unique_lock<std::shared_mutex>(mutex_);
        return data_;
    }

    T& GetDataUnsafe() {
        return data_;
    }

  private:
    std::shared_mutex mutex_;
    T data_;
};

template <typename T, VkSystemAllocationScope Scope>
class DynamicArray {
  public:
    DynamicArray(VkAllocationCallbacks allocator) : allocator_(allocator) {}
    DynamicArray(VkAllocationCallbacks allocator, uint32_t initial_size) : DynamicArray(allocator) { Resize(initial_size); }
    ~DynamicArray() { CleanupData(); }

    class Iterator {
      public:
        T& operator*() { return (*array_)[index_]; }
        T* operator->() { return &(*array_)[index_]; }

        void operator++() { ++index_; }

        bool operator==(Iterator const& o) { return array_ == o.array_ && index_ == o.index_; }
        bool operator!=(Iterator const& o) { return !(*this == o); }

      private:
        friend class DynamicArray<T, Scope>;

        Iterator(DynamicArray& array, uint32_t index) : array_(&array), index_(index) {}

        DynamicArray* array_;
        uint32_t index_;
    };

    DynamicArray(DynamicArray const& o) { *this = o; }
    DynamicArray(DynamicArray&& o) noexcept { *this = std::move(o); }
    DynamicArray& operator=(DynamicArray const& o) {
        CleanupData();

        allocator_ = o.allocator_;
        used_ = o.used_;
        capacity_ = o.capacity_;
        if (o.used_ > 0) {
            data_ = (T*)allocator_.pfnAllocation(allocator_.pUserData, sizeof(T) * capacity_, alignof(T), Scope);

            for (uint32_t i = 0; i < used_; ++i) {
                new (data_ + i) T(o.data_[i]);
            }
        }

        return *this;
    }
    DynamicArray& operator=(DynamicArray&& o) noexcept {
        CleanupData();

        allocator_ = o.allocator_;
        used_ = o.used_;
        capacity_ = o.capacity_;
        data_ = o.data_;
        o.data_ = nullptr;
        return *this;
    }

    T& operator[](uint32_t i) {
        ASSERT(i < used_);
        return data_[i];
    }

    T const& operator[](uint32_t i) const {
        ASSERT(i < used_);
        return data_[i];
    }

    void Resize(uint32_t new_size) {
        if (!data_) {
            capacity_ = new_size;
            used_ = new_size;

            data_ = (T*)allocator_.pfnAllocation(allocator_.pUserData, sizeof(T) * capacity_, alignof(T),
                                                 VkSystemAllocationScope::VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
            data_ = new (data_) T[new_size];
            return;
        }

        if (new_size > capacity_) {
            capacity_ = new_size;
            data_ = (T*)allocator_.pfnReallocation(allocator_.pUserData, data_, sizeof(T) * capacity_, alignof(T),
                                                   VkSystemAllocationScope::VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

            new (data_ + used_) T[new_size - used_];
            used_ = new_size;
            return;
        }

        for (uint32_t i = new_size; i < used_; ++i) {
            data_[i].~T();
        }

        used_ = new_size;
    }

    void Clear() { Resize(0); }

    uint32_t GetUsed() const { return used_; }

    bool IsEmpty() const { return GetUsed() == 0; }

    T* GetPointer() const { return data_; }

    Iterator begin() { return Iterator(*this, 0); }

    Iterator end() { return Iterator(*this, used_); }

  private:
    // Destruct and deallocate all data if it's there
    void CleanupData() {
        if (data_) {
            for (uint32_t i = 0; i < used_; ++i) {
                data_[i].~T();
            }

            allocator_.pfnFree(allocator_.pUserData, data_);
            data_ = nullptr;
        }
    }

    T* data_ = nullptr;
    uint32_t capacity_ = 0;
    uint32_t used_ = 0;

    VkAllocationCallbacks allocator_;
};

template <typename Key, typename Value, bool UseMutex = true>
class HashMap {
  public:
    class Iterator {
      public:
        struct ValuePair {
            Key const& key;
            Value& value;
        };

        Iterator(Iterator const& o) { *this = o; }

        Iterator& operator=(Iterator const& o) {
            map_ = o.map_;
            index_ = o.index_;
            return *this;
        }

        ValuePair operator*() {
            HashMap::Slot& slot = map_->slots_[index_];
            return {slot.key, slot.value};
        };

        void operator++() {
            if (index_ < map_->slots_.GetUsed()) {
                ++index_;
            }

            while (index_ < map_->slots_.GetUsed() && map_->slots_[index_].state != Slot::State::OCCUPIED) {
                ++index_;
            }
        }

        bool operator==(Iterator const& o) { return map_ == o.map_ && index_ == o.index_; }
        bool operator!=(Iterator const& o) { return !(*this == o); }

        Key const& GetKey() { return map_->slots_[index_].key; }

        Value& GetValue() { return map_->slots_[index_].value; }

      private:
        friend class HashMap<Key, Value, UseMutex>;

        Iterator(HashMap& map, uint32_t start_index) : map_(&map), index_(start_index) {}

        HashMap* map_;
        uint32_t index_;
    };

    HashMap() : allocator_(kDefaultAllocator), slots_(kDefaultAllocator) {}

    HashMap(HashMap const& o) { *this = o; };
    HashMap(HashMap&& o) noexcept { *this = std::move(o); };

    HashMap& operator=(HashMap const& o) {
        allocator_ = o.allocator_;
        slots_ = o.slots_;
        num_entries_ = o.num_entries_;
        hasher_ = o.hasher_;
        return *this;
    }
    HashMap& operator=(HashMap&& o) noexcept {
        if (this == &o) {
            return *this;
        }

        allocator_ = std::move(o.allocator_);
        slots_ = std::move(o.slots_);
        num_entries_ = std::move(o.num_entries_);
        hasher_ = std::move(o.hasher_);
        return *this;
    }

    void Add(Key const& key, Value const& value) {
        std::unique_lock<std::mutex> lock = UseMutex ? std::unique_lock<std::mutex>(mutex_) : std::unique_lock<std::mutex>{};

        // See if we're updating an existing key
        auto search_it = FindNoLock(key);
        if (search_it != end()) {
            search_it.GetValue() = value;
            return;
        }

        // Otherwise, we're adding a key
        RehashIfNecessary(num_entries_ + 1);

        const size_t hashed_key = hasher_(key);

        // Find which slot we should insert key and value into
        Slot* found_slot = nullptr;
        uint32_t index = (uint32_t)(hashed_key % slots_.GetUsed());
        for (;;) {
            Slot& slot = slots_[index];

            if (slot.state != Slot::State::OCCUPIED) {
                found_slot = &slot;
                break;
            }

            index = (index + 1) % slots_.GetUsed();
        }

        // Fill the slot
        ++num_entries_;
        found_slot->key = key;
        found_slot->value = value;
        found_slot->state = Slot::State::OCCUPIED;
        found_slot->hashed_key = hashed_key;
    }

    // Might rehash
    void Remove(Key const& key) {
        std::unique_lock<std::mutex> lock = UseMutex ? std::unique_lock<std::mutex>(mutex_) : std::unique_lock<std::mutex>{};

        auto it = FindNoLock(key);
        if (it != end()) {
            RemoveNoLock(it);
            RehashIfNecessary(num_entries_);
        }
    }

    // Does not rehash
    Iterator Remove(Iterator it) {
        std::unique_lock<std::mutex> lock = UseMutex ? std::unique_lock<std::mutex>(mutex_) : std::unique_lock<std::mutex>{};

        return RemoveNoLock(it);
    }

    void RemoveAllWithValue(Value const& value) {
        std::unique_lock<std::mutex> lock = UseMutex ? std::unique_lock<std::mutex>(mutex_) : std::unique_lock<std::mutex>{};

        for (auto it = begin(); it != end();) {
            if (it.GetValue() == value) {
                it = RemoveNoLock(it);
            } else {
                ++it;
            }
        }
    }

    void RemoveAllWithValueCustom(Value const& value, std::function<void(Key const&, Value const&)> const& custom_function) {
        std::unique_lock<std::mutex> lock = UseMutex ? std::unique_lock<std::mutex>(mutex_) : std::unique_lock<std::mutex>{};

        for (auto it = begin(); it != end();) {
            if (it.GetValue() == value) {
                custom_function(it.GetKey(), it.GetValue());
                it = RemoveNoLock(it);
            } else {
                ++it;
            }
        }
    }

    void Clear() {
        slots_.Clear();
        num_entries_ = 0;
    }

    const Value& Get(Key const& key) { return *GetOrNullptr(key); }

    const Value* GetOrNullptr(Key const& key) const {
        std::unique_lock<std::mutex> lock = UseMutex ? std::unique_lock<std::mutex>(mutex_) : std::unique_lock<std::mutex>{};

        if (slots_.IsEmpty()) {
            return nullptr;
        }

        const size_t hashed_key = hasher_(key);
        const uint32_t start_index = (uint32_t)(hashed_key % slots_.GetUsed());

        uint32_t index = start_index;
        for (;;) {
            const Slot& slot = slots_[index];

            if (slot.state == Slot::State::OCCUPIED && slot.key == key) {
                // We found the key
                return &slots_[index].value;
            }

            if (slot.state == Slot::State::UNOCCUPIED) {
                // If we came across an unoccupied slot, we've gone past the end of the cluster
                // i.e. we didn't find the key
                return nullptr;
            }

            // If we reach here, we're still in the cluster
            // i.e. this is a deleted slot or it's an occupied slot with the wrong key

            index = (index + 1) % slots_.GetUsed();
            if (index == start_index) {
                // We searched through all the slots and didn't find it
                return nullptr;
            }
        }
    }

    Iterator Find(Key const& key) {
        std::unique_lock<std::mutex> lock = UseMutex ? std::unique_lock<std::mutex>(mutex_) : std::unique_lock<std::mutex>{};
        return FindNoLock(key);
    }

    uint32_t NumSlots() const { return slots_.GetUsed(); }

    uint32_t NumEntries() const { return num_entries_; }

    Iterator begin() {
        auto it = Iterator(*this, 0);

        // Start the iterator at first valid slot
        if (!slots_.IsEmpty() && slots_[0].state != Slot::State::OCCUPIED) {
            ++it;
        }

        return it;
    }

    Iterator end() { return Iterator(*this, slots_.GetUsed()); }

  private:
    friend class HashMap::Iterator;

    struct Slot {
        enum class State { UNOCCUPIED, OCCUPIED, DELETED };

        size_t hashed_key{};
        Key key{};
        Value value{};
        State state = State::UNOCCUPIED;
    };

    Iterator FindNoLock(Key const& key) {
        if (slots_.IsEmpty()) {
            return end();
        }

        const size_t hashed_key = hasher_(key);
        const uint32_t start_index = (uint32_t)(hashed_key % slots_.GetUsed());

        uint32_t index = start_index;
        for (;;) {
            Slot& slot = slots_[index];

            if (slot.state == Slot::State::OCCUPIED && slot.key == key) {
                // We found the key
                return Iterator(*this, index);
            }

            if (slot.state == Slot::State::UNOCCUPIED) {
                // If we came across an unoccupied slot, we've gone past the end of the cluster
                // i.e. we didn't find the key
                return end();
            }

            // If we reach here, we're still in the cluster
            // i.e. this is a deleted slot or it's an occupied slot with the wrong key

            index = (index + 1) % slots_.GetUsed();
            if (index == start_index) {
                // We searched through all the slots and didn't find it
                return end();
            }
        }
    }

    Iterator RemoveNoLock(Iterator it) {
        ASSERT(it.map_ == this);

        if (it == end()) return it;

        // Delete the slot specified by the iterator
        slots_[it.index_].state = Slot::State::DELETED;
        --num_entries_;

        // Return iterator to next element
        uint32_t next_index = (it.index_ + 1) % slots_.GetUsed();
        Iterator next_it(*this, next_index);
        if (slots_[next_index].state != Slot::State::OCCUPIED) {
            ++next_it;
        }
        return next_it;
    }

    static float CalculateLoadFactor(uint32_t num_entries, uint32_t num_buckets) { return num_entries / (float)num_buckets; }

    // If the key exists, the slot is returned. Otherwise, the next unoccupied slot is returned
    template <VkSystemAllocationScope Scope>
    static Slot& FindSlotOrNextUnoccupied(DynamicArray<Slot, Scope>& slots, Key const& key, size_t hashed_key) {
        uint32_t index = (uint32_t)(hashed_key % slots.GetUsed());
        for (;;) {
            Slot& slot = slots[index];

            if ((slot.state == Slot::State::OCCUPIED && slot.key == key) || slot.state == Slot::State::UNOCCUPIED) {
                return slot;
            }

            // Skip if it's a deleted slot or an occupied slot with wrong key

            index = (index + 1) % slots.GetUsed();
        }
    }

    void RehashIfNecessary(uint32_t entries) {
        uint32_t required_slots = entries;
        if (required_slots > slots_.GetUsed()) {
            Resize(required_slots * 2);
            return;
        }

        if (required_slots < slots_.GetUsed() / 4) {
            Resize(slots_.GetUsed() / 2);
            return;
        }
    }

    void Resize(uint32_t new_size) {
        DynamicArray<Slot, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT> new_slots(allocator_, new_size);

        for (Slot& old_slot : slots_) {
            if (old_slot.state != Slot::State::OCCUPIED) {
                continue;
            }

            Slot& found_slot = FindSlotOrNextUnoccupied(new_slots, old_slot.key, old_slot.hashed_key);
            ASSERT(found_slot.state == Slot::State::UNOCCUPIED);
            found_slot.key = std::move(old_slot.key);
            found_slot.value = std::move(old_slot.value);
            found_slot.state = Slot::State::OCCUPIED;
            found_slot.hashed_key = old_slot.hashed_key;
        }

        std::swap(slots_, new_slots);
    }

    VkAllocationCallbacks allocator_;
    DynamicArray<Slot, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT> slots_;

    uint32_t num_entries_ = 0;

    std::hash<Key> hasher_;

    mutable std::mutex mutex_;
};

struct Shader;

// Encapsulation of Shader to accurately compare Shaders even if they are out of their lifetimes (they could alias memory location)
class ComparableShader {
public:
    ComparableShader() = default;
    explicit ComparableShader(Shader* shader);

    bool operator==(ComparableShader const& other) const { return id_ == other.id_; }

    Shader* GetShaderPtr() const { return shader_; }

private:
    Shader* shader_;
    uint64_t id_;
};

struct DeviceData;
class CommandBufferData;
void UpdateDrawState(CommandBufferData& data, VkCommandBuffer commandBuffer);

// All relevant draw state for a single command buffer
struct FullDrawStateData {
    struct Limits {
        Limits() {}

        Limits(VkPhysicalDeviceProperties const& properties)
            : max_color_attachments(properties.limits.maxColorAttachments),
              max_vertex_input_attributes(properties.limits.maxVertexInputAttributes),
              max_vertex_input_bindings(properties.limits.maxVertexInputBindings),
              max_viewports(properties.limits.maxViewports) {}

        uint32_t max_color_attachments{};
        uint32_t max_vertex_input_attributes{};
        uint32_t max_vertex_input_bindings{};
        uint32_t max_viewports{};
    };

    // Key wraps a FullDrawStateData pointer so that it may be used as a key in a HashMap
    class Key {
      public:
        Key() : draw_state_data_(nullptr), is_owner_(false) {}
        ~Key() {
            if (is_owner_ && draw_state_data_) {
                FullDrawStateData::Destroy(draw_state_data_);
                draw_state_data_ = nullptr;
            }
        }

        Key(Key const& o) {
            // When a key is copied, the data that it wraps should be deep copied along with it
            // This happens when they key is inserted into the HashMap

            if (o.draw_state_data_) {
                draw_state_data_ = FullDrawStateData::Copy(o.draw_state_data_);
                is_owner_ = true;
            } else {
                draw_state_data_ = nullptr;
                is_owner_ = false;
            }
        }

        Key(Key&& o) noexcept : draw_state_data_(o.draw_state_data_), is_owner_(o.is_owner_) {
            o.draw_state_data_ = nullptr;
            o.is_owner_ = false;
        }

        Key& operator=(Key const& o) {
            // See copy constructor

            if (&o == this) {
                return *this;
            }

            if (o.draw_state_data_) {
                draw_state_data_ = FullDrawStateData::Copy(o.draw_state_data_);
                is_owner_ = true;
            } else {
                draw_state_data_ = nullptr;
                is_owner_ = false;
            }
            return *this;
        }

        Key& operator=(Key&& o) noexcept {
            if (&o == this) {
                return *this;
            }

            draw_state_data_ = o.draw_state_data_;
            is_owner_ = o.is_owner_;
            o.draw_state_data_ = nullptr;
            o.is_owner_ = false;
            return *this;
        }

        bool operator==(Key const& o) const {
            return draw_state_data_ && o.draw_state_data_ && (*draw_state_data_ == *o.draw_state_data_);
        }

        FullDrawStateData* GetData() const { return draw_state_data_; }

      private:
        friend struct FullDrawStateData;

        Key(FullDrawStateData* data) : draw_state_data_(data), is_owner_(false) {}

        FullDrawStateData* draw_state_data_;
        bool is_owner_ = false;
    };

#include "generated/shader_object_full_draw_state_utility_functions.inl"

    static void InitializeMemory(void* memory, VkPhysicalDeviceProperties const& properties,
                                 bool dynamic_rendering_unused_attachments) {
        FullDrawStateData* state = new (memory) FullDrawStateData{};
        Limits limits(properties);
        SetInternalArrayPointers(state, limits);
        state->limits_ = limits;

        // Set default draw state for feature-rich pipeline compilation
        VkPipelineColorBlendAttachmentState color_blend_attachment_state{
            VK_TRUE,
            VK_BLEND_FACTOR_SRC_COLOR,
            VK_BLEND_FACTOR_DST_COLOR,
            VK_BLEND_OP_ADD,
            VK_BLEND_FACTOR_SRC_ALPHA,
            VK_BLEND_FACTOR_DST_ALPHA,
            VK_BLEND_OP_ADD,
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT};
        state->SetColorBlendAttachmentState(0, color_blend_attachment_state);
        state->SetColorAttachmentFormat(0, VK_FORMAT_R8G8B8A8_UNORM);
        state->SetNumColorAttachments(1);
        state->SetPrimitiveTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        state->SetPolygonMode(VK_POLYGON_MODE_FILL);
        state->SetCullMode(VK_CULL_MODE_FRONT_BIT);
        state->SetDepthCompareOp(VK_COMPARE_OP_LESS);
        state->SetDepthTestEnable(VK_TRUE);
        state->SetDepthWriteEnable(VK_TRUE);
        state->SetDepthBoundsTestEnable(VK_TRUE);
        state->SetStencilTestEnable(VK_TRUE);
        state->dynamic_rendering_unused_attachments_ = dynamic_rendering_unused_attachments;
        if (dynamic_rendering_unused_attachments) {
            for (uint32_t i = 0; i < limits.max_color_attachments; ++i) {
                state->SetColorBlendAttachmentState(i, color_blend_attachment_state);
                state->SetColorAttachmentFormat(i, VK_FORMAT_R8G8B8A8_UNORM);
            }

            state->SetDepthAttachmentFormat(VK_FORMAT_D24_UNORM_S8_UINT);
            state->SetStencilAttachmentFormat(VK_FORMAT_D24_UNORM_S8_UINT);
        }
    }

    static FullDrawStateData* Create(VkPhysicalDeviceProperties const& properties, VkAllocationCallbacks const& allocator,
                                     bool dynamic_rendering_unused_attachments) {
        AlignedMemory aligned_memory;
        ReserveMemory(aligned_memory, properties);

        aligned_memory.Allocate(allocator, VkSystemAllocationScope::VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
        if (!aligned_memory) {
            return nullptr;
        }

        auto state = aligned_memory.GetNextAlignedPtr<FullDrawStateData>();
        InitializeMemory(state, properties, dynamic_rendering_unused_attachments);
        state->allocator_ = allocator;
        return state;
    }

    static FullDrawStateData* Copy(FullDrawStateData const* o) {
        AlignedMemory aligned_memory;
        ReserveMemory(aligned_memory, o->limits_);
        auto allocator = kDefaultAllocator;

        aligned_memory.Allocate(allocator, VkSystemAllocationScope::VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
        if (!aligned_memory) {
            return nullptr;
        }
        memcpy(aligned_memory.GetMemoryWritePtr(), o, aligned_memory.GetSize());

        auto state = aligned_memory.GetNextAlignedPtr<FullDrawStateData>();
        SetInternalArrayPointers(state, o->limits_);
        state->allocator_ = allocator;
        return state;
    }

    static void Destroy(FullDrawStateData* pState) {
        auto allocator = pState->allocator_;
        pState->~FullDrawStateData();
        allocator.pfnFree(allocator.pUserData, pState);
    }

    size_t GetHash() const {
        if (dirty_hash_bits_.any()) {
            for (uint32_t i = 0; i < dirty_hash_bits_.size(); ++i) {
                if (!dirty_hash_bits_.test(i)) {
                    continue;
                }

                // undo previous partial hash
                final_hash_ ^= partial_hashes_[i];

                // compute new partial hash
                partial_hashes_[i] = CalculatePartialHash((StateGroup)i);

                // apply new partial hash
                final_hash_ ^= partial_hashes_[i];

                dirty_hash_bits_.reset(i);
            }
        }

        return final_hash_;
    }

    Key GetKey() { return Key(this); }

    void MarkDirty() { is_dirty_ = true; }

#include "generated/shader_object_full_draw_state_struct_members.inl"

  private:
    friend void UpdateDrawState(CommandBufferData& data, VkCommandBuffer commandBuffer);

    FullDrawStateData() = default;

    Limits limits_;
    VkAllocationCallbacks allocator_;
    bool dynamic_rendering_unused_attachments_;

    mutable size_t final_hash_ = 0;
    mutable size_t partial_hashes_[NUM_STATE_GROUPS]{};
    mutable std::bitset<NUM_STATE_GROUPS> dirty_hash_bits_{0xFFFFFFFF};
    mutable bool is_dirty_ = true;
};

} // namespace shader_object

namespace std {

template<>
struct hash<shader_object::FullDrawStateData::Key> {
    std::size_t operator()(shader_object::FullDrawStateData::Key const& o) const {
        shader_object::FullDrawStateData* data = o.GetData();
        if (!data) {
            return 0;
        }
        return data->GetHash();
    }
};

}  // namespace std

namespace shader_object {

struct PartialPipeline {
    // The precompiled partial pipeline
    VkPipeline pipeline = VK_NULL_HANDLE;

    // Information about the pipeline
    FullDrawStateData* draw_state;
    VkGraphicsPipelineLibraryFlagBitsEXT library_flags;
    VkShaderStageFlags shader_stages;
};

struct Shader {
    struct PrivateDataSlotPair {
        VkPrivateDataSlot slot;
        uint64_t          data;
    };

    static VkResult Create(DeviceData const& deviceData, VkShaderCreateInfoEXT const& createInfo, VkAllocationCallbacks const& allocator, Shader** ppShader);
    static void     Destroy(DeviceData const& deviceData, Shader* pShader, VkAllocationCallbacks const& allocator);

    uint64_t GetPrivateData(DeviceData const& device_data, VkPrivateDataSlot slot);
    void     SetPrivateData(DeviceData const& device_data, VkPrivateDataSlot slot, uint64_t data);

    uint64_t id;

    const char* name;
    size_t      name_byte_count;

    void*  spirv_data;
    size_t spirv_data_size;

    VkPushConstantRange*   push_constant_ranges;
    uint32_t               num_push_constant_ranges;
    VkDescriptorSetLayout* descriptor_set_layouts;
    uint32_t               num_descriptor_set_layouts;

    // Points to specialization_info if there is any specialization info
    VkSpecializationInfo* specialization_info_ptr;
    VkSpecializationInfo  specialization_info;

    VkShaderModule                   shader_module;
    VkShaderStageFlagBits            stage;
    VkPipelineShaderStageCreateFlags flags;

    HashMap<VkPrivateDataSlot, uint64_t> private_data;
    PrivateDataSlotPair*                 reserved_private_data_slots;

    // Associates draw states related to this shader with pipelines. Only used for shaders that are always present (i.e. vertex or mesh)
    ReaderWriterContainer<HashMap<FullDrawStateData::Key, VkPipeline, false>> pipelines;

    // Pipeline cache that is generated at create time (if it's not being created from binary) and is copied into cache
    // This gets serialized into the shader binary
    VkPipelineCache pristine_cache = VK_NULL_HANDLE;

    // The pipeline layout to be used with this shader
    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;

    // Used for draw time pipeline creation
    VkPipelineCache cache = VK_NULL_HANDLE;

    // If possible, holds a partial pipeline created with graphics pipeline library at create time that may be used to speed up draw time pipeline creation
    PartialPipeline partial_pipeline;
};

class ShaderBinary {
  public:
    enum ShaderBinaryFlagBits { NONE = 0x0, HAS_PIPELINE_CACHE = 0x1 };
    using ShaderBinaryFlags = uint16_t;

    static constexpr uint32_t kMagic = 0x50B1EC75; // "S OBJECTS"

    static VkResult Create(DeviceData const& deviceData, Shader const& shader, void* out);

    void const* GetSprivData() const { return reinterpret_cast<uint8_t const*>(this) + sizeof(ShaderBinary); }
    void      * GetSprivData()       { return reinterpret_cast<uint8_t*>(this) + sizeof(ShaderBinary); }
    void const* GetPipelineCacheData() const { return reinterpret_cast<uint8_t const*>(GetSprivData()) + spirv_data_size; }
    void      * GetPipelineCacheData()       { return reinterpret_cast<uint8_t*>(GetSprivData()) + spirv_data_size; }

    uint32_t              magic;
    uint16_t              version;
    ShaderBinaryFlags     flags;
    VkShaderStageFlagBits stage;
    uint64_t              spirv_checksum;
    size_t                spirv_data_size;     // SPIR-V code is required
    size_t                pipeline_cache_size; // Pipeline cache is optional, this may be 0

  private:
    ShaderBinary() = default;
};

// Data that is specific to a single device
struct DeviceData {
    enum FlagBits {
        SHADER_OBJECT_LAYER_ENABLED        = 1u << 0,
        HAS_PRIMITIVE_TOPLOGY_UNRESTRICTED = 1u << 1,
        DISABLE_PIPELINE_PRE_CACHING       = 1u << 2,
    };
    using Flags = uint32_t;

    void*    FindStateSettingFunctionByName(const char* pName);
    void     AddDynamicState(VkDynamicState state);
    bool     HasDynamicState(VkDynamicState state) const;

    VkDevice                   device;
    Flags                      flags;
    AdditionalExtensionFlags   enabled_extensions;
    VkPhysicalDeviceProperties properties;
    LayerDispatchDevice        vtable;
    VkPrivateDataSlot          private_data_slot;
    VkFormat                   supported_depth_stencil_format;
    VkPipelineLayout           dummy_pipeline_layout;
    VkDynamicState             dynamic_states[kMaxDynamicStates];
    uint32_t                   dynamic_state_count;
    uint32_t                   reserved_private_data_slot_count;

    // In the future, this could be improved by utilizing private data if it's available on the device
    HashMap<VkImageView, VkFormat> image_view_format_map;
    struct NameInfo {
        char name[SHADER_OBJECT_DEBUG_UTILS_STR_LENGTH];
    };
    HashMap<Shader*, NameInfo>    debug_utils_object_name_map;
    struct TagInfo {
        uint64_t tagName;
        char     tag[SHADER_OBJECT_DEBUG_UTILS_STR_LENGTH];
    };
    HashMap<Shader*, TagInfo>      debug_utils_object_tag_map;

    #include "generated/shader_object_device_data_declare_extension_variables.inl"
};

class CommandBufferData {
  public:
    static void               ReserveMemory(AlignedMemory& aligned_memory, VkPhysicalDeviceProperties const& properties);
    static CommandBufferData* Create(DeviceData* data, VkAllocationCallbacks allocator);
    static void               Destroy(CommandBufferData** data);

    FullDrawStateData* GetDrawStateData() { return draw_state_data_; }

    DeviceData*           device_data;
    VkAllocationCallbacks allocator;
    VkCommandPool         pool;
    VkPipelineLayout      last_seen_pipeline_layout_;

    // To save 8 bytes, this information could be implicitly embedded in the draw state
    bool graphics_bind_point_belongs_to_layer;

  private:
    CommandBufferData() = default;
    FullDrawStateData* draw_state_data_;
};

}  // namespace shader_object
