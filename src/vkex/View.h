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

#ifndef __VKEX_VIEW_H__
#define __VKEX_VIEW_H__

#include <vkex/Config.h>
#include <vkex/Traits.h>
#include <vkex/VulkanUtil.h>

namespace vkex {

// =================================================================================================
// RenderTargetView
// =================================================================================================
struct RenderTargetViewCreateInfo {
  vkex::AttachmentDescriptionFlags  attachment_description_flags;
  VkFormat                          format;
  VkSampleCountFlagBits             samples;
  VkAttachmentLoadOp                load_op;
  VkAttachmentStoreOp               store_op;
  VkImageLayout                     initial_layout;
  VkImageLayout                     render_layout;
  VkImageLayout                     final_layout;
  VkClearColorValue                 clear_value;
  vkex::ImageView                   attachment;
  vkex::ImageView                   resolve;
};

class CRenderTargetView : public IDeviceObject {
public:
  CRenderTargetView();
  ~CRenderTargetView();

  /** @fn GetResource
   *
   */
  vkex::ImageView GetResource() const;

  const AttachmentDescriptionFlags& GetAttachmentDescriptionFlags() const { return m_create_info.attachment_description_flags; }

  //! @fn GetFormat
  VkFormat GetFormat() const { return m_create_info.format; }

  //! @fn GetSamples
  VkSampleCountFlagBits GetSamples() const { return m_create_info.samples; }

  //! @fn GetLoadpOp
  VkAttachmentLoadOp GetLoadpOp() const { return m_create_info.load_op; }

  //! @fn GetStoreOp
  VkAttachmentStoreOp GetStoreOp() const { return m_create_info.store_op; }

  //! @fn GetInitialLayout
  VkImageLayout GetInitialLayout() const { return m_create_info.initial_layout; }

  //! @fn GetRenderLayout
  VkImageLayout GetRenderLayout() const { return m_create_info.render_layout; }

  //! @fn GetFinalLayout
  VkImageLayout GetFinalLayout() const { return m_create_info.final_layout; }

  //! @fn GetClearValue
  const VkClearColorValue& GetClearValue() const { return m_create_info.clear_value; }

  //! @fn GetColorImageView
  vkex::ImageView GetColorImageView() const { return m_create_info.attachment; }

  //! @fn GetResolveImageView
  vkex::ImageView GetResolveImageView() const { return m_create_info.resolve; }


private:
  friend class CDevice;
  friend class IObjectStorageFunctions;

  /** @fn InternalCreate
   *
   */
  vkex::Result InternalCreate(
    const vkex::RenderTargetViewCreateInfo& create_info,
    const VkAllocationCallbacks*            p_allocator
  );

  /** @fn InternalDestroy
   *
   */
  vkex::Result InternalDestroy(const VkAllocationCallbacks* p_allocator);

private:
  vkex::RenderTargetViewCreateInfo  m_create_info = {};
};

// =================================================================================================
// DepthStencilView
// =================================================================================================
struct DepthStencilViewCreateInfo {
  vkex::AttachmentDescriptionFlags  attachment_description_flags;
  VkFormat                          format;
  VkSampleCountFlagBits             samples;
  VkAttachmentLoadOp                depth_load_op;
  VkAttachmentStoreOp               depth_store_op;
  VkAttachmentLoadOp                stencil_load_op;
  VkAttachmentStoreOp               stencil_store_op;
  VkImageLayout                     initial_layout;
  VkImageLayout                     render_layout;
  VkImageLayout                     final_layout;
  VkClearDepthStencilValue          clear_value;
  vkex::ImageView                   attachment;
};

class CDepthStencilView : public IDeviceObject {
public:
  CDepthStencilView();
  ~CDepthStencilView();

  /** @fn GetResource
   *
   */
  vkex::ImageView GetResource() const;

  const AttachmentDescriptionFlags& GetAttachmentDescriptionFlags() const { return m_create_info.attachment_description_flags; }

  //! @fn GetFormat
  VkFormat GetFormat() const { return m_create_info.format; }

  //! @fn GetSamples
  VkSampleCountFlagBits GetSamples() const { return m_create_info.samples; }

  //! @fn GetDepthLoadpOp
  VkAttachmentLoadOp GetDepthLoadOp() const { return m_create_info.depth_load_op; }

  //! @fn GetDepthStoreOp
  VkAttachmentStoreOp GetDepthStoreOp() const { return m_create_info.depth_store_op; }

  //! @fn GetStencilLoadpOp
  VkAttachmentLoadOp GetStencilLoadOp() const { return m_create_info.stencil_load_op; }

  //! @fn GetStencilStoreOp
  VkAttachmentStoreOp GetStencilStoreOp() const { return m_create_info.stencil_store_op; }

  //! @fn GetInitialLayout
  VkImageLayout GetInitialLayout() const { return m_create_info.initial_layout; }

  //! @fn GetRenderLayout
  VkImageLayout GetRenderLayout() const { return m_create_info.render_layout; }

  //! @fn GetFinalLayout
  VkImageLayout GetFinalLayout() const { return m_create_info.final_layout; }

  //! @fn GetClearValue
  const VkClearDepthStencilValue& GetClearValue() const { return m_create_info.clear_value; }

  //! @fn GetColorImageView
  vkex::ImageView GetDepthStencilImageView() const { return m_create_info.attachment; }

private:
  friend class CDevice;
  friend class IObjectStorageFunctions;

  /** @fn InternalCreate
   *
   */
  vkex::Result InternalCreate(
    const vkex::DepthStencilViewCreateInfo& create_info,
    const VkAllocationCallbacks*            p_allocator
  );

  /** @fn InternalDestroy
   *
   */
  vkex::Result InternalDestroy(const VkAllocationCallbacks* p_allocator);

private:
  vkex::DepthStencilViewCreateInfo  m_create_info = {};
};

} // namespace vkex

#endif // __VKEX_VIEW_H__