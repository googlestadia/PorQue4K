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

#ifndef __CONSTANT_BUFFER_MANAGER_H__
#define __CONSTANT_BUFFER_MANAGER_H__

#include "vkex/Application.h"

class ConstantBufferManager 
{
public:
    ConstantBufferManager() {}
    virtual ~ConstantBufferManager() {}

    vkex::Result Initialize(vkex::Device device, vkex::Queue queue, uint32_t frame_count, size_t per_frame_buffer_size = kDefaultBufferSizeInBytes);

    vkex::Buffer GetBuffer(uint32_t frame_index);

    void NewFrame(uint32_t new_frame_index);

    uint32_t UploadToDynamicBuffer(size_t size, const void* p_src);
    template<typename T>
    uint32_t UploadConstantsToDynamicBuffer(vkex::ConstantBufferData<T>& constant_buffer_data)
    {
        return UploadToDynamicBuffer(constant_buffer_data.size, &constant_buffer_data.data);
    }

protected:

    enum ConstantBufferManagerConstants {
        kDefaultBufferSizeInBytes = 1024 * 1024,
    };

    bool AllocDynamicBufferSpace(size_t size, void** out_ptr, uint32_t& dynamic_offset);

private:
    std::vector<vkex::Buffer> m_dynamic_constant_buffers;
    std::vector<uint8_t*> m_mapped_pointers;
    VkDeviceSize m_min_uniform_buffer_offset_alignment;

    uint32_t m_current_offset = UINT32_MAX;
    uint32_t m_frame_count = 0;
    uint32_t m_current_frame_index = UINT32_MAX;
};

#endif // __CONSTANT_BUFFER_MANAGER_H__
