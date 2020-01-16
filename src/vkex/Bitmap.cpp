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

#include "vkex/Bitmap.h"
#include "vkex/VulkanUtil.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"

namespace vkex {

Bitmap::Bitmap()
{
}

Bitmap::Bitmap(
  uint32_t       width,
  uint32_t       height,
  VkFormat       format,
  uint32_t       level_count,
  const uint8_t* p_src_data,
  uint32_t       src_row_stride,
  uint32_t       src_height)
  : m_format(format)
{
  VKEX_ASSERT_MSG(
    m_format != VK_FORMAT_UNDEFINED, "Cannot create image with format VK_FORMAT_UNDEFINED!");

  m_component_count = vkex::FormatComponentCount(format);
  m_component_size  = vkex::FormatComponentSize(format);

  // Generate MIP layouts
  if (level_count == 0) {
    CalculateMipLevelCount(width, height, &level_count);
  }
  level_count = std::min<uint32_t>(level_count, vkex::Bitmap::MaxMipLevelCount);
  m_mips.resize(level_count);
  GenerateMipLayouts(
    width, height, m_component_count, m_component_size, level_count, m_mips.data());
  // Allocate storage
  m_valid = AllocateStorage();
  if (!m_valid) {
    return;
  }

  // Copy data and generate MIPs if data is specified
  if (p_src_data != nullptr) {
    m_valid = CopyToMip0(p_src_data, src_row_stride, src_height);
    if (!m_valid) {
      return;
    }
    GenerateMips();
  }
}

Bitmap::Bitmap(
  uint64_t       storage_size,
  uint8_t*       p_storage,
  uint32_t       width,
  uint32_t       height,
  VkFormat       format,
  uint32_t       level_count,
  const uint8_t* p_src_data,
  uint32_t       src_row_stride,
  uint32_t       src_height)
  : m_format(format)
{
  VKEX_ASSERT_MSG(m_format != VK_FORMAT_UNDEFINED, "Cannot create image with format VK_FORMAT_UNDEFINED!");

  m_component_count = vkex::FormatComponentCount(m_format);
  m_component_size  = vkex::FormatComponentSize(m_format);

  // Generate MIP layouts
  if (level_count == 0) {
    CalculateMipLevelCount(width, height, &level_count);
  }
  level_count = std::min<uint32_t>(level_count, vkex::Bitmap::MaxMipLevelCount);
  m_mips.resize(level_count);
  GenerateMipLayouts(
    width, height, m_component_count, m_component_size, level_count, m_mips.data());
  // Set storage
  m_data_size = storage_size;
  m_data      = p_storage;

  // Copy data and generate MIPs if data is specified
  if (p_src_data != nullptr) {
    m_valid = CopyToMip0(p_src_data, src_row_stride, src_height);
    if (!m_valid) {
      return;
    }
    GenerateMips();
  }
}

Bitmap::Bitmap(const MIPFile& mip_file)
{
  m_format = VK_FORMAT_R8G8B8A8_UNORM;

  m_component_count = vkex::FormatComponentCount(m_format);
  m_component_size  = vkex::FormatComponentSize(m_format);

  m_mips.resize(mip_file.level_count);
  for (uint32_t level = 0; level < mip_file.level_count; ++level) {
    Mip& mip = m_mips[level];
    const MIPInfo& info = mip_file.infos[level];
    mip.level       = info.level;
    mip.data_offset = info.data_offset;
    mip.data_size   = info.data_size;
    mip.width       = info.width;
    mip.height      = info.height;
    mip.row_stride  = info.row_stride;
  }

  // Allocate storage
  m_valid = AllocateStorage();
  if (!m_valid) {
    return;
  }

  memcpy(m_storage.data(), mip_file.data.data(), m_data_size);
}

Bitmap::~Bitmap()
{
}

bool Bitmap::AllocateStorage()
{
  m_data_size = 0;
  for (const auto& mip : m_mips) {
    m_data_size += mip.data_size;
  }

  if (m_data_size == 0) {
    return false;
  }

  m_storage.resize(static_cast<size_t>(m_data_size));
  uint64_t allocated_size = static_cast<uint64_t>(m_storage.size());
  if (allocated_size != m_data_size) {
    return false;
  }

  m_data = m_storage.data();

  return true;
}

bool Bitmap::CopyToMip0(const uint8_t* p_src_data, uint32_t src_row_stride, uint32_t src_height)
{
  if (m_mips.empty()) {
    return false;
  }

  if (p_src_data == nullptr) {
    return false;
  }

  Mip mip = {};
  if (!GetMipLayout(0, &mip)) {
    return false;
  }

  uint32_t dst_height     = mip.height;
  uint32_t dst_row_stride = mip.row_stride;

  src_height     = (src_height == 0) ? dst_height : src_height;
  src_row_stride = (src_row_stride == 0) ? dst_row_stride : src_row_stride;

  src_row_stride = std::min<uint32_t>(dst_row_stride, src_row_stride);
  src_height     = std::min<uint32_t>(dst_height, src_height);

  uint8_t* p_dst = GetData();
  if (p_dst == nullptr) {
    return false;
  }

  const uint8_t* p_src = p_src_data;
  for (uint32_t y = 0; y < src_height; ++y) {
    std::memcpy(p_dst, p_src, src_row_stride);
    p_dst += dst_row_stride;
    p_src += src_row_stride;
  }

  return true;
}

bool Bitmap::GenerateMips()
{
  uint32_t level_count = GetMipLevels();
  for (uint32_t dst_level = 1; dst_level < level_count; ++dst_level) {
    uint32_t src_level = dst_level - 1;
    // Source level data
    const unsigned char* p_src_data = GetData(src_level);
    if (p_src_data == nullptr) {
      return false;
    }
    // Destination level data
    unsigned char* p_dst_data = GetData(dst_level);
    if (p_dst_data == nullptr) {
      return false;
    }
    // Source MIP data
    Mip src_mip = {};
    if (!GetMipLayout(src_level, &src_mip)) {
      return false;
    }
    // Destination MIP data
    Mip dst_mip = {};
    if (!GetMipLayout(dst_level, &dst_mip)) {
      return false;
    }

    const unsigned char* input_pixels           = reinterpret_cast<const unsigned char*>(p_src_data);
    int                  input_w                = static_cast<int>(src_mip.width);
    int                  input_h                = static_cast<int>(src_mip.height);
    int                  input_stride_in_bytes  = static_cast<int>(src_mip.row_stride);
    unsigned char*       output_pixels          = reinterpret_cast<unsigned char*>(p_dst_data);
    int                  output_w               = static_cast<int>(dst_mip.width);
    int                  output_h               = static_cast<int>(dst_mip.height);
    int                  output_stride_in_bytes = static_cast<int>(dst_mip.row_stride);
    int                  num_channels           = static_cast<int>(m_component_count);
    int                  alpha_channel          = 0;
    int                  flags                  = 0;
    stbir_edge           edge_wrap_mode         = STBIR_EDGE_CLAMP;
    stbir_filter         filter                 = STBIR_FILTER_CATMULLROM;
    stbir_colorspace     space                  = STBIR_COLORSPACE_LINEAR;
    void*                alloc_context          = nullptr;

    int result = stbir_resize_uint8_generic(
      input_pixels,
      input_w,
      input_h,
      input_stride_in_bytes,
      output_pixels,
      output_w,
      output_h,
      output_stride_in_bytes,
      num_channels,
      alpha_channel,
      flags,
      edge_wrap_mode,
      filter,
      space,
      alloc_context);

    if (result == 0) {
      return false;
    }
  }

  return true;
}

VkFormat Bitmap::GetFormat() const
{
  return m_format;
}

uint32_t Bitmap::GetComponentCount() const
{
  return m_component_count;
}

uint32_t Bitmap::GetComponentSize() const
{
  return m_component_size;
}

uint32_t Bitmap::GetMipLevels() const
{
  uint32_t mip_levels = static_cast<uint32_t>(m_mips.size());
  return mip_levels;
}

bool Bitmap::GetMipLayout(uint32_t level, Mip* p_mip) const
{
  uint32_t mip_levels = GetMipLevels();
  if (level >= mip_levels) {
    return false;
  }

  *p_mip = m_mips[level];

  return true;
}

uint32_t Bitmap::GetWidth(uint32_t level) const
{
  uint32_t width = 0;
  Mip      mip   = {};
  if (GetMipLayout(level, &mip)) {
    width = mip.width;
  }
  return width;
}

uint32_t Bitmap::GetHeight(uint32_t level) const
{
  uint32_t height = 0;
  Mip      mip    = {};
  if (GetMipLayout(level, &mip)) {
    height = mip.height;
  }
  return height;
}

uint32_t Bitmap::GetRowStride(uint32_t level)
{
  uint32_t row_stride = 0;
  Mip      mip        = {};
  if (GetMipLayout(level, &mip)) {
    row_stride = mip.row_stride;
  }
  return row_stride;
}

uint64_t Bitmap::GetDataSize(uint32_t level) const
{
  uint64_t data_size = 0;
  Mip      mip       = {};
  if (GetMipLayout(level, &mip)) {
    data_size = mip.data_size;
  }
  return data_size;
}

uint64_t Bitmap::GetDataSizeAllLevels() const
{
  uint64_t data_size = 0;
  Mip      mip       = {};
  uint32_t level_count = GetMipLevels();
  for (uint32_t level = 0; level < level_count; ++level) {
    if (GetMipLayout(level, &mip)) {
      data_size += mip.data_size;
    }
  }
  return data_size;
}

uint8_t* Bitmap::GetData(uint32_t level)
{
  uint8_t* p_data = m_data;
  Mip      mip    = {};
  if ((p_data != nullptr) && GetMipLayout(level, &mip)) {
    p_data += mip.data_offset;
  }
  return p_data;
}

const uint8_t* Bitmap::GetData(uint32_t level) const
{
  const uint8_t* p_data = m_data;
  Mip            mip    = {};
  if ((p_data != nullptr) && GetMipLayout(level, &mip)) {
    p_data += mip.data_offset;
  }
  return p_data;
}

VkExtent3D Bitmap::GetExtent(uint32_t level) const
{
  VkExtent3D extent = {};
  Mip        mip    = {};
  if (GetMipLayout(level, &mip)) {
    extent.width  = mip.width;
    extent.height = mip.height;
    extent.depth  = 1;
  }
  return extent;
}

void Bitmap::CalculateMipLevelCount(uint32_t width, uint32_t height, uint32_t* p_level_count)
{
  uint32_t level_count = 0;
  while ((width > 0) && (height > 0)) {
    // Increment level count
    level_count += 1;

    // Divide width,height by 2
    width >>= 1;
    height >>= 1;
  }

  if (p_level_count != nullptr) {
    *p_level_count = level_count;
  }
}

void Bitmap::GenerateMipLayouts(
  uint32_t           width,
  uint32_t           height,
  uint32_t           component_count,
  uint32_t           component_size,
  const uint32_t     level_count,
  vkex::Bitmap::Mip* p_mips
)
{
  uint32_t level       = 0;
  uint64_t data_offset = 0;
  while ((width > 0) && (height > 0) && (level < level_count)) {
    uint32_t row_stride = width * component_count * component_size;
    uint64_t data_size  = row_stride * height;

    Mip mip         = {};
    mip.level       = level;
    mip.data_offset = data_offset;
    mip.data_size   = data_size;
    mip.width       = width;
    mip.height      = height;
    mip.row_stride  = row_stride;
    p_mips[level]   = mip;

    // Increment level
    level += 1;

    // Increment data offset
    data_offset += data_size;

    // Divide width,height by 2
    width >>= 1;
    height >>= 1;
  }
}

vkex::Result Bitmap::Create(
  const fs::path&                file_path,
  uint32_t                       level_count,
  std::unique_ptr<vkex::Bitmap>* p_bitmap)
{
  int width             = 0;
  int height            = 0;
  int channels          = 0;
  int required_channels = 4;
  // Force 4 channels since some Vulkan implementations won't have 3 channel
  // image support
  unsigned char* p_image =
    stbi_load(file_path.c_str(), &width, &height, &channels, required_channels);
  if (p_image == nullptr) {
    return vkex::Result::ErrorImageLoadFailed;
  }

  VkFormat format     = VK_FORMAT_R8G8B8A8_UNORM;
  uint32_t row_stride = static_cast<uint32_t>(width) * static_cast<uint32_t>(required_channels);
  std::unique_ptr<vkex::Bitmap> bitmap = std::make_unique<vkex::Bitmap>(
    static_cast<uint32_t>(width),
    static_cast<uint32_t>(height),
    format,
    level_count,
    p_image,
    row_stride,
    height);

  stbi_image_free(p_image);
  p_image = nullptr;

  *p_bitmap = std::move(bitmap);

  return vkex::Result::Success;
}

vkex::Result Bitmap::Create(
  size_t                         src_data_size,
  const uint8_t*                 p_src_data,
  uint32_t                       level_count,
  std::unique_ptr<vkex::Bitmap>* p_bitmap)
{
  int width             = 0;
  int height            = 0;
  int channels          = 0;
  int required_channels = 4;
  // Force 4 channels since some Vulkan implementations won't have 3 channel image support
  unsigned char* p_image = stbi_load_from_memory(
    p_src_data, static_cast<int>(src_data_size), &width, &height, &channels, required_channels);
  if (p_image == nullptr) {
    return vkex::Result::ErrorImageLoadFailed;
  }

  VkFormat format     = VK_FORMAT_R8G8B8A8_UNORM;
  uint32_t row_stride = static_cast<uint32_t>(width) * static_cast<uint32_t>(required_channels);
  std::unique_ptr<vkex::Bitmap> bitmap = std::make_unique<vkex::Bitmap>(
    static_cast<uint32_t>(width),
    static_cast<uint32_t>(height),
    format,
    level_count,
    p_image,
    row_stride,
    height);

  stbi_image_free(p_image);
  p_image = nullptr;

  *p_bitmap = std::move(bitmap);

  return vkex::Result::Success;
}

vkex::Result Bitmap::Create(
  size_t                         src_data_size,
  const uint8_t*                 p_src_data,
  uint32_t                       level_count,
  std::unique_ptr<vkex::Bitmap>* p_bitmap,
  uint64_t                       storage_size,
  uint8_t*                       p_storage)
{
  // Check size
  {
    uint64_t footprint_storage_size = 0;

    vkex::Result vkex_result = GetDataFootprint(
      src_data_size, p_src_data, level_count, nullptr, nullptr, nullptr, &footprint_storage_size);
    if (vkex_result != vkex::Result::Success) {
      return vkex_result;
    }

    if (footprint_storage_size > storage_size) {
      return vkex::Result::ErrorImageStorageSizeInsufficient;
    }
  }

  int width             = 0;
  int height            = 0;
  int channels          = 0;
  int required_channels = 4;
  // Force 4 channels since some Vulkan implementations won't have 3 channel image support
  unsigned char* p_image = stbi_load_from_memory(
    p_src_data, static_cast<int>(src_data_size), &width, &height, &channels, required_channels);
  if (p_image == nullptr) {
    return vkex::Result::ErrorImageLoadFailed;
  }

  VkFormat format     = VK_FORMAT_R8G8B8A8_UNORM;
  uint32_t row_stride = static_cast<uint32_t>(width) * static_cast<uint32_t>(required_channels);
  std::unique_ptr<vkex::Bitmap> bitmap = std::make_unique<vkex::Bitmap>(
    storage_size,
    p_storage,
    static_cast<uint32_t>(width),
    static_cast<uint32_t>(height),
    format,
    level_count,
    p_image,
    row_stride,
    height);

  stbi_image_free(p_image);
  p_image = nullptr;

  *p_bitmap = std::move(bitmap);

  return vkex::Result::Success;
}

vkex::Result Bitmap::Create(
    size_t                         src_data_size,
    const uint8_t*                 p_src_data,
    int                            width,
    int                            height,
    VkFormat                       format,
    uint32_t                       level_count,
    std::unique_ptr<vkex::Bitmap>* p_bitmap)
{
  const int required_channels = 4;

  uint32_t row_stride = static_cast<uint32_t>(width) * static_cast<uint32_t>(required_channels);
  std::unique_ptr<vkex::Bitmap> bitmap = std::make_unique<vkex::Bitmap>(
    static_cast<uint32_t>(width),
    static_cast<uint32_t>(height),
    format,
    level_count,
    p_src_data,
    row_stride,
    height);

  *p_bitmap = std::move(bitmap);

  return vkex::Result::Success;
}

vkex::Result Bitmap::GetDataFootprint(
  const fs::path& file_path,
  uint32_t        level_count,
  uint32_t*       p_width,
  uint32_t*       p_height,
  VkFormat*       p_format,
  uint64_t*       p_data_size)
{
  int width    = 0;
  int height   = 0;
  int channels = 0;
  int res      = stbi_info(file_path.c_str(), &width, &height, &channels);
  if (res != 1) {
    return vkex::Result::ErrorImageInfoFailed;
  }

  if (p_width != nullptr) {
    *p_width = static_cast<uint32_t>(width);
  }

  if (p_height != nullptr) {
    *p_height = static_cast<uint32_t>(height);
  }

  // Force 4 channels since some Vulkan implementations won't have 3 channel image support
  if (p_format != nullptr) {
    *p_format = VK_FORMAT_R8G8B8A8_UNORM;
  }

  if (p_data_size != nullptr) {
    *p_data_size = static_cast<uint64_t>(width) * static_cast<uint64_t>(height) * 4;
  }

  return vkex::Result::Success;
}

vkex::Result Bitmap::GetDataFootprint(
  size_t         src_data_size,
  const uint8_t* p_src_data,
  uint32_t       level_count,
  uint32_t*      p_width,
  uint32_t*      p_height,
  VkFormat*      p_format,
  uint64_t*      p_data_size)
{
  int width    = 0;
  int height   = 0;
  int channels = 0;
  int res =
    stbi_info_from_memory(p_src_data, static_cast<int>(src_data_size), &width, &height, &channels);
  if (res != 1) {
    return vkex::Result::ErrorImageInfoFailed;
  }

  if (p_width != nullptr) {
    *p_width = static_cast<uint32_t>(width);
  }

  if (p_height != nullptr) {
    *p_height = static_cast<uint32_t>(height);
  }

  // Force 4 channels since some Vulkan implementations won't have 3 channel image support
  if (p_format != nullptr) {
    *p_format = VK_FORMAT_R8G8B8A8_UNORM;
  }

  if (p_data_size != nullptr) {
    // Figure out MIP level count
    if (level_count == 0) {
      CalculateMipLevelCount(width, height, &level_count);
    }
    level_count = std::min<uint32_t>(level_count, vkex::Bitmap::MaxMipLevelCount);
    // Generate MIP layouts
    uint32_t                       component_count = 4;
    uint32_t                       component_size  = 1;
    std::vector<vkex::Bitmap::Mip> mips(level_count);
    GenerateMipLayouts(width, height, component_count, component_size, level_count, mips.data());
    // Get toal data size
    uint64_t data_size = 0;
    for (auto& mip : mips) {
      data_size += mip.data_size;
    }

    *p_data_size = data_size;
  }

  return vkex::Result::Success;
}

vkex::Result Bitmap::WriteJPG(
  const fs::path& file_path,
  uint32_t        width,
  uint32_t        height,
  uint32_t        component_count,
  uint32_t        row_stride,
  const void*     p_data
)
{
  int result = stbi_write_jpg(
    file_path.c_str(),
    static_cast<int>(width),
    static_cast<int>(height),
    static_cast<int>(component_count),
    p_data,
    static_cast<int>(row_stride));

  if (result == 0) {
    return vkex::Result::ErrorImageWriteFailed;
  }

  return vkex::Result::Success;
}

} // namespace vkex