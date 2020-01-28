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

#ifndef __VKEX_DESCRIPTOR_H__
#define __VKEX_DESCRIPTOR_H__

#include <vkex/Config.h>
#include <vkex/Traits.h>
#include <vkex/VulkanUtil.h>

namespace vkex {

// =================================================================================================
// DescriptorSetLayout
// =================================================================================================

/** @struct DescriptorSetLayoutCreateInfo 
 *
 */
struct DescriptorSetLayoutCreateInfo {
  vkex::DescriptorLayoutCreateFlags         flags;
  std::vector<VkDescriptorSetLayoutBinding> bindings;
};

/** @class IDescriptorSetLayout
 *
 */ 
class CDescriptorSetLayout : public IDeviceObject {
public:
  CDescriptorSetLayout();
  ~CDescriptorSetLayout();

  /** @fn operator VkDescriptorSetLayout()
   *
   */
  operator VkDescriptorSetLayout() const { 
    return m_vk_object; 
  }

  /** @fn GetVkObject
   *
   */
  VkDescriptorSetLayout GetVkObject() const { 
    return m_vk_object; 
  }
  
  /** @fn GetBindings
   *
   */
  const std::vector<VkDescriptorSetLayoutBinding>& GetBindings() const {
    return m_create_info.bindings;
  }

private:
  friend class CDevice;
  friend class IObjectStorageFunctions;

  /** @fn InternalCreate
   *
   */
  vkex::Result InternalCreate(
    const vkex::DescriptorSetLayoutCreateInfo&  create_info,
    const VkAllocationCallbacks*                p_allocator
  );

  /** @fn InternalDestroy
   *
   */
  vkex::Result InternalDestroy(const VkAllocationCallbacks* p_allocator);

private:
  vkex::DescriptorSetLayoutCreateInfo m_create_info = {};
  VkDescriptorSetLayoutCreateInfo     m_vk_create_info = {};
  VkDescriptorSetLayout               m_vk_object = VK_NULL_HANDLE;
};

// =================================================================================================
// DescriptorSet
// =================================================================================================

/** @struct DescriptorSetCreateInfo 
 *
 */
struct DescriptorSetCreateInfo {
  VkDescriptorSet                           vk_object;
  std::vector<VkDescriptorSetLayoutBinding> bindings;
};

/** @class IDescriptorSet
 *
 */ 
class CDescriptorSet : public IDeviceObject {
public:
  CDescriptorSet();
  ~CDescriptorSet();

  /** @fn operator VkDescriptorSet()
   *
   */
  operator VkDescriptorSet() const { 
    return m_create_info.vk_object; 
  }

  /** @fn GetVkObject
   *
   */
  VkDescriptorSet GetVkObject() const { 
    return m_create_info.vk_object; 
  }

  /** @fn UpdateDescriptors
   *
   */
  void UpdateDescriptors(uint32_t binding, VkDescriptorType descriptor_type, uint32_t array_element, uint32_t count, const VkDescriptorBufferInfo* p_infos);

  /** @fn UpdateDescriptors
   *
   */
  void UpdateDescriptors(uint32_t binding, VkDescriptorType descriptor_type, uint32_t array_element, uint32_t count, const VkDescriptorImageInfo* p_infos);

  /** @fn UpdateDescriptor
   *
   */
  vkex::Result UpdateDescriptor(uint32_t binding, const vkex::Buffer buffer, uint32_t array_element = 0);

  /** @fn UpdateDescriptor
   *
   */
  vkex::Result UpdateDescriptor(uint32_t binding, const vkex::Buffer buffer, VkDeviceSize dynamic_range, uint32_t array_element = 0);

  /** @fn UpdateDescriptor
   *
   */
  vkex::Result UpdateDescriptor(uint32_t binding, const vkex::Texture texture, uint32_t array_element = 0);

  /** @fn UpdateDescriptor
   *
   */
  vkex::Result UpdateDescriptor(uint32_t binding, const vkex::Sampler sampler, uint32_t array_element = 0);

private:
  friend class CDescriptorPool;
  friend class IObjectStorageFunctions;

