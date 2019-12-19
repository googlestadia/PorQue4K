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

#include <vkex/Swapchain.h>
#include <vkex/Instance.h>
#include <vkex/Queue.h>
#include <vkex/ToString.h>

namespace vkex {

// =================================================================================================
// Surface
// =================================================================================================
CSurface::CSurface()
{
}

CSurface::~CSurface()
{
}

vkex::Result CSurface::InternalCreate(
  const vkex::SurfaceCreateInfo&  create_info,
  const VkAllocationCallbacks*    p_allocator
)
{
  // Copy create info
  m_create_info = create_info;

  // Surface
  {
#if defined(VKEX_WIN32)
    m_vk_create_info = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
    m_vk_create_info.hinstance = create_info.hinstance;
    m_vk_create_info.hwnd = create_info.hwnd;
    VkResult vk_result = vkex::CreateWin32SurfaceKHR(
      *m_instance,
      &m_vk_create_info,
      p_allocator,
      &m_vk_object);
#elif defined(VKEX_LINUX_GGP)
    m_vk_create_info = { VK_STRUCTURE_TYPE_YETI_SURFACE_CREATE_INFO_GOOGLE };
    m_vk_create_info.streamIndex = 0;
    VkResult vk_result = vkex::CreateYetiSurfaceGOOGLE(
      *m_instance,
      &m_vk_create_info,
      p_allocator,
      &m_vk_object);
#elif defined(VKEX_LINUX)
    m_vk_create_info = { VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR };
    m_vk_create_info.connection = create_info.connection;
    m_vk_create_info.window = create_info.window;
    VkResult vk_result = vkex::CreateXcbSurfaceKHR(
      *m_instance,
      &m_vk_create_info,
      p_allocator,
      &m_vk_object);
#endif
    if (vk_result != VK_SUCCESS) {
      return vkex::Result(vk_result);
    }
  }

  // Surface capabilities
  {
    VkPhysicalDeviceSurfaceInfo2KHR surface_info = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR };
    surface_info.surface = m_vk_object;
    VkResult vk_result = InvalidValue<VkResult>::Value;
    m_vk_surface_capabilities = { VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR };
    VKEX_VULKAN_RESULT_CALL(
      vk_result,
      vkex::GetPhysicalDeviceSurfaceCapabilities2VKEX_SAFE(
            *m_create_info.physical_device,
            &surface_info,
            &m_vk_surface_capabilities);
    );
    if (vk_result != VK_SUCCESS) {
      return vkex::Result(vk_result);
    }
  }

  // Surface formats
  {
    VkPhysicalDeviceSurfaceInfo2KHR surface_info = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR };
    surface_info.surface = m_vk_object;
    VkResult vk_result = InvalidValue<VkResult>::Value;
    VKEX_VULKAN_RESULT_CALL(
      vk_result,
      vkex::GetPhysicalDeviceSurfaceFormats2VKEX_SAFE(
        *m_create_info.physical_device,
        &surface_info,
        &m_vk_surface_formats)
    );
    if (vk_result != VK_SUCCESS) {
      return vkex::Result(vk_result);
    }
  }

  // Bail if there's surface formats
  if (m_vk_surface_formats.empty()) {
    VKEX_LOG_ERROR("No surface formats found for " << m_create_info.physical_device->GetDeviceName());
    return vkex::Result::ErrorInvalidSurfaceFormatCount;
  }

  // Presentable queue families
  {
    auto& queue_families = m_create_info.physical_device->GetQueueFamilyProperties();
    const uint32_t queue_family_count = static_cast<uint32_t>(queue_families.size());
    for (uint32_t queue_family_index = 0; queue_family_index < queue_family_count; ++queue_family_index) {
      VkBool32 supported = false;
      VkResult vk_result = InvalidValue<VkResult>::Value;
      VKEX_VULKAN_RESULT_CALL(
        vk_result,
        vkex::GetPhysicalDeviceSurfaceSupportKHR(
          *m_create_info.physical_device,
          queue_family_index,
          m_vk_object,
          &supported);
      );
      if (vk_result != VK_SUCCESS) {
        return vkex::Result(vk_result);
      }

      if (supported == VK_TRUE) {
        m_vk_presentable_queue_families.push_back(queue_family_index);
      }
    }
  }

  // Presentable queue families
  {
    VkResult vk_result = InvalidValue<VkResult>::Value;
    VKEX_VULKAN_RESULT_CALL(
      vk_result,
      vkex::GetPhysicalDeviceSurfacePresentModesVKEX(
        *m_create_info.physical_device,
        m_vk_object,
        &m_vk_present_modes);
    );
    if (vk_result != VK_SUCCESS) {
      return vkex::Result(vk_result);
    }
  }

  return vkex::Result::Success;
}

