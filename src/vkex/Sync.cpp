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

#include "vkex/Sync.h"
#include "vkex/Device.h"
#include "vkex/ToString.h"

namespace vkex {

// =================================================================================================
// Event
// =================================================================================================
CEvent::CEvent()
{
}

CEvent::~CEvent()
{
}

vkex::Result CEvent::InternalCreate(
  const vkex::EventCreateInfo&  create_info,
  const VkAllocationCallbacks*  p_allocator
)
{
  // Copy create info
  m_create_info = create_info;

  // Vulkan create info
  {
    m_vk_create_info = { VK_STRUCTURE_TYPE_EVENT_CREATE_INFO };
    m_vk_create_info.flags  = m_create_info.flags;
  }

  // Create Vulkan object
  {
    VkResult vk_result = InvalidValue<VkResult>::Value;
    VKEX_VULKAN_RESULT_CALL(
      vk_result,
      vkex::CreateEvent(
        *m_device,
        &m_vk_create_info,
        p_allocator,
        &m_vk_object)
    );
    if (vk_result != VK_SUCCESS) {
      return vkex::Result(vk_result);
    }
  }

  return vkex::Result::Success;
}

vkex::Result CEvent::InternalDestroy(const VkAllocationCallbacks* p_allocator)
{
  if (m_vk_object != VK_NULL_HANDLE) {
    vkex::DestroyEvent(
      *m_device,
      m_vk_object,
      p_allocator);

    m_vk_object = VK_NULL_HANDLE;
  }

  return vkex::Result::Success;
}

// =================================================================================================
// Fence
// =================================================================================================
CFence::CFence()
{
}

CFence::~CFence()
{
}

vkex::Result CFence::InternalCreate(
  const vkex::FenceCreateInfo&  create_info,
  const VkAllocationCallbacks*  p_allocator
)
{
  // Copy create info
  m_create_info = create_info;

  // Vulkan create info
  {
    m_vk_create_info = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
    m_vk_create_info.flags  = m_create_info.flags.flags;
  }

  // Create Vulkan object
  {
    VkResult vk_result = InvalidValue<VkResult>::Value;
    VKEX_VULKAN_RESULT_CALL(
      vk_result,
      vkex::CreateFence(
        *m_device,
        &m_vk_create_info,
        p_allocator,
        &m_vk_object)
    );
    if (vk_result != VK_SUCCESS) {
      return vkex::Result(vk_result);
    }
  }

  //// Set object name
  //if (!m_create_info.object_name.empty()) {
  //  VkDebugUtilsObjectNameInfoEXT vk_name_info = { VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT };
  //  vk_name_info.objectType   = VK_OBJECT_TYPE_FENCE;
  //  vk_name_info.objectHandle = reinterpret_cast<uintptr_t>(m_vk_object);
  //  vk_name_info.pObjectName  = DataPtr(m_create_info.object_name);
  //  VkResult vk_result = InvalidValue<VkResult>::Value;
  //  VKEX_VULKAN_RESULT_CALL(
  //    vk_result,
  //    vkex::SetDebugUtilsObjectNameEXT(
  //      *m_device,
  //      &vk_name_info)          
  //  );
  //  if (vk_result != VK_SUCCESS) {
  //    return vkex::Result(vk_result);
  //  }
  //}

  return vkex::Result::Success;
}

vkex::Result CFence::InternalDestroy(const VkAllocationCallbacks* p_allocator)
{
  if (m_vk_object != VK_NULL_HANDLE) {
    vkex::DestroyFence(
      *m_device,
      m_vk_object,
      p_allocator);

    m_vk_object = VK_NULL_HANDLE;
  }

  return vkex::Result::Success;
}

VkResult CFence::ResetFence()
{
  VkResult vk_result = vkex::ResetFenceVKEX(
    *m_device,
    m_vk_object);
  if (vk_result != VK_SUCCESS) {
    return vk_result;
  }
  return VK_SUCCESS;
}

VkResult CFence::GetFenceStatus()
{
  VkResult vk_result = vkex::GetFenceStatus(
    *m_device,
    m_vk_object);
  if (vk_result != VK_SUCCESS) {
    return vk_result;
  }
  return VK_SUCCESS;
}

VkResult CFence::WaitForFence(uint64_t timeout)
{
  VkResult vk_result = vkex::WaitForFenceVKEX(
    *m_device,
    m_vk_object,
    VK_TRUE,
    timeout);
  if (vk_result != VK_SUCCESS) {
    return vk_result;
  }
  return VK_SUCCESS;
}

// =================================================================================================
// Semaphore
// =================================================================================================
CSemaphore::CSemaphore()
{
}

CSemaphore::~CSemaphore()
{
}

vkex::Result CSemaphore::InternalCreate(
  const vkex::SemaphoreCreateInfo&  create_info,
  const VkAllocationCallbacks*      p_allocator
)
{
  // Copy create info
  m_create_info = create_info;

  // Vulkan create info
  {
    m_vk_create_info = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    m_vk_create_info.flags  = m_create_info.flags.flags;
  }

  // Create Vulkan object
  {
    VkResult vk_result = InvalidValue<VkResult>::Value;
    VKEX_VULKAN_RESULT_CALL(
      vk_result,
      vkex::CreateSemaphore(
        *m_device,
        &m_vk_create_info,
        p_allocator,
        &m_vk_object)
    );
    if (vk_result != VK_SUCCESS) {
      return vkex::Result(vk_result);
    }
  }

  //// Set object name
  //if (!m_create_info.object_name.empty()) {
  //  VkDebugUtilsObjectNameInfoEXT vk_name_info = { VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT };
  //  vk_name_info.objectType   = VK_OBJECT_TYPE_SEMAPHORE;
  //  vk_name_info.objectHandle = reinterpret_cast<uintptr_t>(m_vk_object);
  //  vk_name_info.pObjectName  = DataPtr(m_create_info.object_name);
  //  VkResult vk_result = InvalidValue<VkResult>::Value;
  //  VKEX_VULKAN_RESULT_CALL(
  //    vk_result,
  //    vkex::SetDebugUtilsObjectNameEXT(
  //      *m_device,
  //      &vk_name_info)          
  //  );
  //  if (vk_result != VK_SUCCESS) {
  //    return vkex::Result(vk_result);
  //  }
  //}

  return vkex::Result::Success;
}

vkex::Result CSemaphore::InternalDestroy(const VkAllocationCallbacks* p_allocator)
{
  if (m_vk_object != VK_NULL_HANDLE) {
    vkex::DestroySemaphore(
      *m_device,
      m_vk_object,
      p_allocator);

    m_vk_object = VK_NULL_HANDLE;
  }

  return vkex::Result::Success;
}

} // namespace vkex