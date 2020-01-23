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

#include "vkex/Instance.h"
#include "vkex/Log.h"
#include "vkex/ToString.h"

#include <sstream>

namespace vkex {

// =================================================================================================
// Static storage for IInstance
// =================================================================================================

/**
 * Admittedly a weird pattern, but since IInstance is only meant to
 * only ever have one copy of itself, it needs static storage.
 *
 */ 
static std::unique_ptr<CInstance> s_static_storage;

// =================================================================================================
// Default debug utils messenger callback
// =================================================================================================
#if defined(VKEX_WIN32)
static void DefaultConsoleWrite(const char* s)
{
  OutputDebugString(s);
}
#else
static void DefaultConsoleWrite(const char* s)
{
  std::cout << s;
}
#endif

static VkBool32 VKAPI_PTR DefaultDebugUtilsMessenger(
  VkDebugUtilsMessageSeverityFlagBitsEXT           message_severity,
  VkDebugUtilsMessageTypeFlagsEXT                  message_type,
  const VkDebugUtilsMessengerCallbackDataEXT*      p_callback_data,
  void*                                            p_user_data
)
{
  // Severity
  std::string severity = "<UNKNOWN MESSAGE SEVERITY>";
  switch (message_severity) {
    default                                              : break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT : severity = "VERBOSE"; break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT    : severity = "INFO"; break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT : severity = "WARNING"; break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT   : severity = "ERROR"; break;
  }

  // Type
  std::stringstream ss_type;
  ss_type << "[";
  {
    uint32_t type_count = 0;
    if (message_type & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) {
      ss_type << "GENERAL";
      ++type_count;
    }
    if (message_type & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
      if (type_count > 0) {
        ss_type <<", ";
      }
      ss_type << "VALIDATION";
      ++type_count;
    }
    if (message_type & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) {
      if (type_count > 0) {
        ss_type <<", ";
      }
      ss_type << "PERFORMANCE";
    }
  }
  ss_type << "]";
  std::string type = ss_type.str();
  if (type.empty()) {
    type = "<UNKNOWN MESSAGE TYPE>";
  }

  std::stringstream ss;
  ss << "\n";
  ss << "*** VULKAN VALIDATION " << severity << " MESSAGE ***" << "\n";
  ss << "Severity : " << severity << "\n";
  ss << "Type     : " << type << "\n";

  if (p_callback_data->objectCount > 0) {
    ss << "Objects  : ";
    for (uint32_t i = 0; i < p_callback_data->objectCount; ++i) {
      auto object_name_info = p_callback_data->pObjects[i];
      std::string name = (object_name_info.pObjectName != nullptr) 
                         ? object_name_info.pObjectName
                         : "<UNNAMED OBJECT>";
      if (i > 0) {
        ss << "           ";
      }
      ss << "[" << i << "]" << ": " << name << "\n";
    }
  }

  ss << "Message  : " << p_callback_data->pMessage;
  ss << std::endl;

  bool is_error = (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT);
  bool is_validation = (message_type & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT);
  if (is_error && is_validation) {
    VKEX_ASSERT_EXTRA(
      false,
      "Vulkan validation error encountered!",
      ss.str()
    );
  }
  else {
    DefaultConsoleWrite(ss.str().c_str());
  }
    
  return VK_FALSE;
}

// =================================================================================================
// Instance
// =================================================================================================
CInstance::CInstance()
{
}

CInstance::~CInstance()
{
}

vkex::Result CInstance::InitializeLayers()
{
  // Enumerate layers
  {
    std::vector<VkLayerProperties> properties_list;
    VkResult vk_result = InvalidValue<VkResult>::Value;
    VKEX_VULKAN_RESULT_CALL(
      vk_result,
      vkex::EnumerateInstanceLayerPropertiesVKEX(&properties_list)
    );
    if (vk_result != VK_SUCCESS) {
      return vkex::Result(vk_result);
    }

    // Copy layer names
    for (auto& properties : properties_list) {
      std::string name = properties.layerName;
      bool found = Contains(m_found_layers, name);
      // Skip adding if layer name already exists
      if (found) {
        continue;
      }
      m_found_layers.push_back(name);
    }
  }

  // Replace layers if specified in the Vulkan create info
  if (m_vk_create_info.enabledLayerCount > 0) {
    m_create_info.layers.clear();
    uint32_t count = m_vk_create_info.enabledLayerCount;
    for (uint32_t i = 0; i < count; ++i) {
      std::string name = m_vk_create_info.ppEnabledLayerNames[i];
      m_create_info.layers.push_back(name);
    }
  }

  // Set required layers
  {
    std::vector<std::string> required;
    
    if (m_create_info.debug_utils.enable) {
      required.push_back(VK_LAYER_LUNARG_STANDARD_VALIDATION_LAYER_NAME);
    }

    for (auto& name : required) {
      bool found = Contains(m_create_info.layers, name);
      // Skip adding if layer name already exists
      if (found) {
        continue;
      }
      m_create_info.layers.push_back(name);
    }
  }

  // Check layer names
  for (auto& name : m_create_info.layers) {
    bool found = Contains(m_found_layers, name);     
    // Bail if requested layer isn't found
    if (!found) {
      return vkex::Result::ErrorInstanceLayerNotFound;
    }
  }

  return vkex::Result::Success;
}

vkex::Result CInstance::InitializeExtensions()
{
  // Enumerate extensions
  {
    // Grab layer names
    std::vector<const char*> c_str_layers = m_c_str_layers;
    // Put in null for standalone extensions
    c_str_layers.push_back(nullptr);

    for (auto& layer_name : c_str_layers) {
      std::vector<VkExtensionProperties> properties_list;
      VkResult vk_result = InvalidValue<VkResult>::Value;
      VKEX_VULKAN_RESULT_CALL(
        vk_result,
        vkex::EnumerateInstanceExtensionPropertiesVKEX(
          layer_name, 
          &properties_list)
      );
      if (vk_result != VK_SUCCESS) {
        return vkex::Result(vk_result);
      }

      // Copy extension names
      for (auto& properties : properties_list) {
        std::string name = properties.extensionName;
        bool found = Contains(m_found_extensions, name);
        // Skip adding if extension name already exists
        if (found) {
          continue;
        }
        m_found_extensions.push_back(name);
      }
    }
  }

  // Replace layers if specified in the Vulkan create info
  if (m_vk_create_info.enabledExtensionCount > 0) {
    m_create_info.extensions.clear();
    uint32_t count = m_vk_create_info.enabledExtensionCount;
    for (uint32_t i = 0; i < count; ++i) {
      std::string name = m_vk_create_info.ppEnabledExtensionNames[i];
      m_create_info.extensions.push_back(name);
    }
  }

  // Set required extensions
  {
    std::vector<std::string> required;
    required.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
    required.push_back(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
  
    if (m_create_info.enable_swapchain) {
#if defined(VKEX_WIN32)
      required.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(VKEX_LINUX)
 #if defined(VKEX_LINUX_WAYLAND)
      required.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
 #elif defined(VKEX_LINUX_XCB)
      required.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);;
 #elif defined(VKEX_LINUX_XLIB)
      required.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
 #elif defined(VKEX_LINUX_GGP)     
      required.push_back(VK_GGP_STREAM_DESCRIPTOR_SURFACE_EXTENSION_NAME);
 #endif
#endif
    }

    if (m_create_info.debug_utils.enable) {
      required.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    for (auto& name : required) {
      // Check to make sure extension is available
      bool found = Contains(m_found_extensions, name);
      if (!found) {
        VKEX_ASSERT_MSG(found, "Required instance extension not found: " << name);
        return vkex::Result::ErrorInstanceExtensionNotFound;
      }     
      // Skip adding if extension name already exists
      bool already_exists = Contains(m_create_info.extensions, name);
      if (already_exists) {
        continue;
      }
      m_create_info.extensions.push_back(name);
    }
  }

  // Set optional extensions
  {
    std::vector<std::string> optional;
    optional.push_back(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);

    for (auto& name : optional) {
      // Check to make sure extension is available
      bool found = Contains(m_found_extensions, name);
      if (!found) {
        VKEX_LOG_WARN("Optional instance extension not found: " << name);
        VKEX_LOG_WARN("");
      }     
      // Skip adding if extension name already exists
      bool already_exists = Contains(m_create_info.extensions, name);
      if (already_exists) {
        continue;
      }
      m_create_info.extensions.push_back(name);
    }
  }

  // Check extension names
  for (auto& name : m_create_info.extensions) {
    bool found = Contains(m_found_extensions, name);
    // Bail if requested layer isn't found
    if (!found) {
      return vkex::Result::ErrorInstanceExtensionNotFound;
    }
  }
  
  return vkex::Result::Success;
}

vkex::Result CInstance::InitializeDebugUtilMessenger(const VkAllocationCallbacks* p_allocator)
{
  // Message severity
  VkDebugUtilsMessageSeverityFlagsEXT message_severity = 0;
  {
    // Verbose
    if (m_create_info.debug_utils.message_severity.verbose) {
      message_severity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
    }
    // Info
    if (m_create_info.debug_utils.message_severity.info) {
      message_severity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
    }
    // Warning
    if (m_create_info.debug_utils.message_severity.warning) {
      message_severity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    }
    // Error
    if (m_create_info.debug_utils.message_severity.error) {
      message_severity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    }
  }

  // Message type
  VkDebugUtilsMessageTypeFlagsEXT message_type = 0;
  {
    // General
    if (m_create_info.debug_utils.message_type.general) {
      message_type |= VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
    }
    // Validation
    if (m_create_info.debug_utils.message_type.validation) {
      message_type |= VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    }
    // Performance
    if (m_create_info.debug_utils.message_type.performance) {
      message_type |= VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    }
  }

  // Turn on severity defaults if nothing is set
  if (message_severity == 0) {
    // Error
    m_create_info.debug_utils.message_severity.error = true;
    message_severity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  }
  // Turn on type defaults if nothing is set
  if (message_type == 0) {
    // General
    m_create_info.debug_utils.message_type.validation = true;
    message_type |= VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
  }
  // Use a default call back if one is not set
  if (m_create_info.debug_utils.callback == nullptr) {
    m_create_info.debug_utils.callback = vkex::DefaultDebugUtilsMessenger;
  }

  // Create debug utils messenger
  {
    m_vk_debug_utils_messenger_create_info.sType            = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    m_vk_debug_utils_messenger_create_info.messageSeverity  = message_severity;
    m_vk_debug_utils_messenger_create_info.messageType      = message_type;
    m_vk_debug_utils_messenger_create_info.pfnUserCallback  = m_create_info.debug_utils.callback;
    m_vk_debug_utils_messenger_create_info.pUserData        = m_create_info.debug_utils.user_data;
    VkResult vk_result = InvalidValue<VkResult>::Value;
    VKEX_VULKAN_RESULT_CALL(
      vk_result,
      vkex::CreateDebugUtilsMessengerEXT(
        m_vk_object,
        &m_vk_debug_utils_messenger_create_info,
        p_allocator,
        &m_vk_messenger)
    );
    if (vk_result != VK_SUCCESS) {
      return vkex::Result(vk_result);
    }
  }

  return vkex::Result::Success;
}

vkex::Result CInstance::InitializePhysicalDevices()
{
  std::vector<VkPhysicalDevice> vk_physical_devices;
  VkResult vk_result = InvalidValue<VkResult>::Value;
  VKEX_VULKAN_RESULT_CALL(
    vk_result,
    vkex::EnumeratePhysicalDevicesVKEX(
      m_vk_object,
      &vk_physical_devices)
  );
  if (vk_result != VK_SUCCESS) {
    return vkex::Result(vk_result);
  }

  if (vk_physical_devices.empty()) {
    return vkex::Result::ErrorPhysicalDevicesNotFound;
  }

  for (auto& vk_physical_device : vk_physical_devices) {
    // Skip if not an AMD, Intel, or NVIDIA GPU for now
    VkPhysicalDeviceProperties vk_properties = {};
    vkGetPhysicalDeviceProperties(vk_physical_device, &vk_properties);
    if ((vk_properties.vendorID != VKEX_IHV_VENDOR_ID_AMD) &&
        (vk_properties.vendorID != VKEX_IHV_VENDOR_ID_INTEL) &&
        (vk_properties.vendorID != VKEX_IHV_VENDOR_ID_NVIDIA)) {
      continue;
    }

    vkex::PhysicalDeviceCreateInfo create_info = {};
    create_info.vk_object = vk_physical_device;
    vkex::PhysicalDevice physical_device = nullptr;
    vkex::Result vkex_result = CreateObject<CPhysicalDevice>(
      create_info,
      nullptr,
      m_stored_physical_devices,
      &CPhysicalDevice::SetInstance,
      this,
      &physical_device);
    if (!vkex_result) {
      return vkex_result;
    }
  }

  return vkex::Result::Success;
}

vkex::Result CInstance::InternalCreate(
  const vkex::InstanceCreateInfo& create_info,
  const VkAllocationCallbacks*    p_allocator
)
{
  // Copy create info
  m_create_info = create_info;

  // Initialize loader
  vkex::VkexLoaderInitialize(vkex::LOAD_MODE_SO_DIRECT);

  // Enumerate instance API version
  {
    VkResult vk_result = InvalidValue<VkResult>::Value;
    VKEX_VULKAN_RESULT_CALL(
      vk_result,
      vkex::EnumerateInstanceVersion(&m_found_api_version)
    );
    if (vk_result != VK_SUCCESS) {
      return vkex::Result(vk_result);
    }
  }

  // Check Vulkan API version number - VKEX requires at least Vulkan 1.1
  {
    int major = VK_VERSION_MAJOR(m_found_api_version);
    int minor = VK_VERSION_MINOR(m_found_api_version);
    bool is_valid_major = (major >= 1);
    bool is_valid_minor = (minor >= 1);
    if (!(is_valid_major && is_valid_minor)) {
      return vkex::Result::ErrorVkexRequiresAtLeastVulkan11;
    }

    VKEX_LOG_INFO("Enumerated Vulkan instance version: " << major << "." << minor);
    VKEX_LOG_INFO("");
  }

  // Configure layers
  {
    vkex::Result vkex_result = InitializeLayers();
    if (!vkex_result) {
      return vkex_result;
    }
  }

  // Turn on debug utils if standard validation is found
  {
    bool found = Contains(m_create_info.layers, std::string(VK_LAYER_LUNARG_STANDARD_VALIDATION_LAYER_NAME));
    if (found) {
      m_create_info.debug_utils.enable = true;
    }
  }

  // Configure extensions
  {
    vkex::Result vkex_result = InitializeExtensions();
    if (!vkex_result) {
      return vkex_result;
    }
  }

  // Application info
  {
    m_vk_application_info = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
    m_vk_application_info.pNext               = m_create_info.application_info.p_next;
    m_vk_application_info.pApplicationName    = DataPtr(m_create_info.application_info.application_name);
    m_vk_application_info.applicationVersion  = m_create_info.application_info.application_version;
    m_vk_application_info.pEngineName         = DataPtr(m_create_info.application_info.engine_name);
    m_vk_application_info.engineVersion       = m_create_info.application_info.engine_version;
    m_vk_application_info.apiVersion          = VKEX_MINIMUM_REQUIRED_VULKAN_VERSION;
  }

  // Create info
  {
    m_c_str_layers = GetCStrings(m_create_info.layers);
    m_c_str_extensions = GetCStrings(m_create_info.extensions);

    m_vk_create_info = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    m_vk_create_info.pNext                    = m_create_info.p_next;
    m_vk_create_info.flags                    = 0;
    m_vk_create_info.pApplicationInfo         = &m_vk_application_info;
    m_vk_create_info.enabledLayerCount        = CountU32(m_c_str_layers);
    m_vk_create_info.ppEnabledLayerNames      = DataPtr(m_c_str_layers);
    m_vk_create_info.enabledExtensionCount    = CountU32(m_c_str_extensions);
    m_vk_create_info.ppEnabledExtensionNames  = DataPtr(m_c_str_extensions);
  }

  // Create Vulkan instance
  {
    VkResult vk_result = InvalidValue<VkResult>::Value;
    VKEX_VULKAN_RESULT_CALL(
      vk_result,
      vkex::CreateInstance(
        &m_vk_create_info,
        p_allocator,
        &m_vk_object)
    );
    if (vk_result != VK_SUCCESS) {
      return vkex::Result(vk_result);
    }
  }
 
  // Set validation layers loaded
  {
    auto it = Find(m_create_info.layers, std::string(VK_LAYER_LUNARG_STANDARD_VALIDATION_LAYER_NAME));
    m_validation_layers_loaded = (it != std::end(m_create_info.layers));
  }

  // Log instance creation
  {
    int major = VK_VERSION_MAJOR(m_vk_create_info.pApplicationInfo->apiVersion);
    int minor = VK_VERSION_MINOR(m_vk_create_info.pApplicationInfo->apiVersion);
    VKEX_LOG_INFO("");
    VKEX_LOG_INFO("Vulkan " << major << "." << minor << " instance created (object=VkInstance)");
    // Layers
    if (m_vk_create_info.enabledLayerCount > 0) {
      VKEX_LOG_INFO("Layers loaded:");
      for (uint32_t i = 0; i < m_vk_create_info.enabledLayerCount; ++i) {
        VKEX_LOG_INFO("   " << m_vk_create_info.ppEnabledLayerNames[i]);
      }
    }
    // Extensions
    if (m_vk_create_info.enabledExtensionCount > 0) {
      VKEX_LOG_INFO("Extensions loaded:");
      for (uint32_t i = 0; i < m_vk_create_info.enabledExtensionCount; ++i) {
        VKEX_LOG_INFO("   " << m_vk_create_info.ppEnabledExtensionNames[i]);
      }
    }
    VKEX_LOG_INFO("");
  }

  //if (ValidationLayersLoaded()) {
  //  vkex::LoadProcAddressInstance(m_vk_object, vkGetInstanceProcAddr);
  //}
  //else {
  //  vkex::LoadVkFunctionsInstance(m_vk_object, vkGetInstanceProcAddr);
  //}
  vkex::VkexLoaderLoadInstance(m_vk_object, vkGetInstanceProcAddr);
  //// Load instance functions
  //{
  //  vkex::Result vkex_result = LoadFunctions(m_vk_object);
  //  if (!vkex_result) {
  //    return vkex_result;
  //  }
  //}

  // Create debug utils messenger
  if (m_create_info.debug_utils.enable) {
    vkex::Result vkex_result = InitializeDebugUtilMessenger(p_allocator);
    if (!vkex_result) {
      return vkex_result;
    }
  }

  // Enumerate physical devices
  {
    vkex::Result vkex_result = InitializePhysicalDevices();
    if (!vkex_result) {
      return vkex_result;
    }
  }

  return vkex::Result::Success;
}

vkex::Result CInstance::InternalCreate(
  const VkInstanceCreateInfo*         p_vk_create_info,
  const VkAllocationCallbacks*        p_allocator
)
{
  if (p_vk_create_info == nullptr) {
    return vkex::Result::ErrorUnexpectedNullPointer;
  }

  // Check Vulkan API version number - VKEX requires at least Vulkan 1.1
  {
    int major = VK_VERSION_MAJOR(p_vk_create_info->pApplicationInfo->apiVersion);
    int minor = VK_VERSION_MINOR(p_vk_create_info->pApplicationInfo->apiVersion);
    bool is_valid_major = (major >= 1);
    bool is_valid_minor = (minor >= 1);
    if (!(is_valid_major && is_valid_minor)) {
      return vkex::Result::ErrorVkexRequiresAtLeastVulkan11;
    }
  }

  vkex::InstanceCreateInfo create_info = {};
  // Copy application info
  {
    create_info.application_info.p_next               = p_vk_create_info->pApplicationInfo->pNext;
    create_info.application_info.application_name     = p_vk_create_info->pApplicationInfo->pApplicationName;
    create_info.application_info.application_version  = p_vk_create_info->pApplicationInfo->applicationVersion;
    create_info.application_info.engine_name          = p_vk_create_info->pApplicationInfo->pEngineName;
    create_info.application_info.engine_version       = p_vk_create_info->pApplicationInfo->engineVersion;
    create_info.application_info.api_version          = p_vk_create_info->pApplicationInfo->apiVersion;
  }
  // Copy next 
  create_info.p_next = p_vk_create_info->pNext;
  // Copy layers
  {
    uint32_t n = p_vk_create_info->enabledLayerCount;
    for (uint32_t i = 0; i < n; ++i) {
      std::string name = p_vk_create_info->ppEnabledLayerNames[i];
      create_info.layers.push_back(name);
    }
  }
  // Copy extensions
  {
    uint32_t n = p_vk_create_info->enabledExtensionCount;
    for (uint32_t i = 0; i < n; ++i) {
      std::string name = p_vk_create_info->ppEnabledExtensionNames[i];
      create_info.extensions.push_back(name);
    }
  }

  // Create instance
  vkex::Result vkex_result = InternalCreate(create_info, p_allocator);
  if (!vkex_result) {
    return vkex_result;
  }

  return vkex::Result::Success;
}

vkex::Result CInstance::InternalDestroy(const VkAllocationCallbacks* p_allocator)
{
  // Destroy all devices
  for (auto& device : m_stored_devices) {
    vkex::Result vkex_result = device->InternalDestroy(p_allocator);
    if (!vkex_result) {
      return vkex_result;
    }  
  }

  // Destroy DebugUtils
  if (m_vk_messenger != VK_NULL_HANDLE) {
    vkex::DestroyDebugUtilsMessengerEXT(
      m_vk_object,
      m_vk_messenger,
      p_allocator);

    m_vk_messenger = VK_NULL_HANDLE;
  }

  // Destroy instance
  if (m_vk_object != VK_NULL_HANDLE) {
    vkex::DestroyInstance(
      m_vk_object,
      p_allocator);

    m_vk_object = VK_NULL_HANDLE;
  }

  return vkex::Result::Success;
}

vkex::PhysicalDevice CInstance::GetPhysicalDevice(uint32_t index) const
{
  vkex::PhysicalDevice physical_device = nullptr;
  uint32_t count = GetPhysicalDeviceCount();
  if (index < count) {
    physical_device = m_stored_physical_devices[index].get();
  }
  return physical_device;
}

vkex::PhysicalDevice CInstance::FindPhysicalDevice(const PhysicalDeviceCriteria& criteria) const
{
  // Find all devices matching vendor id
  std::vector<vkex::PhysicalDevice> found_devices_0;
  {
    bool is_amd    = criteria.vendor_id == VKEX_IHV_VENDOR_ID_AMD;
    bool is_intel  = criteria.vendor_id == VKEX_IHV_VENDOR_ID_INTEL;
    bool is_nvidia = criteria.vendor_id == VKEX_IHV_VENDOR_ID_NVIDIA;
    if (is_amd || is_intel || is_nvidia) {
      uint32_t count = GetPhysicalDeviceCount();
      for (uint32_t i = 0; i < count; ++i) {
        vkex::PhysicalDevice device = m_stored_physical_devices[i].get();
        uint32_t vendor_id = device->GetVendorId();
        if (vendor_id == criteria.vendor_id) {
          found_devices_0.push_back(device);
        }
      }
    }
    else {
      uint32_t count = GetPhysicalDeviceCount();
      for (uint32_t i = 0; i < count; ++i) {
        vkex::PhysicalDevice device = m_stored_physical_devices[i].get();
        found_devices_0.push_back(device);
      }
    }
  }
    
  // Find all devices matching name
  std::vector<vkex::PhysicalDevice> found_devices_1;
  {
    if (!criteria.name.empty()) {
      uint32_t count = CountU32(found_devices_0);
      for (uint32_t i = 0; i < count; ++i) {
        vkex::PhysicalDevice device = found_devices_0[i];
        std::string name = device->GetDeviceName();
        if (name == criteria.name) {
          found_devices_1.push_back(device);
        }
      }
    }
    else {
      found_devices_1 = found_devices_0;
    }
  }

  // Find devices matching vendor properties
  std::vector<vkex::PhysicalDevice> found_devices_2;
  {
    uint32_t count = CountU32(found_devices_1);
    for (uint32_t i = 0; i < count; ++i) {
      vkex::PhysicalDevice device = found_devices_1[i];
      // AMD CU count
      if ((criteria.amd.cu_count > 0) && device->IsAMD()) {
        auto& shader_core_properties = device->GetShaderCorePropertiesAMD();
        uint32_t cu_count = shader_core_properties.shaderEngineCount *
                            shader_core_properties.shaderArraysPerEngineCount *
                            shader_core_properties.computeUnitsPerShaderArray;
        if (cu_count == criteria.amd.cu_count) {
          found_devices_2.push_back(device);
        }
      }
    }
  }

  vkex::PhysicalDevice found_device = nullptr;
  if (!found_devices_2.empty()) {
    found_device = found_devices_2[0];
  }
  else if (!found_devices_1.empty()) {
    found_device = found_devices_1[0];
  }

  return found_device;
}

vkex::Result CInstance::CreateDevice(
  const vkex::DeviceCreateInfo&  create_info,
  vkex::Device*                  p_object,
  const VkAllocationCallbacks*   p_allocator
)
{
  vkex::Result vkex_result = CreateObject<CDevice>(
    create_info,
    p_allocator,
    m_stored_devices,
    &CDevice::SetInstance,
    this,
    p_object);

  if (!vkex_result) {
    return vkex_result;
  }

  return vkex::Result::Success;
}

vkex::Result CInstance::DestroyDevice(
  vkex::Device                  object, 
  const VkAllocationCallbacks*  p_allocator
)
{
  vkex::Result vkex_result = DestroyObject<CDevice>(
    m_stored_devices,
    object,
    p_allocator);

  if (!vkex_result) {
    return vkex_result;
  }

  return vkex::Result::Success;
}

vkex::Result CInstance::CreateSurface(
  const vkex::SurfaceCreateInfo&  create_info,
  vkex::Surface*                  p_object,
  const VkAllocationCallbacks*    p_allocator
)
{
  vkex::Result vkex_result = CreateObject<CSurface>(
    create_info,
    p_allocator,
    m_stored_surfaces,
    &CSurface::SetInstance,
    this,
    p_object);

  if (!vkex_result) {
    return vkex_result;
  }

  return vkex::Result::Success;
}

vkex::Result CInstance::DestroySurface(
  vkex::Surface                 object,
  const VkAllocationCallbacks*  p_allocator
)
{
  vkex::Result vkex_result = DestroyObject<CSurface>(
    m_stored_surfaces,
    object,
    p_allocator);

  if (!vkex_result) {
    return vkex_result;
  }

  return vkex::Result::Success;
}

// =================================================================================================
// Create and destroy function for vkex::Instance
// =================================================================================================

/** @fn CreateInstance
 *
 */
vkex::Result CreateInstanceVKEX(
  const vkex::InstanceCreateInfo&  create_info, 
  vkex::Instance*                  p_object,
  const VkAllocationCallbacks*     p_allocator
)
{
  if (s_static_storage) {
    return vkex::Result::ErrorInstanceAlreadyExists;
  }

  s_static_storage = std::make_unique<CInstance>();
  if (!s_static_storage) {
    return vkex::Result::ErrorAllocationFailed;
  }

  vkex::Result vkex_result = s_static_storage->InternalCreate(create_info, p_allocator);
  if (!vkex_result) {
    return vkex_result;
  }

  *p_object = s_static_storage.get();

  return vkex::Result::Success;
}

vkex::Result CreateInstanceVKEX(
  const VkInstanceCreateInfo*   p_vk_create_info,
  vkex::Instance*               p_object,
  const VkAllocationCallbacks*  p_allocator
)
{
  if (s_static_storage) {
    return vkex::Result::ErrorInstanceAlreadyExists;
  }

  s_static_storage = std::make_unique<CInstance>();
  if (!s_static_storage) {
    return vkex::Result::ErrorAllocationFailed;
  }

  vkex::Result vkex_result = s_static_storage->InternalCreate(p_vk_create_info, p_allocator);
  if (!vkex_result) {
    return vkex_result;
  }

  *p_object = s_static_storage.get();

  return vkex::Result::Success;
}

/** @fn DestroyVkexInstance
 *
 */
vkex::Result DestroyInstanceVKEX(
  vkex::Instance                object, 
  const VkAllocationCallbacks*  p_allocator
)
{
  if (object == nullptr) {
    return vkex::Result::ErrorUnexpectedNullPointer;
  }

  if (!s_static_storage) {
    return vkex::Result::ErrorInstanceNotAllocated;
  }

  if (s_static_storage.get() != object) {
    return vkex::Result::ErrorInvalidInstanceAddress;
  }

  if (s_static_storage) {
    vkex::Result vkex_result = s_static_storage->InternalDestroy(p_allocator);
    if (!vkex_result) {
      return vkex_result;
    }
  }

  s_static_storage.reset();

  return vkex::Result::Success;
}

} // namespace vkex