vkex::Result CSurface::InternalDestroy(const VkAllocationCallbacks* p_allocator)
{
  if (m_vk_object != VK_NULL_HANDLE) {
    vkex::DestroySurfaceKHR(
      *m_instance,
      m_vk_object,
      p_allocator);

    m_vk_object = VK_NULL_HANDLE;
  }

  return vkex::Result::Success;
}

bool CSurface::SupportsSurfaceFormat2(VkFormat vk_format, VkColorSpaceKHR vk_color_space) const
{
  if (m_vk_surface_formats.empty()) {
    return false;
  }

  auto it = FindIf(
    m_vk_surface_formats,
    [vk_format, vk_color_space](const VkSurfaceFormat2KHR& elem) -> bool {
      return (elem.surfaceFormat.format == vk_format) &&
             (elem.surfaceFormat.colorSpace == vk_color_space); });

  bool found = (it != std::end(m_vk_surface_formats));
  return found;
}

bool CSurface::SupportsPresentMode(VkPresentModeKHR vk_present_mode) const
{
  bool supported = Contains(m_vk_present_modes, vk_present_mode);
  return supported;
}

// =================================================================================================
// Swapchain
// =================================================================================================
CSwapchain::CSwapchain()
{
}

CSwapchain::~CSwapchain()
{
}

