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

#ifndef __VKEX_LOADER_HELPER_H__
#define __VKEX_LOADER_HELPER_H__

#include "VkexLoader.h"

#include <vector>

namespace vkex {

// =================================================================================================
// VKEX Vulkan API convenience functions
// =================================================================================================
VkResult  EnumerateInstanceExtensionPropertiesVKEX(const char* pLayerName, std::vector<VkExtensionProperties>* pProperties);
VkResult  EnumerateInstanceLayerPropertiesVKEX(std::vector<VkLayerProperties>* pProperties);
VkResult  EnumeratePhysicalDevicesVKEX(VkInstance instance, std::vector<VkPhysicalDevice>* pPhysicalDevices);
VkResult  EnumerateDeviceExtensionPropertiesVKEX(VkPhysicalDevice physicalDevice, const char* pLayerName, std::vector<VkExtensionProperties>* pProperties);

VkResult  ResetFenceVKEX(VkDevice device, const VkFence fence);
VkResult  WaitForFenceVKEX(VkDevice device, const VkFence fence, VkBool32 waitAll, uint64_t timeout);

VkResult  GetSwapchainImagesVKEX(VkDevice device, VkSwapchainKHR swapchain, std::vector<VkImage>* pSwapchainImages);

VkResult  GetPhysicalDeviceSurfaceFormatsVKEX(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, std::vector<VkSurfaceFormatKHR>* pSurfaceFormats);
VkResult  GetPhysicalDeviceSurfacePresentModesVKEX(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, std::vector<VkPresentModeKHR>* pPresentModes);

void      GetPhysicalDeviceQueueFamilyProperties2VKEX(VkPhysicalDevice physicalDevice, std::vector<VkQueueFamilyProperties2>* pQueueFamilyProperties);

VkResult  GetPhysicalDeviceSurfaceCapabilities2VKEX_SAFE(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo, VkSurfaceCapabilities2KHR* pSurfaceCapabilities);
VkResult  GetPhysicalDeviceSurfaceFormats2VKEX(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo, std::vector<VkSurfaceFormat2KHR>* pSurfaceFormats);
VkResult  GetPhysicalDeviceSurfaceFormats2VKEX_SAFE(VkPhysicalDevice physicalDevice, const VkPhysicalDeviceSurfaceInfo2KHR* pSurfaceInfo, std::vector<VkSurfaceFormat2KHR>* pSurfaceFormats);

} // namespace vkex

#endif // __VKEX_LOADER_HELPER_H__