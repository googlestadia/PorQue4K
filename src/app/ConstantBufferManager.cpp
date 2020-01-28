/*
 Copyright 2020 Google Inc.

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
*/

#include "ConstantBufferManager.h"

#include "AssetUtil.h"

vkex::Result ConstantBufferManager::Initialize(vkex::Device device, vkex::Queue queue, uint32_t frame_count, size_t per_frame_buffer_size)
{
    m_frame_count = frame_count;
    m_dynamic_constant_buffers.resize(frame_count);
    m_mapped_pointers.resize(frame_count);

    m_current_frame_index = 0;
    m_current_offset = 0;
    m_min_uniform_buffer_offset_alignment = device->GetPhysicalDevice()->GetPhysicalDeviceLimits().minUniformBufferOffsetAlignment;

    for (uint32_t frame_index = 0; frame_index < frame_count; frame_index++) {
        VKEX_CALL(asset_util::CreateConstantBuffer(
            per_frame_buffer_size,
            nullptr,
            queue,
            asset_util::MEMORY_USAGE_CPU_TO_GPU,
            &m_dynamic_constant_buffers[frame_index]));

        {
            VkResult vk_result;
            VKEX_VULKAN_RESULT_CALL(vk_result, 
                m_dynamic_constant_buffers[frame_index]->MapMemory((void**)&(m_mapped_pointers[frame_index])));

            // TODO: Tear this down later?
        }
    }


    return vkex::Result::Success;
}

vkex::Buffer ConstantBufferManager::GetBuffer(uint32_t frame_index)
{
    VKEX_ASSERT(frame_index < m_frame_count);
    return m_dynamic_constant_buffers[frame_index];
}

void ConstantBufferManager::NewFrame(uint32_t new_frame_index)
{
    VKEX_ASSERT(new_frame_index < m_frame_count);

    m_current_offset = 0;
    m_current_frame_index = new_frame_index;
}

bool ConstantBufferManager::AllocDynamicBufferSpace(size_t size, void** out_ptr, uint32_t& dynamic_offset)
{
    auto current_offset = m_current_offset;

    const auto alignment = m_min_uniform_buffer_offset_alignment;
    const auto aligned_size = (size + (alignment - 1)) & ~(alignment - 1);

    const auto buffer_size = m_dynamic_constant_buffers[m_current_frame_index]->GetMemorySize();
    if ((current_offset + aligned_size) > buffer_size) {
        return false;
    }

    dynamic_offset = current_offset;
    *out_ptr = (void*)(m_mapped_pointers[m_current_frame_index] + current_offset);

    m_current_offset += uint32_t(aligned_size);

    return true;
}

uint32_t ConstantBufferManager::UploadToDynamicBuffer(size_t size, const void* p_src)
{
    uint32_t dynamic_offset = UINT32_MAX;
    void* dest_ptr = nullptr;

    bool result = AllocDynamicBufferSpace(size, &dest_ptr, dynamic_offset);
    VKEX_ASSERT(result);
    std::memcpy(dest_ptr, p_src, size);

    return dynamic_offset;
}
