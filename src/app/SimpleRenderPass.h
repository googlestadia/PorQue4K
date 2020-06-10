/*
 Copyright 2020 Google Inc.

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

#ifndef __SIMPLE_RENDER_PASS_H__
#define __SIMPLE_RENDER_PASS_H__

#include "vkex/RenderPass.h"
#include "vkex/Texture.h"
#include "vkex/View.h"

/** @struct SimpleRenderPass
 *
 */
struct SimpleRenderPass {
  vkex::Texture color_texture;
  vkex::Texture velocity_texture;
  vkex::Texture dsv_texture;
  VkClearColorValue rtv_clear_value;
  VkClearDepthStencilValue dsv_clear_value;
  vkex::RenderTargetView color_rtv;
  vkex::RenderTargetView velocity_rtv;
  vkex::DepthStencilView dsv;
  vkex::RenderPass render_pass;
};

vkex::Result CreateSimpleRenderPass(vkex::Device device, vkex::Queue queue,
                                    uint32_t width, uint32_t height,
                                    VkFormat color_format,
                                    VkFormat depth_format,
                                    SimpleRenderPass* p_simple_pass);

vkex::Result CreateSimpleMSRenderPass(
    vkex::Device device, vkex::Queue queue, uint32_t width, uint32_t height,
    VkFormat color_format, VkFormat depth_format,
    VkSampleCountFlagBits sample_count,
    VkImageCreateFlags extra_depth_usage_flags,
    SimpleRenderPass* p_simple_pass);

#endif  // __SIMPLE_RENDER_PASS_H__
