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

#ifndef __VKEX_SAMPLER_H__
#define __VKEX_SAMPLER_H__

#include <vkex/Config.h>
#include <vkex/Traits.h>
#include <vkex/VulkanUtil.h>

namespace vkex {

// =================================================================================================
// Sampler
// =================================================================================================

/** @struct SamplerCreateInfo 
 *
 */
struct SamplerCreateInfo {
  vkex::SamplerCreateFlags  flags;
  VkFilter                  mag_filter;
  VkFilter                  min_filter;
  VkSamplerMipmapMode       mipmap_mode;
  VkSamplerAddressMode      address_mode_u;
  VkSamplerAddressMode      address_mode_v;
  VkSamplerAddressMode      address_mode_w;
  float                     mip_lod_bias;
  VkBool32                  anisotropy_enable;
  float                     max_anisotropy;
  VkBool32                  compare_enable;
  VkCompareOp               compare_op;
  float                     min_lod;
  float                     max_lod;
  VkBorderColor             border_color;
  VkBool32                  unnormalized_coordinates;
};

/** @class ISampler
 *
 */ 
class CSampler : public IDeviceObject {
public:
  CSampler();
  ~CSampler();

  /** @fn operator VkSampler()
   *
   */
  operator VkSampler() const { 
    return m_vk_object; 
  }

  /** @fn GetVkObject
   *
   */
  VkSampler GetVkObject() const { 
    return m_vk_object; 
  }

private:
  friend class CDevice;
  friend class IObjectStorageFunctions;

  /** @fn InternalCreate
   *
   */
  vkex::Result InternalCreate(
    const vkex::SamplerCreateInfo&  create_info,
    const VkAllocationCallbacks*    p_allocator
  );

  /** @fn InternalDestroy
   *
   */
  vkex::Result InternalDestroy(const VkAllocationCallbacks* p_allocator);

private:
  vkex::SamplerCreateInfo m_create_info = {};
  VkSamplerCreateInfo     m_vk_create_info = {};
  VkSampler               m_vk_object = VK_NULL_HANDLE;
};

} // namespace vkex

#endif // __VKEX_SAMPLER_H__