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

#ifndef __VKEX_INSTANCE_H__
#define __VKEX_INSTANCE_H__

#include "vkex/Device.h"
#include "vkex/Traits.h"

namespace vkex {

// =================================================================================================
// Instance
// =================================================================================================

/** @struct ApplicationInfo 
 *
 */
struct ApplicationInfo {
  const void*   p_next;
  std::string   application_name;
  uint32_t      application_version;
  std::string   engine_name;
  uint32_t      engine_version;
  uint32_t      api_version;
};

/** @struct DebugUtilsMessageSeverity 
 *
 */
struct DebugUtilsMessageSeverity {
  bool          verbose : 1;
  bool          info    : 1;
  bool          warning : 1;
  bool          error   : 1;
};

/** @struct DebugUtilsMessageType 
 *
 */
struct DebugUtilsMessageType {
  bool          general     : 1;
  bool          validation  : 1;
  bool          performance : 1;
};

/** @struct DebugUtils 
 *
 */
struct DebugUtils {
  bool                                  enable;
  DebugUtilsMessageSeverity             message_severity;
  DebugUtilsMessageType                 message_type;
  PFN_vkDebugUtilsMessengerCallbackEXT  callback;
  void*                                 user_data;
};

/** @struct PhysicalDeviceCriteria
 *
 */
struct PhysicalDeviceCriteria {
  uint32_t      vendor_id;
  std::string   name;

