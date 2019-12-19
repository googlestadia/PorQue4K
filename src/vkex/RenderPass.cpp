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

#include "vkex/RenderPass.h"
#include "vkex/Device.h"
#include "vkex/Image.h"
#include "vkex/ToString.h"
#include "vkex/View.h"

namespace vkex {

// =================================================================================================
// RenderPass
// =================================================================================================
CRenderPass::CRenderPass()
{
}

CRenderPass::~CRenderPass()
{
}

vkex::Result CRenderPass::InitializeColorAttachmentsMultiSample()
{
  // Figure out RTV count
  uint32_t rtv_count = CountU32(m_create_info.rtvs);
  if (m_is_transient) {
    rtv_count = CountU32(m_create_info.transient.rtv_formats);
  }

  for (uint32_t rtv_index = 0; rtv_index < rtv_count; ++rtv_index) {
    vkex::RenderTargetView        rtv               = nullptr;
    VkAttachmentDescriptionFlags  vk_flags          = 0;
    VkFormat                      vk_format         = VK_FORMAT_UNDEFINED;
    VkSampleCountFlagBits         vk_samples        = VK_SAMPLE_COUNT_1_BIT;
    VkAttachmentLoadOp            vk_load_op        = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    VkAttachmentStoreOp           vk_store_op       = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    VkImageLayout                 vk_initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImageLayout                 vk_final_layout   = VK_IMAGE_LAYOUT_UNDEFINED;
    if (m_is_transient) {
      vk_format         = m_create_info.transient.rtv_formats[rtv_index];
      vk_samples        = m_create_info.transient.samples;
      vk_final_layout   = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
    else {
      rtv               = m_create_info.rtvs[rtv_index];
      vk_flags          = rtv->GetAttachmentDescriptionFlags().flags;
      vk_format         = rtv->GetFormat();
      vk_samples        = rtv->GetSamples();
      vk_load_op        = rtv->GetLoadpOp();
      vk_store_op       = rtv->GetStoreOp();
      vk_initial_layout = rtv->GetInitialLayout();
      vk_final_layout   = rtv->GetFinalLayout();
    }

    // Resolve attachment (single sample)
    {
      // Attachment description
      VkAttachmentDescription desc = {};
      desc.flags          = 0;
      desc.format         = vk_format;
      desc.samples        = VK_SAMPLE_COUNT_1_BIT;
      desc.loadOp         = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      desc.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      desc.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      desc.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
      desc.finalLayout    = vk_final_layout;
      m_vk_attachment_descriptions.push_back(desc);

      // Only care about these if this isn't a transient render pass
      if (!m_is_transient) {
        // Attachment reference
        VkAttachmentReference ref = {};
        ref.attachment  = CountU32(m_vk_attachment_descriptions);
        ref.layout      = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        m_vk_resolve_attachment_references.push_back(ref);
      }
    }

    // Color attachment (multi sample)
    {
      // Attachment description
      VkAttachmentDescription desc = {};
      desc.flags          = vk_flags;
      desc.format         = vk_format;
      desc.samples        = vk_samples;
      desc.loadOp         = vk_load_op;
      desc.storeOp        = vk_store_op;
      desc.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      desc.initialLayout  = vk_initial_layout;
      desc.finalLayout    = vk_final_layout;
      m_vk_attachment_descriptions.push_back(desc);

      // Only care about these if this isn't a transient render pass
      if (!m_is_transient) {
        // Attachment reference
        uint32_t attachment_index = CountU32(m_vk_attachment_descriptions) - 1;
        VkAttachmentReference ref = {};
        ref.attachment  = attachment_index;
        ref.layout      = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        m_vk_color_attachment_references.push_back(ref);

        // Framebuffer attachment
        vkex::ImageView image_view = rtv->GetColorImageView();
        if (image_view != nullptr) {
          VkImageView vk_image_view = *image_view;
          m_vk_framebuffer_attachments.push_back(vk_image_view);
        }
      }
    }

    // Clear value
    VkClearValue vk_clear_value = {};
    vk_clear_value.color = rtv->GetClearValue();
    m_clear_values.push_back(vk_clear_value);
  }

  return vkex::Result::Success;
}

vkex::Result CRenderPass::InitializeColorAttachemntsNonMultiSample()
{
  // Figure out RTV count
  uint32_t rtv_count = CountU32(m_create_info.rtvs);
  if (m_is_transient) {
    rtv_count = CountU32(m_create_info.transient.rtv_formats);
  }

  for (uint32_t rtv_index = 0; rtv_index < rtv_count; ++rtv_index) {
    vkex::RenderTargetView        rtv               = nullptr;
    VkAttachmentDescriptionFlags  vk_flags          = 0;
    VkFormat                      vk_format         = VK_FORMAT_UNDEFINED;
    VkSampleCountFlagBits         vk_samples        = VK_SAMPLE_COUNT_1_BIT;
    VkAttachmentLoadOp            vk_load_op        = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    VkAttachmentStoreOp           vk_store_op       = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    VkImageLayout                 vk_initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImageLayout                 vk_final_layout   = VK_IMAGE_LAYOUT_UNDEFINED;
    if (m_is_transient) {
      vk_format         = m_create_info.transient.rtv_formats[rtv_index];
      vk_samples        = m_create_info.transient.samples;
      vk_final_layout   = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
    else {
      rtv               = m_create_info.rtvs[rtv_index];
      vk_flags          = rtv->GetAttachmentDescriptionFlags().flags;
      vk_format         = rtv->GetFormat();
      vk_samples        = rtv->GetSamples();
      vk_load_op        = rtv->GetLoadpOp();
      vk_store_op       = rtv->GetStoreOp();
      vk_initial_layout = rtv->GetInitialLayout();
      vk_final_layout   = rtv->GetFinalLayout();
    }
 
    // Attachment description
    VkAttachmentDescription desc = {};
    desc.flags          = vk_flags;
    desc.format         = vk_format;
    desc.samples        = vk_samples;
    desc.loadOp         = vk_load_op;
    desc.storeOp        = vk_store_op;
    desc.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    desc.initialLayout  = vk_initial_layout;
    desc.finalLayout    = vk_final_layout;
    m_vk_attachment_descriptions.push_back(desc);

    // Attachment reference
    uint32_t attachment_index = CountU32(m_vk_attachment_descriptions) - 1;
    VkAttachmentReference ref = {};
    ref.attachment  = attachment_index;
    ref.layout      = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    m_vk_color_attachment_references.push_back(ref);

    // Only care about these if this isn't a transient render pass
    if (!m_is_transient) {
      // Framebuffer attachment
      vkex::ImageView image_view = rtv->GetColorImageView();
      if (image_view != nullptr) {
        VkImageView vk_image_view = *image_view;
        m_vk_framebuffer_attachments.push_back(vk_image_view);
      }

      // Clear value
      VkClearValue vk_clear_value = {};
      vk_clear_value.color = rtv->GetClearValue();
      m_clear_values.push_back(vk_clear_value);
    }
  }

  return vkex::Result::Success;
}

vkex::Result CRenderPass::InitializeDepthStencilAttachment()
{
  if (!m_has_depth_stencil) {
    return vkex::Result::Success;
  }

  vkex::DepthStencilView        dsv                 = nullptr;
  VkAttachmentDescriptionFlags  vk_flags            = 0;
  VkFormat                      vk_format           = VK_FORMAT_UNDEFINED;
  VkSampleCountFlagBits         vk_samples          = VK_SAMPLE_COUNT_1_BIT;
  VkAttachmentLoadOp            vk_depth_load_op    = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  VkAttachmentStoreOp           vk_depth_store_op   = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  VkAttachmentLoadOp            vk_stencil_load_op  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  VkAttachmentStoreOp           vk_stencil_store_op = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  VkImageLayout                 vk_initial_layout   = VK_IMAGE_LAYOUT_UNDEFINED;
  VkImageLayout                 vk_final_layout     = VK_IMAGE_LAYOUT_UNDEFINED;
  if (m_is_transient) {
    vk_format         = m_create_info.transient.dsv_format;
    vk_samples        = m_create_info.transient.samples;
    vk_final_layout   = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  }
  else {
    dsv                 = m_create_info.dsv;
    vk_flags            = dsv->GetAttachmentDescriptionFlags().flags;
    vk_format           = dsv->GetFormat();
    vk_samples          = dsv->GetSamples();
    vk_depth_load_op    = dsv->GetDepthLoadOp();
    vk_depth_store_op   = dsv->GetDepthStoreOp();
    vk_stencil_load_op  = dsv->GetStencilLoadOp();
    vk_stencil_store_op = dsv->GetStencilStoreOp();
    vk_initial_layout   = dsv->GetInitialLayout();
    vk_final_layout     = dsv->GetFinalLayout();
  }
 
  // Attachment description
  VkAttachmentDescription desc = {};
  desc.flags          = vk_flags;
  desc.format         = vk_format;
  desc.samples        = vk_samples;
  desc.loadOp         = vk_depth_load_op;
  desc.storeOp        = vk_depth_store_op;
  desc.stencilLoadOp  = vk_stencil_load_op;
  desc.stencilStoreOp = vk_stencil_store_op;
  desc.initialLayout  = vk_initial_layout;
  desc.finalLayout    = vk_final_layout;
  m_vk_attachment_descriptions.push_back(desc);

  // Attachment reference
  uint32_t attachment_index = CountU32(m_vk_attachment_descriptions) - 1;
  VkAttachmentReference ref = {};
  ref.attachment  = attachment_index;
  ref.layout      = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  m_vk_depth_stencil_attachment_reference = ref;

  // Only care about these if this isn't a transient render pass
  if (!m_is_transient) {
    // Framebuffer attachment
    vkex::ImageView image_view = dsv->GetDepthStencilImageView();
    if (image_view != nullptr) {
      VkImageView vk_image_view = *image_view;
      m_vk_framebuffer_attachments.push_back(vk_image_view);
    }

    // Clear value
    VkClearValue vk_clear_value = {};
    vk_clear_value.depthStencil = dsv->GetClearValue();
    m_clear_values.push_back(vk_clear_value);
  }

  return vkex::Result::Success;
}

vkex::Result CRenderPass::InitializeAttachments()
{
  // Multi sample 
  if (m_is_multi_sample) {
    vkex::Result vkex_result = InitializeColorAttachmentsMultiSample();
    if (!vkex_result) {
      return vkex_result;
    }
  }
  // Single sample
  else {
    vkex::Result vkex_result = InitializeColorAttachemntsNonMultiSample();
    if (!vkex_result) {
      return vkex_result;
    }
  }

  vkex::Result vkex_result = InitializeDepthStencilAttachment();
  if (!vkex_result) {
    return vkex_result;
  }

  return vkex::Result::Success;
}

vkex::Result CRenderPass::InitializeSubpassDescriptions()
{
  //
  // @TODO: Add more robust subpass handling
  //
  
  // Graphics only bind point for now
  VkSubpassDescription desc = {};
  desc.flags                    = 0;
  desc.pipelineBindPoint        = VK_PIPELINE_BIND_POINT_GRAPHICS;
  desc.inputAttachmentCount     = CountU32(m_vk_input_attachment_references);
  desc.pInputAttachments        = DataPtr(m_vk_input_attachment_references);
  desc.colorAttachmentCount     = CountU32(m_vk_color_attachment_references);
  desc.pColorAttachments        = DataPtr(m_vk_color_attachment_references);
  desc.pResolveAttachments      = DataPtr(m_vk_resolve_attachment_references);
  desc.pDepthStencilAttachment  = m_has_depth_stencil ? &m_vk_depth_stencil_attachment_reference : nullptr;
  desc.preserveAttachmentCount  = 0;
  desc.pPreserveAttachments     = nullptr;
  m_vk_subpass_descriptions.push_back(desc);

  return vkex::Result::Success;
}

vkex::Result CRenderPass::InitializeSubpassDependecies()
{
  // @TODO: Add more robust subpass handling

  VkSubpassDependency dep = {};
  dep.srcSubpass = VK_SUBPASS_EXTERNAL;
  dep.dstSubpass = 0;
  dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dep.srcAccessMask = 0;
  dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  dep.dependencyFlags = 0;
  m_vk_subpass_dependencies.push_back(dep);

/*
  const uint32_t subpass_count = CountU32(m_vk_subpass_descriptions);
  const uint32_t first_subpass_index = 0;
  const uint32_t last_subpass_index = subpass_count - 1;
  for (uint32_t subpass_index = 0 ; subpass_index < subpass_count; ++subpass_index) {
    if (subpass_index == first_subpass_index) {
      VkSubpassDependency dep = {};
      dep.srcSubpass       = VK_SUBPASS_EXTERNAL;
      dep.dstSubpass       = subpass_index;
      dep.srcStageMask     = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
      dep.dstStageMask     = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      dep.srcAccessMask    = VK_ACCESS_MEMORY_READ_BIT;
      dep.dstAccessMask    = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      dep.dependencyFlags  = VK_DEPENDENCY_BY_REGION_BIT;
      m_vk_subpass_dependencies.push_back(dep);
    }
    else {
      assert(false && "not implemented");
    }

    if (subpass_index == last_subpass_index) {
      VkSubpassDependency dep = {};
      dep.srcSubpass       = subpass_index;
      dep.dstSubpass       = VK_SUBPASS_EXTERNAL;
      dep.srcStageMask     = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      dep.dstStageMask     = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
      dep.srcAccessMask    = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      dep.dstAccessMask    = VK_ACCESS_MEMORY_READ_BIT;
      dep.dependencyFlags  = VK_DEPENDENCY_BY_REGION_BIT;
      m_vk_subpass_dependencies.push_back(dep);
    }
  }
*/

  return vkex::Result::Success;
}

vkex::Result CRenderPass::InternalCreate(
  const vkex::RenderPassCreateInfo& create_info,
  const VkAllocationCallbacks*      p_allocator
)
{
  // Copy create info
  m_create_info = create_info;

  // Determine if transient
  m_is_transient = false;
  if (m_create_info.rtvs.empty() && (m_create_info.dsv == nullptr)) {
    m_is_transient = (!m_create_info.transient.rtv_formats.empty()) || 
                     (m_create_info.transient.dsv_format != VK_NULL_HANDLE);
  }

  // Determine if has depth/stencil
  m_has_depth_stencil = (m_create_info.dsv != nullptr);
  if (m_is_transient) {
    m_has_depth_stencil = (m_create_info.transient.dsv_format != VK_FORMAT_UNDEFINED);
  }

  // Determine if multi sample
  {
    if (m_is_transient) {
      m_is_multi_sample = m_create_info.transient.samples > VK_SAMPLE_COUNT_1_BIT;
    }
    else {
      uint32_t rtv_count = CountU32(m_create_info.rtvs);
      for (uint32_t rtv_index = 0; rtv_index < rtv_count; ++rtv_index) {
        vkex::RenderTargetView rtv = m_create_info.rtvs[rtv_index];
        vkex::ImageView resolve = rtv->GetResolveImageView();
        if (resolve != nullptr) {
          m_is_multi_sample = true;
          break;
        }
      }
    }   
  }

  // Attachments
  vkex::Result vkex_result = InitializeAttachments();
  if (!vkex_result) {
    return vkex_result;
  }

  // Render pass
  {
    vkex_result = InitializeSubpassDescriptions();
    if (!vkex_result) {
      return vkex_result;
    }

    vkex_result = InitializeSubpassDependecies();
    if (!vkex_result) {
      return vkex_result;
    }

    // Create info
    {
      m_vk_create_info = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
      m_vk_create_info.flags            = 0;
      m_vk_create_info.attachmentCount  = CountU32(m_vk_attachment_descriptions);
      m_vk_create_info.pAttachments     = DataPtr(m_vk_attachment_descriptions);
      m_vk_create_info.subpassCount     = CountU32(m_vk_subpass_descriptions);
      m_vk_create_info.pSubpasses       = DataPtr(m_vk_subpass_descriptions);
      m_vk_create_info.dependencyCount  = CountU32(m_vk_subpass_dependencies);
      m_vk_create_info.pDependencies    = DataPtr(m_vk_subpass_dependencies);
    }

    // Create render pass
    {
      VkResult vk_result = InvalidValue<VkResult>::Value;
      VKEX_VULKAN_RESULT_CALL(
        vk_result,
        vkex::CreateRenderPass(
          *m_device,
          &m_vk_create_info,
          p_allocator,
          &m_vk_object)
      );
      if (vk_result != VK_SUCCESS) {
        return vkex::Result(vk_result);
      }
    }
  }

  // Frame buffer
  if (!m_vk_framebuffer_attachments.empty()) {
    // Create info
    {
      m_vk_framebuffer_create_info = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
      m_vk_framebuffer_create_info.flags            = 0;
      m_vk_framebuffer_create_info.renderPass       = m_vk_object;
      m_vk_framebuffer_create_info.attachmentCount  = CountU32(m_vk_framebuffer_attachments);
      m_vk_framebuffer_create_info.pAttachments     = DataPtr(m_vk_framebuffer_attachments);
      m_vk_framebuffer_create_info.width            = m_create_info.extent.width;
      m_vk_framebuffer_create_info.height           = m_create_info.extent.height;
      m_vk_framebuffer_create_info.layers           = 1;
    }

    // Create framebuffer
    {
      VkResult vk_result = InvalidValue<VkResult>::Value;
      VKEX_VULKAN_RESULT_CALL(
        vk_result,
        vkex::CreateFramebuffer(
          *m_device,
          &m_vk_framebuffer_create_info,
          p_allocator,
          &m_vk_framebuffer_object)
      );
      if (vk_result != VK_SUCCESS) {
        return vkex::Result(vk_result);
      }
    }
  }

  // Full render area
  {
    m_full_render_area.offset = { 0, 0 };
    m_full_render_area.extent = m_create_info.extent;
  }

  return vkex::Result::Success;
}

vkex::Result CRenderPass::InternalDestroy(const VkAllocationCallbacks* p_allocator)
{
  if (m_vk_framebuffer_object != VK_NULL_HANDLE) {
    vkex::DestroyFramebuffer(
      *m_device,
      m_vk_framebuffer_object,
      p_allocator);

    m_vk_framebuffer_object = VK_NULL_HANDLE;
  }

  if (m_vk_object != VK_NULL_HANDLE) {
    vkex::DestroyRenderPass(
      *m_device,
      m_vk_object,
      p_allocator);

    m_vk_object = VK_NULL_HANDLE;
  }

  return vkex::Result::Success;
}

} // namespace vkex