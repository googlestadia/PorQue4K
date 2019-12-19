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

#ifndef __VKEX_CAST_H__
#define __VKEX_CAST_H__

#include <vkex/Descriptor.h>
#include <vkex/VulkanUtil.h>

namespace vkex {

// =================================================================================================
// ToVulkan functions
// =================================================================================================
template <typename VulkanObjectT, typename VkexObjectT>
std::vector<VulkanObjectT> ToVulkan(const std::vector<VkexObjectT>& vkex_objects)
{
  std::vector<VulkanObjectT> vulkan_objects;
  for (auto& vkex_object : vkex_objects) {
    VKEX_ASSERT_MSG(vkex_object != nullptr, "VKEX source object is null!");
    VulkanObjectT vulkan_object = vkex_object->GetVkObject();
    vulkan_objects.push_back(vulkan_object);
  }
  return vulkan_objects;
}

VkImageType                               ToVulkan(vkex::ImageType value);
VkImageViewType                           ToVulkan(vkex::ImageViewType value);

std::vector<VkDescriptorSetLayoutBinding> ToVulkan(const std::vector<vkex::ShaderInterface::Binding>& vkex_objects);
VkDescriptorSetLayout                     ToVulkan(const vkex::DescriptorSetLayout& vkex_object);
std::vector<VkDescriptorSetLayout>        ToVulkan(const std::vector<vkex::DescriptorSetLayout>& vkex_objects);

// =================================================================================================
// ToVkex functions
// =================================================================================================
vkex::ImageType                           ToVkex(VkImageType value);
vkex::ImageViewType                       ToVkex(VkImageViewType value);

vkex::DescriptorSetLayoutCreateInfo               ToVkexCreateInfo(const vkex::ShaderInterface::Set& set);
std::vector<vkex::DescriptorSetLayoutCreateInfo>  ToVkexCreateInfo(const std::vector<vkex::ShaderInterface::Set>& sets);

// =================================================================================================
// DeriveVkex functions
// =================================================================================================
vkex::ImageViewType                       DeriveVkex(vkex::ImageType value, uint32_t array_layers);

} // namespace vkex

#endif // __VKEX_CAST_H__