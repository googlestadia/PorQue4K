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

#include "vkex/Buffer.h"
#include "vkex/Device.h"
#include "vkex/ToString.h"

namespace vkex {

// =================================================================================================
// Buffer
// =================================================================================================
CBuffer::CBuffer()
{
}

CBuffer::~CBuffer()
{
}

VkResult CBuffer::InitializeCommitted(bool host_visible, bool device_local)
{
  // Allocate
  VmaMemoryUsage usage = VMA_MEMORY_USAGE_GPU_ONLY;
  if (host_visible && device_local) {
    usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
  }
  else if (host_visible) {
    usage = VMA_MEMORY_USAGE_CPU_ONLY;
  }

  VkResult vk_result = AllocateMemory(usage);
  if (vk_result != VK_SUCCESS) {
    return vk_result;
  }
  // Bind
  vk_result = BindMemory();
  if (vk_result != VK_SUCCESS) {
    return vk_result;
  }
  return VK_SUCCESS;
}

vkex::Result CBuffer::InternalCreate(
  const vkex::BufferCreateInfo& create_info,
  const VkAllocationCallbacks*  p_allocator
)
{
  // Copy create info
  m_create_info = create_info;

  // Buffer size must be greater than zero
  if (m_create_info.size <= 0) {
    return vkex::Result::ErrorBufferSizeMustBeGreaterThanZero;
  }

  // Create Vulkan buffer object
  {
    // Vulkan create info
    m_vk_create_info = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    m_vk_create_info.flags                  = m_create_info.create_flags.flags;
    m_vk_create_info.size                   = m_create_info.size;
    m_vk_create_info.usage                  = m_create_info.usage_flags.flags;
    m_vk_create_info.sharingMode            = m_create_info.sharing_mode;
    m_vk_create_info.queueFamilyIndexCount  = CountU32(m_create_info.queue_family_indices);
    m_vk_create_info.pQueueFamilyIndices    = DataPtr(m_create_info.queue_family_indices);
    // Create Vulkan object
    VkResult vk_result = InvalidValue<VkResult>::Value;
    VKEX_VULKAN_RESULT_CALL(
      vk_result,
      vkex::CreateBuffer(
        *m_device,
        &m_vk_create_info,
        p_allocator,
        &m_vk_object)
    );
    if (vk_result != VK_SUCCESS) {
      return vkex::Result(vk_result);
    }
  }

  // Allocate and bind if resource is committed.
  if (m_create_info.committed) {
    VkResult vk_result = InvalidValue<VkResult>::Value;
    VKEX_VULKAN_RESULT_CALL(
      vk_result,
      InitializeCommitted(m_create_info.host_visible, m_create_info.device_local)
    );
    if (vk_result != VK_SUCCESS) {
      return vkex::Result(vk_result);
    }
  }

  return vkex::Result::Success;
}

vkex::Result CBuffer::InternalDestroy(const VkAllocationCallbacks* p_allocator)
{
  // Free memory
  FreeMemory();

  // Destroy Vulkan object
  if (m_vk_object != VK_NULL_HANDLE) {
    vkex::DestroyBuffer(
      *m_device,
      m_vk_object,
      p_allocator);

    m_vk_object = VK_NULL_HANDLE;
  }

  return vkex::Result::Success;
}

VkResult CBuffer::AllocateMemory(
  VmaMemoryUsage  usage, 
  VmaPool         pool
)
{
  m_vma_allocation_create_info = {};
  m_vma_allocation_create_info.flags          = 0;
  m_vma_allocation_create_info.usage          = usage;
  m_vma_allocation_create_info.requiredFlags  = 0;
  m_vma_allocation_create_info.preferredFlags = 0;
  m_vma_allocation_create_info.memoryTypeBits = 0;
  m_vma_allocation_create_info.pool           = pool;
  m_vma_allocation_create_info.pUserData      = nullptr;

  VkResult vk_result = vmaAllocateMemoryForBuffer(
    m_device->GetVmaAllocator(),
    m_vk_object,
    &m_vma_allocation_create_info,
    &m_vma_allocation,
    &m_vma_allocation_info);
  if (vk_result != VK_SUCCESS) {
    return vk_result;
  }

  return VK_SUCCESS;
}

void CBuffer::FreeMemory()
{
  if (m_vma_allocation != VK_NULL_HANDLE) {
    if (IsMemoryMapped()) {
      UnmapMemory();
    }   

    vmaFreeMemory(
      m_device->GetVmaAllocator(),
      m_vma_allocation);

    m_vma_allocation = VK_NULL_HANDLE;
  }
}

bool CBuffer::IsMemoryAllocated() const
{
  bool is_allocated = (m_vma_allocation != VK_NULL_HANDLE);
  return is_allocated;
}

VkResult CBuffer::BindMemory()
{
  VkResult vk_result = vmaBindBufferMemory(
    m_device->GetVmaAllocator(),
    m_vma_allocation,
    m_vk_object);
  if (vk_result != VK_SUCCESS) {
    return vk_result;
  }

  return VK_SUCCESS;
}

VkResult CBuffer::MapMemory(void** pp_mapped_address)
{
  bool has_allocation = (m_vma_allocation != VK_NULL_HANDLE);
  VKEX_ASSERT_MSG(
    (m_vma_allocation != VK_NULL_HANDLE),
    "Attempting to map buffer that doesn't have an allocation!"
  );
  if (m_vma_allocation == VK_NULL_HANDLE) {
    // Not the most ideal error message
    return VK_ERROR_INITIALIZATION_FAILED;
  }

  if (m_mapped_address != nullptr) {
    *pp_mapped_address = m_mapped_address;
    return VK_SUCCESS;
  }

  VkResult vk_result = vmaMapMemory(
    m_device->GetVmaAllocator(),
    m_vma_allocation,
    &m_mapped_address);
  if (vk_result != VK_SUCCESS) {
    return vk_result;
  }

  *pp_mapped_address = m_mapped_address;

  return VK_SUCCESS;
}

void CBuffer::UnmapMemory()
{
  vmaUnmapMemory(
    m_device->GetVmaAllocator(),
    m_vma_allocation);

  m_mapped_address = nullptr;
}

bool CBuffer::IsMemoryMapped() const
{
  bool is_mapped = (m_mapped_address != nullptr);
  return is_mapped;
}

VkDeviceSize CBuffer::GetMemoryOffset() const
{
  return m_vma_allocation_info.offset;
}

VkDeviceSize CBuffer::GetMemorySize() const
{
  return m_vma_allocation_info.size;
}

vkex::Result CBuffer::Copy(size_t size, const void* p_src)
{
  bool is_host_visible = IsHostVisible();
  VKEX_ASSERT_MSG(is_host_visible, "Buffer resource must be host visible for direct copy!");
  if (!is_host_visible) {
    return vkex::Result::ErrorResourceIsNotHostVisible;
  }

  VkDeviceSize resource_size = GetMemorySize();
  if (resource_size < size) {
    return vkex::Result::ErrorResouceSizeIsInsufficient;
  }

  // Get host visible address
  bool     is_mapped        = IsMemoryMapped();
  bool     unmap_after_copy = !is_mapped;
  void*    p_dst            = nullptr;
  VkResult vk_result        = MapMemory(&p_dst);
  if (vk_result != VK_SUCCESS) {
    return vkex::Result(vk_result);
  }
  // Copy
  std::memcpy(p_dst, p_src, size);
  // Unmap if needed
  if (unmap_after_copy) {
    UnmapMemory();
  }

  return vkex::Result::Success;
}

} // namespace vkex