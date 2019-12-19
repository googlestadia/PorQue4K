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

#include "vkex/QueryPool.h"
#include "vkex/Device.h"
#include "vkex/ToString.h"

namespace vkex {

// =================================================================================================
// QueryPool
// =================================================================================================
CQueryPool::CQueryPool()
{
}

CQueryPool::~CQueryPool()
{
}

vkex::Result CQueryPool::InternalCreate(
  const vkex::QueryPoolCreateInfo&  create_info,
  const VkAllocationCallbacks*      p_allocator
)
{
  // Copy create info
  m_create_info = create_info;

  // Create Vulkan sampler
  {
    // Create info
    m_vk_create_info = { VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO };
    m_vk_create_info.queryType          = m_create_info.query_type;
    m_vk_create_info.queryCount         = m_create_info.query_count;
    m_vk_create_info.pipelineStatistics = m_create_info.pipeline_statistics.flags;
    // Create Vulkan sampler
    VkResult vk_result = InvalidValue<VkResult>::Value;
    VKEX_VULKAN_RESULT_CALL(
      vk_result,
      vkex::CreateQueryPool(
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

vkex::Result CQueryPool::InternalDestroy(const VkAllocationCallbacks* p_allocator)
{
  if (m_vk_object != VK_NULL_HANDLE) {
    vkex::DestroyQueryPool(
      *m_device,
      m_vk_object,
      p_allocator);

    m_vk_object = VK_NULL_HANDLE;
  }
  return vkex::Result::Success;
}

} // namespace vkex