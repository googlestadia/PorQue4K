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

#ifndef __VKEX_BUFFER_H__
#define __VKEX_BUFFER_H__

#include <vkex/Config.h>
#include <vkex/Traits.h>
#include <vkex/VulkanUtil.h>

namespace vkex {

// =================================================================================================
// Buffer
// =================================================================================================

/** @struct BufferCreateInfo 
 *
 */
struct BufferCreateInfo {
  VkDeviceSize                size;
  BufferUsageFlags            usage_flags;
  BufferCreateFlags           create_flags;
  VkSharingMode               sharing_mode;
  std::vector<uint32_t>       queue_family_indices;
  bool                        committed;
  // Textures are Device Local unless host_visibile' is true.
  bool                        host_visible;
  // 'device_local' is only applicable to AMD GPUs.
  bool                        device_local;
};

/** @class IBuffer
 *
 */ 
class CBuffer : public IDeviceObject {
public:
  CBuffer();
  ~CBuffer();

  /** @fn operator VkBuffer()
   *
   */
  operator VkBuffer() const { 
    return m_vk_object; 
  }

  /** @fn GetVkObject
   *
   */
  VkBuffer GetVkObject() const { 
    return m_vk_object; 
  }

  /** @fn GetSize
   *
   */
  VkDeviceSize GetSize() const {
    return m_create_info.size;
  }

  /** @fn GetUsage
   *
   */
  const vkex::BufferUsageFlags GetUsageFlags() const {
    return m_create_info.usage_flags;
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

  /** @fn AllocateMemory
   *
   */
  VkResult AllocateMemory(
    VmaMemoryUsage  usage, 
    VmaPool         pool = VK_NULL_HANDLE
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


  /** @fn Copy
   *
   */
  vkex::Result Copy(size_t size, const void* p_src);

private:
  friend class CDevice;
  friend class IObjectStorageFunctions;

  VkResult InitializeCommitted(bool host_visible, bool device_local);

  /** @fn InternalCreate
   *
   */
  vkex::Result InternalCreate(
    const vkex::BufferCreateInfo& create_info,
    const VkAllocationCallbacks*  p_allocator
  );

  /** @fn InternalDestroy
   *
   */
  vkex::Result InternalDestroy(const VkAllocationCallbacks* p_allocator);

private:
  vkex::BufferCreateInfo      m_create_info = {};
  VkBufferCreateInfo          m_vk_create_info = {};
  VkBuffer                    m_vk_object = VK_NULL_HANDLE;
  VmaAllocationCreateInfo     m_vma_allocation_create_info = {};
  VmaAllocation               m_vma_allocation = VK_NULL_HANDLE;
  VmaAllocationInfo           m_vma_allocation_info = {};
  void*                       m_mapped_address = nullptr;
};

} // namespace vkex

#endif // __VKEX_BUFFER_H__