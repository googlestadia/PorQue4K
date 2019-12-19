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

#ifndef __VKEX_QUERY_POOL_H__
#define __VKEX_QUERY_POOL_H__

#include <vkex/Config.h>
#include <vkex/Traits.h>
#include <vkex/VulkanUtil.h>

namespace vkex {

// =================================================================================================
// QueryPool
// =================================================================================================

/** @struct QueryPoolCreateInfo 
 *
 */
struct QueryPoolCreateInfo {
  VkQueryType                 query_type;
  uint32_t                    query_count;
  QueryPipelineStatisticFlags pipeline_statistics;
};

/** @class IQueryPool
 *
 */ 
class CQueryPool : public IDeviceObject {
public:
  CQueryPool();
  ~CQueryPool();

  /** @fn operator VkQueryPool()
   *
   */
  operator VkQueryPool() const { 
    return m_vk_object; 
  }

  /** @fn GetVkObject
   *
   */
  VkQueryPool GetVkObject() const { 
    return m_vk_object; 
  }

  /** @fn GetQueryType
   *
   */
  VkQueryType GetQueryType() const {
    return m_create_info.query_type;
  }

  /** @fn GetQueryCount
   *
   */
  uint32_t GetQueryCount() const {
    return m_create_info.query_count;
  }

private:
  friend class CDevice;
  friend class IObjectStorageFunctions;

  /** @fn InternalCreate
   *
   */
  vkex::Result InternalCreate(
    const vkex::QueryPoolCreateInfo&  create_info,
    const VkAllocationCallbacks*    p_allocator
  );

  /** @fn InternalDestroy
   *
   */
  vkex::Result InternalDestroy(const VkAllocationCallbacks* p_allocator);

private:
  vkex::QueryPoolCreateInfo m_create_info = {};
  VkQueryPoolCreateInfo     m_vk_create_info = {};
  VkQueryPool               m_vk_object = VK_NULL_HANDLE;
};

} // namespace vkex

#endif // __VKEX_QUERY_POOL_H__