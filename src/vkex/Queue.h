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

#ifndef __VKEX_QUEUE_H__
#define __VKEX_QUEUE_H__

#include <vkex/Config.h>
#include <vkex/Traits.h>
#include <vkex/VulkanUtil.h>

namespace vkex {

// =================================================================================================
// Queue
// =================================================================================================


/** @struct QueueCreateInfo 
 *
 */
struct QueueCreateInfo {
  vkex::QueueFlags  requested_queue_flags;
  vkex::QueueFlags  supported_queue_flags;
  uint32_t          queue_family_index;
  uint32_t          queue_index;
  VkQueue           vk_object;
};

/** @class IQueue
 *
 */ 
class CQueue : public IDeviceObject {
public:
  CQueue();
  ~CQueue();

  /** @fn operator VkQueue()
   *
   */
  operator VkQueue() const { 
    return m_create_info.vk_object; 
  }

  /** @fn GetVkObject
   *
   */
  VkQueue GetVkObject() const { 
    return m_create_info.vk_object; 
  }

  /** @fn GetVkQueueFamilyIndex
   *
   */
  uint32_t GetVkQueueFamilyIndex() const {
    return m_create_info.queue_family_index;
  }

  /** @fn GetVkQueueIndex
   *
   */
  uint32_t GetVkQueueIndex() const {
    return m_create_info.queue_index;
  }

  /** @fn GetRequestedQueueFlags
   *
   */
  const vkex::QueueFlags& GetRequestedQueueFlags() const {
    return m_create_info.requested_queue_flags;
  }

  /** @fn GetSupportedQueueFlags
   *
   */
  const vkex::QueueFlags& GetSupportedQueueFlags() const {
    return m_create_info.supported_queue_flags;
  }

  /** @fn SupportsPresent
   *
   */
  VkBool32 SupportsPresent(const vkex::DisplayInfo& display_info) const;

  /** @fn WaitIdle
   *
   */
  VkResult WaitIdle();

private:  
  friend class CDevice;
  friend class IObjectStorageFunctions;

  /** @fn InternalCreate
   *
   */
  vkex::Result InternalCreate(
    const vkex::QueueCreateInfo&  create_info,
    const VkAllocationCallbacks*  p_allocator
  );

  /** @fn InternalDestroy
   *
   */
  vkex::Result InternalDestroy(const VkAllocationCallbacks* p_allocator);

private:
  vkex::QueueCreateInfo m_create_info = {};
};

} // namespace vkex

#endif // __VKEX_QUEUE_H__