vkex::Result CSwapchain::InternalCreate(
  const vkex::SwapchainCreateInfo&  create_info,
  const VkAllocationCallbacks*      p_allocator
)
{
  // Copy create info
  m_create_info = create_info;

  // Surface capabilities check
  {
    VkSurfaceCapabilitiesKHR surface_capabilites = m_create_info.surface->GetVkSurfaceCapabilities2().surfaceCapabilities;
    m_create_info.image_count = std::max<uint32_t>(m_create_info.image_count, surface_capabilites.minImageCount);
    if (surface_capabilites.maxImageCount > 0) {
      m_create_info.image_count = std::min<uint32_t>(m_create_info.image_count, surface_capabilites.maxImageCount);
    }
  }

  // Surface format
  {
    // Use B8G8R8A8 if format is not specified
    if (m_create_info.color_format == VK_FORMAT_UNDEFINED) {
      m_create_info.color_format = VK_FORMAT_B8G8R8A8_UNORM;
      m_create_info.color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    }

    // Check format
    if (!m_create_info.surface->SupportsSurfaceFormat2(m_create_info.color_format, m_create_info.color_space)) {
      VKEX_LOG_WARN("Format " << ToString(m_create_info.color_format) << " and " <<
                    "color space " << ToString(m_create_info.color_space) << " not supported, " <<
                    "using format VK_FORMAT_B8G8R8A8_UNORM and color space VK_COLOR_SPACE_SRGB_NONLINEAR_KHR");
      m_create_info.color_format = VK_FORMAT_B8G8R8A8_UNORM;
      m_create_info.color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    }
  }

  // Present mode
  VkPresentModeKHR vk_present_mode = m_create_info.present_mode;
  if (vk_present_mode != VK_PRESENT_MODE_FIFO_KHR) {
    if (!m_create_info.surface->SupportsPresentMode(vk_present_mode)) {
      VKEX_LOG_WARN("Present mode " << ToString(vk_present_mode) << " not supported, using VK_PRESENT_MODE_FIFO_KHR");
      vk_present_mode = VK_PRESENT_MODE_FIFO_KHR;
    }
  }
  m_create_info.present_mode = vk_present_mode;
  
  // Image usage
  ImageUsageFlags swapchain_image_usage = {};
  {
    VkSurfaceCapabilitiesKHR surface_capabilites = m_create_info.surface->GetVkSurfaceCapabilities2().surfaceCapabilities;
    swapchain_image_usage.flags = surface_capabilites.supportedUsageFlags;
    
    VkFormatProperties2 vk_format_properties = { VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2 };
    vkex::PhysicalDevice physical_device = m_device->GetPhysicalDevice();
    vkex::GetPhysicalDeviceFormatProperties2(
      *physical_device,
      m_create_info.color_format,
      &vk_format_properties);
    
    bool supports_storage = (vk_format_properties.formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT);
    swapchain_image_usage.bits.storage = supports_storage;
  }

  // Create swapchain
  {
    m_vk_create_info = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
    m_vk_create_info.pNext                  = nullptr;
    m_vk_create_info.flags                  = 0;
    m_vk_create_info.surface                = m_create_info.surface->GetVkObject();
    m_vk_create_info.minImageCount          = m_create_info.image_count;
    m_vk_create_info.imageFormat            = m_create_info.color_format;
    m_vk_create_info.imageColorSpace        = m_create_info.color_space;
    m_vk_create_info.imageExtent            = { m_create_info.width, m_create_info.height };
    m_vk_create_info.imageArrayLayers       = 1;
    m_vk_create_info.imageUsage             = swapchain_image_usage.flags;
    m_vk_create_info.imageSharingMode       = VK_SHARING_MODE_EXCLUSIVE;
    m_vk_create_info.queueFamilyIndexCount  = 0;
    m_vk_create_info.pQueueFamilyIndices    = nullptr;
    m_vk_create_info.preTransform           = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    m_vk_create_info.compositeAlpha         = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    m_vk_create_info.presentMode            = m_create_info.present_mode;
    m_vk_create_info.clipped                = VK_FALSE;
    m_vk_create_info.oldSwapchain           = VK_NULL_HANDLE;
    VkResult vk_result = InvalidValue<VkResult>::Value;
    VKEX_VULKAN_RESULT_CALL(
      vk_result,
      vkex::CreateSwapchainKHR(
        *m_device,
        &m_vk_create_info,
        p_allocator,
        &m_vk_object);
    );
    if (vk_result != VK_SUCCESS) {
      return vkex::Result(vk_result);
    }
  }

  // Get swapchain color images
  {
    std::vector<VkImage> vk_swapchain_images;
    VkResult vk_result = InvalidValue<VkResult>::Value;
    VKEX_VULKAN_RESULT_CALL(
      vk_result,
      vkex::GetSwapchainImagesVKEX(
        *m_device,
        m_vk_object,
        &vk_swapchain_images);
    );
    if (vk_result != VK_SUCCESS) {
      return vkex::Result(vk_result);
    }

    // Create VKEX color images
    {
      // Queue family index
      uint32_t queue_family_index = m_create_info.queue->GetVkQueueFamilyIndex();
      // Images
      for (const auto& vk_image : vk_swapchain_images) {
        vkex::ImageCreateInfo image_create_info = {};
        image_create_info.create_flags                 = 0;
        image_create_info.image_type            = VK_IMAGE_TYPE_2D;
        image_create_info.format                = { m_create_info.color_format };
        image_create_info.extent                = { m_create_info.width, m_create_info.height, 1 };
        image_create_info.mip_levels            = 1;
        image_create_info.array_layers          = 1;
        image_create_info.samples               = VK_SAMPLE_COUNT_1_BIT;
        image_create_info.tiling                = VK_IMAGE_TILING_OPTIMAL;
        image_create_info.usage_flags           = swapchain_image_usage;
        image_create_info.sharing_mode          = VK_SHARING_MODE_EXCLUSIVE;
        image_create_info.queue_family_indices  = { queue_family_index };
        image_create_info.initial_layout        = VK_IMAGE_LAYOUT_UNDEFINED;
        image_create_info.committed             = false;
        image_create_info.host_visible          = false;
        image_create_info.vk_object             = vk_image;
        vkex::Image image = nullptr;
        vkex::Result vkex_result = vkex::Result::Undefined;
        VKEX_RESULT_CALL(
          vkex_result,
          m_device->CreateImage(image_create_info, &image)
        );
        if (!vkex_result) {
          return vkex_result;
        }

        m_color_images.push_back(image);
      }
    }
  }
  
  // Create VKEX depth stencil images
  if (m_create_info.depth_stencil_format != VK_FORMAT_UNDEFINED) {
    // Queue family index
    uint32_t queue_family_index = m_create_info.queue->GetVkQueueFamilyIndex();
    // Image usage
    vkex::ImageUsageFlags usage = {};
    usage.bits.sampled                  = true;
    usage.bits.depth_stencil_attachment = true;
    // Images
    for (uint32_t i = 0; i < m_create_info.image_count; ++i) {
      vkex::ImageCreateInfo image_create_info = {};
      image_create_info.create_flags                 = 0;
      image_create_info.image_type            = VK_IMAGE_TYPE_2D;
      image_create_info.format                = { m_create_info.depth_stencil_format };
      image_create_info.extent                = { m_create_info.width, m_create_info.height, 1 };
      image_create_info.mip_levels            = 1;
      image_create_info.array_layers          = 1;
      image_create_info.samples               = VK_SAMPLE_COUNT_1_BIT;
      image_create_info.tiling                = VK_IMAGE_TILING_OPTIMAL;
      image_create_info.usage_flags           = usage;
      image_create_info.sharing_mode          = VK_SHARING_MODE_EXCLUSIVE;
      image_create_info.queue_family_indices  = { queue_family_index };
      image_create_info.initial_layout        = VK_IMAGE_LAYOUT_UNDEFINED;
      image_create_info.committed             = true;
      image_create_info.host_visible          = false;
      image_create_info.memory_pool           = m_create_info.image_memory_pool;
      image_create_info.vk_object             = VK_NULL_HANDLE;
      vkex::Image image = nullptr;
      vkex::Result vkex_result = vkex::Result::Undefined;
      VKEX_RESULT_CALL(
        vkex_result,
        m_device->CreateImage(image_create_info, &image)
      );
      if (!vkex_result) {
        return vkex_result;
      }

      m_depth_stencil_images.push_back(image);
    }
  }

  return vkex::Result::Success;
}

