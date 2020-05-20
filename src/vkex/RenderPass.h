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

#ifndef __VKEX_RENDER_PASS_H__
#define __VKEX_RENDER_PASS_H__

#include <vkex/Config.h>
#include <vkex/Traits.h>
#include <vkex/View.h>
#include <vkex/VulkanUtil.h>

namespace vkex {

// =================================================================================================
// RenderPass
// =================================================================================================

/** @struct RenderPassCreateInfo 
 *
 * To create a usable render pass, use \b rtvs and \b dsv. 
 *
 * To create a transient render pass, use \b transient.rtv_formats,
 * \b transient.dsv_format, and \b transient.sample_count. If 
 * \b sample_count is zero, a sample count of 1 will be used.
 *
 * If either \b rtvs or \b dsv is present, transient will be ignored.
 *
 * Q: What are transient render passes?
 * A: Transient render passes are temporary render pass objects 
 *    without a framebuffer object. Transient render passes are
 *    used when a 'compatible' VkRenderPass object is needed 
 *    to create another Vulkan object. The transient render pass
 *    is destroyed once the desired Vulkan object is created. 
 *    A 'usable render pass matching the compatible requirements
 *    of the transient render pass is later created for actual
 *    use with the Vulkan object.
 * 
 */
struct RenderPassCreateInfo {
  vkex::RenderPassCreateFlags         flags;
  std::vector<vkex::RenderTargetView> rtvs;
  vkex::DepthStencilView              dsv;
  VkExtent2D                          extent;

  struct {
    std::vector<VkFormat>             rtv_formats;
    VkFormat                          dsv_format;
    VkSampleCountFlagBits             samples;
  } transient;
};

/** @class IRenderPass
 *
 */ 
class CRenderPass : public IDeviceObject {
public:
  CRenderPass();
  ~CRenderPass();

  /** @fn operator VkRenderPass()
   *
   */
  operator VkRenderPass() const { 
    return m_vk_object; 
  }

  /** @fn GetVkObject
   *
   */
  VkRenderPass GetVkObject() const { 
    return m_vk_object; 
  }

  /** @fn IsTransient
   *
   */
  bool IsTransient() const {
    return m_is_transient;
  }

  /** @fn IsMultiSample
   *
   */
  bool IsMultiSample() const {
    return m_is_multi_sample;
  }

  /** @fn HasDepthStencil
   *
   */
  bool HasDepthStencil() const {
    return m_has_depth_stencil;
  }

  /** @fn GetVkFramebufferObject
   *
   */
  VkFramebuffer GetVkFramebufferObject() const { 
    return m_vk_framebuffer_object; 
  }

  const std::vector<VkClearValue>& GetClearValues() const {
    return m_clear_values;
  }

  /** @fn GetFullRenderArea
   *
   */
  const VkRect2D& GetFullRenderArea() const {
    return m_full_render_area;
  }

  /** @fn GetRtvs
   *
   */
  const std::vector<vkex::RenderTargetView>& GetRtvs() const {
    return m_create_info.rtvs;
  }

  VkAttachmentReference GetDepthStencilAttachmentReference() const {
      return m_vk_depth_stencil_attachment_reference;
  }

private:
  friend class CDevice;
  friend class IObjectStorageFunctions;

  /** @fn InitializeColorAttachmentsMultiSample
   *
   */
  vkex::Result InitializeColorAttachmentsMultiSample();

  /** @fn InitializeColorAttachemntsNonMultiSample
   *
   */
  vkex::Result InitializeColorAttachemntsNonMultiSample();

  /** @fn InitializeDepthStencilAttachment
   *
   */
  vkex::Result InitializeDepthStencilAttachment();

  /** @fn InitializeAttachments
   *
   */
  vkex::Result InitializeAttachments();

  /** @fn InitializeSubpassDescriptions
   *
   */
  vkex::Result InitializeSubpassDescriptions();

  /** @fn InitializeSubpassDependecies
   *
   */
  vkex::Result InitializeSubpassDependecies();

  /** @fn InternalCreate
   *
   */
  vkex::Result InternalCreate(
    const vkex::RenderPassCreateInfo& create_info,
    const VkAllocationCallbacks*      p_allocator
  );

  /** @fn InternalDestroy
   *
   */
  vkex::Result InternalDestroy(const VkAllocationCallbacks* p_allocator);

private:
  vkex::RenderPassCreateInfo            m_create_info = {};
  bool                                  m_is_transient = false;
  bool                                  m_is_multi_sample = false;
  bool                                  m_has_depth_stencil = false;
  VkRenderPassCreateInfo                m_vk_create_info = {};
  VkRenderPass                          m_vk_object = VK_NULL_HANDLE;
  std::vector<VkAttachmentDescription>  m_vk_attachment_descriptions;
  std::vector<VkAttachmentReference>    m_vk_input_attachment_references;
  std::vector<VkAttachmentReference>    m_vk_color_attachment_references;
  std::vector<VkAttachmentReference>    m_vk_resolve_attachment_references;
  VkAttachmentReference                 m_vk_depth_stencil_attachment_reference;
  std::vector<uint32_t>                 m_vk_preserve_attachments;
  std::vector<VkSubpassDescription>     m_vk_subpass_descriptions;
  std::vector<VkSubpassDependency>      m_vk_subpass_dependencies;

  std::vector<VkImageView>              m_vk_framebuffer_attachments;
  VkFramebufferCreateInfo               m_vk_framebuffer_create_info = {};
  VkFramebuffer                         m_vk_framebuffer_object = VK_NULL_HANDLE;

  std::vector<VkClearValue>             m_clear_values;
  VkRect2D                              m_full_render_area = {};
};

} // namespace vkex

#endif // __VKEX_RENDER_PASS_H__