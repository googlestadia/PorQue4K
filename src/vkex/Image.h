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

#ifndef __VKEX_IMAGE_H__
#define __VKEX_IMAGE_H__

#include <vkex/Config.h>
#include <vkex/Traits.h>
#include <vkex/VulkanUtil.h>
#include <vk_mem_alloc.h>

namespace vkex {

// =================================================================================================
// Image
// =================================================================================================

/** @struct ImageCreateInfo 
 *
 */
struct ImageCreateInfo {
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
  VkImage                     vk_object;
};

/** @class IImage
 *
 */ 
class CImage : public IDeviceObject {
public:
  CImage();
  ~CImage();

  /** @fn operator VkImage()
   *
   */
  operator VkImage() const { 
    return m_vk_object; 
  }

  /** @fn GetVkObject
   *
   */
  VkImage GetVkObject() const { 
    return m_vk_object; 
  }

  /** @fn GetCreateFlags
   *
   */
  const vkex::ImageCreateFlags& GetCreateFlags() const {
    return m_create_info.create_flags;
  }

  /** @fn GetImageType
   *
   */
  VkImageType GetImageType() const {
    return m_create_info.image_type;
  }

  /** @fn GetFormat
   *
   */
  VkFormat GetFormat() const {
    return m_create_info.format;
  }

  /** @fn GetFormat
   *
   */
  uint32_t GetMipLevels() const {
    return m_create_info.mip_levels;
  }

  /** @fn GetArrayLayers
   *
   */
  uint32_t GetArrayLayers() const {
    return m_create_info.array_layers;
  }

  /** @fn GetExtent
   *
   */
  const VkExtent3D& GetExtent() const {
    return m_create_info.extent;
  }

  /** @fn GetSamples
   *
   */
  VkSampleCountFlagBits GetSamples() const {
    return m_create_info.samples;
  }

  /** @fn GetTiling
   *
   */
  VkImageTiling GetTiling() const {
    return m_create_info.tiling;
  }

  /** @fn GetUsage
   *
   */
  const vkex::ImageUsageFlags& GetUsageFlags() const {
    return m_create_info.usage_flags;
  }

  /** @fn GetSharingMode
   *
   */
  VkSharingMode GetSharingMode() const {
    return m_create_info.sharing_mode;
  }

  /** @fn GetSharingMode
   *
   */
  const std::vector<uint32_t>& GetQueueFamilyIndices() const {
    return m_create_info.queue_family_indices;
  }

  /** @fn GetInitialLayout
   *
   */
  VkImageLayout GetInitialLayout() const {
    return m_create_info.initial_layout;
  }

  /** @fn IsCommitted
   *
   */
  bool IsCommited() const {
    return m_create_info.committed;
  }

  /** @fn IsHostVisible
   *
   */
  bool IsHostVisible() const {
    return m_create_info.host_visible;
  }
  
  /** @fn IsDeviceLocal
   *
   */
  bool IsDeviceLocal() const {
    return m_create_info.device_local;
  }

  /** @fn GetMemoryPool
   *
   */
  VmaPool GetMemoryPool() const {
    return m_create_info.memory_pool;
  }

  /** @fn GetImageAspectFlags
   *
   */
  const vkex::ImageAspectFlags& GetAspectFlags() const {
    return m_aspect_flags;
  }

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

private:
  friend class CDevice;
  friend class IObjectStorageFunctions;

  /** @fn InitializeCommitted
   *
   */
  VkResult InitializeCommitted(bool host_visible, bool device_local, VmaPool memory_pool);

  /** @fn InternalCreate
   *
   */
  vkex::Result InternalCreate(
    const vkex::ImageCreateInfo&  create_info,
    const VkAllocationCallbacks*  p_allocator
  );

  /** @fn InternalDestroy
   *
   */
  vkex::Result InternalDestroy(const VkAllocationCallbacks* p_allocator);

private:
  vkex::ImageCreateInfo       m_create_info = {};
  VkImageCreateInfo           m_vk_create_info = {};
  VkImage                     m_vk_object = VK_NULL_HANDLE;
  vkex::ImageAspectFlags      m_aspect_flags = {};
  VmaAllocationCreateInfo     m_vma_allocation_create_info = {};
  VmaAllocation               m_vma_allocation = VK_NULL_HANDLE;
  VmaAllocationInfo           m_vma_allocation_info = {};
  void*                       m_mapped_address = nullptr; 
};

// =================================================================================================
// ImageView
// =================================================================================================

/** @struct ImageViewCreateInfo 
 *
 */
struct ImageViewCreateInfo {
  vkex::ImageViewCreateFlags  create_flags;
  vkex::Image                 image;
  VkImageViewType             view_type;
  VkFormat                    format;
  VkSampleCountFlagBits       samples; 
  VkComponentMapping          components;
  VkImageSubresourceRange     subresource_range;
};

/** @class IImageView
 *
 */ 
class CImageView : public IDeviceObject {
public:
  CImageView();
  ~CImageView();

  /** @fn operator VkImageView()
   *
   */
  operator VkImageView() const { 
    return m_vk_object; 
  }

  /** @fn GetVkObject
   *
   */
  VkImageView GetVkObject() const { 
    return m_vk_object; 
  }

  /** @fn GetImage
   *
   */
  vkex::Image GetImage() const {
    return m_create_info.image;
  }

  /** @fn GetFormat
   *
   */
  VkFormat GetFormat() const {
    return m_create_info.format;
  }

  /** @fn GetSamples
   *
   */
  VkSampleCountFlagBits GetSamples() const {
    return m_create_info.samples;
  }

private:
  friend class CDevice;
  friend class IObjectStorageFunctions;

  /** @fn InternalCreate
   *
   */
  vkex::Result InternalCreate(
    const vkex::ImageViewCreateInfo&  create_info,
    const VkAllocationCallbacks*      p_allocator
  );

  /** @fn InternalDestroy
   *
   */
  vkex::Result InternalDestroy(const VkAllocationCallbacks* p_allocator);

private:
  vkex::ImageViewCreateInfo m_create_info = {};
  VkImageViewCreateInfo     m_vk_create_info = {};
  VkImageView               m_vk_object = VK_NULL_HANDLE;
};

} // namespace vkex

#endif // __VKEX_IMAGE_H__