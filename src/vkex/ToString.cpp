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

#include <vkex/ToString.h>

namespace vkex {

std::string ToString(VkResult vk_result)
{
  switch (vk_result) {
    default: break;
    case VK_SUCCESS                                            : return"VK_SUCCESS"; break;
    case VK_NOT_READY                                          : return"VK_NOT_READY"; break;
    case VK_TIMEOUT                                            : return"VK_TIMEOUT"; break;
    case VK_EVENT_SET                                          : return"VK_EVENT_SET"; break;
    case VK_EVENT_RESET                                        : return"VK_EVENT_RESET"; break;
    case VK_INCOMPLETE                                         : return"VK_INCOMPLETE"; break;
    case VK_ERROR_OUT_OF_HOST_MEMORY                           : return"VK_ERROR_OUT_OF_HOST_MEMORY"; break;
    case VK_ERROR_OUT_OF_DEVICE_MEMORY                         : return"VK_ERROR_OUT_OF_DEVICE_MEMORY"; break;
    case VK_ERROR_INITIALIZATION_FAILED                        : return"VK_ERROR_INITIALIZATION_FAILED"; break;
    case VK_ERROR_DEVICE_LOST                                  : return"VK_ERROR_DEVICE_LOST"; break;
    case VK_ERROR_MEMORY_MAP_FAILED                            : return"VK_ERROR_MEMORY_MAP_FAILED"; break;
    case VK_ERROR_LAYER_NOT_PRESENT                            : return"VK_ERROR_LAYER_NOT_PRESENT"; break;
    case VK_ERROR_EXTENSION_NOT_PRESENT                        : return"VK_ERROR_EXTENSION_NOT_PRESENT"; break;
    case VK_ERROR_FEATURE_NOT_PRESENT                          : return"VK_ERROR_FEATURE_NOT_PRESENT"; break;
    case VK_ERROR_INCOMPATIBLE_DRIVER                          : return"VK_ERROR_INCOMPATIBLE_DRIVER"; break;
    case VK_ERROR_TOO_MANY_OBJECTS                             : return"VK_ERROR_TOO_MANY_OBJECTS"; break;
    case VK_ERROR_FORMAT_NOT_SUPPORTED                         : return"VK_ERROR_FORMAT_NOT_SUPPORTED"; break;
    case VK_ERROR_FRAGMENTED_POOL                              : return"VK_ERROR_FRAGMENTED_POOL"; break;
    case VK_ERROR_OUT_OF_POOL_MEMORY                           : return"VK_ERROR_OUT_OF_POOL_MEMORY"; break;
    case VK_ERROR_INVALID_EXTERNAL_HANDLE                      : return"VK_ERROR_INVALID_EXTERNAL_HANDLE"; break;
    case VK_ERROR_SURFACE_LOST_KHR                             : return"VK_ERROR_SURFACE_LOST_KHR"; break;
    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR                     : return"VK_ERROR_NATIVE_WINDOW_IN_USE_KHR"; break;
    case VK_SUBOPTIMAL_KHR                                     : return"VK_SUBOPTIMAL_KHR"; break;
    case VK_ERROR_OUT_OF_DATE_KHR                              : return"VK_ERROR_OUT_OF_DATE_KHR"; break;
    case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR                     : return"VK_ERROR_INCOMPATIBLE_DISPLAY_KHR"; break;
    case VK_ERROR_VALIDATION_FAILED_EXT                        : return"VK_ERROR_VALIDATION_FAILED_EXT"; break;
    case VK_ERROR_INVALID_SHADER_NV                            : return"VK_ERROR_INVALID_SHADER_NV"; break;
    case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT : return"VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT"; break;
    case VK_ERROR_FRAGMENTATION_EXT                            : return"VK_ERROR_FRAGMENTATION_EXT"; break;
    case VK_ERROR_NOT_PERMITTED_EXT                            : return"VK_ERROR_NOT_PERMITTED_EXT"; break;
#if VK_HEADER_VERSION > 100
    case VK_ERROR_INVALID_DEVICE_ADDRESS_EXT                   : return"VK_ERROR_INVALID_DEVICE_ADDRESS_EXT"; break;
#endif
  }
  return "<UNKNOWN>";
}

std::string ToStringShort(VkPhysicalDeviceType value)
{
  switch (value) {
    default                                     : break;
    case VK_PHYSICAL_DEVICE_TYPE_OTHER          : return "OTHER"; break;
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU : return "INTEGRATED_GPU"; break;
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU   : return "DISCRETE_GPU"; break;
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU    : return "VIRTUAL_GPU"; break;
    case VK_PHYSICAL_DEVICE_TYPE_CPU            : return "CPU "; break;
  }
  return "<UNKNOWN>";
}

std::string ToString(VkPhysicalDeviceType value)
{
  switch (value) {
    default                                     : break;
    case VK_PHYSICAL_DEVICE_TYPE_OTHER          : return "VK_PHYSICAL_DEVICE_TYPE_OTHER"; break;
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU : return "VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU"; break;
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU   : return "VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU"; break;
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU    : return "VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU"; break;
    case VK_PHYSICAL_DEVICE_TYPE_CPU            : return "VK_PHYSICAL_DEVICE_TYPE_CPU "; break;
  }
  return "<UNKNOWN>";
}

std::string ToStringShort(VkFormat value)
{
  switch (value) {
    default                                                    : break;
    case VK_FORMAT_UNDEFINED                                   : return "UNDEFINED"; break;
    case VK_FORMAT_R4G4_UNORM_PACK8                            : return "R4G4_UNORM_PACK8"; break;
    case VK_FORMAT_R4G4B4A4_UNORM_PACK16                       : return "R4G4B4A4_UNORM_PACK16"; break;
    case VK_FORMAT_B4G4R4A4_UNORM_PACK16                       : return "B4G4R4A4_UNORM_PACK16"; break;
    case VK_FORMAT_R5G6B5_UNORM_PACK16                         : return "R5G6B5_UNORM_PACK16"; break;
    case VK_FORMAT_B5G6R5_UNORM_PACK16                         : return "B5G6R5_UNORM_PACK16"; break;
    case VK_FORMAT_R5G5B5A1_UNORM_PACK16                       : return "R5G5B5A1_UNORM_PACK16"; break;
    case VK_FORMAT_B5G5R5A1_UNORM_PACK16                       : return "B5G5R5A1_UNORM_PACK16"; break;
    case VK_FORMAT_A1R5G5B5_UNORM_PACK16                       : return "A1R5G5B5_UNORM_PACK16"; break;
    case VK_FORMAT_R8_UNORM                                    : return "R8_UNORM"; break;
    case VK_FORMAT_R8_SNORM                                    : return "R8_SNORM"; break;
    case VK_FORMAT_R8_USCALED                                  : return "R8_USCALED"; break;
    case VK_FORMAT_R8_SSCALED                                  : return "R8_SSCALED"; break;
    case VK_FORMAT_R8_UINT                                     : return "R8_UINT"; break;
    case VK_FORMAT_R8_SINT                                     : return "R8_SINT"; break;
    case VK_FORMAT_R8_SRGB                                     : return "R8_SRGB"; break;
    case VK_FORMAT_R8G8_UNORM                                  : return "R8G8_UNORM"; break;
    case VK_FORMAT_R8G8_SNORM                                  : return "R8G8_SNORM"; break;
    case VK_FORMAT_R8G8_USCALED                                : return "R8G8_USCALED"; break;
    case VK_FORMAT_R8G8_SSCALED                                : return "R8G8_SSCALED"; break;
    case VK_FORMAT_R8G8_UINT                                   : return "R8G8_UINT"; break;
    case VK_FORMAT_R8G8_SINT                                   : return "R8G8_SINT"; break;
    case VK_FORMAT_R8G8_SRGB                                   : return "R8G8_SRGB"; break;
    case VK_FORMAT_R8G8B8_UNORM                                : return "R8G8B8_UNORM"; break;
    case VK_FORMAT_R8G8B8_SNORM                                : return "R8G8B8_SNORM"; break;
    case VK_FORMAT_R8G8B8_USCALED                              : return "R8G8B8_USCALED"; break;
    case VK_FORMAT_R8G8B8_SSCALED                              : return "R8G8B8_SSCALED"; break;
    case VK_FORMAT_R8G8B8_UINT                                 : return "R8G8B8_UINT"; break;
    case VK_FORMAT_R8G8B8_SINT                                 : return "R8G8B8_SINT"; break;
    case VK_FORMAT_R8G8B8_SRGB                                 : return "R8G8B8_SRGB"; break;
    case VK_FORMAT_B8G8R8_UNORM                                : return "B8G8R8_UNORM"; break;
    case VK_FORMAT_B8G8R8_SNORM                                : return "B8G8R8_SNORM"; break;
    case VK_FORMAT_B8G8R8_USCALED                              : return "B8G8R8_USCALED"; break;
    case VK_FORMAT_B8G8R8_SSCALED                              : return "B8G8R8_SSCALED"; break;
    case VK_FORMAT_B8G8R8_UINT                                 : return "B8G8R8_UINT"; break;
    case VK_FORMAT_B8G8R8_SINT                                 : return "B8G8R8_SINT"; break;
    case VK_FORMAT_B8G8R8_SRGB                                 : return "B8G8R8_SRGB"; break;
    case VK_FORMAT_R8G8B8A8_UNORM                              : return "R8G8B8A8_UNORM"; break;
    case VK_FORMAT_R8G8B8A8_SNORM                              : return "R8G8B8A8_SNORM"; break;
    case VK_FORMAT_R8G8B8A8_USCALED                            : return "R8G8B8A8_USCALED"; break;
    case VK_FORMAT_R8G8B8A8_SSCALED                            : return "R8G8B8A8_SSCALED"; break;
    case VK_FORMAT_R8G8B8A8_UINT                               : return "R8G8B8A8_UINT"; break;
    case VK_FORMAT_R8G8B8A8_SINT                               : return "R8G8B8A8_SINT"; break;
    case VK_FORMAT_R8G8B8A8_SRGB                               : return "R8G8B8A8_SRGB"; break;
    case VK_FORMAT_B8G8R8A8_UNORM                              : return "B8G8R8A8_UNORM"; break;
    case VK_FORMAT_B8G8R8A8_SNORM                              : return "B8G8R8A8_SNORM"; break;
    case VK_FORMAT_B8G8R8A8_USCALED                            : return "B8G8R8A8_USCALED"; break;
    case VK_FORMAT_B8G8R8A8_SSCALED                            : return "B8G8R8A8_SSCALED"; break;
    case VK_FORMAT_B8G8R8A8_UINT                               : return "B8G8R8A8_UINT"; break;
    case VK_FORMAT_B8G8R8A8_SINT                               : return "B8G8R8A8_SINT"; break;
    case VK_FORMAT_B8G8R8A8_SRGB                               : return "B8G8R8A8_SRGB"; break;
    case VK_FORMAT_A8B8G8R8_UNORM_PACK32                       : return "A8B8G8R8_UNORM_PACK32"; break;
    case VK_FORMAT_A8B8G8R8_SNORM_PACK32                       : return "A8B8G8R8_SNORM_PACK32"; break;
    case VK_FORMAT_A8B8G8R8_USCALED_PACK32                     : return "A8B8G8R8_USCALED_PACK32"; break;
    case VK_FORMAT_A8B8G8R8_SSCALED_PACK32                     : return "A8B8G8R8_SSCALED_PACK32"; break;
    case VK_FORMAT_A8B8G8R8_UINT_PACK32                        : return "A8B8G8R8_UINT_PACK32"; break;
    case VK_FORMAT_A8B8G8R8_SINT_PACK32                        : return "A8B8G8R8_SINT_PACK32"; break;
    case VK_FORMAT_A8B8G8R8_SRGB_PACK32                        : return "A8B8G8R8_SRGB_PACK32"; break;
    case VK_FORMAT_A2R10G10B10_UNORM_PACK32                    : return "A2R10G10B10_UNORM_PACK32"; break;
    case VK_FORMAT_A2R10G10B10_SNORM_PACK32                    : return "A2R10G10B10_SNORM_PACK32"; break;
    case VK_FORMAT_A2R10G10B10_USCALED_PACK32                  : return "A2R10G10B10_USCALED_PACK32"; break;
    case VK_FORMAT_A2R10G10B10_SSCALED_PACK32                  : return "A2R10G10B10_SSCALED_PACK32"; break;
    case VK_FORMAT_A2R10G10B10_UINT_PACK32                     : return "A2R10G10B10_UINT_PACK32"; break;
    case VK_FORMAT_A2R10G10B10_SINT_PACK32                     : return "A2R10G10B10_SINT_PACK32"; break;
    case VK_FORMAT_A2B10G10R10_UNORM_PACK32                    : return "A2B10G10R10_UNORM_PACK32"; break;
    case VK_FORMAT_A2B10G10R10_SNORM_PACK32                    : return "A2B10G10R10_SNORM_PACK32"; break;
    case VK_FORMAT_A2B10G10R10_USCALED_PACK32                  : return "A2B10G10R10_USCALED_PACK32"; break;
    case VK_FORMAT_A2B10G10R10_SSCALED_PACK32                  : return "A2B10G10R10_SSCALED_PACK32"; break;
    case VK_FORMAT_A2B10G10R10_UINT_PACK32                     : return "A2B10G10R10_UINT_PACK32"; break;
    case VK_FORMAT_A2B10G10R10_SINT_PACK32                     : return "A2B10G10R10_SINT_PACK32"; break;
    case VK_FORMAT_R16_UNORM                                   : return "R16_UNORM"; break;
    case VK_FORMAT_R16_SNORM                                   : return "R16_SNORM"; break;
    case VK_FORMAT_R16_USCALED                                 : return "R16_USCALED"; break;
    case VK_FORMAT_R16_SSCALED                                 : return "R16_SSCALED"; break;
    case VK_FORMAT_R16_UINT                                    : return "R16_UINT"; break;
    case VK_FORMAT_R16_SINT                                    : return "R16_SINT"; break;
    case VK_FORMAT_R16_SFLOAT                                  : return "R16_SFLOAT"; break;
    case VK_FORMAT_R16G16_UNORM                                : return "R16G16_UNORM"; break;
    case VK_FORMAT_R16G16_SNORM                                : return "R16G16_SNORM"; break;
    case VK_FORMAT_R16G16_USCALED                              : return "R16G16_USCALED"; break;
    case VK_FORMAT_R16G16_SSCALED                              : return "R16G16_SSCALED"; break;
    case VK_FORMAT_R16G16_UINT                                 : return "R16G16_UINT"; break;
    case VK_FORMAT_R16G16_SINT                                 : return "R16G16_SINT"; break;
    case VK_FORMAT_R16G16_SFLOAT                               : return "R16G16_SFLOAT"; break;
    case VK_FORMAT_R16G16B16_UNORM                             : return "R16G16B16_UNORM"; break;
    case VK_FORMAT_R16G16B16_SNORM                             : return "R16G16B16_SNORM"; break;
    case VK_FORMAT_R16G16B16_USCALED                           : return "R16G16B16_USCALED"; break;
    case VK_FORMAT_R16G16B16_SSCALED                           : return "R16G16B16_SSCALED"; break;
    case VK_FORMAT_R16G16B16_UINT                              : return "R16G16B16_UINT"; break;
    case VK_FORMAT_R16G16B16_SINT                              : return "R16G16B16_SINT"; break;
    case VK_FORMAT_R16G16B16_SFLOAT                            : return "R16G16B16_SFLOAT"; break;
    case VK_FORMAT_R16G16B16A16_UNORM                          : return "R16G16B16A16_UNORM"; break;
    case VK_FORMAT_R16G16B16A16_SNORM                          : return "R16G16B16A16_SNORM"; break;
    case VK_FORMAT_R16G16B16A16_USCALED                        : return "R16G16B16A16_USCALED"; break;
    case VK_FORMAT_R16G16B16A16_SSCALED                        : return "R16G16B16A16_SSCALED"; break;
    case VK_FORMAT_R16G16B16A16_UINT                           : return "R16G16B16A16_UINT"; break;
    case VK_FORMAT_R16G16B16A16_SINT                           : return "R16G16B16A16_SINT"; break;
    case VK_FORMAT_R16G16B16A16_SFLOAT                         : return "R16G16B16A16_SFLOAT"; break;
    case VK_FORMAT_R32_UINT                                    : return "R32_UINT"; break;
    case VK_FORMAT_R32_SINT                                    : return "R32_SINT"; break;
    case VK_FORMAT_R32_SFLOAT                                  : return "R32_SFLOAT"; break;
    case VK_FORMAT_R32G32_UINT                                 : return "R32G32_UINT"; break;
    case VK_FORMAT_R32G32_SINT                                 : return "R32G32_SINT"; break;
    case VK_FORMAT_R32G32_SFLOAT                               : return "R32G32_SFLOAT"; break;
    case VK_FORMAT_R32G32B32_UINT                              : return "R32G32B32_UINT"; break;
    case VK_FORMAT_R32G32B32_SINT                              : return "R32G32B32_SINT"; break;
    case VK_FORMAT_R32G32B32_SFLOAT                            : return "R32G32B32_SFLOAT"; break;
    case VK_FORMAT_R32G32B32A32_UINT                           : return "R32G32B32A32_UINT"; break;
    case VK_FORMAT_R32G32B32A32_SINT                           : return "R32G32B32A32_SINT"; break;
    case VK_FORMAT_R32G32B32A32_SFLOAT                         : return "R32G32B32A32_SFLOAT"; break;
    case VK_FORMAT_R64_UINT                                    : return "R64_UINT"; break;
    case VK_FORMAT_R64_SINT                                    : return "R64_SINT"; break;
    case VK_FORMAT_R64_SFLOAT                                  : return "R64_SFLOAT"; break;
    case VK_FORMAT_R64G64_UINT                                 : return "R64G64_UINT"; break;
    case VK_FORMAT_R64G64_SINT                                 : return "R64G64_SINT"; break;
    case VK_FORMAT_R64G64_SFLOAT                               : return "R64G64_SFLOAT"; break;
    case VK_FORMAT_R64G64B64_UINT                              : return "R64G64B64_UINT"; break;
    case VK_FORMAT_R64G64B64_SINT                              : return "R64G64B64_SINT"; break;
    case VK_FORMAT_R64G64B64_SFLOAT                            : return "R64G64B64_SFLOAT"; break;
    case VK_FORMAT_R64G64B64A64_UINT                           : return "R64G64B64A64_UINT"; break;
    case VK_FORMAT_R64G64B64A64_SINT                           : return "R64G64B64A64_SINT"; break;
    case VK_FORMAT_R64G64B64A64_SFLOAT                         : return "R64G64B64A64_SFLOAT"; break;
    case VK_FORMAT_B10G11R11_UFLOAT_PACK32                     : return "B10G11R11_UFLOAT_PACK32"; break;
    case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32                      : return "E5B9G9R9_UFLOAT_PACK32"; break;
    case VK_FORMAT_D16_UNORM                                   : return "D16_UNORM"; break;
    case VK_FORMAT_X8_D24_UNORM_PACK32                         : return "X8_D24_UNORM_PACK32"; break;
    case VK_FORMAT_D32_SFLOAT                                  : return "D32_SFLOAT"; break;
    case VK_FORMAT_S8_UINT                                     : return "S8_UINT"; break;
    case VK_FORMAT_D16_UNORM_S8_UINT                           : return "D16_UNORM_S8_UINT"; break;
    case VK_FORMAT_D24_UNORM_S8_UINT                           : return "D24_UNORM_S8_UINT"; break;
    case VK_FORMAT_D32_SFLOAT_S8_UINT                          : return "D32_SFLOAT_S8_UINT"; break;
    case VK_FORMAT_BC1_RGB_UNORM_BLOCK                         : return "BC1_RGB_UNORM_BLOCK"; break;
    case VK_FORMAT_BC1_RGB_SRGB_BLOCK                          : return "BC1_RGB_SRGB_BLOCK"; break;
    case VK_FORMAT_BC1_RGBA_UNORM_BLOCK                        : return "BC1_RGBA_UNORM_BLOCK"; break;
    case VK_FORMAT_BC1_RGBA_SRGB_BLOCK                         : return "BC1_RGBA_SRGB_BLOCK"; break;
    case VK_FORMAT_BC2_UNORM_BLOCK                             : return "BC2_UNORM_BLOCK"; break;
    case VK_FORMAT_BC2_SRGB_BLOCK                              : return "BC2_SRGB_BLOCK"; break;
    case VK_FORMAT_BC3_UNORM_BLOCK                             : return "BC3_UNORM_BLOCK"; break;
    case VK_FORMAT_BC3_SRGB_BLOCK                              : return "BC3_SRGB_BLOCK"; break;
    case VK_FORMAT_BC4_UNORM_BLOCK                             : return "BC4_UNORM_BLOCK"; break;
    case VK_FORMAT_BC4_SNORM_BLOCK                             : return "BC4_SNORM_BLOCK"; break;
    case VK_FORMAT_BC5_UNORM_BLOCK                             : return "BC5_UNORM_BLOCK"; break;
    case VK_FORMAT_BC5_SNORM_BLOCK                             : return "BC5_SNORM_BLOCK"; break;
    case VK_FORMAT_BC6H_UFLOAT_BLOCK                           : return "BC6H_UFLOAT_BLOCK"; break;
    case VK_FORMAT_BC6H_SFLOAT_BLOCK                           : return "BC6H_SFLOAT_BLOCK"; break;
    case VK_FORMAT_BC7_UNORM_BLOCK                             : return "BC7_UNORM_BLOCK"; break;
    case VK_FORMAT_BC7_SRGB_BLOCK                              : return "BC7_SRGB_BLOCK"; break;
    case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK                     : return "ETC2_R8G8B8_UNORM_BLOCK"; break;
    case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK                      : return "ETC2_R8G8B8_SRGB_BLOCK"; break;
    case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK                   : return "ETC2_R8G8B8A1_UNORM_BLOCK"; break;
    case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK                    : return "ETC2_R8G8B8A1_SRGB_BLOCK"; break;
    case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK                   : return "ETC2_R8G8B8A8_UNORM_BLOCK"; break;
    case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK                    : return "ETC2_R8G8B8A8_SRGB_BLOCK"; break;
    case VK_FORMAT_EAC_R11_UNORM_BLOCK                         : return "EAC_R11_UNORM_BLOCK"; break;
    case VK_FORMAT_EAC_R11_SNORM_BLOCK                         : return "EAC_R11_SNORM_BLOCK"; break;
    case VK_FORMAT_EAC_R11G11_UNORM_BLOCK                      : return "EAC_R11G11_UNORM_BLOCK"; break;
    case VK_FORMAT_EAC_R11G11_SNORM_BLOCK                      : return "EAC_R11G11_SNORM_BLOCK"; break;
    case VK_FORMAT_ASTC_4x4_UNORM_BLOCK                        : return "ASTC_4x4_UNORM_BLOCK"; break;
    case VK_FORMAT_ASTC_4x4_SRGB_BLOCK                         : return "ASTC_4x4_SRGB_BLOCK"; break;
    case VK_FORMAT_ASTC_5x4_UNORM_BLOCK                        : return "ASTC_5x4_UNORM_BLOCK"; break;
    case VK_FORMAT_ASTC_5x4_SRGB_BLOCK                         : return "ASTC_5x4_SRGB_BLOCK"; break;
    case VK_FORMAT_ASTC_5x5_UNORM_BLOCK                        : return "ASTC_5x5_UNORM_BLOCK"; break;
    case VK_FORMAT_ASTC_5x5_SRGB_BLOCK                         : return "ASTC_5x5_SRGB_BLOCK"; break;
    case VK_FORMAT_ASTC_6x5_UNORM_BLOCK                        : return "ASTC_6x5_UNORM_BLOCK"; break;
    case VK_FORMAT_ASTC_6x5_SRGB_BLOCK                         : return "ASTC_6x5_SRGB_BLOCK"; break;
    case VK_FORMAT_ASTC_6x6_UNORM_BLOCK                        : return "ASTC_6x6_UNORM_BLOCK"; break;
    case VK_FORMAT_ASTC_6x6_SRGB_BLOCK                         : return "ASTC_6x6_SRGB_BLOCK"; break;
    case VK_FORMAT_ASTC_8x5_UNORM_BLOCK                        : return "ASTC_8x5_UNORM_BLOCK"; break;
    case VK_FORMAT_ASTC_8x5_SRGB_BLOCK                         : return "ASTC_8x5_SRGB_BLOCK"; break;
    case VK_FORMAT_ASTC_8x6_UNORM_BLOCK                        : return "ASTC_8x6_UNORM_BLOCK"; break;
    case VK_FORMAT_ASTC_8x6_SRGB_BLOCK                         : return "ASTC_8x6_SRGB_BLOCK"; break;
    case VK_FORMAT_ASTC_8x8_UNORM_BLOCK                        : return "ASTC_8x8_UNORM_BLOCK"; break;
    case VK_FORMAT_ASTC_8x8_SRGB_BLOCK                         : return "ASTC_8x8_SRGB_BLOCK"; break;
    case VK_FORMAT_ASTC_10x5_UNORM_BLOCK                       : return "ASTC_10x5_UNORM_BLOCK"; break;
    case VK_FORMAT_ASTC_10x5_SRGB_BLOCK                        : return "ASTC_10x5_SRGB_BLOCK"; break;
    case VK_FORMAT_ASTC_10x6_UNORM_BLOCK                       : return "ASTC_10x6_UNORM_BLOCK"; break;
    case VK_FORMAT_ASTC_10x6_SRGB_BLOCK                        : return "ASTC_10x6_SRGB_BLOCK"; break;
    case VK_FORMAT_ASTC_10x8_UNORM_BLOCK                       : return "ASTC_10x8_UNORM_BLOCK"; break;
    case VK_FORMAT_ASTC_10x8_SRGB_BLOCK                        : return "ASTC_10x8_SRGB_BLOCK"; break;
    case VK_FORMAT_ASTC_10x10_UNORM_BLOCK                      : return "ASTC_10x10_UNORM_BLOCK"; break;
    case VK_FORMAT_ASTC_10x10_SRGB_BLOCK                       : return "ASTC_10x10_SRGB_BLOCK"; break;
    case VK_FORMAT_ASTC_12x10_UNORM_BLOCK                      : return "ASTC_12x10_UNORM_BLOCK"; break;
    case VK_FORMAT_ASTC_12x10_SRGB_BLOCK                       : return "ASTC_12x10_SRGB_BLOCK"; break;
    case VK_FORMAT_ASTC_12x12_UNORM_BLOCK                      : return "ASTC_12x12_UNORM_BLOCK"; break;
    case VK_FORMAT_ASTC_12x12_SRGB_BLOCK                       : return "ASTC_12x12_SRGB_BLOCK"; break;
    case VK_FORMAT_G8B8G8R8_422_UNORM                          : return "G8B8G8R8_422_UNORM"; break;
    case VK_FORMAT_B8G8R8G8_422_UNORM                          : return "B8G8R8G8_422_UNORM"; break;
    case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM                   : return "G8_B8_R8_3PLANE_420_UNORM"; break;
    case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM                    : return "G8_B8R8_2PLANE_420_UNORM"; break;
    case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM                   : return "G8_B8_R8_3PLANE_422_UNORM"; break;
    case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM                    : return "G8_B8R8_2PLANE_422_UNORM"; break;
    case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM                   : return "G8_B8_R8_3PLANE_444_UNORM"; break;
    case VK_FORMAT_R10X6_UNORM_PACK16                          : return "R10X6_UNORM_PACK16"; break;
    case VK_FORMAT_R10X6G10X6_UNORM_2PACK16                    : return "R10X6G10X6_UNORM_2PACK16"; break;
    case VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16          : return "R10X6G10X6B10X6A10X6_UNORM_4PACK16"; break;
    case VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16      : return "G10X6B10X6G10X6R10X6_422_UNORM_4PACK16"; break;
    case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16      : return "B10X6G10X6R10X6G10X6_422_UNORM_4PACK16"; break;
    case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16  : return "G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16"; break;
    case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16   : return "G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16"; break;
    case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16  : return "G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16"; break;
    case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16   : return "G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16"; break;
    case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16  : return "G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16"; break;
    case VK_FORMAT_R12X4_UNORM_PACK16                          : return "R12X4_UNORM_PACK16"; break;
    case VK_FORMAT_R12X4G12X4_UNORM_2PACK16                    : return "R12X4G12X4_UNORM_2PACK16"; break;
    case VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16          : return "R12X4G12X4B12X4A12X4_UNORM_4PACK16"; break;
    case VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16      : return "G12X4B12X4G12X4R12X4_422_UNORM_4PACK16"; break;
    case VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16      : return "B12X4G12X4R12X4G12X4_422_UNORM_4PACK16"; break;
    case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16  : return "G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16"; break;
    case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16   : return "G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16"; break;
    case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16  : return "G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16"; break;
    case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16   : return "G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16"; break;
    case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16  : return "G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16"; break;
    case VK_FORMAT_G16B16G16R16_422_UNORM                      : return "G16B16G16R16_422_UNORM"; break;
    case VK_FORMAT_B16G16R16G16_422_UNORM                      : return "B16G16R16G16_422_UNORM"; break;
    case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM                : return "G16_B16_R16_3PLANE_420_UNORM"; break;
    case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM                 : return "G16_B16R16_2PLANE_420_UNORM"; break;
    case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM                : return "G16_B16_R16_3PLANE_422_UNORM"; break;
    case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM                 : return "G16_B16R16_2PLANE_422_UNORM"; break;
    case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM                : return "G16_B16_R16_3PLANE_444_UNORM"; break;
    case VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG                 : return "PVRTC1_2BPP_UNORM_BLOCK_IMG"; break;
    case VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG                 : return "PVRTC1_4BPP_UNORM_BLOCK_IMG"; break;
    case VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG                 : return "PVRTC2_2BPP_UNORM_BLOCK_IMG"; break;
    case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG                 : return "PVRTC2_4BPP_UNORM_BLOCK_IMG"; break;
    case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG                  : return "PVRTC1_2BPP_SRGB_BLOCK_IMG"; break;
    case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG                  : return "PVRTC1_4BPP_SRGB_BLOCK_IMG"; break;
    case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG                  : return "PVRTC2_2BPP_SRGB_BLOCK_IMG"; break;
    case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG                  : return "PVRTC2_4BPP_SRGB_BLOCK_IMG"; break;
  }
  return "<UNKNOWN>";
}

std::string ToString(VkFormat value)
{
  switch (value) {
    default                                                    : break;
    case VK_FORMAT_UNDEFINED                                   : return "VK_FORMAT_UNDEFINED"; break;
    case VK_FORMAT_R4G4_UNORM_PACK8                            : return "VK_FORMAT_R4G4_UNORM_PACK8"; break;
    case VK_FORMAT_R4G4B4A4_UNORM_PACK16                       : return "VK_FORMAT_R4G4B4A4_UNORM_PACK16"; break;
    case VK_FORMAT_B4G4R4A4_UNORM_PACK16                       : return "VK_FORMAT_B4G4R4A4_UNORM_PACK16"; break;
    case VK_FORMAT_R5G6B5_UNORM_PACK16                         : return "VK_FORMAT_R5G6B5_UNORM_PACK16"; break;
    case VK_FORMAT_B5G6R5_UNORM_PACK16                         : return "VK_FORMAT_B5G6R5_UNORM_PACK16"; break;
    case VK_FORMAT_R5G5B5A1_UNORM_PACK16                       : return "VK_FORMAT_R5G5B5A1_UNORM_PACK16"; break;
    case VK_FORMAT_B5G5R5A1_UNORM_PACK16                       : return "VK_FORMAT_B5G5R5A1_UNORM_PACK16"; break;
    case VK_FORMAT_A1R5G5B5_UNORM_PACK16                       : return "VK_FORMAT_A1R5G5B5_UNORM_PACK16"; break;
    case VK_FORMAT_R8_UNORM                                    : return "VK_FORMAT_R8_UNORM"; break;
    case VK_FORMAT_R8_SNORM                                    : return "VK_FORMAT_R8_SNORM"; break;
    case VK_FORMAT_R8_USCALED                                  : return "VK_FORMAT_R8_USCALED"; break;
    case VK_FORMAT_R8_SSCALED                                  : return "VK_FORMAT_R8_SSCALED"; break;
    case VK_FORMAT_R8_UINT                                     : return "VK_FORMAT_R8_UINT"; break;
    case VK_FORMAT_R8_SINT                                     : return "VK_FORMAT_R8_SINT"; break;
    case VK_FORMAT_R8_SRGB                                     : return "VK_FORMAT_R8_SRGB"; break;
    case VK_FORMAT_R8G8_UNORM                                  : return "VK_FORMAT_R8G8_UNORM"; break;
    case VK_FORMAT_R8G8_SNORM                                  : return "VK_FORMAT_R8G8_SNORM"; break;
    case VK_FORMAT_R8G8_USCALED                                : return "VK_FORMAT_R8G8_USCALED"; break;
    case VK_FORMAT_R8G8_SSCALED                                : return "VK_FORMAT_R8G8_SSCALED"; break;
    case VK_FORMAT_R8G8_UINT                                   : return "VK_FORMAT_R8G8_UINT"; break;
    case VK_FORMAT_R8G8_SINT                                   : return "VK_FORMAT_R8G8_SINT"; break;
    case VK_FORMAT_R8G8_SRGB                                   : return "VK_FORMAT_R8G8_SRGB"; break;
    case VK_FORMAT_R8G8B8_UNORM                                : return "VK_FORMAT_R8G8B8_UNORM"; break;
    case VK_FORMAT_R8G8B8_SNORM                                : return "VK_FORMAT_R8G8B8_SNORM"; break;
    case VK_FORMAT_R8G8B8_USCALED                              : return "VK_FORMAT_R8G8B8_USCALED"; break;
    case VK_FORMAT_R8G8B8_SSCALED                              : return "VK_FORMAT_R8G8B8_SSCALED"; break;
    case VK_FORMAT_R8G8B8_UINT                                 : return "VK_FORMAT_R8G8B8_UINT"; break;
    case VK_FORMAT_R8G8B8_SINT                                 : return "VK_FORMAT_R8G8B8_SINT"; break;
    case VK_FORMAT_R8G8B8_SRGB                                 : return "VK_FORMAT_R8G8B8_SRGB"; break;
    case VK_FORMAT_B8G8R8_UNORM                                : return "VK_FORMAT_B8G8R8_UNORM"; break;
    case VK_FORMAT_B8G8R8_SNORM                                : return "VK_FORMAT_B8G8R8_SNORM"; break;
    case VK_FORMAT_B8G8R8_USCALED                              : return "VK_FORMAT_B8G8R8_USCALED"; break;
    case VK_FORMAT_B8G8R8_SSCALED                              : return "VK_FORMAT_B8G8R8_SSCALED"; break;
    case VK_FORMAT_B8G8R8_UINT                                 : return "VK_FORMAT_B8G8R8_UINT"; break;
    case VK_FORMAT_B8G8R8_SINT                                 : return "VK_FORMAT_B8G8R8_SINT"; break;
    case VK_FORMAT_B8G8R8_SRGB                                 : return "VK_FORMAT_B8G8R8_SRGB"; break;
    case VK_FORMAT_R8G8B8A8_UNORM                              : return "VK_FORMAT_R8G8B8A8_UNORM"; break;
    case VK_FORMAT_R8G8B8A8_SNORM                              : return "VK_FORMAT_R8G8B8A8_SNORM"; break;
    case VK_FORMAT_R8G8B8A8_USCALED                            : return "VK_FORMAT_R8G8B8A8_USCALED"; break;
    case VK_FORMAT_R8G8B8A8_SSCALED                            : return "VK_FORMAT_R8G8B8A8_SSCALED"; break;
    case VK_FORMAT_R8G8B8A8_UINT                               : return "VK_FORMAT_R8G8B8A8_UINT"; break;
    case VK_FORMAT_R8G8B8A8_SINT                               : return "VK_FORMAT_R8G8B8A8_SINT"; break;
    case VK_FORMAT_R8G8B8A8_SRGB                               : return "VK_FORMAT_R8G8B8A8_SRGB"; break;
    case VK_FORMAT_B8G8R8A8_UNORM                              : return "VK_FORMAT_B8G8R8A8_UNORM"; break;
    case VK_FORMAT_B8G8R8A8_SNORM                              : return "VK_FORMAT_B8G8R8A8_SNORM"; break;
    case VK_FORMAT_B8G8R8A8_USCALED                            : return "VK_FORMAT_B8G8R8A8_USCALED"; break;
    case VK_FORMAT_B8G8R8A8_SSCALED                            : return "VK_FORMAT_B8G8R8A8_SSCALED"; break;
    case VK_FORMAT_B8G8R8A8_UINT                               : return "VK_FORMAT_B8G8R8A8_UINT"; break;
    case VK_FORMAT_B8G8R8A8_SINT                               : return "VK_FORMAT_B8G8R8A8_SINT"; break;
    case VK_FORMAT_B8G8R8A8_SRGB                               : return "VK_FORMAT_B8G8R8A8_SRGB"; break;
    case VK_FORMAT_A8B8G8R8_UNORM_PACK32                       : return "VK_FORMAT_A8B8G8R8_UNORM_PACK32"; break;
    case VK_FORMAT_A8B8G8R8_SNORM_PACK32                       : return "VK_FORMAT_A8B8G8R8_SNORM_PACK32"; break;
    case VK_FORMAT_A8B8G8R8_USCALED_PACK32                     : return "VK_FORMAT_A8B8G8R8_USCALED_PACK32"; break;
    case VK_FORMAT_A8B8G8R8_SSCALED_PACK32                     : return "VK_FORMAT_A8B8G8R8_SSCALED_PACK32"; break;
    case VK_FORMAT_A8B8G8R8_UINT_PACK32                        : return "VK_FORMAT_A8B8G8R8_UINT_PACK32"; break;
    case VK_FORMAT_A8B8G8R8_SINT_PACK32                        : return "VK_FORMAT_A8B8G8R8_SINT_PACK32"; break;
    case VK_FORMAT_A8B8G8R8_SRGB_PACK32                        : return "VK_FORMAT_A8B8G8R8_SRGB_PACK32"; break;
    case VK_FORMAT_A2R10G10B10_UNORM_PACK32                    : return "VK_FORMAT_A2R10G10B10_UNORM_PACK32"; break;
    case VK_FORMAT_A2R10G10B10_SNORM_PACK32                    : return "VK_FORMAT_A2R10G10B10_SNORM_PACK32"; break;
    case VK_FORMAT_A2R10G10B10_USCALED_PACK32                  : return "VK_FORMAT_A2R10G10B10_USCALED_PACK32"; break;
    case VK_FORMAT_A2R10G10B10_SSCALED_PACK32                  : return "VK_FORMAT_A2R10G10B10_SSCALED_PACK32"; break;
    case VK_FORMAT_A2R10G10B10_UINT_PACK32                     : return "VK_FORMAT_A2R10G10B10_UINT_PACK32"; break;
    case VK_FORMAT_A2R10G10B10_SINT_PACK32                     : return "VK_FORMAT_A2R10G10B10_SINT_PACK32"; break;
    case VK_FORMAT_A2B10G10R10_UNORM_PACK32                    : return "VK_FORMAT_A2B10G10R10_UNORM_PACK32"; break;
    case VK_FORMAT_A2B10G10R10_SNORM_PACK32                    : return "VK_FORMAT_A2B10G10R10_SNORM_PACK32"; break;
    case VK_FORMAT_A2B10G10R10_USCALED_PACK32                  : return "VK_FORMAT_A2B10G10R10_USCALED_PACK32"; break;
    case VK_FORMAT_A2B10G10R10_SSCALED_PACK32                  : return "VK_FORMAT_A2B10G10R10_SSCALED_PACK32"; break;
    case VK_FORMAT_A2B10G10R10_UINT_PACK32                     : return "VK_FORMAT_A2B10G10R10_UINT_PACK32"; break;
    case VK_FORMAT_A2B10G10R10_SINT_PACK32                     : return "VK_FORMAT_A2B10G10R10_SINT_PACK32"; break;
    case VK_FORMAT_R16_UNORM                                   : return "VK_FORMAT_R16_UNORM"; break;
    case VK_FORMAT_R16_SNORM                                   : return "VK_FORMAT_R16_SNORM"; break;
    case VK_FORMAT_R16_USCALED                                 : return "VK_FORMAT_R16_USCALED"; break;
    case VK_FORMAT_R16_SSCALED                                 : return "VK_FORMAT_R16_SSCALED"; break;
    case VK_FORMAT_R16_UINT                                    : return "VK_FORMAT_R16_UINT"; break;
    case VK_FORMAT_R16_SINT                                    : return "VK_FORMAT_R16_SINT"; break;
    case VK_FORMAT_R16_SFLOAT                                  : return "VK_FORMAT_R16_SFLOAT"; break;
    case VK_FORMAT_R16G16_UNORM                                : return "VK_FORMAT_R16G16_UNORM"; break;
    case VK_FORMAT_R16G16_SNORM                                : return "VK_FORMAT_R16G16_SNORM"; break;
    case VK_FORMAT_R16G16_USCALED                              : return "VK_FORMAT_R16G16_USCALED"; break;
    case VK_FORMAT_R16G16_SSCALED                              : return "VK_FORMAT_R16G16_SSCALED"; break;
    case VK_FORMAT_R16G16_UINT                                 : return "VK_FORMAT_R16G16_UINT"; break;
    case VK_FORMAT_R16G16_SINT                                 : return "VK_FORMAT_R16G16_SINT"; break;
    case VK_FORMAT_R16G16_SFLOAT                               : return "VK_FORMAT_R16G16_SFLOAT"; break;
    case VK_FORMAT_R16G16B16_UNORM                             : return "VK_FORMAT_R16G16B16_UNORM"; break;
    case VK_FORMAT_R16G16B16_SNORM                             : return "VK_FORMAT_R16G16B16_SNORM"; break;
    case VK_FORMAT_R16G16B16_USCALED                           : return "VK_FORMAT_R16G16B16_USCALED"; break;
    case VK_FORMAT_R16G16B16_SSCALED                           : return "VK_FORMAT_R16G16B16_SSCALED"; break;
    case VK_FORMAT_R16G16B16_UINT                              : return "VK_FORMAT_R16G16B16_UINT"; break;
    case VK_FORMAT_R16G16B16_SINT                              : return "VK_FORMAT_R16G16B16_SINT"; break;
    case VK_FORMAT_R16G16B16_SFLOAT                            : return "VK_FORMAT_R16G16B16_SFLOAT"; break;
    case VK_FORMAT_R16G16B16A16_UNORM                          : return "VK_FORMAT_R16G16B16A16_UNORM"; break;
    case VK_FORMAT_R16G16B16A16_SNORM                          : return "VK_FORMAT_R16G16B16A16_SNORM"; break;
    case VK_FORMAT_R16G16B16A16_USCALED                        : return "VK_FORMAT_R16G16B16A16_USCALED"; break;
    case VK_FORMAT_R16G16B16A16_SSCALED                        : return "VK_FORMAT_R16G16B16A16_SSCALED"; break;
    case VK_FORMAT_R16G16B16A16_UINT                           : return "VK_FORMAT_R16G16B16A16_UINT"; break;
    case VK_FORMAT_R16G16B16A16_SINT                           : return "VK_FORMAT_R16G16B16A16_SINT"; break;
    case VK_FORMAT_R16G16B16A16_SFLOAT                         : return "VK_FORMAT_R16G16B16A16_SFLOAT"; break;
    case VK_FORMAT_R32_UINT                                    : return "VK_FORMAT_R32_UINT"; break;
    case VK_FORMAT_R32_SINT                                    : return "VK_FORMAT_R32_SINT"; break;
    case VK_FORMAT_R32_SFLOAT                                  : return "VK_FORMAT_R32_SFLOAT"; break;
    case VK_FORMAT_R32G32_UINT                                 : return "VK_FORMAT_R32G32_UINT"; break;
    case VK_FORMAT_R32G32_SINT                                 : return "VK_FORMAT_R32G32_SINT"; break;
    case VK_FORMAT_R32G32_SFLOAT                               : return "VK_FORMAT_R32G32_SFLOAT"; break;
    case VK_FORMAT_R32G32B32_UINT                              : return "VK_FORMAT_R32G32B32_UINT"; break;
    case VK_FORMAT_R32G32B32_SINT                              : return "VK_FORMAT_R32G32B32_SINT"; break;
    case VK_FORMAT_R32G32B32_SFLOAT                            : return "VK_FORMAT_R32G32B32_SFLOAT"; break;
    case VK_FORMAT_R32G32B32A32_UINT                           : return "VK_FORMAT_R32G32B32A32_UINT"; break;
    case VK_FORMAT_R32G32B32A32_SINT                           : return "VK_FORMAT_R32G32B32A32_SINT"; break;
    case VK_FORMAT_R32G32B32A32_SFLOAT                         : return "VK_FORMAT_R32G32B32A32_SFLOAT"; break;
    case VK_FORMAT_R64_UINT                                    : return "VK_FORMAT_R64_UINT"; break;
    case VK_FORMAT_R64_SINT                                    : return "VK_FORMAT_R64_SINT"; break;
    case VK_FORMAT_R64_SFLOAT                                  : return "VK_FORMAT_R64_SFLOAT"; break;
    case VK_FORMAT_R64G64_UINT                                 : return "VK_FORMAT_R64G64_UINT"; break;
    case VK_FORMAT_R64G64_SINT                                 : return "VK_FORMAT_R64G64_SINT"; break;
    case VK_FORMAT_R64G64_SFLOAT                               : return "VK_FORMAT_R64G64_SFLOAT"; break;
    case VK_FORMAT_R64G64B64_UINT                              : return "VK_FORMAT_R64G64B64_UINT"; break;
    case VK_FORMAT_R64G64B64_SINT                              : return "VK_FORMAT_R64G64B64_SINT"; break;
    case VK_FORMAT_R64G64B64_SFLOAT                            : return "VK_FORMAT_R64G64B64_SFLOAT"; break;
    case VK_FORMAT_R64G64B64A64_UINT                           : return "VK_FORMAT_R64G64B64A64_UINT"; break;
    case VK_FORMAT_R64G64B64A64_SINT                           : return "VK_FORMAT_R64G64B64A64_SINT"; break;
    case VK_FORMAT_R64G64B64A64_SFLOAT                         : return "VK_FORMAT_R64G64B64A64_SFLOAT"; break;
    case VK_FORMAT_B10G11R11_UFLOAT_PACK32                     : return "VK_FORMAT_B10G11R11_UFLOAT_PACK32"; break;
    case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32                      : return "VK_FORMAT_E5B9G9R9_UFLOAT_PACK32"; break;
    case VK_FORMAT_D16_UNORM                                   : return "VK_FORMAT_D16_UNORM"; break;
    case VK_FORMAT_X8_D24_UNORM_PACK32                         : return "VK_FORMAT_X8_D24_UNORM_PACK32"; break;
    case VK_FORMAT_D32_SFLOAT                                  : return "VK_FORMAT_D32_SFLOAT"; break;
    case VK_FORMAT_S8_UINT                                     : return "VK_FORMAT_S8_UINT"; break;
    case VK_FORMAT_D16_UNORM_S8_UINT                           : return "VK_FORMAT_D16_UNORM_S8_UINT"; break;
    case VK_FORMAT_D24_UNORM_S8_UINT                           : return "VK_FORMAT_D24_UNORM_S8_UINT"; break;
    case VK_FORMAT_D32_SFLOAT_S8_UINT                          : return "VK_FORMAT_D32_SFLOAT_S8_UINT"; break;
    case VK_FORMAT_BC1_RGB_UNORM_BLOCK                         : return "VK_FORMAT_BC1_RGB_UNORM_BLOCK"; break;
    case VK_FORMAT_BC1_RGB_SRGB_BLOCK                          : return "VK_FORMAT_BC1_RGB_SRGB_BLOCK"; break;
    case VK_FORMAT_BC1_RGBA_UNORM_BLOCK                        : return "VK_FORMAT_BC1_RGBA_UNORM_BLOCK"; break;
    case VK_FORMAT_BC1_RGBA_SRGB_BLOCK                         : return "VK_FORMAT_BC1_RGBA_SRGB_BLOCK"; break;
    case VK_FORMAT_BC2_UNORM_BLOCK                             : return "VK_FORMAT_BC2_UNORM_BLOCK"; break;
    case VK_FORMAT_BC2_SRGB_BLOCK                              : return "VK_FORMAT_BC2_SRGB_BLOCK"; break;
    case VK_FORMAT_BC3_UNORM_BLOCK                             : return "VK_FORMAT_BC3_UNORM_BLOCK"; break;
    case VK_FORMAT_BC3_SRGB_BLOCK                              : return "VK_FORMAT_BC3_SRGB_BLOCK"; break;
    case VK_FORMAT_BC4_UNORM_BLOCK                             : return "VK_FORMAT_BC4_UNORM_BLOCK"; break;
    case VK_FORMAT_BC4_SNORM_BLOCK                             : return "VK_FORMAT_BC4_SNORM_BLOCK"; break;
    case VK_FORMAT_BC5_UNORM_BLOCK                             : return "VK_FORMAT_BC5_UNORM_BLOCK"; break;
    case VK_FORMAT_BC5_SNORM_BLOCK                             : return "VK_FORMAT_BC5_SNORM_BLOCK"; break;
    case VK_FORMAT_BC6H_UFLOAT_BLOCK                           : return "VK_FORMAT_BC6H_UFLOAT_BLOCK"; break;
    case VK_FORMAT_BC6H_SFLOAT_BLOCK                           : return "VK_FORMAT_BC6H_SFLOAT_BLOCK"; break;
    case VK_FORMAT_BC7_UNORM_BLOCK                             : return "VK_FORMAT_BC7_UNORM_BLOCK"; break;
    case VK_FORMAT_BC7_SRGB_BLOCK                              : return "VK_FORMAT_BC7_SRGB_BLOCK"; break;
    case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK                     : return "VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK"; break;
    case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK                      : return "VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK"; break;
    case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK                   : return "VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK"; break;
    case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK                    : return "VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK"; break;
    case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK                   : return "VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK"; break;
    case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK                    : return "VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK"; break;
    case VK_FORMAT_EAC_R11_UNORM_BLOCK                         : return "VK_FORMAT_EAC_R11_UNORM_BLOCK"; break;
    case VK_FORMAT_EAC_R11_SNORM_BLOCK                         : return "VK_FORMAT_EAC_R11_SNORM_BLOCK"; break;
    case VK_FORMAT_EAC_R11G11_UNORM_BLOCK                      : return "VK_FORMAT_EAC_R11G11_UNORM_BLOCK"; break;
    case VK_FORMAT_EAC_R11G11_SNORM_BLOCK                      : return "VK_FORMAT_EAC_R11G11_SNORM_BLOCK"; break;
    case VK_FORMAT_ASTC_4x4_UNORM_BLOCK                        : return "VK_FORMAT_ASTC_4x4_UNORM_BLOCK"; break;
    case VK_FORMAT_ASTC_4x4_SRGB_BLOCK                         : return "VK_FORMAT_ASTC_4x4_SRGB_BLOCK"; break;
    case VK_FORMAT_ASTC_5x4_UNORM_BLOCK                        : return "VK_FORMAT_ASTC_5x4_UNORM_BLOCK"; break;
    case VK_FORMAT_ASTC_5x4_SRGB_BLOCK                         : return "VK_FORMAT_ASTC_5x4_SRGB_BLOCK"; break;
    case VK_FORMAT_ASTC_5x5_UNORM_BLOCK                        : return "VK_FORMAT_ASTC_5x5_UNORM_BLOCK"; break;
    case VK_FORMAT_ASTC_5x5_SRGB_BLOCK                         : return "VK_FORMAT_ASTC_5x5_SRGB_BLOCK"; break;
    case VK_FORMAT_ASTC_6x5_UNORM_BLOCK                        : return "VK_FORMAT_ASTC_6x5_UNORM_BLOCK"; break;
    case VK_FORMAT_ASTC_6x5_SRGB_BLOCK                         : return "VK_FORMAT_ASTC_6x5_SRGB_BLOCK"; break;
    case VK_FORMAT_ASTC_6x6_UNORM_BLOCK                        : return "VK_FORMAT_ASTC_6x6_UNORM_BLOCK"; break;
    case VK_FORMAT_ASTC_6x6_SRGB_BLOCK                         : return "VK_FORMAT_ASTC_6x6_SRGB_BLOCK"; break;
    case VK_FORMAT_ASTC_8x5_UNORM_BLOCK                        : return "VK_FORMAT_ASTC_8x5_UNORM_BLOCK"; break;
    case VK_FORMAT_ASTC_8x5_SRGB_BLOCK                         : return "VK_FORMAT_ASTC_8x5_SRGB_BLOCK"; break;
    case VK_FORMAT_ASTC_8x6_UNORM_BLOCK                        : return "VK_FORMAT_ASTC_8x6_UNORM_BLOCK"; break;
    case VK_FORMAT_ASTC_8x6_SRGB_BLOCK                         : return "VK_FORMAT_ASTC_8x6_SRGB_BLOCK"; break;
    case VK_FORMAT_ASTC_8x8_UNORM_BLOCK                        : return "VK_FORMAT_ASTC_8x8_UNORM_BLOCK"; break;
    case VK_FORMAT_ASTC_8x8_SRGB_BLOCK                         : return "VK_FORMAT_ASTC_8x8_SRGB_BLOCK"; break;
    case VK_FORMAT_ASTC_10x5_UNORM_BLOCK                       : return "VK_FORMAT_ASTC_10x5_UNORM_BLOCK"; break;
    case VK_FORMAT_ASTC_10x5_SRGB_BLOCK                        : return "VK_FORMAT_ASTC_10x5_SRGB_BLOCK"; break;
    case VK_FORMAT_ASTC_10x6_UNORM_BLOCK                       : return "VK_FORMAT_ASTC_10x6_UNORM_BLOCK"; break;
    case VK_FORMAT_ASTC_10x6_SRGB_BLOCK                        : return "VK_FORMAT_ASTC_10x6_SRGB_BLOCK"; break;
    case VK_FORMAT_ASTC_10x8_UNORM_BLOCK                       : return "VK_FORMAT_ASTC_10x8_UNORM_BLOCK"; break;
    case VK_FORMAT_ASTC_10x8_SRGB_BLOCK                        : return "VK_FORMAT_ASTC_10x8_SRGB_BLOCK"; break;
    case VK_FORMAT_ASTC_10x10_UNORM_BLOCK                      : return "VK_FORMAT_ASTC_10x10_UNORM_BLOCK"; break;
    case VK_FORMAT_ASTC_10x10_SRGB_BLOCK                       : return "VK_FORMAT_ASTC_10x10_SRGB_BLOCK"; break;
    case VK_FORMAT_ASTC_12x10_UNORM_BLOCK                      : return "VK_FORMAT_ASTC_12x10_UNORM_BLOCK"; break;
    case VK_FORMAT_ASTC_12x10_SRGB_BLOCK                       : return "VK_FORMAT_ASTC_12x10_SRGB_BLOCK"; break;
    case VK_FORMAT_ASTC_12x12_UNORM_BLOCK                      : return "VK_FORMAT_ASTC_12x12_UNORM_BLOCK"; break;
    case VK_FORMAT_ASTC_12x12_SRGB_BLOCK                       : return "VK_FORMAT_ASTC_12x12_SRGB_BLOCK"; break;
    case VK_FORMAT_G8B8G8R8_422_UNORM                          : return "VK_FORMAT_G8B8G8R8_422_UNORM"; break;
    case VK_FORMAT_B8G8R8G8_422_UNORM                          : return "VK_FORMAT_B8G8R8G8_422_UNORM"; break;
    case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM                   : return "VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM"; break;
    case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM                    : return "VK_FORMAT_G8_B8R8_2PLANE_420_UNORM"; break;
    case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM                   : return "VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM"; break;
    case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM                    : return "VK_FORMAT_G8_B8R8_2PLANE_422_UNORM"; break;
    case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM                   : return "VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM"; break;
    case VK_FORMAT_R10X6_UNORM_PACK16                          : return "VK_FORMAT_R10X6_UNORM_PACK16"; break;
    case VK_FORMAT_R10X6G10X6_UNORM_2PACK16                    : return "VK_FORMAT_R10X6G10X6_UNORM_2PACK16"; break;
    case VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16          : return "VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16"; break;
    case VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16      : return "VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16"; break;
    case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16      : return "VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16"; break;
    case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16  : return "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16"; break;
    case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16   : return "VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16"; break;
    case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16  : return "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16"; break;
    case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16   : return "VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16"; break;
    case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16  : return "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16"; break;
    case VK_FORMAT_R12X4_UNORM_PACK16                          : return "VK_FORMAT_R12X4_UNORM_PACK16"; break;
    case VK_FORMAT_R12X4G12X4_UNORM_2PACK16                    : return "VK_FORMAT_R12X4G12X4_UNORM_2PACK16"; break;
    case VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16          : return "VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16"; break;
    case VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16      : return "VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16"; break;
    case VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16      : return "VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16"; break;
    case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16  : return "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16"; break;
    case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16   : return "VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16"; break;
    case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16  : return "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16"; break;
    case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16   : return "VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16"; break;
    case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16  : return "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16"; break;
    case VK_FORMAT_G16B16G16R16_422_UNORM                      : return "VK_FORMAT_G16B16G16R16_422_UNORM"; break;
    case VK_FORMAT_B16G16R16G16_422_UNORM                      : return "VK_FORMAT_B16G16R16G16_422_UNORM"; break;
    case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM                : return "VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM"; break;
    case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM                 : return "VK_FORMAT_G16_B16R16_2PLANE_420_UNORM"; break;
    case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM                : return "VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM"; break;
    case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM                 : return "VK_FORMAT_G16_B16R16_2PLANE_422_UNORM"; break;
    case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM                : return "VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM"; break;
    case VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG                 : return "VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG"; break;
    case VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG                 : return "VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG"; break;
    case VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG                 : return "VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG"; break;
    case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG                 : return "VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG"; break;
    case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG                  : return "VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG"; break;
    case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG                  : return "VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG"; break;
    case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG                  : return "VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG"; break;
    case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG                  : return "VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG"; break;
  }
  return "<UNKNOWN>";
}

std::string ToStringShort(VkColorSpaceKHR value)
{
  switch (value) {
    default                                          : break;
    case VK_COLOR_SPACE_SRGB_NONLINEAR_KHR           : return "SRGB_NONLINEAR"; break;
    case VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT     : return "DISPLAY_P3_NONLINEAR"; break;
    case VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT     : return "EXTENDED_SRGB_LINEAR"; break;
    case VK_COLOR_SPACE_DCI_P3_LINEAR_EXT            : return "DCI_P3_LINEAR"; break;
    case VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT         : return "DCI_P3_NONLINEAR"; break;
    case VK_COLOR_SPACE_BT709_LINEAR_EXT             : return "BT709_LINEAR"; break;
    case VK_COLOR_SPACE_BT709_NONLINEAR_EXT          : return "BT709_NONLINEAR"; break;
    case VK_COLOR_SPACE_BT2020_LINEAR_EXT            : return "BT2020_LINEAR"; break;
    case VK_COLOR_SPACE_HDR10_ST2084_EXT             : return "HDR10_ST2084"; break;
    case VK_COLOR_SPACE_DOLBYVISION_EXT              : return "DOLBYVISION"; break;
    case VK_COLOR_SPACE_HDR10_HLG_EXT                : return "HDR10_HLG"; break;
    case VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT          : return "ADOBERGB_LINEAR"; break;
    case VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT       : return "ADOBERGB_NONLINEAR"; break;
    case VK_COLOR_SPACE_PASS_THROUGH_EXT             : return "PASS_THROUGH"; break;
    case VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT  : return "EXTENDED_SRGB_NONLINEAR"; break;
  }
  return "<UNKNOWN>";
}

std::string ToString(VkColorSpaceKHR value) 
{
  switch (value) {
    default                                          : break;
    case VK_COLOR_SPACE_SRGB_NONLINEAR_KHR           : return "VK_COLOR_SPACE_SRGB_NONLINEAR_KHR"; break;
    case VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT     : return "VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT"; break;
    case VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT     : return "VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT"; break;
    case VK_COLOR_SPACE_DCI_P3_LINEAR_EXT            : return "VK_COLOR_SPACE_DCI_P3_LINEAR_EXT"; break;
    case VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT         : return "VK_COLOR_SPACE_DCI_P3_NONLINEAR_EXT"; break;
    case VK_COLOR_SPACE_BT709_LINEAR_EXT             : return "VK_COLOR_SPACE_BT709_LINEAR_EXT"; break;
    case VK_COLOR_SPACE_BT709_NONLINEAR_EXT          : return "VK_COLOR_SPACE_BT709_NONLINEAR_EXT"; break;
    case VK_COLOR_SPACE_BT2020_LINEAR_EXT            : return "VK_COLOR_SPACE_BT2020_LINEAR_EXT"; break;
    case VK_COLOR_SPACE_HDR10_ST2084_EXT             : return "VK_COLOR_SPACE_HDR10_ST2084_EXT"; break;
    case VK_COLOR_SPACE_DOLBYVISION_EXT              : return "VK_COLOR_SPACE_DOLBYVISION_EXT"; break;
    case VK_COLOR_SPACE_HDR10_HLG_EXT                : return "VK_COLOR_SPACE_HDR10_HLG_EXT"; break;
    case VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT          : return "VK_COLOR_SPACE_ADOBERGB_LINEAR_EXT"; break;
    case VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT       : return "VK_COLOR_SPACE_ADOBERGB_NONLINEAR_EXT"; break;
    case VK_COLOR_SPACE_PASS_THROUGH_EXT             : return "VK_COLOR_SPACE_PASS_THROUGH_EXT"; break;
    case VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT  : return "VK_COLOR_SPACE_EXTENDED_SRGB_NONLINEAR_EXT"; break;
  }
  return "<UNKNOWN>";
}

std::string ToStringShort(VkPresentModeKHR value)
{
  switch (value) {
    default                                             : break;
    case VK_PRESENT_MODE_IMMEDIATE_KHR                  : return "IMMEDIATE"; break;
    case VK_PRESENT_MODE_MAILBOX_KHR                    : return "MAILBOX"; break;
    case VK_PRESENT_MODE_FIFO_KHR                       : return "FIFO"; break;
    case VK_PRESENT_MODE_FIFO_RELAXED_KHR               : return "FIFO_RELAXED"; break;
    case VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR      : return "SHARED_DEMAND_REFRESH"; break;
    case VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR  : return "SHARED_CONTINUOUS_REFRESH"; break;
  }
  return "<UNKNOWN>";
}

std::string ToString(VkPresentModeKHR value)
{
  switch (value) {
    default                                             : break;
    case VK_PRESENT_MODE_IMMEDIATE_KHR                  : return "VK_PRESENT_MODE_IMMEDIATE_KHR"; break;
    case VK_PRESENT_MODE_MAILBOX_KHR                    : return "VK_PRESENT_MODE_MAILBOX_KHR"; break;
    case VK_PRESENT_MODE_FIFO_KHR                       : return "VK_PRESENT_MODE_FIFO_KHR"; break;
    case VK_PRESENT_MODE_FIFO_RELAXED_KHR               : return "VK_PRESENT_MODE_FIFO_RELAXED_KHR"; break;
    case VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR      : return "VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR"; break;
    case VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR  : return "VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR"; break;
  }
  return "<UNKNOWN>";
}

} // namespace vkex