  // AMD
  struct {
    uint32_t  cu_count;
  } amd;
};

/** @struct InstanceCreateInfo 
 *
 */
struct InstanceCreateInfo {
  const void*                           p_next;
  ApplicationInfo                       application_info;
  std::vector<std::string>              layers;
  std::vector<std::string>              extensions;
  DebugUtils                            debug_utils;
  bool                                  enable_swapchain;
};

/** @class Instance
 * 
 */ 
class CInstance
  : //public InstanceFunctionSet,
    protected IObjectStorageFunctions
{
public:
  CInstance();
  ~CInstance();

  /** @fn operator VkInstance()
   *
   */
  operator VkInstance() const { 
    return m_vk_object; 
  }

  /** @fn GetVkObject
   *
   */
  VkInstance GetVkObject() const { 
    return m_vk_object; 
  }

  /** @fn ValidationLayersLoaded
   *
   */
  bool ValidationLayersLoaded() const {
    return m_validation_layers_loaded;
  }

  /** @fn GetLoadedLayers
   *
   */
  const std::vector<std::string> GetLoadedLayers() const {
    return m_create_info.layers;
  }

  /** @fn GetLoadedExtensions
   *
   */
  const std::vector<std::string> GetLoadedExtensions() const {
    return m_create_info.extensions;
  }

  /** @fn GetPhysicalDeviceCount
   *
   */
  uint32_t GetPhysicalDeviceCount() const {
    uint32_t count = CountU32(m_stored_physical_devices);
    return count;
  }

  /** @fn GetPhysicalDevice
   *
   */
  vkex::PhysicalDevice GetPhysicalDevice(uint32_t index) const;

  /** @fn FindPhysicalDevice
   *
   */
  vkex::PhysicalDevice FindPhysicalDevice(const PhysicalDeviceCriteria& criteria) const;

  /** @fn IsSwapchainEnabled
   *
   */
  bool IsSwapchainEnabled() const {
    return m_create_info.enable_swapchain;
  }

  /** @fn CreateDevice
   *
   */
  vkex::Result CreateDevice(
    const vkex::DeviceCreateInfo&  create_info,
    vkex::Device*                  p_object,
    const VkAllocationCallbacks*   p_allocator = nullptr
  );

  /** @fn DestroyDevice
   *
   */
  vkex::Result DestroyDevice(
    vkex::Device                  object, 
    const VkAllocationCallbacks*  p_allocator = nullptr
  );

  /** @fn CreateSurface
   *
   */
  vkex::Result CreateSurface(
    const vkex::SurfaceCreateInfo&  create_info,
    vkex::Surface*                  p_object,
    const VkAllocationCallbacks*      p_allocator = nullptr
  );

  /** @fn DestroySurface
   *
   */
  vkex::Result DestroySurface(
    vkex::Surface               object,
    const VkAllocationCallbacks*  p_allocator = nullptr
  );

protected:
  //virtual vkex::Result LoadFunctionSetLookups(InstanceFunctionSetLookUp* p_look_up);

private:
  // -----------------------------------------------------------------------------------------------
  // Friend functions begin
  // -----------------------------------------------------------------------------------------------
  friend vkex::Result CreateInstanceVKEX(
    const vkex::InstanceCreateInfo&  create_info, 
    vkex::Instance*                  p_object,
    const VkAllocationCallbacks*     p_allocator
  );

  friend vkex::Result CreateInstanceVKEX(
    const VkInstanceCreateInfo*   p_vk_create_info,
    vkex::Instance*               p_object,
    const VkAllocationCallbacks*  p_allocator
  );

  friend vkex::Result DestroyInstanceVKEX(
    vkex::Instance                object, 
    const VkAllocationCallbacks*  p_allocator
  );
  // -----------------------------------------------------------------------------------------------
  // Friend functions end
  // -----------------------------------------------------------------------------------------------

  /** @fn InitializeLayers
   *
   */
  vkex::Result InitializeLayers();

  /** @fn InitializeExtensions
   *
   */
  vkex::Result InitializeExtensions();

  /** @fn InitializeDebugUtilMessenger
   *
   */
  vkex::Result InitializeDebugUtilMessenger(const VkAllocationCallbacks* p_allocator);

  /** @fn InitializePhysicalDevices
   *
   */
  vkex::Result InitializePhysicalDevices();

  /** @fn InternalCreate
   *
   */
  vkex::Result InternalCreate(
    const vkex::InstanceCreateInfo& create_info,
    const VkAllocationCallbacks*    p_allocator
  );

  /** @fn InternalCreate
   *
   */
  vkex::Result InternalCreate(
    const VkInstanceCreateInfo*         p_vk_create_info,
    const VkAllocationCallbacks*        p_allocator
  );

  /** @fn InternalDestroy
   *
   */
  vkex::Result InternalDestroy(const VkAllocationCallbacks* p_allocator);

private:
  InstanceCreateInfo                    m_create_info = {};
  uint32_t                              m_found_api_version;
  std::vector<std::string>              m_found_layers;
  std::vector<std::string>              m_found_extensions;
  std::vector<const char*>              m_c_str_layers;
  std::vector<const char*>              m_c_str_extensions;
  VkApplicationInfo                     m_vk_application_info = {};
  VkInstanceCreateInfo                  m_vk_create_info = {};
  VkDebugUtilsMessengerCreateInfoEXT    m_vk_debug_utils_messenger_create_info = {};
  VkInstance                            m_vk_object = VK_NULL_HANDLE;
  VkDebugUtilsMessengerEXT              m_vk_messenger = VK_NULL_HANDLE;
  bool                                  m_validation_layers_loaded = false;

  std::vector<std::unique_ptr<CPhysicalDevice>> m_stored_physical_devices;
  std::vector<std::unique_ptr<CDevice>>         m_stored_devices;
  std::vector<std::unique_ptr<CSurface>>        m_stored_surfaces;
};

// =================================================================================================
// Create and destroy function for vkex::Instance
// =================================================================================================

/** @fn CreateInstance
  *
  */
vkex::Result CreateInstanceVKEX(
  const vkex::InstanceCreateInfo& create_info, 
  vkex::Instance*                 p_object,
  const VkAllocationCallbacks*    p_allocator = nullptr
);

/** @fn CreateInstance
  *
  */
vkex::Result CreateInstanceVKEX(
  const VkInstanceCreateInfo*   p_vk_create_info,
  vkex::Instance*               p_object,
  const VkAllocationCallbacks*  p_allocator = nullptr
);

/** @fn DestroyInstance
  *
  */
vkex::Result DestroyInstanceVKEX(
  vkex::Instance                object, 
  const VkAllocationCallbacks*  p_allocator = nullptr
);

} // namespace vkex

#endif // __VKEX_INSTANCE_H__