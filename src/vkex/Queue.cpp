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

#include <vkex/Queue.h>
#include <vkex/Device.h>

namespace vkex {

CQueue::CQueue()
{
}

CQueue::~CQueue()
{
}

vkex::Result CQueue::InternalCreate(
  const vkex::QueueCreateInfo&  create_info,
  const VkAllocationCallbacks*  p_allocator
)
{
  // Copy create info
  m_create_info = create_info;

  return vkex::Result::Success;
}

vkex::Result CQueue::InternalDestroy(const VkAllocationCallbacks* p_allocator)
{
  return vkex::Result::Success;
}

VkBool32 CQueue::SupportsPresent(const vkex::DisplayInfo& display_info) const
{
  VkBool32 supported = GetDevice()->GetPhysicalDevice()->SupportsPresent(
    m_create_info.queue_family_index,
    display_info);
  return supported;
}

VkResult CQueue::WaitIdle()
{
  VkResult vk_result = vkex::QueueWaitIdle(
    m_create_info.vk_object);
  if (vk_result != VK_SUCCESS) {
    return vk_result;
  }
  return VK_SUCCESS;
}

} // namespace vkex