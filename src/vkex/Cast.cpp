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

#include <vkex/Cast.h>

namespace vkex {

// =================================================================================================
// ToVulkan functions
// =================================================================================================
VkImageType ToVulkan(vkex::ImageType value)
{
  VkImageType vk_value = vkex::InvalidValue<VkImageType>::Value;
  switch (value) {
    default                : break;
    case vkex::ImageType1D : vk_value = VK_IMAGE_TYPE_1D; break;
    case vkex::ImageType2D : vk_value = VK_IMAGE_TYPE_2D; break;
    case vkex::ImageType3D : vk_value = VK_IMAGE_TYPE_3D; break;
  }
  return vk_value;
}

VkImageViewType ToVulkan(vkex::ImageViewType value)
{
  VkImageViewType vk_value = vkex::InvalidValue<VkImageViewType>::Value;
  switch (value) {
    default                           : break;
    case vkex::ImageViewType1D        : vk_value = VK_IMAGE_VIEW_TYPE_1D; break;
    case vkex::ImageViewType2D        : vk_value = VK_IMAGE_VIEW_TYPE_2D; break;
    case vkex::ImageViewType3D        : vk_value = VK_IMAGE_VIEW_TYPE_3D; break;
    case vkex::ImageViewTypeCube      : vk_value = VK_IMAGE_VIEW_TYPE_CUBE; break;
    case vkex::ImageViewType1DArray   : vk_value = VK_IMAGE_VIEW_TYPE_1D_ARRAY; break;
    case vkex::ImageViewType2DArray   : vk_value = VK_IMAGE_VIEW_TYPE_2D_ARRAY; break;
    case vkex::ImageViewTypeCubeArray : vk_value = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY; break;
  }
  return vk_value;
}

std::vector<VkDescriptorSetLayout> ToVulkan(const std::vector<DescriptorSetLayout>& vkex_objects)
{
  return ToVulkan<VkDescriptorSetLayout, vkex::DescriptorSetLayout>(vkex_objects);
}

std::vector<VkDescriptorSetLayoutBinding> ToVulkan(const std::vector<ShaderInterface::Binding>& vkex_objects)
{
  std::vector<VkDescriptorSetLayoutBinding> vulkan_objects;
  for (auto& vkex_object : vkex_objects) {
    VkDescriptorSetLayoutBinding vulkan_object = {};
    vulkan_object.binding         = vkex_object.binding_number;
    vulkan_object.descriptorType  = vkex_object.descriptor_type;
    vulkan_object.descriptorCount = vkex_object.descriptor_count;
    vulkan_object.stageFlags      = vkex_object.stage_flags;
    vulkan_objects.push_back(vulkan_object);
  }
  return vulkan_objects;
}

VkDescriptorSetLayout ToVulkan(const DescriptorSetLayout& vkex_object)
{
  VkDescriptorSetLayout vk_object = vkex_object->GetVkObject();
  return vk_object;
}

// =================================================================================================
// ToVkex functions
// =================================================================================================
vkex::ImageType ToVkex(VkImageType value)
{
  vkex::ImageType vkex_value = vkex::ImageTypeUndefined;
  switch (value) {
    default               : break;
    case VK_IMAGE_TYPE_1D : vkex_value = vkex::ImageType1D; break;
    case VK_IMAGE_TYPE_2D : vkex_value = vkex::ImageType2D; break;
    case VK_IMAGE_TYPE_3D : vkex_value = vkex::ImageType3D; break;
  }
  return vkex_value;
}

vkex::ImageViewType ToVkex(VkImageViewType value)
{
  vkex::ImageViewType vkex_value = vkex::ImageViewTypeUndefined;
  switch (value) {
    default                            : break;
    case VK_IMAGE_VIEW_TYPE_1D         : vkex_value = vkex::ImageViewType1D; break;
    case VK_IMAGE_VIEW_TYPE_2D         : vkex_value = vkex::ImageViewType2D; break;
    case VK_IMAGE_VIEW_TYPE_3D         : vkex_value = vkex::ImageViewType3D; break;
    case VK_IMAGE_VIEW_TYPE_CUBE       : vkex_value = vkex::ImageViewTypeCube; break;
    case VK_IMAGE_VIEW_TYPE_1D_ARRAY   : vkex_value = vkex::ImageViewType1DArray; break;
    case VK_IMAGE_VIEW_TYPE_2D_ARRAY   : vkex_value = vkex::ImageViewType2DArray; break;
    case VK_IMAGE_VIEW_TYPE_CUBE_ARRAY : vkex_value = vkex::ImageViewTypeCubeArray; break;
  }
  return vkex_value;
}

std::vector<vkex::DescriptorSetLayoutCreateInfo> ToVkexCreateInfo(const std::vector<ShaderInterface::Set>& sets)
{
  std::vector<vkex::DescriptorSetLayoutCreateInfo> create_infos;
  for (auto& set : sets) {
    vkex::DescriptorSetLayoutCreateInfo create_info = {};
    create_info.flags     = 0;
    create_info.bindings  = ToVulkan(set.bindings);
    create_infos.push_back(create_info);
  }
  return create_infos;
}

vkex::DescriptorSetLayoutCreateInfo ToVkexCreateInfo(const ShaderInterface::Set& set)
{
  vkex::DescriptorSetLayoutCreateInfo create_info = {};
  create_info.flags     = 0;
  create_info.bindings  = ToVulkan(set.bindings);  
  return create_info;
}

// =================================================================================================
// DeriveVkex functions
// =================================================================================================
vkex::ImageViewType DeriveVkex(vkex::ImageType value, uint32_t array_layers)
{
  const bool is_array = (array_layers > 1);
  vkex::ImageViewType vkex_value = vkex::ImageViewTypeUndefined;
  switch (value) {
    default                : break;
    case vkex::ImageType1D : vkex_value = is_array ? vkex::ImageViewType1DArray : vkex::ImageViewType1D; break;
    case vkex::ImageType2D : vkex_value = is_array ? vkex::ImageViewType2DArray : vkex::ImageViewType2D; break;
    case vkex::ImageType3D : vkex_value = vkex::ImageViewType3D; break;
  }
  return vkex_value;
}

} // namespace vkex