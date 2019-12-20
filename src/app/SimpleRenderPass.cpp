#include "SimpleRenderPass.h"
#include "AssetUtil.h"

#include "vkex/Device.h"

#include <map>

vkex::Result CreateSimpleRenderPassColorAttachmentOnly(
  vkex::Device      device,
  uint32_t          width,
  uint32_t          height,
  VkFormat          format,
  SimpleRenderPass* p_simple_pass
)
{
  SimpleRenderPass simple_pass = {};
  simple_pass.rtv_clear_value = { 0.0f, 0.0f, 0.0f, 0.0f };
  // Image
  {
    vkex::TextureCreateInfo create_info = {};
    // Color
    create_info.image.image_type                        = VK_IMAGE_TYPE_2D;
    create_info.image.format                            = format;
    create_info.image.extent                            = { width, height, 1 };
    create_info.image.mip_levels                        = 1;
    create_info.image.array_layers                      = 1;
    create_info.image.samples                           = VK_SAMPLE_COUNT_1_BIT;
    create_info.image.tiling                            = VK_IMAGE_TILING_OPTIMAL;
    create_info.image.usage_flags.bits.color_attachment = true;
    create_info.image.usage_flags.bits.transfer_src     = true;
    create_info.image.sharing_mode                      = VK_SHARING_MODE_EXCLUSIVE;
    create_info.image.initial_layout                    = VK_IMAGE_LAYOUT_UNDEFINED;
    create_info.image.committed                         = true;
    create_info.image.host_visible                      = false;
    create_info.image.device_local                      = true;
    create_info.view.derive_from_image                  = true;
    vkex::Result result = device->CreateTexture(create_info, &simple_pass.rtv_texture);
    if (!result) {
      return result;
    }
  }
  // RTV
  {
    vkex::RenderTargetViewCreateInfo create_info ={};
    create_info.format            = simple_pass.rtv_texture->GetFormat();
    create_info.samples           = simple_pass.rtv_texture->GetSamples();
    create_info.load_op           = VK_ATTACHMENT_LOAD_OP_CLEAR;
    create_info.store_op          = VK_ATTACHMENT_STORE_OP_STORE;
    create_info.final_layout      = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    create_info.clear_value       = { 0.0f, 0.0f, 0.0f, 0.0f };
    create_info.attachment        = simple_pass.rtv_texture->GetImageView();
    vkex::Result result = device->CreateRenderTargetView(create_info, &simple_pass.rtv);
    if (!result) {
      return result;
    }
  }
  // Render pass
  {
    vkex::RenderPassCreateInfo create_info = {};
    create_info.flags   = 0;
    create_info.rtvs    = { simple_pass.rtv };
    create_info.extent  = { width, height };
    vkex::Result result = device->CreateRenderPass(create_info, &simple_pass.render_pass);
    if (!result) {
      return result;
    }
  }
  // Assign output
  *p_simple_pass = simple_pass;
  // Done
  return vkex::Result::Success;
}