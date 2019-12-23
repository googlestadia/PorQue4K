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

#ifndef __VKEX_SWAPCHAIN_H__
#define __VKEX_SWAPCHAIN_H__

#include <vkex/Config.h>
#include <vkex/Traits.h>

namespace vkex {

// =================================================================================================
// Surface
// =================================================================================================

/** @struct SurfaceCreateInfo
 *
 */
struct SurfaceCreateInfo {
  vkex::PhysicalDevice  physical_device;
#if defined(VKEX_WIN32)
  HINSTANCE             hinstance;
  HWND                  hwnd;
#elif defined(VKEX_LINUX_WAYLAND)
  struct wl_display*    display;
  struct wl_surface*    surface;
#elif defined(VKEX_LINUX_XCB)
  xcb_connection_t*     connection;
  xcb_window_t          window;
#elif defined(VKEX_LINUX_XLIB)
  Display*              dpy;
  Window                window;
#endif
};

/** @class ISurface
 *
 */
class CSurface {
public:
  CSurface();
  ~CSurface();

  /** @fn operator VkSurfaceKHR()
   *
   */
  explicit operator VkSurfaceKHR() const {
    return m_vk_object;
  }

  /** @fn GetVkObject
   *
   */
  VkSurfaceKHR GetVkObject() const {
    return m_vk_object;
  }

  /** @fn GetVkSurfaceCapabilities2
   *
   */
  const VkSurfaceCapabilities2KHR GetVkSurfaceCapabilities2() const {
    return m_vk_surface_capabilities;
  }

  /** @fn SupportsPresentMode
   *
   */
  bool SupportsSurfaceFormat2(VkFormat vk_format, VkColorSpaceKHR vk_color_space) const;

  /** @fn SupportsPresentMode
   *
   */
  bool SupportsPresentMode(VkPresentModeKHR vk_present_mode) const;

private:
  friend class CInstance;
  friend class IObjectStorageFunctions;

  /** @fn InternalCreate
   *
   */
  vkex::Result InternalCreate(
    const vkex::SurfaceCreateInfo&  create_info,
    const VkAllocationCallbacks*    p_allocator
  );

  /** @fn InternalDestroy
   *
   */
  vkex::Result InternalDestroy(const VkAllocationCallbacks* p_allocator);

  /** @fn SetInstance
   *
   */
  void SetInstance(vkex::Instance instance) {
    m_instance = instance;
  }

private:
  vkex::Instance                    m_instance = nullptr;
  vkex::SurfaceCreateInfo           m_create_info = {};

#if defined(VKEX_WIN32)
  VkWin32SurfaceCreateInfoKHR               m_vk_create_info = {};
#elif defined(VKEX_LINUX)
# if defined(VKEX_LINUX_WAYLAND)
  VkWaylandSurfaceCreateInfoKHR             m_vk_create_info = {};
# elif defined(VKEX_LINUX_XCB)
  VkXcbSurfaceCreateInfoKHR                 m_vk_create_info = {};
# elif defined(VKEX_LINUX_XLIB)
  VkXlibSurfaceCreateInfoKHR                m_vk_create_info = {};
# elif defined(VKEX_LINUX_GGP)
  VkStreamDescriptorSurfaceCreateInfoGGP    m_vk_create_info = {};
# endif
#endif
  VkSurfaceKHR                      m_vk_object = VK_NULL_HANDLE;
  VkSurfaceCapabilities2KHR         m_vk_surface_capabilities = {};
  std::vector<VkSurfaceFormat2KHR>  m_vk_surface_formats;
  std::vector<uint32_t>             m_vk_presentable_queue_families;
  std::vector<VkPresentModeKHR>     m_vk_present_modes;
};

// =================================================================================================
// Swapchain
// =================================================================================================

/** @struct SwapchainCreateInfo
 *
 */
struct SwapchainCreateInfo {
  Surface           surface;
  uint32_t          image_count;
  VkFormat          color_format;
  VkColorSpaceKHR   color_space;
  VkFormat          depth_stencil_format;
  uint32_t          width;
  uint32_t          height;
  VkPresentModeKHR  present_mode;
  vkex::Queue       queue;
  VmaPool           image_memory_pool;
};

/** @class ISwapchain
 *
 */
class CSwapchain : public IDeviceObject {
public:
  CSwapchain();
  ~CSwapchain();

  /** @fn GetDevice
   *
   */
  vkex::Device GetDevice() const {
    return m_device;
  }

  /** @fn operator VkSwapchainKHR()
   *
   */
  operator VkSwapchainKHR() const {
    return m_vk_object;
  }

  /** @fn GetVkObject
   *
   */
  VkSwapchainKHR GetVkObject() const {
    return m_vk_object;
  }

  /** @fn GetImageCount
   *
   */
  uint32_t GetImageCount() const {
    return m_create_info.image_count;
  }

  /** @fn GetColorImage
   *
   */
  vkex::Result GetColorImage(uint32_t image_index, vkex::Image* p_image) const;

  /** @fn GetColorImage
   *
   */
  vkex::Result GetDepthStencilImage(uint32_t image_index, vkex::Image* p_image) const;

  /** @fn AcquireNextImage
   *
   */
  VkResult AcquireNextImage(uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex);

private:
  friend class CDevice;
  friend class IObjectStorageFunctions;

  /** @fn InternalCreate
   *
   */
  vkex::Result InternalCreate(
    const vkex::SwapchainCreateInfo&  create_info,
    const VkAllocationCallbacks*      p_allocator
  );

  /** @fn InternalDestroy
   *
   */
  vkex::Result InternalDestroy(const VkAllocationCallbacks* p_allocator);

private:
  vkex::SwapchainCreateInfo m_create_info = {};
  VkSwapchainCreateInfoKHR  m_vk_create_info = {};
  VkSwapchainKHR            m_vk_object = VK_NULL_HANDLE;
  std::vector<vkex::Image>  m_color_images;
  std::vector<vkex::Image>  m_depth_stencil_images;
};

} // namespace vkex

#endif // __VKEX_SWAPCHAIN_H__