  /** @fn InternalCreate
   *
   */
  vkex::Result InternalCreate(
    const vkex::DescriptorSetCreateInfo&  create_info,
    const VkAllocationCallbacks*          p_allocator
  );

  /** @fn InternalDestroy
   *
   */
  vkex::Result InternalDestroy(const VkAllocationCallbacks* p_allocator);

  /** @fn SetPool
   *
   */
  void SetPool(vkex::DescriptorPool pool);

  /** @fn FindBinding
   *
   */
  const VkDescriptorSetLayoutBinding* FindDescriptorBinding(uint32_t binding) const;

private:
  vkex::DescriptorPool          m_pool = nullptr;
  vkex::DescriptorSetCreateInfo m_create_info = {};
};

// =================================================================================================
// DescriptorPool
// =================================================================================================

/** @struct CommandBufferAllocateInfo 
 *
 */
struct DescriptorSetAllocateInfo {
  std::vector<vkex::DescriptorSetLayout>  layouts;
};

/** @struct DescriptorPoolCreateInfo 
 *
 */
struct DescriptorPoolCreateInfo {
  vkex::DescriptorPoolCreateFlags flags;
  uint32_t                        max_sets;
  vkex::DescriptorPoolSizes       pool_sizes = {};
  uint32_t                        size_multiplier = 1;
};

/** @class IDescriptorPool
 *
 */ 
class CDescriptorPool 
  : public IDeviceObject,
    protected IObjectStorageFunctions
{
public:
  CDescriptorPool();
  ~CDescriptorPool();

  /** @fn operator VkDescriptorPool()
   *
   */
  operator VkDescriptorPool() const { 
    return m_vk_object; 
  }

  /** @fn GetVkObject
   *
   */
  VkDescriptorPool GetVkObject() const { 
    return m_vk_object; 
  }

  /** @fn AllocateDescriptorSets
   *
   */
  vkex::Result AllocateDescriptorSets(
    const vkex::DescriptorSetAllocateInfo&  allocate_info, 
    vkex::DescriptorSet*                    p_descriptor_sets
  );
  
  /** @fn AllocateDescriptorSets
   *
   */
  vkex::Result AllocateDescriptorSets(
    const vkex::DescriptorSetAllocateInfo&  allocate_info, 
    std::vector<vkex::DescriptorSet>*       p_descriptor_sets
  );

  /** @fn AllocateDescriptorSet
   *
   */
  vkex::Result AllocateDescriptorSet(
    const vkex::DescriptorSetAllocateInfo&  allocate_info, 
    vkex::DescriptorSet*                    p_descriptor_set
  );

  /** @fn FreeDescriptorSets
   *
   */
  void FreeDescriptorSets(
    uint32_t                    descriptor_set_count, 
    const vkex::DescriptorSet*  p_descriptor_sets
  );

  /** @fn FreeDescriptorSets
   *
   */
  void FreeDescriptorSets(const std::vector<vkex::DescriptorSet>* p_descriptor_sets);

  /** @fn FreeDescriptorSet
   *
   */
  void FreeDescriptorSet(const vkex::DescriptorSet descriptor_set);


private:
  friend class CDevice;
  friend class IObjectStorageFunctions;

  /** @fn InternalCreate
   *
   */
  vkex::Result InternalCreate(
    const vkex::DescriptorPoolCreateInfo& create_info,
    const VkAllocationCallbacks*          p_allocator
  );

  /** @fn InternalDestroy
   *
   */
  vkex::Result InternalDestroy(const VkAllocationCallbacks* p_allocator);

private:
  vkex::DescriptorPoolCreateInfo                m_create_info = {};
  VkDescriptorPoolCreateInfo                    m_vk_create_info = {};
  std::vector<VkDescriptorPoolSize>             m_vk_descriptor_pool_sizes;
  VkDescriptorPool                              m_vk_object = VK_NULL_HANDLE;
  std::vector<std::unique_ptr<CDescriptorSet>>  m_stored_descriptor_sets;
};

} // namespace vkex

#endif // __VKEX_DESCRIPTOR_H__