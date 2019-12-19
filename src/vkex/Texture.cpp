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

#include <vkex/Texture.h>
#include <vkex/Device.h>

namespace vkex {

CTexture::CTexture()
{
}

CTexture::~CTexture()
{
}

vkex::Result CTexture::InternalCreate(
  const vkex::TextureCreateInfo&  create_info,
  const VkAllocationCallbacks*    p_allocator
)
{
  // Copy create info
  m_create_info = create_info;

  // Image
  if (m_create_info.existing_image != nullptr) {
    m_image = m_create_info.existing_image;
    m_create_info.image.create_flags          = m_image->GetCreateFlags();
    m_create_info.image.image_type            = m_image->GetImageType();
    m_create_info.image.format                = m_image->GetFormat();
    m_create_info.image.extent                = m_image->GetExtent();
    m_create_info.image.mip_levels            = m_image->GetMipLevels();
    m_create_info.image.array_layers          = m_image->GetArrayLayers();
    m_create_info.image.samples               = m_image->GetSamples();
    m_create_info.image.tiling                = m_image->GetTiling();
    m_create_info.image.usage_flags           = m_image->GetUsageFlags();
    m_create_info.image.sharing_mode          = m_image->GetSharingMode();
    m_create_info.image.queue_family_indices  = m_image->GetQueueFamilyIndices();
    m_create_info.image.initial_layout        = m_image->GetInitialLayout();
    m_create_info.image.committed             = m_image->IsCommited();
    m_create_info.image.host_visible          = m_image->IsHostVisible();
    m_create_info.image.device_local          = m_image->IsDeviceLocal();
    m_create_info.image.memory_pool           = m_image->GetMemoryPool();
  }
  else {
    // Fill in defaults
    {
      // Mip levels
      if (m_create_info.image.mip_levels == 0) {
        m_create_info.image.mip_levels = 1;
      }
      // Array layers
      if (m_create_info.image.array_layers == 0) {
        m_create_info.image.array_layers = 1;
      }
      // Samples
      if (m_create_info.image.samples == 0) {
        m_create_info.image.samples = VK_SAMPLE_COUNT_1_BIT;
      }
      // Tiling - force to LINEAR if host visible
      //
      // NOTE: Still may produce an error if format does not
      //       support linear tiling.
      //
      if (m_create_info.image.host_visible) {
        m_create_info.image.tiling = VK_IMAGE_TILING_LINEAR;
      }
      // Usage flags - turn everything besides storage on. Will preserve the
      // value for storage that was passed in.
      //
      {
        vkex::ImageAspectFlags image_aspect = vkex::DetermineAspectMask(m_create_info.image.format);
        m_create_info.image.usage_flags.bits.transfer_src             = true;
        m_create_info.image.usage_flags.bits.transfer_dst             = true;
        m_create_info.image.usage_flags.bits.sampled                  = true;
        m_create_info.image.usage_flags.bits.color_attachment         = image_aspect.bits.color_bit ? true : false;
        m_create_info.image.usage_flags.bits.depth_stencil_attachment = (image_aspect.bits.depth_bit || image_aspect.bits.stencil_bit) ? true : false;
        m_create_info.image.usage_flags.bits.transient_attachment     = false;
        m_create_info.image.usage_flags.bits.input_attachment         = true;
      }
    }

    vkex::ImageCreateInfo image_create_info = {};
    image_create_info.create_flags          = m_create_info.image.create_flags;
    image_create_info.image_type            = m_create_info.image.image_type;
    image_create_info.format                = m_create_info.image.format;
    image_create_info.extent                = m_create_info.image.extent;
    image_create_info.mip_levels            = m_create_info.image.mip_levels;
    image_create_info.array_layers          = m_create_info.image.array_layers;
    image_create_info.samples               = m_create_info.image.samples;
    image_create_info.tiling                = m_create_info.image.tiling;
    image_create_info.usage_flags           = m_create_info.image.usage_flags;
    image_create_info.sharing_mode          = m_create_info.image.sharing_mode;
    image_create_info.queue_family_indices  = m_create_info.image.queue_family_indices;
    image_create_info.initial_layout        = m_create_info.image.initial_layout;
    image_create_info.committed             = m_create_info.image.committed;
    image_create_info.host_visible          = m_create_info.image.host_visible;
    image_create_info.device_local          = m_create_info.image.device_local;
    image_create_info.memory_pool           = m_create_info.image.memory_pool;
    vkex::Result vkex_result = GetDevice()->CreateImage(image_create_info, &m_image, p_allocator);
    if (!vkex_result) {
      return vkex_result;
    }

    m_owns_image = true;
  }

  // Image view
  {
    if (m_create_info.view.derive_from_image) {
      switch (m_create_info.image.image_type) {
        case VK_IMAGE_TYPE_1D: m_create_info.view.view_type = VK_IMAGE_VIEW_TYPE_1D; break;
        case VK_IMAGE_TYPE_2D: m_create_info.view.view_type = VK_IMAGE_VIEW_TYPE_2D; break;
        case VK_IMAGE_TYPE_3D: m_create_info.view.view_type = VK_IMAGE_VIEW_TYPE_3D; break;
      }

      m_create_info.view.format             = m_create_info.image.format;
      m_create_info.view.samples            = m_create_info.image.samples;
      m_create_info.view.components         = vkex::ComponentMappingRGBA();
      m_create_info.view.subresource_range  = vkex::ImageSubresourceRange(m_image->GetAspectFlags());
    }

    vkex::ImageViewCreateInfo image_view_create_info = {};
    image_view_create_info.create_flags       = m_create_info.view.create_flags;
    image_view_create_info.image              = m_image;
    image_view_create_info.view_type          = m_create_info.view.view_type;
    image_view_create_info.format             = m_create_info.view.format;
    image_view_create_info.samples            = m_create_info.view.samples;
    image_view_create_info.components         = m_create_info.view.components;
    image_view_create_info.subresource_range  = m_create_info.view.subresource_range;
    vkex::Result vkex_result = GetDevice()->CreateImageView(image_view_create_info, &m_image_view, p_allocator);
    if (!vkex_result) {
      return vkex_result;
    }
  }

  return vkex::Result::Success;
}

vkex::Result CTexture::InternalDestroy(const VkAllocationCallbacks* p_allocator)
{
  if (m_owns_image) {
    if (m_image != nullptr) {
      vkex::Result vkex_result = GetDevice()->DestroyImage(m_image, p_allocator);
      if (!vkex_result) {
        // @TOD: Issue warning
      }
      m_image = nullptr;
    }
    m_owns_image = false;
  }

  if (m_image_view != nullptr) {
      vkex::Result vkex_result = GetDevice()->DestroyImageView(m_image_view, p_allocator);
      if (!vkex_result) {
        // @TOD: Issue warning
      }
      m_image_view = nullptr;
  }

  return vkex::Result::Success;
}

vkex::Image CTexture::GetImage() const
{
  return m_image;
}

vkex::ImageView CTexture::GetImageView() const
{
  return m_image_view;
}

VkImageType CTexture::GetImageType() const
{
  return m_image->GetImageType();
}

VkFormat CTexture::GetFormat() const
{
  return m_image->GetFormat();
}

uint32_t CTexture::GetMipLevels() const
{
  return m_image->GetMipLevels();
}

uint32_t CTexture::GetArrayLayers() const
{
  return m_image->GetArrayLayers();
}

const VkExtent3D& CTexture::GetExtent() const
{
  return m_image->GetExtent();
}

VkSampleCountFlagBits CTexture::GetSamples() const
{
  return m_image->GetSamples();
}

const vkex::ImageUsageFlags CTexture::GetUsageFlags() const
{
  return m_image->GetUsageFlags();
}

bool CTexture::IsCommited() const
{
  return m_image->IsCommited();
}

bool CTexture::IsHostVisible() const
{
  return m_image->IsHostVisible();
}

const vkex::ImageAspectFlags& CTexture::GetAspectFlags() const
{
  return m_image->GetAspectFlags();
}

VkResult CTexture::AllocateMemory(
  VmaMemoryUsage  usage, 
  VmaPool         memory_pool
)
{
  return m_image->AllocateMemory(usage, memory_pool);
}

void CTexture::FreeMemory()
{
  m_image->FreeMemory();
}

bool CTexture::IsMemoryAllocated() const
{
  return m_image->IsMemoryAllocated();
}

VkResult CTexture::BindMemory()
{
  return m_image->BindMemory();
}

VkResult CTexture::MapMemory(void** pp_mapped_address)
{
  return m_image->MapMemory(pp_mapped_address);
}

void CTexture::UnmapMemory()
{
  return m_image->UnmapMemory();
}

bool CTexture::IsMemoryMapped() const
{
  return m_image->IsMemoryMapped();
}

VkDeviceSize CTexture::GetMemoryOffset() const
{
  return m_image->GetMemoryOffset();
}

VkDeviceSize CTexture::GetMemorySize() const
{
  return m_image->GetMemorySize();
}

vkex::Result CTexture::CopyToMipLevel(
  uint32_t       mip_level,
  uint32_t       array_layer,
  uint32_t       src_row_stride,
  uint32_t       src_height,
  const uint8_t* p_src_data)
{
  bool is_host_visible = m_image->IsHostVisible();
  VKEX_ASSERT_MSG(is_host_visible, "Texture resource must be host visible for direct copy!");
  if (!is_host_visible) {
    return vkex::Result::ErrorResourceIsNotHostVisible;
  }

  VkImageSubresource subresource         = {};
  subresource.aspectMask                 = m_image->GetAspectFlags().flags;
  subresource.mipLevel                   = mip_level;
  subresource.arrayLayer                 = array_layer;
  VkSubresourceLayout subresource_layout = {};
  vkex::GetImageSubresourceLayout(*(m_image->GetDevice()), *m_image, &subresource, &subresource_layout);

  const uint32_t image_height   = m_image->GetExtent().height;
  const uint32_t dst_height     = image_height >> mip_level;
  const uint32_t dst_row_stride = static_cast<uint32_t>(subresource_layout.rowPitch);
  src_height                    = std::min<uint32_t>(src_height, dst_height);
  src_row_stride                = std::min<uint32_t>(src_row_stride, dst_row_stride);

  // Get host visible address
  bool     is_mapped        = IsMemoryMapped();
  bool     unmap_after_copy = !is_mapped;
  void*    p_mapped_address = nullptr;
  VkResult vk_result        = MapMemory(&p_mapped_address);
  if (vk_result != VK_SUCCESS) {
    return vkex::Result(vk_result);
  }
  // Copy
  {
    const uint8_t* p_src = p_src_data;
    uint8_t*       p_dst = static_cast<uint8_t*>(p_mapped_address) + subresource_layout.offset;
    for (uint32_t y = 0; y < src_height; ++y) {
      std::memcpy(p_dst, p_src, src_row_stride);
      p_dst += dst_row_stride;
      p_src += src_row_stride;
    }
  }
  // Unmap if needed
  if (unmap_after_copy) {
    UnmapMemory();
  }

  return vkex::Result::Success;
}

} // namespace vkex