vkex::Result CSwapchain::InternalDestroy(const VkAllocationCallbacks* p_allocator)
{
  if (!m_depth_stencil_images.empty()) {
    for (auto& image : m_depth_stencil_images) {
      vkex::Result vkex_result = vkex::Result::Undefined;
      VKEX_RESULT_CALL(vkex_result, m_device->DestroyImage(image));
      if (!vkex_result) {
        return vkex_result;
      }
    }

    m_depth_stencil_images.clear();
  }

  if (m_vk_object) {
    vkex::DestroySwapchainKHR(
      *m_device,
      m_vk_object,
      p_allocator);
    
    m_vk_object = VK_NULL_HANDLE;
  }

  return vkex::Result::Success;
}

vkex::Result CSwapchain::GetColorImage(uint32_t image_index, vkex::Image* p_image) const
{
  uint32_t count = GetImageCount();
  if (image_index >= count) {
    return vkex::Result::ErrorInvalidSwapchainImageIndex;
  }

  *p_image = m_color_images[image_index];

  return vkex::Result::Success;
}

vkex::Result CSwapchain::GetDepthStencilImage(uint32_t image_index, vkex::Image* p_image) const
{
  uint32_t count = GetImageCount();
  if (image_index >= count) {
    return vkex::Result::ErrorInvalidSwapchainImageIndex;
  }

  *p_image = m_depth_stencil_images[image_index];

  return vkex::Result::Success;
}

VkResult CSwapchain::AcquireNextImage(uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex)
{
  VkResult vk_result = vkex::AcquireNextImageKHR(
    *m_device,
    m_vk_object,
    timeout,
    semaphore,
    fence,
    pImageIndex);
  if (vk_result != VK_SUCCESS) {
    return vk_result;
  }
  return VK_SUCCESS;
}

} // namespace vkex
