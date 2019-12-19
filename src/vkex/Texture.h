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

#ifndef __VKEX_TEXTURE_H__
#define __VKEX_TEXTURE_H__

#include <vkex/Config.h>
#include <vkex/Image.h>
#include <vkex/Traits.h>
#include <vkex/VulkanUtil.h>

namespace vkex {

// =================================================================================================
// Texture
// =================================================================================================

/** @struct TextureCreateInfo 
 *
 */
struct TextureCreateInfo {
  // Create Method 1 - Use and existing image.
  //
  vkex::Image                   existing_image;

  // Create Method 2 - Specify image description
  //
  struct {
    vkex::ImageCreateFlags      create_flags;
    VkImageType                 image_type;
    VkFormat                    format;
    VkExtent3D                  extent;
    uint32_t                    mip_levels;
    uint32_t                    array_layers;
    VkSampleCountFlagBits       samples;
    VkImageTiling               tiling;
    vkex::ImageUsageFlags       usage_flags;
    VkSharingMode               sharing_mode;
    std::vector<uint32_t>       queue_family_indices;
    VkImageLayout               initial_layout;
    bool                        committed;
    // Textures are Device Local unless host_visibile' is true.
    bool                        host_visible;
    // 'device_local' is only applicable to AMD GPUs.
    bool                        device_local;
    VmaPool                     memory_pool;
  } image;

  // Image view descriptions
  //
  struct {
    // If \b derive_from_image is true, the values for the fields
    // in the next section can be left uninitialized.
    //
    bool                        derive_from_image;

    // If \b derived_from_image is true, values for the fields below
    // will be overridden with derived values.
    //
    vkex::ImageViewCreateFlags  create_flags;
    VkImageViewType             view_type;
    VkFormat                    format;
    VkSampleCountFlagBits       samples; 
    VkComponentMapping          components;
    VkImageSubresourceRange     subresource_range;  
  } view; 
};

/** @class IQueue
 *
 */ 
class CTexture : public IDeviceObject {
public:
  CTexture();
  ~CTexture();

  /** @fn GetImage
   *
   */
  vkex::Image GetImage() const;

  /** @fn GetImageView
   *
   */
  vkex::ImageView GetImageView() const;

  /** @fn GetImageType
   *
   */
  VkImageType GetImageType() const;

  /** @fn GetFormat
   *
   */
  VkFormat GetFormat() const;

  /** @fn GetFormat
   *
   */
  uint32_t GetMipLevels() const;

  /** @fn GetArrayLayers
   *
   */
  uint32_t GetArrayLayers() const;

  /** @fn GetExtent
   *
   */
  const VkExtent3D& GetExtent() const;

  /** @fn GetSamples
   *
   */
  VkSampleCountFlagBits GetSamples() const;

  /** @fn GetUsage
   *
   */
  const vkex::ImageUsageFlags GetUsageFlags() const;

/** @fn IsCommitted
   *
   */
  bool IsCommited() const;

  /** @fn IsHostVisible
   *
   */
  bool IsHostVisible() const;

  /** @fn GetImageAspectFlags
   *
   */
  const vkex::ImageAspectFlags& GetAspectFlags() const;

  /** @fn AllocateMemory
   *
   */
  VkResult AllocateMemory(
    VmaMemoryUsage  usage, 
    VmaPool         memory_pool = VK_NULL_HANDLE
  );

  /** @fn FreeMemory
   *
   */
  void FreeMemory();

  /** @fn IsAllocated
   *
   */
  bool IsMemoryAllocated() const;

  /** @fn BindMemory
   *
   */
  VkResult BindMemory();

  /** @fn Map
   *
   */
  VkResult MapMemory(void** pp_mapped_address);

  /** @fn Unmap
   *
   */
  void UnmapMemory();

  /** @fn IsMapped
   *
   */
  bool IsMemoryMapped() const;

  /** @fn GetOffset
   *
   */
  VkDeviceSize GetMemoryOffset() const;

  /** @fn GetSize
   *
   */
  VkDeviceSize GetMemorySize() const;

  /** @fn CopyToMipLevel
   *
   */
  vkex::Result CopyToMipLevel(
    uint32_t       mip_level,
    uint32_t       array_layer,
    uint32_t       src_row_stride,
    uint32_t       src_height,
    const uint8_t* p_src_data
  );

private:
  friend class CDevice;
  friend class IObjectStorageFunctions;

  /** @fn InternalCreate
   *
   */
  vkex::Result InternalCreate(
    const vkex::TextureCreateInfo&  create_info,
    const VkAllocationCallbacks*    p_allocator
  );

  /** @fn InternalDestroy
   *
   */
  vkex::Result InternalDestroy(const VkAllocationCallbacks* p_allocator);

private:
  vkex::TextureCreateInfo m_create_info = {};
  vkex::Image             m_image = nullptr;
  bool                    m_owns_image = false;
  vkex::ImageView         m_image_view = nullptr;
};

} // namespace vkex

#endif // __VKEX_TEXTURE_H__