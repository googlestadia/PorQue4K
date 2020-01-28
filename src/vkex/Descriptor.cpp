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

#include "vkex/Descriptor.h"
#include "vkex/Device.h"
#include "vkex/ToString.h"

namespace vkex {

// =================================================================================================
// DescriptorSetLayout
// =================================================================================================
CDescriptorSetLayout::CDescriptorSetLayout()
{
}

CDescriptorSetLayout::~CDescriptorSetLayout()
{
}

vkex::Result CDescriptorSetLayout::InternalCreate(
  const vkex::DescriptorSetLayoutCreateInfo&  create_info,
  const VkAllocationCallbacks*                p_allocator
)
{
  // Copy create info
  m_create_info = create_info;

  // Create Vulkan object
  m_vk_create_info = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
  m_vk_create_info.flags        = m_create_info.flags.flags;
  m_vk_create_info.bindingCount = CountU32(m_create_info.bindings);
  m_vk_create_info.pBindings    = DataPtr(m_create_info.bindings);
  VkResult vk_result = InvalidValue<VkResult>::Value;
  VKEX_VULKAN_RESULT_CALL(
    vk_result,
    vkex::CreateDescriptorSetLayout(
      *m_device,
      &m_vk_create_info,
      p_allocator,
      &m_vk_object)
  );

  return vkex::Result::Success;
}

vkex::Result CDescriptorSetLayout::InternalDestroy(const VkAllocationCallbacks* p_allocator)
{
  if (m_vk_object != VK_NULL_HANDLE) {
    vkex::DestroyDescriptorSetLayout(
      *m_device,
      m_vk_object,
      p_allocator);

    m_vk_object = VK_NULL_HANDLE;
  }

  return vkex::Result::Success;
}

// =================================================================================================
// DescriptorSet
// =================================================================================================
CDescriptorSet::CDescriptorSet()
{
}

CDescriptorSet::~CDescriptorSet()
{
}

vkex::Result CDescriptorSet::InternalCreate(
  const vkex::DescriptorSetCreateInfo&  create_info,
  const VkAllocationCallbacks*          p_allocator
)
{
  // Copy create info
  m_create_info = create_info;

  return vkex::Result::Success;
}

vkex::Result CDescriptorSet::InternalDestroy(const VkAllocationCallbacks* p_allocator)
{
  m_create_info.vk_object = VK_NULL_HANDLE;

  return vkex::Result::Success;
}

void CDescriptorSet::SetPool(vkex::DescriptorPool pool)
{
  m_pool = pool;
}

const VkDescriptorSetLayoutBinding* CDescriptorSet::FindDescriptorBinding(uint32_t binding) const
{
  const VkDescriptorSetLayoutBinding* p_descriptor_binding = nullptr;

  auto it = std::find_if(
    std::begin(m_create_info.bindings),
    std::end(m_create_info.bindings),
    [binding](const VkDescriptorSetLayoutBinding& elem) -> bool {
      return elem.binding == binding; });

  if (it != m_create_info.bindings.end()) {
    p_descriptor_binding = &(*it);
  }

  return p_descriptor_binding;
}

void CDescriptorSet::UpdateDescriptors(uint32_t binding, VkDescriptorType descriptor_type, uint32_t array_element, uint32_t count, const VkDescriptorBufferInfo* p_infos)
{
  VkWriteDescriptorSet vk_write_descriptor = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
  vk_write_descriptor.dstSet            = m_create_info.vk_object;
  vk_write_descriptor.dstBinding        = binding;
  vk_write_descriptor.dstArrayElement   = array_element;
  vk_write_descriptor.descriptorCount   = count;
  vk_write_descriptor.descriptorType    = descriptor_type;
  vk_write_descriptor.pImageInfo        = nullptr;
  vk_write_descriptor.pBufferInfo       = p_infos;
  vk_write_descriptor.pTexelBufferView  = nullptr;

  vkUpdateDescriptorSets(
    *(m_pool->GetDevice()),
    1,
    &vk_write_descriptor,
    0,
    nullptr);
}

void CDescriptorSet::UpdateDescriptors(uint32_t binding, VkDescriptorType descriptor_type, uint32_t array_element, uint32_t count, const VkDescriptorImageInfo* p_infos)
{
  VkWriteDescriptorSet vk_write_descriptor = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
  vk_write_descriptor.dstSet            = m_create_info.vk_object;
  vk_write_descriptor.dstBinding        = binding;
  vk_write_descriptor.dstArrayElement   = array_element;
  vk_write_descriptor.descriptorCount   = count;
  vk_write_descriptor.descriptorType    = descriptor_type;
  vk_write_descriptor.pImageInfo        = p_infos;
  vk_write_descriptor.pBufferInfo       = nullptr;
  vk_write_descriptor.pTexelBufferView  = nullptr;

  vkUpdateDescriptorSets(
    *(m_pool->GetDevice()),
    1,
    &vk_write_descriptor,
    0,
    nullptr);
}

vkex::Result CDescriptorSet::UpdateDescriptor(uint32_t binding, const vkex::Buffer buffer, uint32_t array_element)
{
  const VkDescriptorSetLayoutBinding* p_descriptor_binding = FindDescriptorBinding(binding);
  if (p_descriptor_binding == nullptr) {
    return vkex::Result::ErrorInvalidDescriptorBinding;
  }

  VkDescriptorType descriptor_type = p_descriptor_binding->descriptorType;
  bool is_storage_buffer = (descriptor_type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
  bool is_uniform_buffer = (descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
  bool is_uniform_texel_buffer = (descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER);

  if (!(is_storage_buffer || is_uniform_buffer || is_uniform_texel_buffer))  {
    return vkex::Result::ErrorInvalidDescriptorType;
  }

  VkDescriptorBufferInfo info  {};
  info.buffer = *buffer;
  info.offset = 0;
  info.range  = buffer->GetSize();

  const uint32_t count = 1;
  UpdateDescriptors(
    binding,
    descriptor_type,
    array_element,
    count,
    &info);

  return vkex::Result::Success;
}

vkex::Result CDescriptorSet::UpdateDescriptor(uint32_t binding, const vkex::Buffer buffer, VkDeviceSize dynamic_range, uint32_t array_element)
{
  const VkDescriptorSetLayoutBinding* p_descriptor_binding = FindDescriptorBinding(binding);
  if (p_descriptor_binding == nullptr) {
    return vkex::Result::ErrorInvalidDescriptorBinding;
  }

  VkDescriptorType descriptor_type = p_descriptor_binding->descriptorType;
  bool is_dynamic_uniform_buffer = (descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);

  if (!is_dynamic_uniform_buffer)  {
    return vkex::Result::ErrorInvalidDescriptorType;
  }

  VkDescriptorBufferInfo info  {};
  info.buffer = *buffer;
  info.offset = 0;
  info.range  = dynamic_range;

  const uint32_t count = 1;
  UpdateDescriptors(
    binding,
    descriptor_type,
    array_element,
    count,
    &info);

  return vkex::Result::Success;
}

vkex::Result CDescriptorSet::UpdateDescriptor(uint32_t binding, const vkex::Texture texture, uint32_t array_element)
{
  const VkDescriptorSetLayoutBinding* p_descriptor_binding = FindDescriptorBinding(binding);
  if (p_descriptor_binding == nullptr) {
    return vkex::Result::ErrorInvalidDescriptorBinding;
  }

  VkDescriptorType descriptor_type = p_descriptor_binding->descriptorType;
  bool is_sampled_image = (descriptor_type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
  bool is_storage_image = (descriptor_type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
  if (!(is_sampled_image || is_storage_image))  {
    return vkex::Result::ErrorInvalidDescriptorType;
  }

  VkImageLayout image_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  if (is_storage_image) {
    image_layout = VK_IMAGE_LAYOUT_GENERAL;
  }

  VkDescriptorImageInfo info  {};
  info.sampler      = VK_NULL_HANDLE;
  info.imageView    = *(texture->GetImageView());
  info.imageLayout  = image_layout;

  const uint32_t count = 1;
  UpdateDescriptors(
    binding,
    descriptor_type,
    array_element,
    count,
    &info);

  return vkex::Result::Success;
}

vkex::Result CDescriptorSet::UpdateDescriptor(uint32_t binding, const vkex::Sampler sampler, uint32_t array_element)
{
  const VkDescriptorSetLayoutBinding* p_descriptor_binding = FindDescriptorBinding(binding);
  if (p_descriptor_binding == nullptr) {
    return vkex::Result::ErrorInvalidDescriptorBinding;
  }

  VkDescriptorType descriptor_type = p_descriptor_binding->descriptorType;
  bool is_sampler = (descriptor_type == VK_DESCRIPTOR_TYPE_SAMPLER);
  if (!is_sampler)  {
    return vkex::Result::ErrorInvalidDescriptorType;
  }

  VkDescriptorImageInfo info  {};
  info.sampler      = *sampler;
  info.imageView    = VK_NULL_HANDLE;
  info.imageLayout  = VK_IMAGE_LAYOUT_UNDEFINED;

  const uint32_t count = 1;
  UpdateDescriptors(
    binding,
    descriptor_type,
    array_element,
    count,
    &info);

  return vkex::Result::Success;
}

// =================================================================================================
// DescriptorPool
// =================================================================================================
CDescriptorPool::CDescriptorPool()
{
}

CDescriptorPool::~CDescriptorPool()
{
}

vkex::Result CDescriptorPool::InternalCreate(
  const vkex::DescriptorPoolCreateInfo& create_info,
  const VkAllocationCallbacks*          p_allocator
)
{
  // Copy create info
  m_create_info = create_info;

  // Go wide if max sets is zero
  if (m_create_info.max_sets == 0) {
    m_create_info.max_sets
      = m_create_info.pool_sizes.sampler
      + m_create_info.pool_sizes.combined_image_sampler
      + m_create_info.pool_sizes.sampled_image
      + m_create_info.pool_sizes.storage_image
      + m_create_info.pool_sizes.uniform_texel_buffer
      + m_create_info.pool_sizes.storage_texel_buffer
      + m_create_info.pool_sizes.uniform_buffer
      + m_create_info.pool_sizes.storage_buffer
      + m_create_info.pool_sizes.uniform_buffer_dynamic
      + m_create_info.pool_sizes.storage_buffer_dynamic
      + m_create_info.pool_sizes.input_attachment;
  }

  // Create descriptor pool sizes
  {
    const std::vector<VkDescriptorType> k_vk_descriptor_types = {
      VK_DESCRIPTOR_TYPE_SAMPLER,
      VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
      VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
      VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
      VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
      VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
      VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
      VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
      VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
      VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
    };
    
    for (uint32_t i = 0; i < VK_DESCRIPTOR_TYPE_RANGE_SIZE; ++i) {
      uint32_t descriptorCount = m_create_info.pool_sizes.sizes[i];
      if (descriptorCount == 0) {
        continue;
      }
      VkDescriptorPoolSize vk_pool_size = {};
      vk_pool_size.type = k_vk_descriptor_types[i];
      vk_pool_size.descriptorCount = descriptorCount;
      m_vk_descriptor_pool_sizes.push_back(vk_pool_size);
    }
  }

  // Create Vulkan object
  m_vk_create_info = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
  m_vk_create_info.flags          = m_create_info.flags.flags;
  m_vk_create_info.maxSets        = m_create_info.max_sets;
  m_vk_create_info.poolSizeCount  = CountU32(m_vk_descriptor_pool_sizes);
  m_vk_create_info.pPoolSizes     = DataPtr(m_vk_descriptor_pool_sizes);
  VkResult vk_result = InvalidValue<VkResult>::Value;
  VKEX_VULKAN_RESULT_CALL(
    vk_result,
    vkex::CreateDescriptorPool(
      *m_device,
      &m_vk_create_info,
      p_allocator,
      &m_vk_object)
  );
  if (vk_result != VK_SUCCESS) {
    return vkex::Result(vk_result);
  }

  return vkex::Result::Success;
}

vkex::Result CDescriptorPool::InternalDestroy(const VkAllocationCallbacks* p_allocator)
{
  if (m_vk_object != VK_NULL_HANDLE) {
    vkex::DestroyDescriptorPool(
      *m_device,
      m_vk_object,
      p_allocator);

    m_vk_object = VK_NULL_HANDLE;
  }

  return vkex::Result::Success;
}

vkex::Result CDescriptorPool::AllocateDescriptorSets(
  const vkex::DescriptorSetAllocateInfo&  allocate_info, 
  vkex::DescriptorSet*                    p_descriptor_sets
)
{
  size_t layout_count = allocate_info.layouts.size();
  if (layout_count == 0) {
    return vkex::Result::ErrorDescriptorSetLayoutsMustBeMoreThanZero;
  }

  std::vector<VkDescriptorSetLayout> vk_layouts;
  for (auto& layout : allocate_info.layouts) {
    VkDescriptorSetLayout vk_layout = *(layout);
    vk_layouts.push_back(vk_layout);
  }

  VkDescriptorSetAllocateInfo vk_allocate_info = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
  vk_allocate_info.descriptorPool     = m_vk_object;
  vk_allocate_info.descriptorSetCount = CountU32(vk_layouts);
  vk_allocate_info.pSetLayouts        = DataPtr(vk_layouts);

  std::vector<VkDescriptorSet> vk_descriptor_sets(layout_count);
  VkResult vk_result = InvalidValue<VkResult>::Value;
  VKEX_VULKAN_RESULT_CALL(
    vk_result,
    vkex::AllocateDescriptorSets(
      *m_device,
      &vk_allocate_info,
      vk_descriptor_sets.data())
  );
  if (vk_result != VK_SUCCESS) {
    return vkex::Result(vk_result);
  }

  std::vector<vkex::DescriptorSet> descriptor_sets;
  vkex::Result vkex_result = vkex::Result::Undefined;
  for (uint32_t i = 0; i < layout_count; ++i) {
    vkex::DescriptorSetCreateInfo descriptor_set_create_info = {};
    descriptor_set_create_info.vk_object = vk_descriptor_sets[i];
    descriptor_set_create_info.bindings  = allocate_info.layouts[i]->GetBindings();
    
    vkex::DescriptorSet descriptor_set = nullptr;
    vkex_result = CreateObject<CDescriptorSet>(
      descriptor_set_create_info,
      nullptr,
      m_stored_descriptor_sets,
      &CDescriptorSet::SetPool,
      this,
      &descriptor_set);

    if (!vkex_result) {
      break;
    }

    descriptor_sets.push_back(descriptor_set);
  }

  if (!vkex_result) {
    this->FreeDescriptorSets(
      CountU32(descriptor_sets),
      DataPtr(descriptor_sets));
    return vkex_result;
  }

  for (uint32_t i = 0; i < layout_count; ++i) {
    vkex::DescriptorSet descriptor_set = descriptor_sets[i];
    p_descriptor_sets[i] = descriptor_set;
  }
  
  return vkex::Result::Success;
}
  
vkex::Result CDescriptorPool::AllocateDescriptorSets(
  const vkex::DescriptorSetAllocateInfo&  allocate_info, 
  std::vector<vkex::DescriptorSet>*       p_descriptor_sets
)
{
  size_t layout_count = allocate_info.layouts.size();
  if (layout_count == 0) {
    return vkex::Result::ErrorDescriptorSetLayoutsMustBeMoreThanZero;
  }

  std::vector<vkex::DescriptorSet> descriptor_sets(layout_count);
  vkex::Result vkex_result = AllocateDescriptorSets(
    allocate_info,
    descriptor_sets.data());
  
  if (!vkex_result) {
    return vkex_result;
  }

  for (auto& descriptor_set : descriptor_sets) {
    p_descriptor_sets->push_back(descriptor_set);
  }

  return vkex::Result::Success;
}

vkex::Result CDescriptorPool::AllocateDescriptorSet(
  const vkex::DescriptorSetAllocateInfo&  allocate_info, 
  vkex::DescriptorSet*                    p_descriptor_set
)
{
  size_t layout_count = allocate_info.layouts.size();
  if (layout_count != 1) {
    return vkex::Result::ErrorDescriptorSetLayoutsMustBeOne;
  }

  std::vector<vkex::DescriptorSet> descriptor_sets(layout_count);
  vkex::Result vkex_result = AllocateDescriptorSets(
    allocate_info,
    descriptor_sets.data());
  
  if (!vkex_result) {
    return vkex_result;
  }

  *p_descriptor_set = descriptor_sets[0];

  return vkex::Result::Success;
}

void CDescriptorPool::FreeDescriptorSets(
  uint32_t                    descriptor_set_count, 
  const vkex::DescriptorSet*  p_descriptor_sets
)
{
  std::vector<VkDescriptorSet> vk_descriptor_sets;
  for (uint32_t i = 0; i < descriptor_set_count; ++i) {
    vkex::DescriptorSet descriptor_set = p_descriptor_sets[i];
    // Copy Vulkan object
    vk_descriptor_sets.push_back(descriptor_set->GetVkObject());
    // Destroy the stored object
    DestroyObject<CDescriptorSet>(
      m_stored_descriptor_sets,      
      descriptor_set,
      nullptr);
  }

  vkex::FreeDescriptorSets(
    *m_device,
    m_vk_object,
    CountU32(vk_descriptor_sets),
    DataPtr(vk_descriptor_sets));
}

void CDescriptorPool::FreeDescriptorSets(const std::vector<vkex::DescriptorSet>* p_descriptor_sets)
{
  this->FreeDescriptorSets(
    CountU32(*p_descriptor_sets),
    DataPtr(*p_descriptor_sets));
}

void CDescriptorPool::FreeDescriptorSet(const vkex::DescriptorSet descriptor_set)
{
  this->FreeDescriptorSets(
    1,
    &descriptor_set);  
}

} // namespace vkex