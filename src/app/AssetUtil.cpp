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

#include "AssetUtil.h"
#include "vkex/MIPFile.h"

namespace asset_util {

std::vector<uint8_t> LoadFile(const vkex::fs::path& file_path)
{
  if (!vkex::fs::exists(file_path) || !vkex::fs::is_file(file_path)) {
    VKEX_LOG_ERROR("File does not exist: " << file_path);
    return std::vector<uint8_t>();
  }

  std::vector<uint8_t> data = vkex::fs::load_file(file_path);
  VKEX_LOG_INFO("File loaded: " << file_path);

  return data;
}

void DetermineMemoryFlags(
  MemoryUsage memory_usage,
  bool&       device_local,
  bool&       host_visible
)
{
  switch (memory_usage) {
    default: VKEX_ASSERT_MSG(false, "Unknown memory usage!"); break;

    case MEMORY_USAGE_CPU_ONLY: {
      device_local = false;
      host_visible = true;
    }
    break;

    case MEMORY_USAGE_GPU_ONLY: {
      device_local = true;
      host_visible = false;
    }
    break;

    case MEMORY_USAGE_CPU_TO_GPU: {
      device_local = true;
      host_visible = true;
    }
    break;
  }
}

vkex::Result CreateShaderProgramCompute(
  vkex::Device          device,
  const vkex::fs::path& cs_path,
  vkex::ShaderProgram*  p_shader_program
)
{
  auto cs = asset_util::LoadFile(cs_path);
  VKEX_ASSERT_MSG(!cs.empty(), "Compute shader failed to load!");
  if (cs.empty()) {
    return vkex::Result::ErrorComputeShaderLoadFailed;
  }

  vkex::Result result = vkex::CreateShaderProgram(
    device, 
    cs, 
    p_shader_program);
  if (!result) {
    return result;
  }

  return vkex::Result::Success;
}

vkex::Result CreateShaderProgram(
  vkex::Device          device,
  const vkex::fs::path& vs_path,
  const vkex::fs::path& ps_path,
  vkex::ShaderProgram*  p_shader_program
)
{
  auto vs = asset_util::LoadFile(vs_path);
  VKEX_ASSERT_MSG(!vs.empty(), "Vertex shader failed to load!");
  if (vs.empty()) {
    return vkex::Result::ErrorVertexShaderLoadFailed;
  }

  auto ps = asset_util::LoadFile(ps_path);
  VKEX_ASSERT_MSG(!ps.empty(), "Pixel shader failed to load!");
  if (ps.empty()) {
    return vkex::Result::ErrorPixelShaderLoadFailed;
  }

  vkex::Result result = vkex::CreateShaderProgram(
    device, 
    vs, 
    ps, 
    p_shader_program);
  if (!result) {
    return result;
  }

  return vkex::Result::Success;
}

vkex::Result CreateTexture(
  const vkex::fs::path& image_file_path,
  vkex::Queue           queue,
  MemoryUsage           memory_usage,
  vkex::Texture*        p_texture)
{
  VKEX_ASSERT_MSG(queue, "Invalid queue object");
  if (!queue) {
    return vkex::Result::ErrorInvalidQueueObject;
  }

  VKEX_ASSERT_MSG(p_texture != nullptr, "Target texture object is null");
  if (p_texture == nullptr) {
    return vkex::Result::ErrorUnexpectedNullPointer;
  }

  vkex::Device device = queue->GetDevice();
 
  // Load bitmap
  std::unique_ptr<vkex::Bitmap> bitmap;  
  vkex::fs::path mip_path = image_file_path + ".mip";
  if (vkex::fs::exists(mip_path)) {
    MIPFile mip_file = {};    
    bool result = MIPLoadFile(mip_path, &mip_file);
    VKEX_LOG_INFO("File loaded: " << mip_path);

    bitmap = std::make_unique<vkex::Bitmap>(mip_file);
  }
  else {
    auto file_data = LoadFile(image_file_path);
    VKEX_ASSERT_MSG(!file_data.empty(), "Texture failed to load!");

    vkex::Result result = vkex::Bitmap::Create(
      file_data.size(),
      file_data.data(),
      0,
      &bitmap);
    if (!result) {
      return result;
    }
  }

  // Create staging buffer and copy bitmap
  vkex::Buffer cpu_buffer = nullptr;
  {
    uint64_t data_size = bitmap->GetDataSizeAllLevels();

    vkex::BufferCreateInfo create_info        = {};
    create_info.size                          = data_size;
    create_info.usage_flags.bits.transfer_src = true;
    create_info.committed                     = true;
    DetermineMemoryFlags(MEMORY_USAGE_CPU_ONLY, create_info.device_local, create_info.host_visible);
    vkex::Result result = device->CreateStorageBuffer(create_info, &cpu_buffer);
    if (!result) {
      return result;
    }

    // Copy bitmap to data
    result = cpu_buffer->Copy(data_size, bitmap->GetData());
    if (!result) {
      return result;
    }
  }

  // Create image
  {
    vkex::TextureCreateInfo create_info             = {};
    create_info.image.image_type                    = VK_IMAGE_TYPE_2D;
    create_info.image.format                        = bitmap->GetFormat();
    create_info.image.extent                        = bitmap->GetExtent();
    create_info.image.mip_levels                    = bitmap->GetMipLevels();
    create_info.image.tiling                        = VK_IMAGE_TILING_OPTIMAL;
    create_info.image.usage_flags.bits.transfer_dst = true;
    create_info.image.initial_layout                = VK_IMAGE_LAYOUT_UNDEFINED;
    create_info.image.committed                     = true;
    create_info.view.derive_from_image              = true;
    DetermineMemoryFlags(memory_usage, create_info.image.device_local, create_info.image.host_visible);
    vkex::Result result = device->CreateTexture(create_info, p_texture);
    if (!result) {
      return result;
    }
  }

  // Transition from VK_IMAGE_LAYOUT_PREINITIALIZED to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
  // for VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT.
  vkex::Result result = vkex::TransitionImageLayout(
    queue,
    (*p_texture)->GetImage(),
    VK_IMAGE_LAYOUT_UNDEFINED,
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
  if (!result) {
    return result;
  }

  std::vector<VkBufferImageCopy> regions;
  for (uint32_t level = 0; level < bitmap->GetMipLevels(); ++level) {
    vkex::Bitmap::Mip mip = {};
    bitmap->GetMipLayout(level, &mip);
    VkBufferImageCopy region               = {};
    region.bufferOffset                    = mip.data_offset;
    region.bufferRowLength                 = mip.width;
    region.bufferImageHeight               = mip.height;
    region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel       = level;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount     = 1;
    region.imageOffset.x                   = 0;
    region.imageOffset.y                   = 0;
    region.imageOffset.z                   = 0;
    region.imageExtent.width               = mip.width;
    region.imageExtent.height              = mip.height;
    region.imageExtent.depth               = 1;
    regions.push_back(region);
  }

  vkex::CopyResource(
    queue,
    cpu_buffer,
    (*p_texture)->GetImage(),
    vkex::CountU32(regions),
    vkex::DataPtr(regions));

  // Transition from VK_IMAGE_LAYOUT_PREINITIALIZED to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
  // for VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT.
  result = vkex::TransitionImageLayout(
    queue,
    (*p_texture)->GetImage(),
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT); 
  if (!result) {
    return result;
  }

  // Destroy temp CPU buffer
  result = device->DestroyStorageBuffer(cpu_buffer);
  if (!result) {
    return result;
  }

  return vkex::Result::Success;
}

vkex::Result CreateTexture(
    size_t          src_data_size,
    const uint8_t*  p_src_data,
    int             width,
    int             height,
    VkFormat        format,
    vkex::Queue     queue,
    MemoryUsage     memory_usage,
    vkex::Texture*  p_texture)
{
  VKEX_ASSERT_MSG(queue, "Invalid queue object");
  if (!queue) {
    return vkex::Result::ErrorInvalidQueueObject;
  }

  VKEX_ASSERT_MSG(p_texture != nullptr, "Target texture object is null");
  if (p_texture == nullptr) {
    return vkex::Result::ErrorUnexpectedNullPointer;
  }

  vkex::Device device = queue->GetDevice();
 
  // Load bitmap
  std::unique_ptr<vkex::Bitmap> bitmap;  
  {
    vkex::Result result = vkex::Bitmap::Create(
      src_data_size,
      p_src_data,
      width,
      height,
      format,
      0,
      &bitmap);
    if (!result) {
      return result;
    }
  }

  // Create staging buffer and copy bitmap
  vkex::Buffer cpu_buffer = nullptr;
  {
    uint64_t data_size = bitmap->GetDataSizeAllLevels();

    vkex::BufferCreateInfo create_info        = {};
    create_info.size                          = data_size;
    create_info.usage_flags.bits.transfer_src = true;
    create_info.committed                     = true;
    DetermineMemoryFlags(MEMORY_USAGE_CPU_ONLY, create_info.device_local, create_info.host_visible);
    vkex::Result result = device->CreateStorageBuffer(create_info, &cpu_buffer);
    if (!result) {
      return result;
    }

    // Copy bitmap to data
    result = cpu_buffer->Copy(data_size, bitmap->GetData());
    if (!result) {
      return result;
    }
  }

  // Create image
  {
    vkex::TextureCreateInfo create_info             = {};
    create_info.image.image_type                    = VK_IMAGE_TYPE_2D;
    create_info.image.format                        = bitmap->GetFormat();
    create_info.image.extent                        = bitmap->GetExtent();
    create_info.image.mip_levels                    = bitmap->GetMipLevels();
    create_info.image.tiling                        = VK_IMAGE_TILING_OPTIMAL;
    create_info.image.usage_flags.bits.sampled      = true;
    create_info.image.usage_flags.bits.transfer_dst = true;
    create_info.image.initial_layout                = VK_IMAGE_LAYOUT_UNDEFINED;
    create_info.image.committed                     = true;
    create_info.view.derive_from_image              = true;
    DetermineMemoryFlags(memory_usage, create_info.image.device_local, create_info.image.host_visible);
    vkex::Result result = device->CreateTexture(create_info, p_texture);
    if (!result) {
      return result;
    }
  }

  vkex::Result result = vkex::TransitionImageLayout(
    queue,
    (*p_texture)->GetImage(),
    VK_IMAGE_LAYOUT_UNDEFINED,
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
      VK_PIPELINE_STAGE_TRANSFER_BIT);
  if (!result) {
    return result;
  }

  std::vector<VkBufferImageCopy> regions;
  for (uint32_t level = 0; level < bitmap->GetMipLevels(); ++level) {
    vkex::Bitmap::Mip mip = {};
    bitmap->GetMipLayout(level, &mip);
    VkBufferImageCopy region               = {};
    region.bufferOffset                    = mip.data_offset;
    region.bufferRowLength                 = mip.width;
    region.bufferImageHeight               = mip.height;
    region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel       = level;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount     = 1;
    region.imageOffset.x                   = 0;
    region.imageOffset.y                   = 0;
    region.imageOffset.z                   = 0;
    region.imageExtent.width               = mip.width;
    region.imageExtent.height              = mip.height;
    region.imageExtent.depth               = 1;
    regions.push_back(region);
  }

  vkex::CopyResource(
    queue,
    cpu_buffer,
    (*p_texture)->GetImage(),
    vkex::CountU32(regions),
    vkex::DataPtr(regions));

  result = vkex::TransitionImageLayout(
    queue,
    (*p_texture)->GetImage(),
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT); 
  if (!result) {
    return result;
  }

  result = device->DestroyStorageBuffer(cpu_buffer);
  if (!result) {
    return result;
  }

  return vkex::Result::Success;
}

static vkex::Result CreateBuffer(
  size_t                        size,
  const void*                   p_data,
  vkex::Queue                   queue,
  const vkex::BufferUsageFlags& usage_flags,
  MemoryUsage                   memory_usage,
  vkex::Buffer*                 p_buffer
)
{
  VKEX_ASSERT_MSG(queue, "Invalid queue object");
  if (!queue) {
    return vkex::Result::ErrorInvalidQueueObject;
  }

  VKEX_ASSERT_MSG(p_buffer != nullptr, "Target buffer object is null");
  if (p_buffer == nullptr) {
    return vkex::Result::ErrorUnexpectedNullPointer;
  }

  // Grab device
  vkex::Device device = queue->GetDevice();

  // Create requested buffer
  {
    vkex::BufferCreateInfo create_info = {};
    create_info.size          = size;
    create_info.usage_flags   = usage_flags;
    create_info.committed     = true;   
    DetermineMemoryFlags(memory_usage, create_info.device_local, create_info.host_visible);
    vkex::Result result = device->CreateBuffer(create_info, p_buffer);
    if (!result) {
      return result;
    }
  }

  if ((memory_usage == MEMORY_USAGE_CPU_ONLY) || (memory_usage == MEMORY_USAGE_CPU_TO_GPU)) {
    // Copy data to buffer
    if (p_data != nullptr) {
      vkex::Result result = (*p_buffer)->Copy(size, p_data);
      if (!result) {
        return result;
      }
    }
  }
  else {
    // Create staging buffer and copy data
    if (p_data != nullptr) {
      vkex::Buffer cpu_buffer = nullptr;
      {
        // Create buffer
        vkex::BufferCreateInfo create_info        = {};
        create_info.size                          = size;
        create_info.usage_flags.bits.transfer_src = true;
        create_info.committed                     = true;
        DetermineMemoryFlags(MEMORY_USAGE_CPU_ONLY, create_info.device_local, create_info.host_visible);
        vkex::Result result = device->CreateBuffer(create_info, &cpu_buffer);
        if (!result) {
          return result;
        }

        // Copy data
        result = cpu_buffer->Copy(size, p_data);
        if (!result) {
          return result;
        }

      }
      // Copy resource
      VkBufferCopy region = {};
      region.srcOffset    = 0;
      region.dstOffset    = 0;
      region.size         = size;
      vkex::Result result = vkex::CopyResource(
        queue,
        cpu_buffer,
        *p_buffer,
        1,
        &region);
      if (!result) {
        return result;
      }

    }
  }
  // Success
  return vkex::Result::Success;
}

vkex::Result CreateConstantBuffer(
  size_t        size,
  const void*   p_data,
  vkex::Queue   queue,
  MemoryUsage   memory_usage,
  vkex::Buffer* p_buffer
)
{
  vkex::BufferUsageFlags usage_flags = {};
  usage_flags.bits.transfer_dst = true;
  usage_flags.bits.uniform_buffer = true;

  vkex::Result result = CreateBuffer(
    size,
    p_data,
    queue,
    usage_flags,
    memory_usage,
    p_buffer);
  if (!result) {
    return result;
  }
  return vkex::Result::Success;
}
  
vkex::Result CreateIndexBuffer(
  size_t        size,
  const void*   p_data,
  vkex::Queue   queue,
  MemoryUsage   memory_usage,
  vkex::Buffer* p_buffer
)
{
  vkex::BufferUsageFlags usage_flags = {};
  usage_flags.bits.transfer_dst = true;
  usage_flags.bits.index_buffer = true;

  vkex::Result result = CreateBuffer(
    size,
    p_data,
    queue,
    usage_flags,
    memory_usage,
    p_buffer);
  if (!result) {
    return result;
  }
  return vkex::Result::Success;
}

vkex::Result CreateVertexBuffer(
  size_t        size,
  const void*   p_data,
  vkex::Queue   queue,
  MemoryUsage   memory_usage,
  vkex::Buffer* p_buffer
)
{
  vkex::BufferUsageFlags usage_flags = {};
  usage_flags.bits.transfer_dst = true;
  usage_flags.bits.vertex_buffer = true;

  vkex::Result result = CreateBuffer(
    size,
    p_data,
    queue,
    usage_flags,
    memory_usage,
    p_buffer);
  if (!result) {
    return result;
  }
  return vkex::Result::Success;
}

vkex::Result CreateGeometryBuffer(
    size_t        size,
    const void*   p_data,
    vkex::Queue   queue,
    MemoryUsage   memory_usage,
    vkex::Buffer* p_buffer
)
{
    vkex::BufferUsageFlags usage_flags = {};
    usage_flags.bits.transfer_dst = true;
    usage_flags.bits.vertex_buffer = true;
    usage_flags.bits.index_buffer = true;

    vkex::Result result = CreateBuffer(
        size,
        p_data,
        queue,
        usage_flags,
        memory_usage,
        p_buffer);
    if (!result) {
        return result;
    }
    return vkex::Result::Success;
}

vkex::Result CreateStorageBuffer(
  size_t        size,
  const void*   p_data,
  vkex::Queue   queue,
  MemoryUsage   memory_usage,
  vkex::Buffer* p_buffer
)
{
  vkex::BufferUsageFlags usage_flags = {};
  usage_flags.bits.transfer_src = true;
  usage_flags.bits.transfer_dst = true;
  usage_flags.bits.storage_buffer = true;

  vkex::Result result = CreateBuffer(
    size,
    p_data,
    queue,
    usage_flags,
    memory_usage,
    p_buffer);
  if (!result) {
    return result;
  }
  return vkex::Result::Success;
}

} // namespace asset_util
