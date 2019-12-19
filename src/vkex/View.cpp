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

#include <vkex/View.h>
#include <vkex/Device.h>
#include <vkex/Image.h>

namespace vkex {

// =================================================================================================
// RenderTargetView
// =================================================================================================
CRenderTargetView::CRenderTargetView()
{
}

CRenderTargetView::~CRenderTargetView()
{
}

vkex::Result CRenderTargetView::InternalCreate(
  const vkex::RenderTargetViewCreateInfo& create_info,
  const VkAllocationCallbacks*            p_allocator
)
{
  // Copy create info
  m_create_info = create_info;

  // Check resource
  if (m_create_info.attachment == nullptr) {
    return vkex::Result::ErrorUnexpectedNullPointer;
  }

  return vkex::Result::Success;
}

vkex::Result CRenderTargetView::InternalDestroy(const VkAllocationCallbacks* p_allocator)
{
  return vkex::Result::Success;
}

vkex::ImageView CRenderTargetView::GetResource() const
{
  return m_create_info.attachment;
}

// =================================================================================================
// DepthStencilView
// =================================================================================================
CDepthStencilView::CDepthStencilView()
{
}

CDepthStencilView::~CDepthStencilView()
{
}

vkex::Result CDepthStencilView::InternalCreate(
  const vkex::DepthStencilViewCreateInfo& create_info,
  const VkAllocationCallbacks*            p_allocator
)
{
  // Copy create info
  m_create_info = create_info;

  // Check resource
  if (m_create_info.attachment == nullptr) {
    return vkex::Result::ErrorUnexpectedNullPointer;
  }

  return vkex::Result::Success;
}

vkex::Result CDepthStencilView::InternalDestroy(const VkAllocationCallbacks* p_allocator)
{
  return vkex::Result::Success;
}

vkex::ImageView CDepthStencilView::GetResource() const
{
  return m_create_info.attachment;
}

} // namespace vkex