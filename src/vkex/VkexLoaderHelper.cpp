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

#include "VkexLoaderHelper.h"

namespace vkex {

// =================================================================================================
// VKEX Vulkan API convenience functions
// =================================================================================================
VkResult EnumerateInstanceExtensionPropertiesVKEX(const char* pLayerName, std::vector<VkExtensionProperties>* pProperties)
{
  uint32_t count = 0;
  VkResult vk_result = vkex::EnumerateInstanceExtensionProperties(
    pLayerName,
    &count,
    nullptr);
  if (vk_result != VK_SUCCESS) {
    return vk_result;
  }

  pProperties->resize(count);
  vk_result = vkex::EnumerateInstanceExtensionProperties(
    pLayerName,
    &count,
    pProperties->data());

  return vk_result;
}

VkResult EnumerateInstanceLayerPropertiesVKEX(std::vector<VkLayerProperties>* pProperties)
{
  uint32_t count = 0;
  VkResult vk_result = vkex::EnumerateInstanceLayerProperties(
    &count,
    nullptr);
  if (vk_result != VK_SUCCESS) {
    return vk_result;
  }

  pProperties->resize(count);
  vk_result = vkex::EnumerateInstanceLayerProperties(
    &count,
    pProperties->data());

  return vk_result;
}

VkResult EnumeratePhysicalDevicesVKEX(VkInstance instance, std::vector<VkPhysicalDevice>* pPhysicalDevices)
{
  uint32_t count = 0;
  VkResult vk_result = vkex::EnumeratePhysicalDevices(
    instance,
    &count,
    nullptr);
  if (vk_result != VK_SUCCESS) {
    return vk_result;
  }

  pPhysicalDevices->resize(count);
  vk_result = vkex::EnumeratePhysicalDevices(
    instance,
    &count,
    pPhysicalDevices->data());

  return vk_result;
}

VkResult EnumerateDeviceExtensionPropertiesVKEX(VkPhysicalDevice physicalDevice, const char* pLayerName, std::vector<VkExtensionProperties>* pProperties)
{
  uint32_t count = 0;
  VkResult vk_result = vkex::EnumerateDeviceExtensionProperties(
    physicalDevice,
    pLayerName,
    &count,
    nullptr);
  if (vk_result != VK_SUCCESS) {
    return vk_result;
  }

  pProperties->resize(count);
  vk_result = vkex::EnumerateDeviceExtensionProperties(
    physicalDevice,
    pLayerName,
    &count,
    pProperties->data());

  return vk_result;
}

VkResult ResetFenceVKEX(VkDevice device, const VkFence fence)
{
  VkResult vk_result = ResetFences(
    device,
    1,
    &fence);
  return vk_result;
}

VkResult WaitForFenceVKEX(VkDevice device, const VkFence fence, VkBool32 waitAll, uint64_t timeout)
{
  VkResult vk_result = WaitForFences(
    device,
    1,
    &fence,
    waitAll,
    timeout);
  return vk_result;
}

VkResult GetSwapchainImagesVKEX(VkDevice device, VkSwapchainKHR swapchain, std::vector<VkImage>* pSwapchainImages)
{
  uint32_t count = 0;
  VkResult vk_result = vkex::GetSwapchainImagesKHR(
    device,
    swapchain,
    &count,
    nullptr);
  if (vk_result != VK_SUCCESS) {
    return vk_result;
  }

  pSwapchainImages->resize(count);
  vk_result = vkex::GetSwapchainImagesKHR(
    device,
    swapchain,
    &count,
    pSwapchainImages->data());
  if (vk_result != VK_SUCCESS) {
    return vk_result;
  }
  
  return vk_result;
}

VkResult GetPhysicalDeviceSurfaceFormatsVKEX(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, std::vector<VkSurfaceFormatKHR>* pSurfaceFormats)
{
  uint32_t count = 0;
  VkResult vk_result = vkex::GetPhysicalDeviceSurfaceFormatsKHR(
    physicalDevice,
    surface,
    &count,
    nullptr);
  if (vk_result != VK_SUCCESS) {
    return vk_result;
  }

  pSurfaceFormats->resize(count);
  vk_result = vkex::GetPhysicalDeviceSurfaceFormatsKHR(
    physicalDevice,
    surface,
    &count,
    pSurfaceFormats->data());
  if (vk_result != VK_SUCCESS) {
    return vk_result;
  }
  
  return vk_result;
}

VkResult GetPhysicalDeviceSurfacePresentModesVKEX(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, std::vector<VkPresentModeKHR>* pPresentModes)
{
  uint32_t count = 0;
  VkResult vk_result = vkex::GetPhysicalDeviceSurfacePresentModesKHR(
    physicalDevice,
    surface,
    &count,
    nullptr);
  if (vk_result != VK_SUCCESS) {
    return vk_result;
  }

  pPresentModes->resize(count);
  vk_result = vkex::GetPhysicalDeviceSurfacePresentModesKHR(
    physicalDevice,
    surface,
    &count,
    pPresentModes->data());
  if (vk_result != VK_SUCCESS) {
    return vk_result;
  }
  
  return vk_result;
}

VkResult GetPhysicalDeviceSurfaceCapabilities2VKEX_SAFE(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo, VkSurfaceCapabilities2KHR* pSurfaceCapabilities)
{
  bool has_ptr_fn = (vkex::GetPhysicalDeviceSurfaceCapabilities2KHR != nullptr);
  if (has_ptr_fn) { 
    VkResult vk_result = vkex::GetPhysicalDeviceSurfaceCapabilities2KHR(
      physicalDevice,
      pSurfaceInfo,
      pSurfaceCapabilities);
  }

  //VKEX_WARN_FALLBACK_USED(
  //  InstanceFunctionSet::pfnGetPhysicalDeviceSurfaceCapabilities2KHR,
  //  InstanceFunctionSet::pfnGetPhysicalDeviceSurfaceCapabilitiesKHR
  //);

  VkResult vk_result = vkex::GetPhysicalDeviceSurfaceCapabilitiesKHR(
    physicalDevice,
    pSurfaceInfo->surface,
    &pSurfaceCapabilities->surfaceCapabilities);

  return vk_result;
}

void GetPhysicalDeviceQueueFamilyProperties2VKEX(VkPhysicalDevice physicalDevice, std::vector<VkQueueFamilyProperties2>* pQueueFamilyProperties)
{
  uint32_t count = 0;
  vkex::GetPhysicalDeviceQueueFamilyProperties2(
    physicalDevice,
    &count,
    nullptr);

  pQueueFamilyProperties->resize(count);
  for (auto& properties : *pQueueFamilyProperties) {
    properties = { VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2 };
  }

  vkex::GetPhysicalDeviceQueueFamilyProperties2(
    physicalDevice,
    &count,
    pQueueFamilyProperties->data());
}

VkResult GetPhysicalDeviceSurfaceFormats2VKEX(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo, std::vector<VkSurfaceFormat2KHR>* pSurfaceFormats)
{
  uint32_t count = 0;
  VkResult vk_result = vkex::GetPhysicalDeviceSurfaceFormats2KHR(
    physicalDevice,
    pSurfaceInfo,
    &count,
    nullptr);
  if (vk_result != VK_SUCCESS) {
    return vk_result;
  }

  pSurfaceFormats->resize(count);
  for (auto& surface_format : *pSurfaceFormats) {
    surface_format = { VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR };
  }
  vk_result = vkex::GetPhysicalDeviceSurfaceFormats2KHR(
    physicalDevice,
    pSurfaceInfo,
    &count,
    pSurfaceFormats->data());

  return vk_result;
}

VkResult GetPhysicalDeviceSurfaceFormats2VKEX_SAFE(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo, std::vector<VkSurfaceFormat2KHR>* pSurfaceFormats)
{
  bool has_ptr_fn = (vkex::GetPhysicalDeviceSurfaceFormats2KHR != nullptr);
  if (has_ptr_fn) { 
    VkResult vk_result = vkex::GetPhysicalDeviceSurfaceFormats2VKEX(
      physicalDevice,
      pSurfaceInfo,
      pSurfaceFormats);
  }

  //VKEX_WARN_FALLBACK_USED(
  //  InstanceFunctionSet::pfnGetPhysicalDeviceSurfaceFormats2KHR,
  //  InstanceFunctionSet::pfnGetPhysicalDeviceSurfaceFormatsKHR
  //);

  std::vector<VkSurfaceFormatKHR> surface_formats;
  VkResult vk_result = vkex::GetPhysicalDeviceSurfaceFormatsVKEX(
    physicalDevice,
    pSurfaceInfo->surface,
    &surface_formats);
  if (vk_result == VK_SUCCESS) {
    for (auto& surface_format : surface_formats) {
      VkSurfaceFormat2KHR surface_format_2 = { VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR };
      surface_format_2.surfaceFormat = surface_format;
      pSurfaceFormats->push_back(surface_format_2);
    }
  }

  return vk_result;
}

} // namespace vkex
