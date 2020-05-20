#ifndef __SIMPLE_RENDER_PASS_H__
#define __SIMPLE_RENDER_PASS_H__

#include "vkex/RenderPass.h"
#include "vkex/Texture.h"
#include "vkex/View.h"

/** @struct SimpleRenderPass
 *
 */
struct SimpleRenderPass {
  vkex::Texture rtv_texture;
  vkex::Texture dsv_texture;
  VkClearColorValue rtv_clear_value;
  VkClearDepthStencilValue dsv_clear_value;
  vkex::RenderTargetView rtv;
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
