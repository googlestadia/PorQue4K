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

#ifndef __VKEX_BITMAP_H__
#define __VKEX_BITMAP_H__

#include "vkex/Config.h"
#include "vkex/MIPFile.h"

namespace vkex {

/** @class Bitmap
 *
 */
class Bitmap {
public:
  enum { MaxMipLevelCount = 32 };

  struct Mip {
    uint32_t level;
    uint64_t data_offset;
    uint64_t data_size;
    uint32_t width;
    uint32_t height;
    uint32_t row_stride;
  };

  Bitmap();
  Bitmap(
    uint32_t       width,
    uint32_t       height,
    VkFormat       format,
    uint32_t       level_count,
    const uint8_t* p_src_data     = nullptr,
    uint32_t       src_row_stride = 0,
    uint32_t       src_height     = 0);
  Bitmap(
    uint64_t       storage_size,
    uint8_t*       p_storage,
    uint32_t       width,
    uint32_t       height,
    VkFormat       format,
    uint32_t       level_count,
    const uint8_t* p_src_data     = nullptr,
    uint32_t       src_row_stride = 0,
    uint32_t       src_height     = 0);
  Bitmap(
    const MIPFile& mip_file);
  ~Bitmap();

  VkFormat        GetFormat() const;
  uint32_t        GetComponentCount() const;
  uint32_t        GetComponentSize() const;

  uint32_t        GetMipLevels() const;
  bool            GetMipLayout(uint32_t level, vkex::Bitmap::Mip* p_mip) const;

  uint32_t        GetWidth(uint32_t level = 0) const;
  uint32_t        GetHeight(uint32_t level = 0) const;
  uint32_t        GetRowStride(uint32_t level = 0);
  uint64_t        GetDataSize(uint32_t level = 0) const;
  uint64_t        GetDataSizeAllLevels() const;
  uint8_t*        GetData(uint32_t level = 0);
  const uint8_t*  GetData(uint32_t level = 0) const;

  VkExtent3D GetExtent(uint32_t level = 0) const;

  // Calculate MIP level count
  static void CalculateMipLevelCount(uint32_t width, uint32_t height, uint32_t* p_level_count);

  // Generate MIP level data. 'p_mips' must be allocated to store 'level_count' MIPs.
  static void GenerateMipLayouts(
    uint32_t           width,
    uint32_t           height,
    uint32_t           component_count,
    uint32_t           component_size,
    const uint32_t     level_count,
    vkex::Bitmap::Mip* p_mips);

  // Create Bitmap from file
  static vkex::Result Create(
    const fs::path&                 file_path, 
    uint32_t                        level_count, 
    std::unique_ptr<vkex::Bitmap>*  p_bitmap);

  // Create Bitmap from memory
  static vkex::Result Create(
    size_t                         src_data_size,
    const uint8_t*                 p_src_data,
    uint32_t                       level_count,
    std::unique_ptr<vkex::Bitmap>* p_bitmap);

  // Create Bitmap from memory using storage provided
  static vkex::Result Create(
    size_t                         src_data_size,
    const uint8_t*                 p_src_data,
    uint32_t                       level_count,
    std::unique_ptr<vkex::Bitmap>* p_bitmap,
    uint64_t                       storage_size,
    uint8_t*                       p_storage);

  static vkex::Result GetDataFootprint(
    const fs::path& file_path,
    uint32_t        level_count,
    uint32_t*       p_width,
    uint32_t*       p_height,
    VkFormat*       p_format,
    uint64_t*       p_data_size);

  static vkex::Result GetDataFootprint(
    size_t         src_data_size,
    const uint8_t* p_src_data,
    uint32_t       level_count,
    uint32_t*      p_width,
    uint32_t*      p_height,
    VkFormat*      p_format,
    uint64_t*      p_data_size);

  static vkex::Result WriteJPG(
    const fs::path& file_path,
    uint32_t        width,
    uint32_t        height,
    uint32_t        component_count,
    uint32_t        row_stride,
    const void*     p_data);

private:
  bool AllocateStorage();
  bool CopyToMip0(const uint8_t* p_src_data, uint32_t src_row_stride, uint32_t src_height);
  bool GenerateMips();

private:
  bool                 m_valid           = false;
  VkFormat             m_format          = VK_FORMAT_UNDEFINED;
  uint32_t             m_component_count = 0;
  uint32_t             m_component_size  = 0;
  uint8_t*             m_data            = nullptr;
  uint64_t             m_data_size       = 0;
  std::vector<uint8_t> m_storage;
  std::vector<Mip>     m_mips;
};

} // namespace vkex

#endif // __VKEX_BITMAP_H__