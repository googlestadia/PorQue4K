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

#include <vkex/Buffer.h>
#include <vkex/Device.h>
#include <vkex/Image.h>
#include <vkex/VulkanUtil.h>
#include <vkex/DebugMarker.h>

namespace vkex {

// =================================================================================================
// Descriptor Binding/Set Related Objects
// =================================================================================================

// =================================================================================================
// Vertex Description
// =================================================================================================
VertexAttributeDescription::VertexAttributeDescription(
  VkFormat           format,
  uint32_t           binding,
  const std::string& name)
{
  m_description.location = 0;
  m_description.binding  = binding;
  m_description.format   = format;
  m_description.offset   = 0;
  m_name                 = name;
}

VertexAttributeDescription::VertexAttributeDescription(
  uint32_t           location,
  uint32_t           binding,
  VkFormat           format,
  uint32_t           offset,
  const std::string& name)
{
  m_description.location = location;
  m_description.binding  = binding;
  m_description.format   = format;
  m_description.offset   = offset;
  m_name                 = name;
}

VertexAttributeDescription::VertexAttributeDescription(
  const VkVertexInputAttributeDescription& description,
  const std::string&                       name)
{
  m_description = description;
  m_name        = name;
}

VertexBindingDescription::VertexBindingDescription(
  uint32_t          binding,
  VkVertexInputRate vertex_input_rate)
{
  m_description.binding   = binding;
  m_description.stride    = 0;
  m_description.inputRate = vertex_input_rate;
}

VertexBindingDescription::VertexBindingDescription(
  uint32_t                                       binding,
  VkVertexInputRate                              vertex_input_rate,
  const std::vector<VertexAttributeDescription>& attributes)
{
  m_description.binding   = binding;
  m_description.stride    = 0;
  m_description.inputRate = vertex_input_rate;
  m_attributes            = attributes;
}

void VertexBindingDescription::AddAttribute(const VertexAttributeDescription& attribute)
{
  m_attributes.push_back(attribute);
  UpdateOffsetsAndStride();
}

void VertexBindingDescription::AddAttribute(
  uint32_t           location,
  VkFormat           format,
  const std::string& name)
{
  VertexAttributeDescription attribute(location, m_description.binding, format, 0, name);
  m_attributes.push_back(attribute);
  UpdateOffsetsAndStride();
}

void VertexBindingDescription::UpdateOffsetsAndStride()
{
  std::sort(
    std::begin(m_attributes),
    std::end(m_attributes),
    [](const vkex::VertexAttributeDescription& a, const vkex::VertexAttributeDescription& b)
      -> bool { return a.m_description.location < b.m_description.location; });

  uint32_t offset = 0;
  uint32_t count  = CountU32(m_attributes);
  for (uint32_t i = 0; i < count; ++i) {
    auto& attribute                = m_attributes[i];
    attribute.m_description.offset = offset;
    uint32_t size                  = FormatSize(attribute.m_description.format);
    offset += size;
  }

  m_description.stride = offset;
}

// =================================================================================================
// ShaderProgramInterface
// =================================================================================================
ShaderInterface::ShaderInterface()
{
}

ShaderInterface::~ShaderInterface()
{
}

vkex::Result ShaderInterface::AddBinding(const vkex::ShaderInterface::Binding& binding)
{
  uint32_t set_number = binding.set_number;
  auto     it_set =
    FindIf(m_descriptor_sets, [set_number](const vkex::ShaderInterface::Set& elem) -> bool {
      return elem.set_number == set_number;
    });

  vkex::ShaderInterface::Set* p_set = nullptr;
  if (it_set != std::end(m_descriptor_sets)) {
    p_set = &(*it_set);
  } else {
    vkex::ShaderInterface::Set set = {};
    set.set_number                 = binding.set_number;
    m_descriptor_sets.push_back(set);
    p_set = &m_descriptor_sets.back();
  }

  auto it_binding =
    FindIf(p_set->bindings, [binding](const vkex::ShaderInterface::Binding& elem) -> bool {
      bool is_same_binding = (elem.binding_number == binding.binding_number);
      bool is_same_type    = (elem.descriptor_type == binding.descriptor_type);
      return is_same_binding && is_same_type;
    });

  // Add if binding isn't found
  if (it_binding == std::end(p_set->bindings)) {
    p_set->bindings.push_back(binding);
  }
  // OR the stage flags if binding is found
  else {
    (*it_binding).stage_flags.flags |= binding.stage_flags.flags;
  }

  return vkex::Result::Success;
}

void ShaderInterface::AddBindings(const vkex::ShaderInterface& interface)
{
  for (auto& set_desc : interface.m_descriptor_sets) {
    for (auto& binding : set_desc.bindings) {
      AddBinding(binding);
    }
  }
}

vkex::DescriptorPoolSizes ShaderInterface::GetDescriptorPoolSizes() const
{
  vkex::DescriptorPoolSizes sizes = {};
  for (auto& set : m_descriptor_sets) {
    for (auto& binding : set.bindings) {
      sizes.sizes[binding.descriptor_type] += binding.descriptor_count;
    }
  }
  return sizes;
}

std::vector<uint32_t> ShaderInterface::GetSetNumbers() const
{
  std::vector<uint32_t> set_numbers;
  for (auto& set : m_descriptor_sets) {
    set_numbers.push_back(set.set_number);
  }
  return set_numbers;
}

const std::vector<vkex::ShaderInterface::Set>& ShaderInterface::GetSets() const
{
  return m_descriptor_sets;
}

vkex::ShaderInterface::Set ShaderInterface::GetSet(uint32_t set_number) const
{
  auto it = FindIf(m_descriptor_sets, [set_number](const vkex::ShaderInterface::Set& elem) -> bool {
    return elem.set_number == set_number;
  });

  vkex::ShaderInterface::Set set = {};
  set.set_number                 = UINT32_MAX;

  if (it != std::end(m_descriptor_sets)) {
    set = *it;
  }

  return set;
}

VkDescriptorType ShaderInterface::GetDescriptorType(uint32_t set_number, uint32_t binding_number) const
{
  VkDescriptorType descriptor_type = InvalidValue<VkDescriptorType>::Value;

  auto it_set = FindIf(
    m_descriptor_sets, 
    [set_number](const vkex::ShaderInterface::Set& elem) -> bool {return elem.set_number == set_number;});

  if (it_set != m_descriptor_sets.end()) {
    auto& bindings = it_set->bindings;
    auto it_binding = FindIf(
      bindings, 
      [binding_number](const vkex::ShaderInterface::Binding& elem) -> bool {return elem.binding_number == binding_number;});
    if (it_binding != bindings.end()) {
      descriptor_type = it_binding->descriptor_type;
    }
  }

  return descriptor_type;
}

// =================================================================================================
// ShaderArguments
// =================================================================================================
ShaderArguments::ShaderArguments()
{
}

ShaderArguments::ShaderArguments(const std::vector<AssignedDescriptorSet>& assigned_sets)
  : m_assigned_sets(assigned_sets)
{
  SortSets();
}

ShaderArguments::~ShaderArguments()
{
}

void ShaderArguments::SortSets()
{
  std::sort(
    std::begin(m_assigned_sets),
    std::end(m_assigned_sets),
    [](const AssignedDescriptorSet& a, AssignedDescriptorSet& b) {
      return a.set_number < b.set_number; });
}

vkex::Result ShaderArguments::AssignSet(uint32_t set_number, vkex::DescriptorSet descriptor_set)
{
  auto it = std::find_if(
    std::begin(m_assigned_sets),
    std::end(m_assigned_sets),
    [set_number](const AssignedDescriptorSet& elem) -> bool {
      return elem.set_number == set_number; });

  if (it != std::end(m_assigned_sets)) {
    it->descriptor_set = descriptor_set;
  }
  else {
    AssignedDescriptorSet assigned_set = { set_number, descriptor_set };
    m_assigned_sets.push_back(assigned_set);
  }

  SortSets();

  return vkex::Result::Success;
}

vkex::Result ShaderArguments::AssignDescriptor(uint32_t set_number, uint32_t binding_number, const vkex::Buffer constant_buffer, uint32_t array_element)
{
  auto it = std::find_if(
    std::begin(m_assigned_sets),
    std::end(m_assigned_sets),
    [set_number](const AssignedDescriptorSet& elem) -> bool {
      return elem.set_number == set_number; });
  if (it == std::end(m_assigned_sets)) {
    return vkex::Result::ErrorDescriptorSetNumberNotFound;
  }

  it->descriptor_set->UpdateDescriptor(binding_number, constant_buffer, array_element);

  return vkex::Result::Success;
}

vkex::Result ShaderArguments::AssignDescriptor(uint32_t set_number, uint32_t binding_number, const vkex::Texture texture, uint32_t array_element)
{
  auto it = std::find_if(
    std::begin(m_assigned_sets),
    std::end(m_assigned_sets),
    [set_number](const AssignedDescriptorSet& elem) -> bool {
      return elem.set_number == set_number; });
  if (it == std::end(m_assigned_sets)) {
    return vkex::Result::ErrorDescriptorSetNumberNotFound;
  }

  it->descriptor_set->UpdateDescriptor(binding_number, texture, array_element);

  return vkex::Result::Success;
}

vkex::Result ShaderArguments::AssignDescriptor(uint32_t set_number, uint32_t binding_number, const vkex::Sampler sampler, uint32_t array_element)
{
  auto it = std::find_if(
    std::begin(m_assigned_sets),
    std::end(m_assigned_sets),
    [set_number](const AssignedDescriptorSet& elem) -> bool {
      return elem.set_number == set_number; });
  if (it == std::end(m_assigned_sets)) {
    return vkex::Result::ErrorDescriptorSetNumberNotFound;
  }

  it->descriptor_set->UpdateDescriptor(binding_number, sampler, array_element);

  return vkex::Result::Success;
}

std::vector<VkDescriptorSet> ShaderArguments::GetVkDescriptorSets(uint32_t first_set_number, uint32_t set_count) const
{
  std::vector<VkDescriptorSet> vk_descriptor_sets;
  for (const auto& assigned_set : m_assigned_sets) {
    if (assigned_set.set_number < first_set_number) {
      continue;
    }
    vk_descriptor_sets.push_back(*assigned_set.descriptor_set);
    if (vk_descriptor_sets.size() == set_count) {
      break;
    }
  }
  if (set_count != kMaxAllSets) {
    VKEX_ASSERT_MSG((vk_descriptor_sets.size() == set_count), "Resulting set count doesn't match requested set count");
  }
  return vk_descriptor_sets;
}

// =================================================================================================
// Utility Functions
// =================================================================================================
vkex::ImageAspectFlags DetermineAspectMask(VkFormat format)
{
  vkex::ImageAspectFlags result = 0;
  switch (format) {
    // Depth
    case VK_FORMAT_D16_UNORM:
    case VK_FORMAT_X8_D24_UNORM_PACK32:
    case VK_FORMAT_D32_SFLOAT: result.bits.depth_bit = true; break;
    // Stencil
    case VK_FORMAT_S8_UINT: result.bits.stencil_bit = true; break;
    // Depth/stencil
    case VK_FORMAT_D16_UNORM_S8_UINT:
    case VK_FORMAT_D24_UNORM_S8_UINT:
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
      result.bits.depth_bit   = true;
      result.bits.stencil_bit = true;
      break;
    // Assume everything else is Color
    default: result.bits.color_bit = true; break;
  }
  return result;
}

VkComponentMapping ComponentMapping(
  VkComponentSwizzle r,
  VkComponentSwizzle g,
  VkComponentSwizzle b,
  VkComponentSwizzle a)
{
  VkComponentMapping mapping = {r, g, b, a};
  return mapping;
}

VkComponentMapping ComponentMappingRGBA()
{
  VkComponentMapping mapping = {
    VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A};
  return mapping;
}

VkImageSubresourceRange ImageSubresourceRange(
  VkImageAspectFlags aspect_mask,
  uint32_t           base_mip_level,
  uint32_t           mip_level_count,
  uint32_t           base_array_layer,
  uint32_t           array_layer_count)
{
  VkImageSubresourceRange subresource_range = {};
  subresource_range.aspectMask              = aspect_mask;
  subresource_range.baseMipLevel            = base_mip_level;
  subresource_range.levelCount              = mip_level_count;
  subresource_range.baseArrayLayer          = base_array_layer;
  subresource_range.layerCount              = array_layer_count;
  return subresource_range;
}

vkex::ComponentType FormatComponentType(VkFormat format)
{
  switch (format) {
    default: break;

    // 8-bit signed normalized
    case VK_FORMAT_R8_SNORM: return ComponentType::INT8; break;
    case VK_FORMAT_R8G8_SNORM: return ComponentType::INT8; break;
    case VK_FORMAT_R8G8B8_SNORM: return ComponentType::INT8; break;
    case VK_FORMAT_R8G8B8A8_SNORM: return ComponentType::INT8; break;
    case VK_FORMAT_B8G8R8_SNORM: return ComponentType::INT8; break;
    case VK_FORMAT_B8G8R8A8_SNORM: return ComponentType::INT8; break;

    // 8-bit unsigned normalized
    case VK_FORMAT_R8_UNORM: return ComponentType::UINT8; break;
    case VK_FORMAT_R8G8_UNORM: return ComponentType::UINT8; break;
    case VK_FORMAT_R8G8B8_UNORM: return ComponentType::UINT8; break;
    case VK_FORMAT_R8G8B8A8_UNORM: return ComponentType::UINT8; break;
    case VK_FORMAT_B8G8R8_UNORM: return ComponentType::UINT8; break;
    case VK_FORMAT_B8G8R8A8_UNORM: return ComponentType::UINT8; break;

    // 8-bit signed integer
    case VK_FORMAT_R8_SINT: return ComponentType::INT8; break;
    case VK_FORMAT_R8G8_SINT: return ComponentType::INT8; break;
    case VK_FORMAT_R8G8B8_SINT: return ComponentType::INT8; break;
    case VK_FORMAT_R8G8B8A8_SINT: return ComponentType::INT8; break;
    case VK_FORMAT_B8G8R8_SINT: return ComponentType::INT8; break;
    case VK_FORMAT_B8G8R8A8_SINT: return ComponentType::INT8; break;

    // 8-bit unsigned integer
    case VK_FORMAT_R8_UINT: return ComponentType::UINT8; break;
    case VK_FORMAT_R8G8_UINT: return ComponentType::UINT8; break;
    case VK_FORMAT_R8G8B8_UINT: return ComponentType::UINT8; break;
    case VK_FORMAT_R8G8B8A8_UINT: return ComponentType::UINT8; break;
    case VK_FORMAT_B8G8R8_UINT: return ComponentType::UINT8; break;
    case VK_FORMAT_B8G8R8A8_UINT: return ComponentType::UINT8; break;

    // 16-bit signed normalized
    case VK_FORMAT_R16_SNORM: return ComponentType::INT16; break;
    case VK_FORMAT_R16G16_SNORM: return ComponentType::INT16; break;
    case VK_FORMAT_R16G16B16_SNORM: return ComponentType::INT16; break;
    case VK_FORMAT_R16G16B16A16_SNORM: return ComponentType::INT16; break;

    // 16-bit unsigned normalized
    case VK_FORMAT_R16_UNORM: return ComponentType::UINT16; break;
    case VK_FORMAT_R16G16_UNORM: return ComponentType::UINT16; break;
    case VK_FORMAT_R16G16B16_UNORM: return ComponentType::UINT16; break;
    case VK_FORMAT_R16G16B16A16_UNORM: return ComponentType::UINT16; break;

    // 16-bit signed integer
    case VK_FORMAT_R16_SINT: return ComponentType::INT16; break;
    case VK_FORMAT_R16G16_SINT: return ComponentType::INT16; break;
    case VK_FORMAT_R16G16B16_SINT: return ComponentType::INT16; break;
    case VK_FORMAT_R16G16B16A16_SINT: return ComponentType::INT16; break;

    // 16-bit unsigned integer
    case VK_FORMAT_R16_UINT: return ComponentType::UINT16; break;
    case VK_FORMAT_R16G16_UINT: return ComponentType::UINT16; break;
    case VK_FORMAT_R16G16B16_UINT: return ComponentType::UINT16; break;
    case VK_FORMAT_R16G16B16A16_UINT: return ComponentType::UINT16; break;

    // 16-bit float
    case VK_FORMAT_R16_SFLOAT: return ComponentType::FLOAT16; break;
    case VK_FORMAT_R16G16_SFLOAT: return ComponentType::FLOAT16; break;
    case VK_FORMAT_R16G16B16_SFLOAT: return ComponentType::FLOAT16; break;
    case VK_FORMAT_R16G16B16A16_SFLOAT: return ComponentType::FLOAT16; break;

    // 32-bit signed integer
    case VK_FORMAT_R32_SINT: return ComponentType::INT32; break;
    case VK_FORMAT_R32G32_SINT: return ComponentType::INT32; break;
    case VK_FORMAT_R32G32B32_SINT: return ComponentType::INT32; break;
    case VK_FORMAT_R32G32B32A32_SINT: return ComponentType::INT32; break;

    // 32-bit unsigned integer
    case VK_FORMAT_R32_UINT: return ComponentType::UINT32; break;
    case VK_FORMAT_R32G32_UINT: return ComponentType::UINT32; break;
    case VK_FORMAT_R32G32B32_UINT: return ComponentType::UINT32; break;
    case VK_FORMAT_R32G32B32A32_UINT: return ComponentType::UINT32; break;

    // 32-bit float
    case VK_FORMAT_R32_SFLOAT: return ComponentType::FLOAT32; break;
    case VK_FORMAT_R32G32_SFLOAT: return ComponentType::FLOAT32; break;
    case VK_FORMAT_R32G32B32_SFLOAT: return ComponentType::FLOAT32; break;
    case VK_FORMAT_R32G32B32A32_SFLOAT: return ComponentType::FLOAT32; break;

    // 8-bit unsigned integer stencil
    case VK_FORMAT_S8_UINT: return ComponentType::UINT8; break;

    // 16-bit unsigned normalized depth
    case VK_FORMAT_D16_UNORM: return ComponentType::UINT16; break;

    // 32-bit float depth
    case VK_FORMAT_D32_SFLOAT: return ComponentType::FLOAT32; break;

    // Depth/stencil combinations
    case VK_FORMAT_D16_UNORM_S8_UINT: return ComponentType::PACKED; break;
    case VK_FORMAT_D24_UNORM_S8_UINT: return ComponentType::PACKED; break;
    case VK_FORMAT_D32_SFLOAT_S8_UINT: return ComponentType::PACKED; break;

    // SRGB
    case VK_FORMAT_R8_SRGB: return ComponentType::UINT8; break;
    case VK_FORMAT_R8G8_SRGB: return ComponentType::UINT8; break;
    case VK_FORMAT_R8G8B8_SRGB: return ComponentType::UINT8; break;
    case VK_FORMAT_R8G8B8A8_SRGB: return ComponentType::UINT8; break;
    case VK_FORMAT_B8G8R8_SRGB: return ComponentType::UINT8; break;
    case VK_FORMAT_B8G8R8A8_SRGB: return ComponentType::UINT8; break;
  }
  return ComponentType::UNDEFINED;
}

uint32_t FormatComponentSize(VkFormat format)
{
  vkex::ComponentType component_type = FormatComponentType(format);
  switch (component_type) {
    default: break;
    case vkex::ComponentType::INT8: return 1; break;
    case vkex::ComponentType::UINT8: return 1; break;
    case vkex::ComponentType::INT16: return 2; break;
    case vkex::ComponentType::UINT16: return 2; break;
    case vkex::ComponentType::INT32: return 4; break;
    case vkex::ComponentType::UINT32: return 4; break;
    case vkex::ComponentType::INT64: return 8; break;
    case vkex::ComponentType::UINT64: return 8; break;
    case vkex::ComponentType::FLOAT16: return 2; break;
    case vkex::ComponentType::FLOAT32: return 4; break;
    case vkex::ComponentType::FLOAT64: return 8; break;
    case vkex::ComponentType::PACKED: return 0; break;
    case vkex::ComponentType::COMPRESSED: return 0; break;
  }
  return 0;
}

uint32_t FormatComponentCount(VkFormat format)
{
  switch (format) {
    default: break;

    // 8-bit signed normalized
    case VK_FORMAT_R8_SNORM: return 1; break;
    case VK_FORMAT_R8G8_SNORM: return 2; break;
    case VK_FORMAT_R8G8B8_SNORM: return 3; break;
    case VK_FORMAT_R8G8B8A8_SNORM: return 4; break;
    case VK_FORMAT_B8G8R8_SNORM: return 3; break;
    case VK_FORMAT_B8G8R8A8_SNORM: return 4; break;

    // 8-bit unsigned normalized
    case VK_FORMAT_R8_UNORM: return 1; break;
    case VK_FORMAT_R8G8_UNORM: return 2; break;
    case VK_FORMAT_R8G8B8_UNORM: return 3; break;
    case VK_FORMAT_R8G8B8A8_UNORM: return 4; break;
    case VK_FORMAT_B8G8R8_UNORM: return 3; break;
    case VK_FORMAT_B8G8R8A8_UNORM: return 4; break;

    // 8-bit signed integer
    case VK_FORMAT_R8_SINT: return 1; break;
    case VK_FORMAT_R8G8_SINT: return 2; break;
    case VK_FORMAT_R8G8B8_SINT: return 3; break;
    case VK_FORMAT_R8G8B8A8_SINT: return 4; break;
    case VK_FORMAT_B8G8R8_SINT: return 3; break;
    case VK_FORMAT_B8G8R8A8_SINT: return 4; break;

    // 8-bit unsigned integer
    case VK_FORMAT_R8_UINT: return 1; break;
    case VK_FORMAT_R8G8_UINT: return 2; break;
    case VK_FORMAT_R8G8B8_UINT: return 3; break;
    case VK_FORMAT_R8G8B8A8_UINT: return 4; break;
    case VK_FORMAT_B8G8R8_UINT: return 3; break;
    case VK_FORMAT_B8G8R8A8_UINT: return 4; break;

    // 16-bit signed normalized
    case VK_FORMAT_R16_SNORM: return 1; break;
    case VK_FORMAT_R16G16_SNORM: return 2; break;
    case VK_FORMAT_R16G16B16_SNORM: return 3; break;
    case VK_FORMAT_R16G16B16A16_SNORM: return 4; break;

    // 16-bit unsigned normalized
    case VK_FORMAT_R16_UNORM: return 1; break;
    case VK_FORMAT_R16G16_UNORM: return 2; break;
    case VK_FORMAT_R16G16B16_UNORM: return 3; break;
    case VK_FORMAT_R16G16B16A16_UNORM: return 4; break;

    // 16-bit signed integer
    case VK_FORMAT_R16_SINT: return 1; break;
    case VK_FORMAT_R16G16_SINT: return 2; break;
    case VK_FORMAT_R16G16B16_SINT: return 3; break;
    case VK_FORMAT_R16G16B16A16_SINT: return 4; break;

    // 16-bit unsigned integer
    case VK_FORMAT_R16_UINT: return 1; break;
    case VK_FORMAT_R16G16_UINT: return 2; break;
    case VK_FORMAT_R16G16B16_UINT: return 3; break;
    case VK_FORMAT_R16G16B16A16_UINT: return 4; break;

    // 16-bit float
    case VK_FORMAT_R16_SFLOAT: return 1; break;
    case VK_FORMAT_R16G16_SFLOAT: return 2; break;
    case VK_FORMAT_R16G16B16_SFLOAT: return 3; break;
    case VK_FORMAT_R16G16B16A16_SFLOAT: return 4; break;

    // 32-bit signed integer
    case VK_FORMAT_R32_SINT: return 1; break;
    case VK_FORMAT_R32G32_SINT: return 2; break;
    case VK_FORMAT_R32G32B32_SINT: return 3; break;
    case VK_FORMAT_R32G32B32A32_SINT: return 4; break;

    // 32-bit unsigned integer
    case VK_FORMAT_R32_UINT: return 1; break;
    case VK_FORMAT_R32G32_UINT: return 2; break;
    case VK_FORMAT_R32G32B32_UINT: return 3; break;
    case VK_FORMAT_R32G32B32A32_UINT: return 4; break;

    // 32-bit float
    case VK_FORMAT_R32_SFLOAT: return 1; break;
    case VK_FORMAT_R32G32_SFLOAT: return 2; break;
    case VK_FORMAT_R32G32B32_SFLOAT: return 3; break;
    case VK_FORMAT_R32G32B32A32_SFLOAT: return 4; break;

    // 8-bit unsigned integer stencil
    case VK_FORMAT_S8_UINT: return 1; break;

    // 16-bit unsigned normalized depth
    case VK_FORMAT_D16_UNORM: return 1; break;

    // 32-bit float depth
    case VK_FORMAT_D32_SFLOAT: return 1; break;

    // Depth/stencil combinations
    case VK_FORMAT_D16_UNORM_S8_UINT: return 2; break;
    case VK_FORMAT_D24_UNORM_S8_UINT: return 2; break;
    case VK_FORMAT_D32_SFLOAT_S8_UINT: return 2; break;

    // SRGB
    case VK_FORMAT_R8_SRGB: return 1; break;
    case VK_FORMAT_R8G8_SRGB: return 2; break;
    case VK_FORMAT_R8G8B8_SRGB: return 3; break;
    case VK_FORMAT_R8G8B8A8_SRGB: return 4; break;
    case VK_FORMAT_B8G8R8_SRGB: return 3; break;
    case VK_FORMAT_B8G8R8A8_SRGB: return 4; break;
  }
  return 0;
}

uint32_t FormatSize(VkFormat format)
{
  uint32_t component_size  = FormatComponentSize(format);
  uint32_t component_count = FormatComponentCount(format);
  uint32_t size            = component_size * component_count;
  return size;
}

// =================================================================================================
// Image Layout Transition Functions
// =================================================================================================
vkex::Result TransitionImageLayout(
  vkex::Queue          queue,
  vkex::Image          image,
  VkImageLayout        old_layout,
  VkImageLayout        new_layout,
  VkPipelineStageFlags new_pipeline_stage)
{

  VKEX_ASSERT(queue != nullptr);
  if (queue == nullptr) {
    return vkex::Result::ErrorUnexpectedNullPointer;
  }

  VKEX_ASSERT(image != nullptr);
  if (queue == nullptr) {
    return vkex::Result::ErrorUnexpectedNullPointer;
  }

  // Grab device
  vkex::Device device = queue->GetDevice();
  // Create command pool
  vkex::CommandPool command_pool = nullptr;
  {
    vkex::CommandPoolCreateInfo create_info = {};
    create_info.flags.bits.transient        = true;
    vkex::Result vkex_result                = device->CreateCommandPool(create_info, &command_pool);
    if (vkex_result != vkex::Result::Success) {
      return vkex_result;
    }
  }
  // Allocate command buffer
  vkex::CommandBuffer command_buffer = nullptr;
  {
    vkex::CommandBufferAllocateInfo allocate_info = {};
    allocate_info.command_buffer_count            = 1;
    vkex::Result vkex_result = command_pool->AllocateCommandBuffer(allocate_info, &command_buffer);
    if (vkex_result != vkex::Result::Success) {
      return vkex_result;
    }
  }

  // Build command buffer
  {
    VkCommandBufferBeginInfo begin_info = {};
    begin_info.flags                    = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    // Begin command buffer
    vkex::Result vkex_result = command_buffer->Begin();
    VKEX_ASSERT(vkex_result == vkex::Result::Success);
    // Commands
    {
      command_buffer->CmdTransitionImageLayout(*image, image->GetAspectFlags(), 0, image->GetMipLevels(), 0, image->GetArrayLayers(), old_layout, new_layout, new_pipeline_stage);
    }
    // End command buffer
    vkex_result = command_buffer->End();
    VKEX_ASSERT(vkex_result == vkex::Result::Success);
  }

  // Submit info
  VkCommandBuffer vk_command_buffer = command_buffer->GetVkObject();
  VkSubmitInfo    vk_submit_info    = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
  vk_submit_info.commandBufferCount = 1;
  vk_submit_info.pCommandBuffers    = &vk_command_buffer;
  // Queue submit
  VkResult vk_result = vkex::QueueSubmit(queue->GetVkObject(), 1, &vk_submit_info, VK_NULL_HANDLE);
  VKEX_ASSERT(vk_result == VK_SUCCESS);

  // Wait for queue to be done
  vk_result = queue->WaitIdle();
  VKEX_ASSERT(vk_result == VK_SUCCESS);

  // Free command buffers
  command_pool->FreeCommandBuffer(command_buffer);
  // Destroy command pool
  vkex::Result vkex_result = device->DestroyCommandPool(command_pool);
  if (vkex_result != vkex::Result::Success) {
    return vkex_result;
  }

  return vkex::Result::Success;
}

vkex::Result TransitionImageLayout(
  vkex::Queue           queue,
  vkex::Texture         texture,
  VkImageLayout         old_layout,
  VkImageLayout         new_layout,
  VkPipelineStageFlags  new_shader_stage)
{
  VKEX_ASSERT(queue != nullptr);
  if (queue == nullptr) {
    return vkex::Result::ErrorUnexpectedNullPointer;
  }

  VKEX_ASSERT(texture != nullptr);
  if (queue == nullptr) {
    return vkex::Result::ErrorUnexpectedNullPointer;
  }

  vkex::Image image = texture->GetImage();
  vkex::Result vkex_result = TransitionImageLayout(queue, image, old_layout, new_layout, new_shader_stage);
  return vkex_result;
}

// =================================================================================================
// Buffer/Image Copy Functions
// =================================================================================================
vkex::Result CopyResource(
  vkex::Queue         queue,
  vkex::Buffer        src,
  vkex::Buffer        dst,
  uint32_t            region_count,
  const VkBufferCopy* p_regions)
{
  // Grab device
  vkex::Device device = queue->GetDevice();
  // Create command pool
  vkex::CommandPool command_pool = nullptr;
  {
    vkex::CommandPoolCreateInfo create_info = {};
    create_info.flags.bits.transient        = true;
    vkex::Result vkex_result                = device->CreateCommandPool(create_info, &command_pool);
    if (vkex_result != vkex::Result::Success) {
      return vkex_result;
    }
  }

  // Allocate command buffer
  vkex::CommandBuffer command_buffer = nullptr;
  {
    vkex::CommandBufferAllocateInfo allocate_info = {};
    allocate_info.command_buffer_count            = 1;
    vkex::Result vkex_result = command_pool->AllocateCommandBuffer(allocate_info, &command_buffer);
    VKEX_ASSERT(vkex_result == vkex::Result::Success);
    if (vkex_result != vkex::Result::Success) {
      return vkex_result;
    }
  }

  // Build command buffer
  {
    VkCommandBufferBeginInfo begin_info = {};
    begin_info.flags                    = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    // Begin command buffer
    vkex::Result vkex_result = command_buffer->Begin();
    VKEX_ASSERT(vkex_result == vkex::Result::Success);
    if (vkex_result != vkex::Result::Success) {
      return vkex_result;
    }
    // Commands
    {
      command_buffer->CmdCopyBuffer(
        src->GetVkObject(),
        dst->GetVkObject(),
        region_count,
        p_regions);
    }
    // End command buffer
    vkex_result = command_buffer->End();
    VKEX_ASSERT(vkex_result == vkex::Result::Success);
    if (vkex_result != vkex::Result::Success) {
      return vkex_result;
    }
  }

  // Submit info
  VkCommandBuffer vk_command_buffer = command_buffer->GetVkObject();
  VkSubmitInfo    vk_submit_info    = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
  vk_submit_info.commandBufferCount = 1;
  vk_submit_info.pCommandBuffers    = &vk_command_buffer;
  // Queue submit
  VkResult vk_result = vkex::QueueSubmit(queue->GetVkObject(), 1, &vk_submit_info, VK_NULL_HANDLE);
  VKEX_ASSERT(vk_result == VK_SUCCESS);
  if (vk_result != VK_SUCCESS) {
    return vkex::Result(vk_result);
  }

  // Wait for queue to be done
  vk_result = queue->WaitIdle();
  VKEX_ASSERT(vk_result == VK_SUCCESS);
  if (vk_result != VK_SUCCESS) {
    return vkex::Result(vk_result);
  }

  // Free command buffers
  command_pool->FreeCommandBuffer(command_buffer);
  // Destroy command pool
  vkex::Result vkex_result = device->DestroyCommandPool(command_pool);
  if (vkex_result != vkex::Result::Success) {
    return vkex_result;
  }

  return vkex::Result::Success;
}

vkex::Result CopyResource(
  vkex::Queue         queue,
  VkDeviceSize        src_size,
  const void*         p_src_data,
  vkex::Buffer        dst)
{
  // Grab device
  vkex::Device device = queue->GetDevice();
  // Create temporary buffer
  vkex::Buffer src_buffer = nullptr;
  {
    vkex::BufferCreateInfo create_info        = {};
    create_info.size                          = src_size;
    create_info.usage_flags.bits.transfer_src = true;
    create_info.committed                     = true;
    create_info.host_visible                  = true;
    create_info.device_local                  = false;
    vkex::Result result = device->CreateBuffer(create_info, &src_buffer);
    if (!result) {
      return result;
    }
  }
  // Copy source data
  {
    vkex::Result result = src_buffer->Copy(src_size, p_src_data);
    if (!result) {
      // Destroy temporary buffer
      if (src_buffer != nullptr) {
        vkex::Result result = device->DestroyBuffer(src_buffer);
        if (!result) {
          // @TODO: Warn!
        }
      }
      return result;
    }
  }
  // Copy buffer
  {
    VkBufferCopy region = {};
     region.srcOffset = 0;
     region.dstOffset = 0;
     region.size      = src_size;

    vkex::Result result = CopyResource(
      queue,
      src_buffer,
      dst,
      1,
      &region);
  }
  // Destroy temporary buffer
  if (src_buffer != nullptr) {
    vkex::Result result = device->DestroyBuffer(src_buffer);
    if (!result) {
      // @TODO: Warn!
    }
  }
  return vkex::Result::Success;
}

vkex::Result CopyResource(
  vkex::Queue        queue,
  vkex::Image        src,
  vkex::Image        dst,
  uint32_t           region_count,
  const VkImageCopy* p_regions)
{
  // Grab device
  vkex::Device device = queue->GetDevice();
  // Create command pool
  vkex::CommandPool command_pool = nullptr;
  {
    vkex::CommandPoolCreateInfo create_info = {};
    create_info.flags.bits.transient        = true;
    vkex::Result vkex_result                = device->CreateCommandPool(create_info, &command_pool);
    if (vkex_result != vkex::Result::Success) {
      return vkex_result;
    }
  }
  // Allocate command buffer
  vkex::CommandBuffer command_buffer = nullptr;
  {
    vkex::CommandBufferAllocateInfo allocate_info = {};
    allocate_info.command_buffer_count            = 1;
    vkex::Result vkex_result = command_pool->AllocateCommandBuffer(allocate_info, &command_buffer);
    if (vkex_result != vkex::Result::Success) {
      return vkex_result;
    }
  }

  // Build command buffer
  {
    VkCommandBufferBeginInfo begin_info = {};
    begin_info.flags                    = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    // Begin command buffer
    vkex::Result vkex_result = command_buffer->Begin();
    VKEX_ASSERT(vkex_result == vkex::Result::Success);
    // Commands
    {
      command_buffer->CmdCopyImage(
        src->GetVkObject(),
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        dst->GetVkObject(),
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        region_count,
        p_regions);
    }
    // End command buffer
    vkex_result = command_buffer->End();
    VKEX_ASSERT(vkex_result == vkex::Result::Success);
  }

  // Submit info
  VkCommandBuffer vk_command_buffer = command_buffer->GetVkObject();
  VkSubmitInfo    vk_submit_info    = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
  vk_submit_info.commandBufferCount = 1;
  vk_submit_info.pCommandBuffers    = &vk_command_buffer;
  // Queue submit
  VkResult vk_result = vkex::QueueSubmit(queue->GetVkObject(), 1, &vk_submit_info, VK_NULL_HANDLE);
  VKEX_ASSERT(vk_result == VK_SUCCESS);

  // Wait for queue to be done
  vk_result = queue->WaitIdle();
  VKEX_ASSERT(vk_result == VK_SUCCESS);

  // Free command buffers
  command_pool->FreeCommandBuffer(command_buffer);
  // Destroy command pool
  vkex::Result vkex_result = device->DestroyCommandPool(command_pool);
  if (vkex_result != vkex::Result::Success) {
    return vkex_result;
  }

  return vkex::Result::Success;
}

vkex::Result CopyResource(
  vkex::Queue              queue,
  vkex::Buffer             src,
  vkex::Image              dst,
  uint32_t                 region_count,
  const VkBufferImageCopy* p_regions)
{
  // Grab device
  vkex::Device device = queue->GetDevice();
  // Create command pool
  vkex::CommandPool command_pool = nullptr;
  {
    vkex::CommandPoolCreateInfo create_info = {};
    create_info.flags.bits.transient        = true;
    vkex::Result vkex_result = device->CreateCommandPool(create_info, &command_pool);
    VKEX_ASSERT(vkex_result == vkex::Result::Success);
    if (!vkex_result) {
      return vkex_result;
    }
  }
  // Allocate command buffer
  vkex::CommandBuffer command_buffer = nullptr;
  {
    vkex::CommandBufferAllocateInfo allocate_info = {};
    allocate_info.command_buffer_count            = 1;
    vkex::Result vkex_result = command_pool->AllocateCommandBuffer(allocate_info, &command_buffer);
    VKEX_ASSERT(vkex_result == vkex::Result::Success);
    if (!vkex_result) {
      return vkex_result;
    }
  }

  // Build command buffer
  {
    VkCommandBufferBeginInfo begin_info = {};
    begin_info.flags                    = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    // Begin command buffer
    vkex::Result vkex_result = command_buffer->Begin();
    VKEX_ASSERT(vkex_result == vkex::Result::Success);
    if (!vkex_result) {
      return vkex_result;
    }
    // Commands
    {
      command_buffer->CmdCopyBufferToImage(
        src->GetVkObject(),
        dst->GetVkObject(),
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        region_count,
        p_regions);
    }
    // End command buffer
    vkex_result = command_buffer->End();
    VKEX_ASSERT(vkex_result == vkex::Result::Success);
    if (!vkex_result) {
      return vkex_result;
    }
  }

  // Submit info
  VkCommandBuffer vk_command_buffer = command_buffer->GetVkObject();
  VkSubmitInfo    vk_submit_info    = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
  vk_submit_info.commandBufferCount = 1;
  vk_submit_info.pCommandBuffers    = &vk_command_buffer;
  // Queue submit
  VkResult vk_result = vkex::QueueSubmit(queue->GetVkObject(), 1, &vk_submit_info, VK_NULL_HANDLE);
  VKEX_ASSERT(vk_result == VK_SUCCESS);
  if (vk_result != VK_SUCCESS) {
    return vkex::Result(vk_result);
  }

  // Wait for queue to be done
  vk_result = queue->WaitIdle();
  VKEX_ASSERT(vk_result == VK_SUCCESS);
  if (vk_result != VK_SUCCESS) {
    return vkex::Result(vk_result);
  }

  // Free command buffers
  command_pool->FreeCommandBuffer(command_buffer);
  // Destroy command pool
  vkex::Result vkex_result = device->DestroyCommandPool(command_pool);
  VKEX_ASSERT(vkex_result == vkex::Result::Success);
  if (vkex_result != vkex::Result::Success) {
    return vkex_result;
  }

  return vkex::Result::Success;
}

} // namespace vkex