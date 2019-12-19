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

#include "vkex/Sampler.h"
#include "vkex/Device.h"
#include "vkex/ToString.h"

namespace vkex {

// =================================================================================================
// Sampler
// =================================================================================================
CSampler::CSampler()
{
}

CSampler::~CSampler()
{
}

vkex::Result CSampler::InternalCreate(
  const vkex::SamplerCreateInfo&  create_info,
  const VkAllocationCallbacks*      p_allocator
)
{
  // Copy create info
  m_create_info = create_info;

  // Create Vulkan sampler
  {
    // Create info
    m_vk_create_info = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
    m_vk_create_info.flags                    = m_create_info.flags;
    m_vk_create_info.magFilter                = m_create_info.mag_filter;
    m_vk_create_info.minFilter                = m_create_info.min_filter;
    m_vk_create_info.mipmapMode               = m_create_info.mipmap_mode;
    m_vk_create_info.addressModeU             = m_create_info.address_mode_u;
    m_vk_create_info.addressModeV             = m_create_info.address_mode_v;
    m_vk_create_info.addressModeW             = m_create_info.address_mode_w;
    m_vk_create_info.mipLodBias               = m_create_info.mip_lod_bias;
    m_vk_create_info.anisotropyEnable         = m_create_info.anisotropy_enable;
    m_vk_create_info.maxAnisotropy            = m_create_info.max_anisotropy;
    m_vk_create_info.compareEnable            = m_create_info.compare_enable;
    m_vk_create_info.compareOp                = m_create_info.compare_op;
    m_vk_create_info.minLod                   = m_create_info.min_lod;
    m_vk_create_info.maxLod                   = m_create_info.max_lod;
    m_vk_create_info.borderColor              = m_create_info.border_color;
    m_vk_create_info.unnormalizedCoordinates  = m_create_info.unnormalized_coordinates;
    // Create Vulkan sampler
    VkResult vk_result = InvalidValue<VkResult>::Value;
    VKEX_VULKAN_RESULT_CALL(
      vk_result,
      vkex::CreateSampler(
        *m_device,
        &m_vk_create_info,
        p_allocator,
        &m_vk_object)
    );
    if (vk_result != VK_SUCCESS) {
      return vkex::Result(vk_result);
    }
  }
  
  return vkex::Result::Success;
}

vkex::Result CSampler::InternalDestroy(const VkAllocationCallbacks* p_allocator)
{
  if (m_vk_object != VK_NULL_HANDLE) {
    vkex::DestroySampler(
      *m_device,
      m_vk_object,
      p_allocator);

    m_vk_object = VK_NULL_HANDLE;
  }
  return vkex::Result::Success;
}

} // namespace vkex