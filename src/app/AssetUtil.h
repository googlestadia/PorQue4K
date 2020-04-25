/*
 Copyright 2018-2019 Google Inc.

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

#ifndef __COMMON_ASSET_UTIL_H__
#define __COMMON_ASSET_UTIL_H__

#include "vkex/Application.h"

namespace asset_util {

enum MemoryUsage {
  MEMORY_USAGE_UNDEFINED = 0,

  // Allocated from system heap (HOST_VISIBLE)
  MEMORY_USAGE_CPU_ONLY,

  // Allocated from device heap (DEVICE_LOCAL)
  MEMORY_USAGE_GPU_ONLY,

  // Allocated from BAR0 heap (HOST_VISIBLE | DEVICE_LOCAL)
  MEMORY_USAGE_CPU_TO_GPU,
};

std::vector<uint8_t> LoadFile(const vkex::fs::path& file_path);
void DetermineMemoryFlags(MemoryUsage memory_usage, bool& device_local,
                          bool& host_visible);

vkex::Result CreateShaderProgramCompute(vkex::Device device,
                                        const vkex::fs::path& cs_path,
                                        vkex::ShaderProgram* p_shader_program);

vkex::Result CreateShaderProgram(vkex::Device device,
                                 const vkex::fs::path& vs_path,
                                 const vkex::fs::path& ps_path,
                                 vkex::ShaderProgram* p_shader_program);

vkex::Result CreateTexture(const vkex::fs::path& image_file_path,
                           vkex::Queue queue, MemoryUsage memory_usage,
                           vkex::Texture* p_texture);

vkex::Result CreateTexture(size_t src_data_size, const uint8_t* p_src_data,
                           int width, int height, VkFormat format,
                           vkex::Queue queue, MemoryUsage memory_usage,
                           vkex::Texture* p_texture);

vkex::Result CreateConstantBuffer(size_t size, const void* p_data,
                                  vkex::Queue queue, MemoryUsage memory_usage,
                                  vkex::Buffer* p_buffer);

vkex::Result CreateIndexBuffer(size_t size, const void* p_data,
                               vkex::Queue queue, MemoryUsage memory_usage,
                               vkex::Buffer* p_buffer);

vkex::Result CreateVertexBuffer(size_t size, const void* p_data,
                                vkex::Queue queue, MemoryUsage memory_usage,
                                vkex::Buffer* p_buffer);

vkex::Result CreateGeometryBuffer(size_t size, const void* p_data,
                                  vkex::Queue queue, MemoryUsage memory_usage,
                                  vkex::Buffer* p_buffer);

vkex::Result CreateStorageBuffer(size_t size, const void* p_data,
                                 vkex::Queue queue, MemoryUsage memory_usage,
                                 vkex::Buffer* p_buffer);

}  // namespace asset_util

#endif  // __COMMON_ASSET_UTIL_H__