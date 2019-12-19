#include "VkexLoader.h"

#if defined(VKEX_LINUX)
# include <dlfcn.h>
#endif

namespace vkex {

// =================================================================================================
// Forward declarations for load/assign functions
// =================================================================================================
static void LoadVkApiFnsExported(vkex::InstanceFunctionTable* p_instance_table, vkex::DeviceFunctionTable* p_device_table);
static void LoadVkApiFnsInstance(VkInstance instance, vkex::GetInstanceProcType fnProcLoad, vkex::InstanceFunctionTable* p_table);
static void LoadVkApiFnsDevice(VkDevice device, vkex::GetDeviceProcType fnProcLoad, vkex::DeviceFunctionTable* p_table);

static void LoadProcAddrInstance(VkInstance instance, vkex::GetInstanceProcType fnProcLoad, vkex::InstanceFunctionTable* p_table);
static void LoadProcAddrDevice(VkDevice device, vkex::GetDeviceProcType fnProcLoad, vkex::DeviceFunctionTable* p_table);

static void AssignProcsInstance(const vkex::InstanceFunctionTable* p_table);
static void AssignProcsDevice(const vkex::DeviceFunctionTable* p_table);

// =================================================================================================
// Functions Implementations
// =================================================================================================
struct InternalData {
  bool                        initialized = false;
  vkex::LoadMode              mode = vkex::LoadMode::LOAD_MODE_VK_API_FN;
  vkex::InstanceFunctionTable instance_table = {};
  vkex::DeviceFunctionTable   device_table = {};
  VkInstance                  instance = VK_NULL_HANDLE;
};

static InternalData s_internal_data = {};

static PFN_vkGetInstanceProcAddr so_fn_vkGetInstanceProcAddr = nullptr;
static PFN_vkGetDeviceProcAddr   so_fn_vkGetDeviceProcAddr = nullptr;

bool VkexLoaderInitialize(LoadMode mode) 
{
  if (s_internal_data.initialized) {
    return true;
  }

  bool is_mode_vk_api_fn = (mode == vkex::LoadMode::LOAD_MODE_VK_API_FN);
  bool is_mode_proc_addr = (mode == vkex::LoadMode::LOAD_MODE_PROC_ADDR);
  bool is_mode_so_direct = (mode == vkex::LoadMode::LOAD_MODE_SO_DIRECT);
  if (!(is_mode_vk_api_fn || is_mode_proc_addr || is_mode_so_direct)) {
    return false;
  }

  //
  // "vkGetInstanceProcAddr" is used for both instance and device load functions. 
  //
  // "vkGetDeviceProcAddr" seems to load functions from the layers and not the shared object directly.
  //
  if (mode == vkex::LoadMode::LOAD_MODE_SO_DIRECT) {
#if defined(_WIN32)
    HMODULE module = ::LoadLibraryA("vulkan-1.dll");
    if (module == nullptr) {
      return false;
    }

    so_fn_vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)::GetProcAddress(module, "vkGetInstanceProcAddr");
    so_fn_vkGetDeviceProcAddr   = (PFN_vkGetDeviceProcAddr)::GetProcAddress(module, "vkGetInstanceProcAddr");
#else
    void* module = dlopen("libvulkan.so", RTLD_NOW | RTLD_LOCAL);
    if (module == nullptr) {
      module = dlopen("libvulkan.so.1", RTLD_NOW | RTLD_LOCAL);
    }
    if (module == nullptr) {
      return false;
    }

    so_fn_vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)dlsym(module, "vkGetInstanceProcAddr");
    so_fn_vkGetDeviceProcAddr   = (PFN_vkGetDeviceProcAddr)dlsym(module, "vkGetInstanceProcAddr");
#endif

    bool has_instance_proc = (so_fn_vkGetInstanceProcAddr != nullptr);
    bool has_device_proc   = (so_fn_vkGetDeviceProcAddr != nullptr);
    if (!(has_instance_proc && has_device_proc)) {
      return false;
    }
  }

  s_internal_data.mode = mode;
  
  if (s_internal_data.mode == vkex::LoadMode::LOAD_MODE_VK_API_FN) {
    LoadVkApiFnsExported(&s_internal_data.instance_table, &s_internal_data.device_table);
    AssignProcsInstance(&s_internal_data.instance_table);
    AssignProcsDevice(&s_internal_data.device_table);
  }
  else if ((s_internal_data.mode == vkex::LoadMode::LOAD_MODE_PROC_ADDR) || (s_internal_data.mode == vkex::LoadMode::LOAD_MODE_SO_DIRECT)) {
    //
    // Load instance related functions so that the following are usable:
    //   vkex::EnumerateInstanceVersion
    //   vkex::EnumerateInstanceExtensionProperties
    //   vkex::EnumerateInstanceLayerProperties
    //   vkex::CreateInstance
    //
    s_internal_data.instance_table.EnumerateInstanceVersion = (PFN_vkEnumerateInstanceVersion)so_fn_vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceVersion");
    s_internal_data.instance_table.EnumerateInstanceExtensionProperties = (PFN_vkEnumerateInstanceExtensionProperties)so_fn_vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceExtensionProperties");
    s_internal_data.instance_table.EnumerateInstanceLayerProperties = (PFN_vkEnumerateInstanceLayerProperties)so_fn_vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceLayerProperties");
    s_internal_data.instance_table.CreateInstance = (PFN_vkCreateInstance)so_fn_vkGetInstanceProcAddr(nullptr, "vkCreateInstance");
    AssignProcsInstance(&s_internal_data.instance_table);
  }

  s_internal_data.initialized = true;

  return s_internal_data.initialized;
}

void VkexLoaderLoadInstance(VkInstance instance, vkex::GetInstanceProcType fnProcLoad, vkex::InstanceFunctionTable* p_table)
{
  if (!s_internal_data.initialized) {
    return;
  }

  bool assign = false;
  if (p_table == nullptr){
    p_table = &s_internal_data.instance_table;
    assign = true;
  }

  if (s_internal_data.mode == vkex::LoadMode::LOAD_MODE_VK_API_FN) {
    LoadVkApiFnsInstance(instance, fnProcLoad, p_table);
  }
  else if(s_internal_data.mode == vkex::LoadMode::LOAD_MODE_PROC_ADDR) {
    LoadProcAddrInstance(instance, fnProcLoad, p_table);
  }
  else if(s_internal_data.mode == vkex::LoadMode::LOAD_MODE_SO_DIRECT) {
    LoadProcAddrInstance(instance, so_fn_vkGetInstanceProcAddr, p_table);
    s_internal_data.instance = instance;
  }

  if (assign) {
    AssignProcsInstance(p_table);
  }
}

void VkexLoaderLoadDevice(VkDevice device, vkex::GetDeviceProcType fnProcLoad, vkex::DeviceFunctionTable* p_table)
{
  if (!s_internal_data.initialized) {
    return;
  }

  bool assign = false;
  if (p_table == nullptr){
    p_table = &s_internal_data.device_table;
    assign = true;
  }

  if (s_internal_data.mode == vkex::LoadMode::LOAD_MODE_VK_API_FN) {
    LoadVkApiFnsDevice(device, fnProcLoad, p_table);
  }
  else if(s_internal_data.mode == vkex::LoadMode::LOAD_MODE_PROC_ADDR) {
    LoadProcAddrDevice(device, fnProcLoad, p_table);
  }
  else if(s_internal_data.mode == vkex::LoadMode::LOAD_MODE_SO_DIRECT) {
    device = reinterpret_cast<VkDevice>(s_internal_data.instance);
    LoadProcAddrDevice(device, so_fn_vkGetDeviceProcAddr, p_table);
  }

  if (assign) {
    AssignProcsDevice(p_table);
  }
}

// =================================================================================================
// Load functions for Vulkan API functions
// =================================================================================================
static void LoadVkApiFnsExported(vkex::InstanceFunctionTable* p_instance_table, vkex::DeviceFunctionTable* p_device_table)
{
  p_instance_table->CreateInstance                                         = vkCreateInstance;
  p_instance_table->DestroyInstance                                        = vkDestroyInstance;
  p_instance_table->EnumeratePhysicalDevices                               = vkEnumeratePhysicalDevices;
  p_instance_table->GetPhysicalDeviceFeatures                              = vkGetPhysicalDeviceFeatures;
  p_instance_table->GetPhysicalDeviceFormatProperties                      = vkGetPhysicalDeviceFormatProperties;
  p_instance_table->GetPhysicalDeviceImageFormatProperties                 = vkGetPhysicalDeviceImageFormatProperties;
  p_instance_table->GetPhysicalDeviceProperties                            = vkGetPhysicalDeviceProperties;
  p_instance_table->GetPhysicalDeviceQueueFamilyProperties                 = vkGetPhysicalDeviceQueueFamilyProperties;
  p_instance_table->GetPhysicalDeviceMemoryProperties                      = vkGetPhysicalDeviceMemoryProperties;
  p_instance_table->GetInstanceProcAddr                                    = vkGetInstanceProcAddr;
  p_device_table->GetDeviceProcAddr                                        = vkGetDeviceProcAddr;
  p_instance_table->CreateDevice                                           = vkCreateDevice;
  p_device_table->DestroyDevice                                            = vkDestroyDevice;
  p_instance_table->EnumerateInstanceExtensionProperties                   = vkEnumerateInstanceExtensionProperties;
  p_instance_table->EnumerateDeviceExtensionProperties                     = vkEnumerateDeviceExtensionProperties;
  p_instance_table->EnumerateInstanceLayerProperties                       = vkEnumerateInstanceLayerProperties;
  p_instance_table->EnumerateDeviceLayerProperties                         = vkEnumerateDeviceLayerProperties;
  p_device_table->GetDeviceQueue                                           = vkGetDeviceQueue;
  p_device_table->QueueSubmit                                              = vkQueueSubmit;
  p_device_table->QueueWaitIdle                                            = vkQueueWaitIdle;
  p_device_table->DeviceWaitIdle                                           = vkDeviceWaitIdle;
  p_device_table->AllocateMemory                                           = vkAllocateMemory;
  p_device_table->FreeMemory                                               = vkFreeMemory;
  p_device_table->MapMemory                                                = vkMapMemory;
  p_device_table->UnmapMemory                                              = vkUnmapMemory;
  p_device_table->FlushMappedMemoryRanges                                  = vkFlushMappedMemoryRanges;
  p_device_table->InvalidateMappedMemoryRanges                             = vkInvalidateMappedMemoryRanges;
  p_device_table->GetDeviceMemoryCommitment                                = vkGetDeviceMemoryCommitment;
  p_device_table->BindBufferMemory                                         = vkBindBufferMemory;
  p_device_table->BindImageMemory                                          = vkBindImageMemory;
  p_device_table->GetBufferMemoryRequirements                              = vkGetBufferMemoryRequirements;
  p_device_table->GetImageMemoryRequirements                               = vkGetImageMemoryRequirements;
  p_device_table->GetImageSparseMemoryRequirements                         = vkGetImageSparseMemoryRequirements;
  p_instance_table->GetPhysicalDeviceSparseImageFormatProperties           = vkGetPhysicalDeviceSparseImageFormatProperties;
  p_device_table->QueueBindSparse                                          = vkQueueBindSparse;
  p_device_table->CreateFence                                              = vkCreateFence;
  p_device_table->DestroyFence                                             = vkDestroyFence;
  p_device_table->ResetFences                                              = vkResetFences;
  p_device_table->GetFenceStatus                                           = vkGetFenceStatus;
  p_device_table->WaitForFences                                            = vkWaitForFences;
  p_device_table->CreateSemaphore                                          = vkCreateSemaphore;
  p_device_table->DestroySemaphore                                         = vkDestroySemaphore;
  p_device_table->CreateEvent                                              = vkCreateEvent;
  p_device_table->DestroyEvent                                             = vkDestroyEvent;
  p_device_table->GetEventStatus                                           = vkGetEventStatus;
  p_device_table->SetEvent                                                 = vkSetEvent;
  p_device_table->ResetEvent                                               = vkResetEvent;
  p_device_table->CreateQueryPool                                          = vkCreateQueryPool;
  p_device_table->DestroyQueryPool                                         = vkDestroyQueryPool;
  p_device_table->GetQueryPoolResults                                      = vkGetQueryPoolResults;
  p_device_table->CreateBuffer                                             = vkCreateBuffer;
  p_device_table->DestroyBuffer                                            = vkDestroyBuffer;
  p_device_table->CreateBufferView                                         = vkCreateBufferView;
  p_device_table->DestroyBufferView                                        = vkDestroyBufferView;
  p_device_table->CreateImage                                              = vkCreateImage;
  p_device_table->DestroyImage                                             = vkDestroyImage;
  p_device_table->GetImageSubresourceLayout                                = vkGetImageSubresourceLayout;
  p_device_table->CreateImageView                                          = vkCreateImageView;
  p_device_table->DestroyImageView                                         = vkDestroyImageView;
  p_device_table->CreateShaderModule                                       = vkCreateShaderModule;
  p_device_table->DestroyShaderModule                                      = vkDestroyShaderModule;
  p_device_table->CreatePipelineCache                                      = vkCreatePipelineCache;
  p_device_table->DestroyPipelineCache                                     = vkDestroyPipelineCache;
  p_device_table->GetPipelineCacheData                                     = vkGetPipelineCacheData;
  p_device_table->MergePipelineCaches                                      = vkMergePipelineCaches;
  p_device_table->CreateGraphicsPipelines                                  = vkCreateGraphicsPipelines;
  p_device_table->CreateComputePipelines                                   = vkCreateComputePipelines;
  p_device_table->DestroyPipeline                                          = vkDestroyPipeline;
  p_device_table->CreatePipelineLayout                                     = vkCreatePipelineLayout;
  p_device_table->DestroyPipelineLayout                                    = vkDestroyPipelineLayout;
  p_device_table->CreateSampler                                            = vkCreateSampler;
  p_device_table->DestroySampler                                           = vkDestroySampler;
  p_device_table->CreateDescriptorSetLayout                                = vkCreateDescriptorSetLayout;
  p_device_table->DestroyDescriptorSetLayout                               = vkDestroyDescriptorSetLayout;
  p_device_table->CreateDescriptorPool                                     = vkCreateDescriptorPool;
  p_device_table->DestroyDescriptorPool                                    = vkDestroyDescriptorPool;
  p_device_table->ResetDescriptorPool                                      = vkResetDescriptorPool;
  p_device_table->AllocateDescriptorSets                                   = vkAllocateDescriptorSets;
  p_device_table->FreeDescriptorSets                                       = vkFreeDescriptorSets;
  p_device_table->UpdateDescriptorSets                                     = vkUpdateDescriptorSets;
  p_device_table->CreateFramebuffer                                        = vkCreateFramebuffer;
  p_device_table->DestroyFramebuffer                                       = vkDestroyFramebuffer;
  p_device_table->CreateRenderPass                                         = vkCreateRenderPass;
  p_device_table->DestroyRenderPass                                        = vkDestroyRenderPass;
  p_device_table->GetRenderAreaGranularity                                 = vkGetRenderAreaGranularity;
  p_device_table->CreateCommandPool                                        = vkCreateCommandPool;
  p_device_table->DestroyCommandPool                                       = vkDestroyCommandPool;
  p_device_table->ResetCommandPool                                         = vkResetCommandPool;
  p_device_table->AllocateCommandBuffers                                   = vkAllocateCommandBuffers;
  p_device_table->FreeCommandBuffers                                       = vkFreeCommandBuffers;
  p_device_table->BeginCommandBuffer                                       = vkBeginCommandBuffer;
  p_device_table->EndCommandBuffer                                         = vkEndCommandBuffer;
  p_device_table->ResetCommandBuffer                                       = vkResetCommandBuffer;
  p_device_table->CmdBindPipeline                                          = vkCmdBindPipeline;
  p_device_table->CmdSetViewport                                           = vkCmdSetViewport;
  p_device_table->CmdSetScissor                                            = vkCmdSetScissor;
  p_device_table->CmdSetLineWidth                                          = vkCmdSetLineWidth;
  p_device_table->CmdSetDepthBias                                          = vkCmdSetDepthBias;
  p_device_table->CmdSetBlendConstants                                     = vkCmdSetBlendConstants;
  p_device_table->CmdSetDepthBounds                                        = vkCmdSetDepthBounds;
  p_device_table->CmdSetStencilCompareMask                                 = vkCmdSetStencilCompareMask;
  p_device_table->CmdSetStencilWriteMask                                   = vkCmdSetStencilWriteMask;
  p_device_table->CmdSetStencilReference                                   = vkCmdSetStencilReference;
  p_device_table->CmdBindDescriptorSets                                    = vkCmdBindDescriptorSets;
  p_device_table->CmdBindIndexBuffer                                       = vkCmdBindIndexBuffer;
  p_device_table->CmdBindVertexBuffers                                     = vkCmdBindVertexBuffers;
  p_device_table->CmdDraw                                                  = vkCmdDraw;
  p_device_table->CmdDrawIndexed                                           = vkCmdDrawIndexed;
  p_device_table->CmdDrawIndirect                                          = vkCmdDrawIndirect;
  p_device_table->CmdDrawIndexedIndirect                                   = vkCmdDrawIndexedIndirect;
  p_device_table->CmdDispatch                                              = vkCmdDispatch;
  p_device_table->CmdDispatchIndirect                                      = vkCmdDispatchIndirect;
  p_device_table->CmdCopyBuffer                                            = vkCmdCopyBuffer;
  p_device_table->CmdCopyImage                                             = vkCmdCopyImage;
  p_device_table->CmdBlitImage                                             = vkCmdBlitImage;
  p_device_table->CmdCopyBufferToImage                                     = vkCmdCopyBufferToImage;
  p_device_table->CmdCopyImageToBuffer                                     = vkCmdCopyImageToBuffer;
  p_device_table->CmdUpdateBuffer                                          = vkCmdUpdateBuffer;
  p_device_table->CmdFillBuffer                                            = vkCmdFillBuffer;
  p_device_table->CmdClearColorImage                                       = vkCmdClearColorImage;
  p_device_table->CmdClearDepthStencilImage                                = vkCmdClearDepthStencilImage;
  p_device_table->CmdClearAttachments                                      = vkCmdClearAttachments;
  p_device_table->CmdResolveImage                                          = vkCmdResolveImage;
  p_device_table->CmdSetEvent                                              = vkCmdSetEvent;
  p_device_table->CmdResetEvent                                            = vkCmdResetEvent;
  p_device_table->CmdWaitEvents                                            = vkCmdWaitEvents;
  p_device_table->CmdPipelineBarrier                                       = vkCmdPipelineBarrier;
  p_device_table->CmdBeginQuery                                            = vkCmdBeginQuery;
  p_device_table->CmdEndQuery                                              = vkCmdEndQuery;
  p_device_table->CmdResetQueryPool                                        = vkCmdResetQueryPool;
  p_device_table->CmdWriteTimestamp                                        = vkCmdWriteTimestamp;
  p_device_table->CmdCopyQueryPoolResults                                  = vkCmdCopyQueryPoolResults;
  p_device_table->CmdPushConstants                                         = vkCmdPushConstants;
  p_device_table->CmdBeginRenderPass                                       = vkCmdBeginRenderPass;
  p_device_table->CmdNextSubpass                                           = vkCmdNextSubpass;
  p_device_table->CmdEndRenderPass                                         = vkCmdEndRenderPass;
  p_device_table->CmdExecuteCommands                                       = vkCmdExecuteCommands;
  p_instance_table->EnumerateInstanceVersion                               = vkEnumerateInstanceVersion;
  p_device_table->BindBufferMemory2                                        = vkBindBufferMemory2;
  p_device_table->BindImageMemory2                                         = vkBindImageMemory2;
  p_device_table->GetDeviceGroupPeerMemoryFeatures                         = vkGetDeviceGroupPeerMemoryFeatures;
  p_device_table->CmdSetDeviceMask                                         = vkCmdSetDeviceMask;
  p_device_table->CmdDispatchBase                                          = vkCmdDispatchBase;
  p_instance_table->EnumeratePhysicalDeviceGroups                          = vkEnumeratePhysicalDeviceGroups;
  p_device_table->GetImageMemoryRequirements2                              = vkGetImageMemoryRequirements2;
  p_device_table->GetBufferMemoryRequirements2                             = vkGetBufferMemoryRequirements2;
  p_device_table->GetImageSparseMemoryRequirements2                        = vkGetImageSparseMemoryRequirements2;
  p_instance_table->GetPhysicalDeviceFeatures2                             = vkGetPhysicalDeviceFeatures2;
  p_instance_table->GetPhysicalDeviceProperties2                           = vkGetPhysicalDeviceProperties2;
  p_instance_table->GetPhysicalDeviceFormatProperties2                     = vkGetPhysicalDeviceFormatProperties2;
  p_instance_table->GetPhysicalDeviceImageFormatProperties2                = vkGetPhysicalDeviceImageFormatProperties2;
  p_instance_table->GetPhysicalDeviceQueueFamilyProperties2                = vkGetPhysicalDeviceQueueFamilyProperties2;
  p_instance_table->GetPhysicalDeviceMemoryProperties2                     = vkGetPhysicalDeviceMemoryProperties2;
  p_instance_table->GetPhysicalDeviceSparseImageFormatProperties2          = vkGetPhysicalDeviceSparseImageFormatProperties2;
  p_device_table->TrimCommandPool                                          = vkTrimCommandPool;
  p_device_table->GetDeviceQueue2                                          = vkGetDeviceQueue2;
  p_device_table->CreateSamplerYcbcrConversion                             = vkCreateSamplerYcbcrConversion;
  p_device_table->DestroySamplerYcbcrConversion                            = vkDestroySamplerYcbcrConversion;
  p_device_table->CreateDescriptorUpdateTemplate                           = vkCreateDescriptorUpdateTemplate;
  p_device_table->DestroyDescriptorUpdateTemplate                          = vkDestroyDescriptorUpdateTemplate;
  p_device_table->UpdateDescriptorSetWithTemplate                          = vkUpdateDescriptorSetWithTemplate;
  p_instance_table->GetPhysicalDeviceExternalBufferProperties              = vkGetPhysicalDeviceExternalBufferProperties;
  p_instance_table->GetPhysicalDeviceExternalFenceProperties               = vkGetPhysicalDeviceExternalFenceProperties;
  p_instance_table->GetPhysicalDeviceExternalSemaphoreProperties           = vkGetPhysicalDeviceExternalSemaphoreProperties;
  p_device_table->GetDescriptorSetLayoutSupport                            = vkGetDescriptorSetLayoutSupport;
  p_instance_table->DestroySurfaceKHR                                      = vkDestroySurfaceKHR;
  p_instance_table->GetPhysicalDeviceSurfaceSupportKHR                     = vkGetPhysicalDeviceSurfaceSupportKHR;
  p_instance_table->GetPhysicalDeviceSurfaceCapabilitiesKHR                = vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
  p_instance_table->GetPhysicalDeviceSurfaceFormatsKHR                     = vkGetPhysicalDeviceSurfaceFormatsKHR;
  p_instance_table->GetPhysicalDeviceSurfacePresentModesKHR                = vkGetPhysicalDeviceSurfacePresentModesKHR;
  p_device_table->CreateSwapchainKHR                                       = vkCreateSwapchainKHR;
  p_device_table->DestroySwapchainKHR                                      = vkDestroySwapchainKHR;
  p_device_table->GetSwapchainImagesKHR                                    = vkGetSwapchainImagesKHR;
  p_device_table->AcquireNextImageKHR                                      = vkAcquireNextImageKHR;
  p_device_table->QueuePresentKHR                                          = vkQueuePresentKHR;
  p_device_table->GetDeviceGroupPresentCapabilitiesKHR                     = vkGetDeviceGroupPresentCapabilitiesKHR;
  p_device_table->GetDeviceGroupSurfacePresentModesKHR                     = vkGetDeviceGroupSurfacePresentModesKHR;
  p_instance_table->GetPhysicalDevicePresentRectanglesKHR                  = vkGetPhysicalDevicePresentRectanglesKHR;
  p_device_table->AcquireNextImage2KHR                                     = vkAcquireNextImage2KHR;
  p_instance_table->GetPhysicalDeviceDisplayPropertiesKHR                  = vkGetPhysicalDeviceDisplayPropertiesKHR;
  p_instance_table->GetPhysicalDeviceDisplayPlanePropertiesKHR             = vkGetPhysicalDeviceDisplayPlanePropertiesKHR;
  p_instance_table->GetDisplayPlaneSupportedDisplaysKHR                    = vkGetDisplayPlaneSupportedDisplaysKHR;
  p_instance_table->GetDisplayModePropertiesKHR                            = vkGetDisplayModePropertiesKHR;
  p_instance_table->CreateDisplayModeKHR                                   = vkCreateDisplayModeKHR;
  p_instance_table->GetDisplayPlaneCapabilitiesKHR                         = vkGetDisplayPlaneCapabilitiesKHR;
  p_instance_table->CreateDisplayPlaneSurfaceKHR                           = vkCreateDisplayPlaneSurfaceKHR;
  p_device_table->CreateSharedSwapchainsKHR                                = vkCreateSharedSwapchainsKHR;
  p_instance_table->GetPhysicalDeviceDisplayProperties2KHR                 = vkGetPhysicalDeviceDisplayProperties2KHR;
  p_instance_table->GetPhysicalDeviceDisplayPlaneProperties2KHR            = vkGetPhysicalDeviceDisplayPlaneProperties2KHR;
  p_instance_table->GetDisplayModeProperties2KHR                           = vkGetDisplayModeProperties2KHR;
  p_instance_table->GetDisplayPlaneCapabilities2KHR                        = vkGetDisplayPlaneCapabilities2KHR;

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
  p_instance_table->CreateAndroidSurfaceKHR                                = vkCreateAndroidSurfaceKHR;
  p_device_table->GetAndroidHardwareBufferPropertiesANDROID                = vkGetAndroidHardwareBufferPropertiesANDROID;
  p_device_table->GetMemoryAndroidHardwareBufferANDROID                    = vkGetMemoryAndroidHardwareBufferANDROID;
#endif // defined(VK_USE_PLATFORM_ANDROID_KHR)

#if defined(VK_USE_PLATFORM_IOS_MVK)
  p_instance_table->CreateIOSSurfaceMVK                                    = vkCreateIOSSurfaceMVK;
#endif // defined(VK_USE_PLATFORM_IOS_MVK)

#if defined(VK_USE_PLATFORM_MACOS_MVK)
  p_instance_table->CreateMacOSSurfaceMVK                                  = vkCreateMacOSSurfaceMVK;
#endif // defined(VK_USE_PLATFORM_MACOS_MVK)

#if defined(VK_USE_PLATFORM_MIR_KHR)
  p_instance_table->CreateMirSurfaceKHR                                    = vkCreateMirSurfaceKHR;
  p_instance_table->GetPhysicalDeviceMirPresentationSupportKHR             = vkGetPhysicalDeviceMirPresentationSupportKHR;
#endif // defined(VK_USE_PLATFORM_MIR_KHR)

#if defined(VK_USE_PLATFORM_VI_NN)
  p_instance_table->CreateViSurfaceNN                                      = vkCreateViSurfaceNN;
#endif // defined(VK_USE_PLATFORM_VI_NN)

#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
  p_instance_table->CreateWaylandSurfaceKHR                                = vkCreateWaylandSurfaceKHR;
  p_instance_table->GetPhysicalDeviceWaylandPresentationSupportKHR         = vkGetPhysicalDeviceWaylandPresentationSupportKHR;
#endif // defined(VK_USE_PLATFORM_WAYLAND_KHR)

#if defined(VK_USE_PLATFORM_WIN32_KHR)
  p_instance_table->CreateWin32SurfaceKHR                                  = vkCreateWin32SurfaceKHR;
  p_instance_table->GetPhysicalDeviceWin32PresentationSupportKHR           = vkGetPhysicalDeviceWin32PresentationSupportKHR;
#endif // defined(VK_USE_PLATFORM_WIN32_KHR)

#if defined(VK_USE_PLATFORM_XCB_KHR)
  p_instance_table->CreateXcbSurfaceKHR                                    = vkCreateXcbSurfaceKHR;
  p_instance_table->GetPhysicalDeviceXcbPresentationSupportKHR             = vkGetPhysicalDeviceXcbPresentationSupportKHR;
#endif // defined(VK_USE_PLATFORM_XCB_KHR)

#if defined(VK_USE_PLATFORM_XLIB_KHR)
  p_instance_table->CreateXlibSurfaceKHR                                   = vkCreateXlibSurfaceKHR;
  p_instance_table->GetPhysicalDeviceXlibPresentationSupportKHR            = vkGetPhysicalDeviceXlibPresentationSupportKHR;
#endif // defined(VK_USE_PLATFORM_XLIB_KHR)

#if defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
  p_instance_table->AcquireXlibDisplayEXT                                  = vkAcquireXlibDisplayEXT;
  p_instance_table->GetRandROutputDisplayEXT                               = vkGetRandROutputDisplayEXT;
#endif // defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
}

static void LoadVkApiFnsInstance(VkInstance instance, vkex::GetInstanceProcType fnProcLoad, vkex::InstanceFunctionTable* p_table)
{
  p_table->GetPhysicalDeviceFeatures2KHR                          = (PFN_vkGetPhysicalDeviceFeatures2KHR)fnProcLoad(instance, "vkGetPhysicalDeviceFeatures2KHR");
  p_table->GetPhysicalDeviceProperties2KHR                        = (PFN_vkGetPhysicalDeviceProperties2KHR)fnProcLoad(instance, "vkGetPhysicalDeviceProperties2KHR");
  p_table->GetPhysicalDeviceFormatProperties2KHR                  = (PFN_vkGetPhysicalDeviceFormatProperties2KHR)fnProcLoad(instance, "vkGetPhysicalDeviceFormatProperties2KHR");
  p_table->GetPhysicalDeviceImageFormatProperties2KHR             = (PFN_vkGetPhysicalDeviceImageFormatProperties2KHR)fnProcLoad(instance, "vkGetPhysicalDeviceImageFormatProperties2KHR");
  p_table->GetPhysicalDeviceQueueFamilyProperties2KHR             = (PFN_vkGetPhysicalDeviceQueueFamilyProperties2KHR)fnProcLoad(instance, "vkGetPhysicalDeviceQueueFamilyProperties2KHR");
  p_table->GetPhysicalDeviceMemoryProperties2KHR                  = (PFN_vkGetPhysicalDeviceMemoryProperties2KHR)fnProcLoad(instance, "vkGetPhysicalDeviceMemoryProperties2KHR");
  p_table->GetPhysicalDeviceSparseImageFormatProperties2KHR       = (PFN_vkGetPhysicalDeviceSparseImageFormatProperties2KHR)fnProcLoad(instance, "vkGetPhysicalDeviceSparseImageFormatProperties2KHR");
  p_table->EnumeratePhysicalDeviceGroupsKHR                       = (PFN_vkEnumeratePhysicalDeviceGroupsKHR)fnProcLoad(instance, "vkEnumeratePhysicalDeviceGroupsKHR");
  p_table->GetPhysicalDeviceExternalBufferPropertiesKHR           = (PFN_vkGetPhysicalDeviceExternalBufferPropertiesKHR)fnProcLoad(instance, "vkGetPhysicalDeviceExternalBufferPropertiesKHR");
  p_table->GetPhysicalDeviceExternalSemaphorePropertiesKHR        = (PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR)fnProcLoad(instance, "vkGetPhysicalDeviceExternalSemaphorePropertiesKHR");
  p_table->GetPhysicalDeviceExternalFencePropertiesKHR            = (PFN_vkGetPhysicalDeviceExternalFencePropertiesKHR)fnProcLoad(instance, "vkGetPhysicalDeviceExternalFencePropertiesKHR");
  p_table->GetPhysicalDeviceSurfaceCapabilities2KHR               = (PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR)fnProcLoad(instance, "vkGetPhysicalDeviceSurfaceCapabilities2KHR");
  p_table->GetPhysicalDeviceSurfaceFormats2KHR                    = (PFN_vkGetPhysicalDeviceSurfaceFormats2KHR)fnProcLoad(instance, "vkGetPhysicalDeviceSurfaceFormats2KHR");
  p_table->CreateDebugReportCallbackEXT                           = (PFN_vkCreateDebugReportCallbackEXT)fnProcLoad(instance, "vkCreateDebugReportCallbackEXT");
  p_table->DestroyDebugReportCallbackEXT                          = (PFN_vkDestroyDebugReportCallbackEXT)fnProcLoad(instance, "vkDestroyDebugReportCallbackEXT");
  p_table->DebugReportMessageEXT                                  = (PFN_vkDebugReportMessageEXT)fnProcLoad(instance, "vkDebugReportMessageEXT");
  p_table->GetPhysicalDeviceExternalImageFormatPropertiesNV       = (PFN_vkGetPhysicalDeviceExternalImageFormatPropertiesNV)fnProcLoad(instance, "vkGetPhysicalDeviceExternalImageFormatPropertiesNV");
  p_table->GetPhysicalDeviceGeneratedCommandsPropertiesNVX        = (PFN_vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX)fnProcLoad(instance, "vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX");
  p_table->ReleaseDisplayEXT                                      = (PFN_vkReleaseDisplayEXT)fnProcLoad(instance, "vkReleaseDisplayEXT");
  p_table->GetPhysicalDeviceSurfaceCapabilities2EXT               = (PFN_vkGetPhysicalDeviceSurfaceCapabilities2EXT)fnProcLoad(instance, "vkGetPhysicalDeviceSurfaceCapabilities2EXT");
  p_table->CreateDebugUtilsMessengerEXT                           = (PFN_vkCreateDebugUtilsMessengerEXT)fnProcLoad(instance, "vkCreateDebugUtilsMessengerEXT");
  p_table->DestroyDebugUtilsMessengerEXT                          = (PFN_vkDestroyDebugUtilsMessengerEXT)fnProcLoad(instance, "vkDestroyDebugUtilsMessengerEXT");
  p_table->SubmitDebugUtilsMessageEXT                             = (PFN_vkSubmitDebugUtilsMessageEXT)fnProcLoad(instance, "vkSubmitDebugUtilsMessageEXT");
  p_table->GetPhysicalDeviceMultisamplePropertiesEXT              = (PFN_vkGetPhysicalDeviceMultisamplePropertiesEXT)fnProcLoad(instance, "vkGetPhysicalDeviceMultisamplePropertiesEXT");

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
#endif // defined(VK_USE_PLATFORM_ANDROID_KHR)

#if defined(VK_USE_PLATFORM_IOS_MVK)
#endif // defined(VK_USE_PLATFORM_IOS_MVK)

#if defined(VK_USE_PLATFORM_MACOS_MVK)
#endif // defined(VK_USE_PLATFORM_MACOS_MVK)

#if defined(VK_USE_PLATFORM_MIR_KHR)
#endif // defined(VK_USE_PLATFORM_MIR_KHR)

#if defined(VK_USE_PLATFORM_VI_NN)
#endif // defined(VK_USE_PLATFORM_VI_NN)

#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
#endif // defined(VK_USE_PLATFORM_WAYLAND_KHR)

#if defined(VK_USE_PLATFORM_WIN32_KHR)
#endif // defined(VK_USE_PLATFORM_WIN32_KHR)

#if defined(VK_USE_PLATFORM_XCB_KHR)
#endif // defined(VK_USE_PLATFORM_XCB_KHR)

#if defined(VK_USE_PLATFORM_XLIB_KHR)
#endif // defined(VK_USE_PLATFORM_XLIB_KHR)

#if defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
#endif // defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
}

static void LoadVkApiFnsDevice(VkDevice device, vkex::GetDeviceProcType fnProcLoad, vkex::DeviceFunctionTable* p_table)
{
  p_table->GetDeviceGroupPeerMemoryFeaturesKHR                    = (PFN_vkGetDeviceGroupPeerMemoryFeaturesKHR)fnProcLoad(device, "vkGetDeviceGroupPeerMemoryFeaturesKHR");
  p_table->CmdSetDeviceMaskKHR                                    = (PFN_vkCmdSetDeviceMaskKHR)fnProcLoad(device, "vkCmdSetDeviceMaskKHR");
  p_table->CmdDispatchBaseKHR                                     = (PFN_vkCmdDispatchBaseKHR)fnProcLoad(device, "vkCmdDispatchBaseKHR");
  p_table->TrimCommandPoolKHR                                     = (PFN_vkTrimCommandPoolKHR)fnProcLoad(device, "vkTrimCommandPoolKHR");
  p_table->GetMemoryFdKHR                                         = (PFN_vkGetMemoryFdKHR)fnProcLoad(device, "vkGetMemoryFdKHR");
  p_table->GetMemoryFdPropertiesKHR                               = (PFN_vkGetMemoryFdPropertiesKHR)fnProcLoad(device, "vkGetMemoryFdPropertiesKHR");
  p_table->ImportSemaphoreFdKHR                                   = (PFN_vkImportSemaphoreFdKHR)fnProcLoad(device, "vkImportSemaphoreFdKHR");
  p_table->GetSemaphoreFdKHR                                      = (PFN_vkGetSemaphoreFdKHR)fnProcLoad(device, "vkGetSemaphoreFdKHR");
  p_table->CmdPushDescriptorSetKHR                                = (PFN_vkCmdPushDescriptorSetKHR)fnProcLoad(device, "vkCmdPushDescriptorSetKHR");
  p_table->CmdPushDescriptorSetWithTemplateKHR                    = (PFN_vkCmdPushDescriptorSetWithTemplateKHR)fnProcLoad(device, "vkCmdPushDescriptorSetWithTemplateKHR");
  p_table->CreateDescriptorUpdateTemplateKHR                      = (PFN_vkCreateDescriptorUpdateTemplateKHR)fnProcLoad(device, "vkCreateDescriptorUpdateTemplateKHR");
  p_table->DestroyDescriptorUpdateTemplateKHR                     = (PFN_vkDestroyDescriptorUpdateTemplateKHR)fnProcLoad(device, "vkDestroyDescriptorUpdateTemplateKHR");
  p_table->UpdateDescriptorSetWithTemplateKHR                     = (PFN_vkUpdateDescriptorSetWithTemplateKHR)fnProcLoad(device, "vkUpdateDescriptorSetWithTemplateKHR");
  p_table->CreateRenderPass2KHR                                   = (PFN_vkCreateRenderPass2KHR)fnProcLoad(device, "vkCreateRenderPass2KHR");
  p_table->CmdBeginRenderPass2KHR                                 = (PFN_vkCmdBeginRenderPass2KHR)fnProcLoad(device, "vkCmdBeginRenderPass2KHR");
  p_table->CmdNextSubpass2KHR                                     = (PFN_vkCmdNextSubpass2KHR)fnProcLoad(device, "vkCmdNextSubpass2KHR");
  p_table->CmdEndRenderPass2KHR                                   = (PFN_vkCmdEndRenderPass2KHR)fnProcLoad(device, "vkCmdEndRenderPass2KHR");
  p_table->GetSwapchainStatusKHR                                  = (PFN_vkGetSwapchainStatusKHR)fnProcLoad(device, "vkGetSwapchainStatusKHR");
  p_table->ImportFenceFdKHR                                       = (PFN_vkImportFenceFdKHR)fnProcLoad(device, "vkImportFenceFdKHR");
  p_table->GetFenceFdKHR                                          = (PFN_vkGetFenceFdKHR)fnProcLoad(device, "vkGetFenceFdKHR");
  p_table->GetImageMemoryRequirements2KHR                         = (PFN_vkGetImageMemoryRequirements2KHR)fnProcLoad(device, "vkGetImageMemoryRequirements2KHR");
  p_table->GetBufferMemoryRequirements2KHR                        = (PFN_vkGetBufferMemoryRequirements2KHR)fnProcLoad(device, "vkGetBufferMemoryRequirements2KHR");
  p_table->GetImageSparseMemoryRequirements2KHR                   = (PFN_vkGetImageSparseMemoryRequirements2KHR)fnProcLoad(device, "vkGetImageSparseMemoryRequirements2KHR");
  p_table->CreateSamplerYcbcrConversionKHR                        = (PFN_vkCreateSamplerYcbcrConversionKHR)fnProcLoad(device, "vkCreateSamplerYcbcrConversionKHR");
  p_table->DestroySamplerYcbcrConversionKHR                       = (PFN_vkDestroySamplerYcbcrConversionKHR)fnProcLoad(device, "vkDestroySamplerYcbcrConversionKHR");
  p_table->BindBufferMemory2KHR                                   = (PFN_vkBindBufferMemory2KHR)fnProcLoad(device, "vkBindBufferMemory2KHR");
  p_table->BindImageMemory2KHR                                    = (PFN_vkBindImageMemory2KHR)fnProcLoad(device, "vkBindImageMemory2KHR");
  p_table->GetDescriptorSetLayoutSupportKHR                       = (PFN_vkGetDescriptorSetLayoutSupportKHR)fnProcLoad(device, "vkGetDescriptorSetLayoutSupportKHR");
  p_table->CmdDrawIndirectCountKHR                                = (PFN_vkCmdDrawIndirectCountKHR)fnProcLoad(device, "vkCmdDrawIndirectCountKHR");
  p_table->CmdDrawIndexedIndirectCountKHR                         = (PFN_vkCmdDrawIndexedIndirectCountKHR)fnProcLoad(device, "vkCmdDrawIndexedIndirectCountKHR");
  p_table->DebugReportCallbackEXT                                 = (PFN_vkDebugReportCallbackEXT)fnProcLoad(device, "vkDebugReportCallbackEXT");
  p_table->DebugMarkerSetObjectTagEXT                             = (PFN_vkDebugMarkerSetObjectTagEXT)fnProcLoad(device, "vkDebugMarkerSetObjectTagEXT");
  p_table->DebugMarkerSetObjectNameEXT                            = (PFN_vkDebugMarkerSetObjectNameEXT)fnProcLoad(device, "vkDebugMarkerSetObjectNameEXT");
  p_table->CmdDebugMarkerBeginEXT                                 = (PFN_vkCmdDebugMarkerBeginEXT)fnProcLoad(device, "vkCmdDebugMarkerBeginEXT");
  p_table->CmdDebugMarkerEndEXT                                   = (PFN_vkCmdDebugMarkerEndEXT)fnProcLoad(device, "vkCmdDebugMarkerEndEXT");
  p_table->CmdDebugMarkerInsertEXT                                = (PFN_vkCmdDebugMarkerInsertEXT)fnProcLoad(device, "vkCmdDebugMarkerInsertEXT");
  p_table->CmdDrawIndirectCountAMD                                = (PFN_vkCmdDrawIndirectCountAMD)fnProcLoad(device, "vkCmdDrawIndirectCountAMD");
  p_table->CmdDrawIndexedIndirectCountAMD                         = (PFN_vkCmdDrawIndexedIndirectCountAMD)fnProcLoad(device, "vkCmdDrawIndexedIndirectCountAMD");
  p_table->GetShaderInfoAMD                                       = (PFN_vkGetShaderInfoAMD)fnProcLoad(device, "vkGetShaderInfoAMD");
  p_table->CmdBeginConditionalRenderingEXT                        = (PFN_vkCmdBeginConditionalRenderingEXT)fnProcLoad(device, "vkCmdBeginConditionalRenderingEXT");
  p_table->CmdEndConditionalRenderingEXT                          = (PFN_vkCmdEndConditionalRenderingEXT)fnProcLoad(device, "vkCmdEndConditionalRenderingEXT");
  p_table->CmdProcessCommandsNVX                                  = (PFN_vkCmdProcessCommandsNVX)fnProcLoad(device, "vkCmdProcessCommandsNVX");
  p_table->CmdReserveSpaceForCommandsNVX                          = (PFN_vkCmdReserveSpaceForCommandsNVX)fnProcLoad(device, "vkCmdReserveSpaceForCommandsNVX");
  p_table->CreateIndirectCommandsLayoutNVX                        = (PFN_vkCreateIndirectCommandsLayoutNVX)fnProcLoad(device, "vkCreateIndirectCommandsLayoutNVX");
  p_table->DestroyIndirectCommandsLayoutNVX                       = (PFN_vkDestroyIndirectCommandsLayoutNVX)fnProcLoad(device, "vkDestroyIndirectCommandsLayoutNVX");
  p_table->CreateObjectTableNVX                                   = (PFN_vkCreateObjectTableNVX)fnProcLoad(device, "vkCreateObjectTableNVX");
  p_table->DestroyObjectTableNVX                                  = (PFN_vkDestroyObjectTableNVX)fnProcLoad(device, "vkDestroyObjectTableNVX");
  p_table->RegisterObjectsNVX                                     = (PFN_vkRegisterObjectsNVX)fnProcLoad(device, "vkRegisterObjectsNVX");
  p_table->UnregisterObjectsNVX                                   = (PFN_vkUnregisterObjectsNVX)fnProcLoad(device, "vkUnregisterObjectsNVX");
  p_table->CmdSetViewportWScalingNV                               = (PFN_vkCmdSetViewportWScalingNV)fnProcLoad(device, "vkCmdSetViewportWScalingNV");
  p_table->DisplayPowerControlEXT                                 = (PFN_vkDisplayPowerControlEXT)fnProcLoad(device, "vkDisplayPowerControlEXT");
  p_table->RegisterDeviceEventEXT                                 = (PFN_vkRegisterDeviceEventEXT)fnProcLoad(device, "vkRegisterDeviceEventEXT");
  p_table->RegisterDisplayEventEXT                                = (PFN_vkRegisterDisplayEventEXT)fnProcLoad(device, "vkRegisterDisplayEventEXT");
  p_table->GetSwapchainCounterEXT                                 = (PFN_vkGetSwapchainCounterEXT)fnProcLoad(device, "vkGetSwapchainCounterEXT");
  p_table->GetRefreshCycleDurationGOOGLE                          = (PFN_vkGetRefreshCycleDurationGOOGLE)fnProcLoad(device, "vkGetRefreshCycleDurationGOOGLE");
  p_table->GetPastPresentationTimingGOOGLE                        = (PFN_vkGetPastPresentationTimingGOOGLE)fnProcLoad(device, "vkGetPastPresentationTimingGOOGLE");
  p_table->CmdSetDiscardRectangleEXT                              = (PFN_vkCmdSetDiscardRectangleEXT)fnProcLoad(device, "vkCmdSetDiscardRectangleEXT");
  p_table->SetHdrMetadataEXT                                      = (PFN_vkSetHdrMetadataEXT)fnProcLoad(device, "vkSetHdrMetadataEXT");
  p_table->DebugUtilsMessengerCallbackEXT                         = (PFN_vkDebugUtilsMessengerCallbackEXT)fnProcLoad(device, "vkDebugUtilsMessengerCallbackEXT");
  p_table->SetDebugUtilsObjectNameEXT                             = (PFN_vkSetDebugUtilsObjectNameEXT)fnProcLoad(device, "vkSetDebugUtilsObjectNameEXT");
  p_table->SetDebugUtilsObjectTagEXT                              = (PFN_vkSetDebugUtilsObjectTagEXT)fnProcLoad(device, "vkSetDebugUtilsObjectTagEXT");
  p_table->QueueBeginDebugUtilsLabelEXT                           = (PFN_vkQueueBeginDebugUtilsLabelEXT)fnProcLoad(device, "vkQueueBeginDebugUtilsLabelEXT");
  p_table->QueueEndDebugUtilsLabelEXT                             = (PFN_vkQueueEndDebugUtilsLabelEXT)fnProcLoad(device, "vkQueueEndDebugUtilsLabelEXT");
  p_table->QueueInsertDebugUtilsLabelEXT                          = (PFN_vkQueueInsertDebugUtilsLabelEXT)fnProcLoad(device, "vkQueueInsertDebugUtilsLabelEXT");
  p_table->CmdBeginDebugUtilsLabelEXT                             = (PFN_vkCmdBeginDebugUtilsLabelEXT)fnProcLoad(device, "vkCmdBeginDebugUtilsLabelEXT");
  p_table->CmdEndDebugUtilsLabelEXT                               = (PFN_vkCmdEndDebugUtilsLabelEXT)fnProcLoad(device, "vkCmdEndDebugUtilsLabelEXT");
  p_table->CmdInsertDebugUtilsLabelEXT                            = (PFN_vkCmdInsertDebugUtilsLabelEXT)fnProcLoad(device, "vkCmdInsertDebugUtilsLabelEXT");
  p_table->CmdSetSampleLocationsEXT                               = (PFN_vkCmdSetSampleLocationsEXT)fnProcLoad(device, "vkCmdSetSampleLocationsEXT");
  p_table->CreateValidationCacheEXT                               = (PFN_vkCreateValidationCacheEXT)fnProcLoad(device, "vkCreateValidationCacheEXT");
  p_table->DestroyValidationCacheEXT                              = (PFN_vkDestroyValidationCacheEXT)fnProcLoad(device, "vkDestroyValidationCacheEXT");
  p_table->MergeValidationCachesEXT                               = (PFN_vkMergeValidationCachesEXT)fnProcLoad(device, "vkMergeValidationCachesEXT");
  p_table->GetValidationCacheDataEXT                              = (PFN_vkGetValidationCacheDataEXT)fnProcLoad(device, "vkGetValidationCacheDataEXT");
  p_table->GetMemoryHostPointerPropertiesEXT                      = (PFN_vkGetMemoryHostPointerPropertiesEXT)fnProcLoad(device, "vkGetMemoryHostPointerPropertiesEXT");
  p_table->CmdWriteBufferMarkerAMD                                = (PFN_vkCmdWriteBufferMarkerAMD)fnProcLoad(device, "vkCmdWriteBufferMarkerAMD");
  p_table->CmdSetCheckpointNV                                     = (PFN_vkCmdSetCheckpointNV)fnProcLoad(device, "vkCmdSetCheckpointNV");
  p_table->GetQueueCheckpointDataNV                               = (PFN_vkGetQueueCheckpointDataNV)fnProcLoad(device, "vkGetQueueCheckpointDataNV");

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
#endif // defined(VK_USE_PLATFORM_ANDROID_KHR)

#if defined(VK_USE_PLATFORM_IOS_MVK)
#endif // defined(VK_USE_PLATFORM_IOS_MVK)

#if defined(VK_USE_PLATFORM_MACOS_MVK)
#endif // defined(VK_USE_PLATFORM_MACOS_MVK)

#if defined(VK_USE_PLATFORM_MIR_KHR)
#endif // defined(VK_USE_PLATFORM_MIR_KHR)

#if defined(VK_USE_PLATFORM_VI_NN)
#endif // defined(VK_USE_PLATFORM_VI_NN)

#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
#endif // defined(VK_USE_PLATFORM_WAYLAND_KHR)

#if defined(VK_USE_PLATFORM_WIN32_KHR)
  p_table->GetMemoryWin32HandleKHR                                = (PFN_vkGetMemoryWin32HandleKHR)fnProcLoad(device, "vkGetMemoryWin32HandleKHR");
  p_table->GetMemoryWin32HandlePropertiesKHR                      = (PFN_vkGetMemoryWin32HandlePropertiesKHR)fnProcLoad(device, "vkGetMemoryWin32HandlePropertiesKHR");
  p_table->ImportSemaphoreWin32HandleKHR                          = (PFN_vkImportSemaphoreWin32HandleKHR)fnProcLoad(device, "vkImportSemaphoreWin32HandleKHR");
  p_table->GetSemaphoreWin32HandleKHR                             = (PFN_vkGetSemaphoreWin32HandleKHR)fnProcLoad(device, "vkGetSemaphoreWin32HandleKHR");
  p_table->ImportFenceWin32HandleKHR                              = (PFN_vkImportFenceWin32HandleKHR)fnProcLoad(device, "vkImportFenceWin32HandleKHR");
  p_table->GetFenceWin32HandleKHR                                 = (PFN_vkGetFenceWin32HandleKHR)fnProcLoad(device, "vkGetFenceWin32HandleKHR");
  p_table->GetMemoryWin32HandleNV                                 = (PFN_vkGetMemoryWin32HandleNV)fnProcLoad(device, "vkGetMemoryWin32HandleNV");
#endif // defined(VK_USE_PLATFORM_WIN32_KHR)

#if defined(VK_USE_PLATFORM_XCB_KHR)
#endif // defined(VK_USE_PLATFORM_XCB_KHR)

#if defined(VK_USE_PLATFORM_XLIB_KHR)
#endif // defined(VK_USE_PLATFORM_XLIB_KHR)

#if defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
#endif // defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
}

// =================================================================================================
// Load functions for load using proc address
// =================================================================================================
static void LoadProcAddrInstance(VkInstance instance, vkex::GetInstanceProcType fnProcLoad, vkex::InstanceFunctionTable* p_table)
{
  p_table->CreateInstance                                         = (PFN_vkCreateInstance)fnProcLoad(instance, "vkCreateInstance");
  p_table->DestroyInstance                                        = (PFN_vkDestroyInstance)fnProcLoad(instance, "vkDestroyInstance");
  p_table->EnumeratePhysicalDevices                               = (PFN_vkEnumeratePhysicalDevices)fnProcLoad(instance, "vkEnumeratePhysicalDevices");
  p_table->GetPhysicalDeviceFeatures                              = (PFN_vkGetPhysicalDeviceFeatures)fnProcLoad(instance, "vkGetPhysicalDeviceFeatures");
  p_table->GetPhysicalDeviceFormatProperties                      = (PFN_vkGetPhysicalDeviceFormatProperties)fnProcLoad(instance, "vkGetPhysicalDeviceFormatProperties");
  p_table->GetPhysicalDeviceImageFormatProperties                 = (PFN_vkGetPhysicalDeviceImageFormatProperties)fnProcLoad(instance, "vkGetPhysicalDeviceImageFormatProperties");
  p_table->GetPhysicalDeviceProperties                            = (PFN_vkGetPhysicalDeviceProperties)fnProcLoad(instance, "vkGetPhysicalDeviceProperties");
  p_table->GetPhysicalDeviceQueueFamilyProperties                 = (PFN_vkGetPhysicalDeviceQueueFamilyProperties)fnProcLoad(instance, "vkGetPhysicalDeviceQueueFamilyProperties");
  p_table->GetPhysicalDeviceMemoryProperties                      = (PFN_vkGetPhysicalDeviceMemoryProperties)fnProcLoad(instance, "vkGetPhysicalDeviceMemoryProperties");
  p_table->GetInstanceProcAddr                                    = (PFN_vkGetInstanceProcAddr)fnProcLoad(instance, "vkGetInstanceProcAddr");
  p_table->CreateDevice                                           = (PFN_vkCreateDevice)fnProcLoad(instance, "vkCreateDevice");
  p_table->EnumerateInstanceExtensionProperties                   = (PFN_vkEnumerateInstanceExtensionProperties)fnProcLoad(instance, "vkEnumerateInstanceExtensionProperties");
  p_table->EnumerateDeviceExtensionProperties                     = (PFN_vkEnumerateDeviceExtensionProperties)fnProcLoad(instance, "vkEnumerateDeviceExtensionProperties");
  p_table->EnumerateInstanceLayerProperties                       = (PFN_vkEnumerateInstanceLayerProperties)fnProcLoad(instance, "vkEnumerateInstanceLayerProperties");
  p_table->EnumerateDeviceLayerProperties                         = (PFN_vkEnumerateDeviceLayerProperties)fnProcLoad(instance, "vkEnumerateDeviceLayerProperties");
  p_table->GetPhysicalDeviceSparseImageFormatProperties           = (PFN_vkGetPhysicalDeviceSparseImageFormatProperties)fnProcLoad(instance, "vkGetPhysicalDeviceSparseImageFormatProperties");
  p_table->EnumerateInstanceVersion                               = (PFN_vkEnumerateInstanceVersion)fnProcLoad(instance, "vkEnumerateInstanceVersion");
  p_table->EnumeratePhysicalDeviceGroups                          = (PFN_vkEnumeratePhysicalDeviceGroups)fnProcLoad(instance, "vkEnumeratePhysicalDeviceGroups");
  p_table->GetPhysicalDeviceFeatures2                             = (PFN_vkGetPhysicalDeviceFeatures2)fnProcLoad(instance, "vkGetPhysicalDeviceFeatures2");
  p_table->GetPhysicalDeviceProperties2                           = (PFN_vkGetPhysicalDeviceProperties2)fnProcLoad(instance, "vkGetPhysicalDeviceProperties2");
  p_table->GetPhysicalDeviceFormatProperties2                     = (PFN_vkGetPhysicalDeviceFormatProperties2)fnProcLoad(instance, "vkGetPhysicalDeviceFormatProperties2");
  p_table->GetPhysicalDeviceImageFormatProperties2                = (PFN_vkGetPhysicalDeviceImageFormatProperties2)fnProcLoad(instance, "vkGetPhysicalDeviceImageFormatProperties2");
  p_table->GetPhysicalDeviceQueueFamilyProperties2                = (PFN_vkGetPhysicalDeviceQueueFamilyProperties2)fnProcLoad(instance, "vkGetPhysicalDeviceQueueFamilyProperties2");
  p_table->GetPhysicalDeviceMemoryProperties2                     = (PFN_vkGetPhysicalDeviceMemoryProperties2)fnProcLoad(instance, "vkGetPhysicalDeviceMemoryProperties2");
  p_table->GetPhysicalDeviceSparseImageFormatProperties2          = (PFN_vkGetPhysicalDeviceSparseImageFormatProperties2)fnProcLoad(instance, "vkGetPhysicalDeviceSparseImageFormatProperties2");
  p_table->GetPhysicalDeviceExternalBufferProperties              = (PFN_vkGetPhysicalDeviceExternalBufferProperties)fnProcLoad(instance, "vkGetPhysicalDeviceExternalBufferProperties");
  p_table->GetPhysicalDeviceExternalFenceProperties               = (PFN_vkGetPhysicalDeviceExternalFenceProperties)fnProcLoad(instance, "vkGetPhysicalDeviceExternalFenceProperties");
  p_table->GetPhysicalDeviceExternalSemaphoreProperties           = (PFN_vkGetPhysicalDeviceExternalSemaphoreProperties)fnProcLoad(instance, "vkGetPhysicalDeviceExternalSemaphoreProperties");
  p_table->DestroySurfaceKHR                                      = (PFN_vkDestroySurfaceKHR)fnProcLoad(instance, "vkDestroySurfaceKHR");
  p_table->GetPhysicalDeviceSurfaceSupportKHR                     = (PFN_vkGetPhysicalDeviceSurfaceSupportKHR)fnProcLoad(instance, "vkGetPhysicalDeviceSurfaceSupportKHR");
  p_table->GetPhysicalDeviceSurfaceCapabilitiesKHR                = (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR)fnProcLoad(instance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
  p_table->GetPhysicalDeviceSurfaceFormatsKHR                     = (PFN_vkGetPhysicalDeviceSurfaceFormatsKHR)fnProcLoad(instance, "vkGetPhysicalDeviceSurfaceFormatsKHR");
  p_table->GetPhysicalDeviceSurfacePresentModesKHR                = (PFN_vkGetPhysicalDeviceSurfacePresentModesKHR)fnProcLoad(instance, "vkGetPhysicalDeviceSurfacePresentModesKHR");
  p_table->GetPhysicalDevicePresentRectanglesKHR                  = (PFN_vkGetPhysicalDevicePresentRectanglesKHR)fnProcLoad(instance, "vkGetPhysicalDevicePresentRectanglesKHR");
  p_table->GetPhysicalDeviceDisplayPropertiesKHR                  = (PFN_vkGetPhysicalDeviceDisplayPropertiesKHR)fnProcLoad(instance, "vkGetPhysicalDeviceDisplayPropertiesKHR");
  p_table->GetPhysicalDeviceDisplayPlanePropertiesKHR             = (PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR)fnProcLoad(instance, "vkGetPhysicalDeviceDisplayPlanePropertiesKHR");
  p_table->GetDisplayPlaneSupportedDisplaysKHR                    = (PFN_vkGetDisplayPlaneSupportedDisplaysKHR)fnProcLoad(instance, "vkGetDisplayPlaneSupportedDisplaysKHR");
  p_table->GetDisplayModePropertiesKHR                            = (PFN_vkGetDisplayModePropertiesKHR)fnProcLoad(instance, "vkGetDisplayModePropertiesKHR");
  p_table->CreateDisplayModeKHR                                   = (PFN_vkCreateDisplayModeKHR)fnProcLoad(instance, "vkCreateDisplayModeKHR");
  p_table->GetDisplayPlaneCapabilitiesKHR                         = (PFN_vkGetDisplayPlaneCapabilitiesKHR)fnProcLoad(instance, "vkGetDisplayPlaneCapabilitiesKHR");
  p_table->CreateDisplayPlaneSurfaceKHR                           = (PFN_vkCreateDisplayPlaneSurfaceKHR)fnProcLoad(instance, "vkCreateDisplayPlaneSurfaceKHR");
  p_table->GetPhysicalDeviceFeatures2KHR                          = (PFN_vkGetPhysicalDeviceFeatures2KHR)fnProcLoad(instance, "vkGetPhysicalDeviceFeatures2KHR");
  p_table->GetPhysicalDeviceProperties2KHR                        = (PFN_vkGetPhysicalDeviceProperties2KHR)fnProcLoad(instance, "vkGetPhysicalDeviceProperties2KHR");
  p_table->GetPhysicalDeviceFormatProperties2KHR                  = (PFN_vkGetPhysicalDeviceFormatProperties2KHR)fnProcLoad(instance, "vkGetPhysicalDeviceFormatProperties2KHR");
  p_table->GetPhysicalDeviceImageFormatProperties2KHR             = (PFN_vkGetPhysicalDeviceImageFormatProperties2KHR)fnProcLoad(instance, "vkGetPhysicalDeviceImageFormatProperties2KHR");
  p_table->GetPhysicalDeviceQueueFamilyProperties2KHR             = (PFN_vkGetPhysicalDeviceQueueFamilyProperties2KHR)fnProcLoad(instance, "vkGetPhysicalDeviceQueueFamilyProperties2KHR");
  p_table->GetPhysicalDeviceMemoryProperties2KHR                  = (PFN_vkGetPhysicalDeviceMemoryProperties2KHR)fnProcLoad(instance, "vkGetPhysicalDeviceMemoryProperties2KHR");
  p_table->GetPhysicalDeviceSparseImageFormatProperties2KHR       = (PFN_vkGetPhysicalDeviceSparseImageFormatProperties2KHR)fnProcLoad(instance, "vkGetPhysicalDeviceSparseImageFormatProperties2KHR");
  p_table->EnumeratePhysicalDeviceGroupsKHR                       = (PFN_vkEnumeratePhysicalDeviceGroupsKHR)fnProcLoad(instance, "vkEnumeratePhysicalDeviceGroupsKHR");
  p_table->GetPhysicalDeviceExternalBufferPropertiesKHR           = (PFN_vkGetPhysicalDeviceExternalBufferPropertiesKHR)fnProcLoad(instance, "vkGetPhysicalDeviceExternalBufferPropertiesKHR");
  p_table->GetPhysicalDeviceExternalSemaphorePropertiesKHR        = (PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR)fnProcLoad(instance, "vkGetPhysicalDeviceExternalSemaphorePropertiesKHR");
  p_table->GetPhysicalDeviceExternalFencePropertiesKHR            = (PFN_vkGetPhysicalDeviceExternalFencePropertiesKHR)fnProcLoad(instance, "vkGetPhysicalDeviceExternalFencePropertiesKHR");
  p_table->GetPhysicalDeviceSurfaceCapabilities2KHR               = (PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR)fnProcLoad(instance, "vkGetPhysicalDeviceSurfaceCapabilities2KHR");
  p_table->GetPhysicalDeviceSurfaceFormats2KHR                    = (PFN_vkGetPhysicalDeviceSurfaceFormats2KHR)fnProcLoad(instance, "vkGetPhysicalDeviceSurfaceFormats2KHR");
  p_table->GetPhysicalDeviceDisplayProperties2KHR                 = (PFN_vkGetPhysicalDeviceDisplayProperties2KHR)fnProcLoad(instance, "vkGetPhysicalDeviceDisplayProperties2KHR");
  p_table->GetPhysicalDeviceDisplayPlaneProperties2KHR            = (PFN_vkGetPhysicalDeviceDisplayPlaneProperties2KHR)fnProcLoad(instance, "vkGetPhysicalDeviceDisplayPlaneProperties2KHR");
  p_table->GetDisplayModeProperties2KHR                           = (PFN_vkGetDisplayModeProperties2KHR)fnProcLoad(instance, "vkGetDisplayModeProperties2KHR");
  p_table->GetDisplayPlaneCapabilities2KHR                        = (PFN_vkGetDisplayPlaneCapabilities2KHR)fnProcLoad(instance, "vkGetDisplayPlaneCapabilities2KHR");
  p_table->CreateDebugReportCallbackEXT                           = (PFN_vkCreateDebugReportCallbackEXT)fnProcLoad(instance, "vkCreateDebugReportCallbackEXT");
  p_table->DestroyDebugReportCallbackEXT                          = (PFN_vkDestroyDebugReportCallbackEXT)fnProcLoad(instance, "vkDestroyDebugReportCallbackEXT");
  p_table->DebugReportMessageEXT                                  = (PFN_vkDebugReportMessageEXT)fnProcLoad(instance, "vkDebugReportMessageEXT");
  p_table->GetPhysicalDeviceExternalImageFormatPropertiesNV       = (PFN_vkGetPhysicalDeviceExternalImageFormatPropertiesNV)fnProcLoad(instance, "vkGetPhysicalDeviceExternalImageFormatPropertiesNV");
  p_table->GetPhysicalDeviceGeneratedCommandsPropertiesNVX        = (PFN_vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX)fnProcLoad(instance, "vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX");
  p_table->ReleaseDisplayEXT                                      = (PFN_vkReleaseDisplayEXT)fnProcLoad(instance, "vkReleaseDisplayEXT");
  p_table->GetPhysicalDeviceSurfaceCapabilities2EXT               = (PFN_vkGetPhysicalDeviceSurfaceCapabilities2EXT)fnProcLoad(instance, "vkGetPhysicalDeviceSurfaceCapabilities2EXT");
  p_table->CreateDebugUtilsMessengerEXT                           = (PFN_vkCreateDebugUtilsMessengerEXT)fnProcLoad(instance, "vkCreateDebugUtilsMessengerEXT");
  p_table->DestroyDebugUtilsMessengerEXT                          = (PFN_vkDestroyDebugUtilsMessengerEXT)fnProcLoad(instance, "vkDestroyDebugUtilsMessengerEXT");
  p_table->SubmitDebugUtilsMessageEXT                             = (PFN_vkSubmitDebugUtilsMessageEXT)fnProcLoad(instance, "vkSubmitDebugUtilsMessageEXT");
  p_table->GetPhysicalDeviceMultisamplePropertiesEXT              = (PFN_vkGetPhysicalDeviceMultisamplePropertiesEXT)fnProcLoad(instance, "vkGetPhysicalDeviceMultisamplePropertiesEXT");

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
  p_table->CreateAndroidSurfaceKHR                                = (PFN_vkCreateAndroidSurfaceKHR)fnProcLoad(instance, "vkCreateAndroidSurfaceKHR");
#endif // defined(VK_USE_PLATFORM_ANDROID_KHR)

#if defined(VK_USE_PLATFORM_IOS_MVK)
  p_table->CreateIOSSurfaceMVK                                    = (PFN_vkCreateIOSSurfaceMVK)fnProcLoad(instance, "vkCreateIOSSurfaceMVK");
#endif // defined(VK_USE_PLATFORM_IOS_MVK)

#if defined(VK_USE_PLATFORM_MACOS_MVK)
  p_table->CreateMacOSSurfaceMVK                                  = (PFN_vkCreateMacOSSurfaceMVK)fnProcLoad(instance, "vkCreateMacOSSurfaceMVK");
#endif // defined(VK_USE_PLATFORM_MACOS_MVK)

#if defined(VK_USE_PLATFORM_MIR_KHR)
  p_table->CreateMirSurfaceKHR                                    = (PFN_vkCreateMirSurfaceKHR)fnProcLoad(instance, "vkCreateMirSurfaceKHR");
  p_table->GetPhysicalDeviceMirPresentationSupportKHR             = (PFN_vkGetPhysicalDeviceMirPresentationSupportKHR)fnProcLoad(instance, "vkGetPhysicalDeviceMirPresentationSupportKHR");
#endif // defined(VK_USE_PLATFORM_MIR_KHR)

#if defined(VK_USE_PLATFORM_VI_NN)
  p_table->CreateViSurfaceNN                                      = (PFN_vkCreateViSurfaceNN)fnProcLoad(instance, "vkCreateViSurfaceNN");
#endif // defined(VK_USE_PLATFORM_VI_NN)

#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
  p_table->CreateWaylandSurfaceKHR                                = (PFN_vkCreateWaylandSurfaceKHR)fnProcLoad(instance, "vkCreateWaylandSurfaceKHR");
  p_table->GetPhysicalDeviceWaylandPresentationSupportKHR         = (PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR)fnProcLoad(instance, "vkGetPhysicalDeviceWaylandPresentationSupportKHR");
#endif // defined(VK_USE_PLATFORM_WAYLAND_KHR)

#if defined(VK_USE_PLATFORM_WIN32_KHR)
  p_table->CreateWin32SurfaceKHR                                  = (PFN_vkCreateWin32SurfaceKHR)fnProcLoad(instance, "vkCreateWin32SurfaceKHR");
  p_table->GetPhysicalDeviceWin32PresentationSupportKHR           = (PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR)fnProcLoad(instance, "vkGetPhysicalDeviceWin32PresentationSupportKHR");
#endif // defined(VK_USE_PLATFORM_WIN32_KHR)

#if defined(VK_USE_PLATFORM_XCB_KHR)
  p_table->CreateXcbSurfaceKHR                                    = (PFN_vkCreateXcbSurfaceKHR)fnProcLoad(instance, "vkCreateXcbSurfaceKHR");
  p_table->GetPhysicalDeviceXcbPresentationSupportKHR             = (PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR)fnProcLoad(instance, "vkGetPhysicalDeviceXcbPresentationSupportKHR");
#endif // defined(VK_USE_PLATFORM_XCB_KHR)

#if defined(VK_USE_PLATFORM_XLIB_KHR)
  p_table->CreateXlibSurfaceKHR                                   = (PFN_vkCreateXlibSurfaceKHR)fnProcLoad(instance, "vkCreateXlibSurfaceKHR");
  p_table->GetPhysicalDeviceXlibPresentationSupportKHR            = (PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR)fnProcLoad(instance, "vkGetPhysicalDeviceXlibPresentationSupportKHR");
#endif // defined(VK_USE_PLATFORM_XLIB_KHR)

#if defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
  p_table->AcquireXlibDisplayEXT                                  = (PFN_vkAcquireXlibDisplayEXT)fnProcLoad(instance, "vkAcquireXlibDisplayEXT");
  p_table->GetRandROutputDisplayEXT                               = (PFN_vkGetRandROutputDisplayEXT)fnProcLoad(instance, "vkGetRandROutputDisplayEXT");
#endif // defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)

#if defined(VK_USE_PLATFORM_GGP_KHR)
  p_table->CreateYetiSurfaceGOOGLE                                = (PFN_vkCreateYetiSurfaceGOOGLE)fnProcLoad(instance, "vkCreateYetiSurfaceGOOGLE");
  p_table->GetPhysicalDeviceYetiPresentationSupportGOOGLE         = (PFN_vkGetPhysicalDeviceYetiPresentationSupportGOOGLE)fnProcLoad(instance, "vkGetPhysicalDeviceYetiPresentationSupportGOOGLE");
#endif // defined(VK_USE_PLATFORM_GGP_KHR)
}

static void LoadProcAddrDevice(VkDevice device, vkex::GetDeviceProcType fnProcLoad, vkex::DeviceFunctionTable* p_table)
{
  p_table->GetDeviceProcAddr                                      = (PFN_vkGetDeviceProcAddr)fnProcLoad(device, "vkGetDeviceProcAddr");
  p_table->DestroyDevice                                          = (PFN_vkDestroyDevice)fnProcLoad(device, "vkDestroyDevice");
  p_table->GetDeviceQueue                                         = (PFN_vkGetDeviceQueue)fnProcLoad(device, "vkGetDeviceQueue");
  p_table->QueueSubmit                                            = (PFN_vkQueueSubmit)fnProcLoad(device, "vkQueueSubmit");
  p_table->QueueWaitIdle                                          = (PFN_vkQueueWaitIdle)fnProcLoad(device, "vkQueueWaitIdle");
  p_table->DeviceWaitIdle                                         = (PFN_vkDeviceWaitIdle)fnProcLoad(device, "vkDeviceWaitIdle");
  p_table->AllocateMemory                                         = (PFN_vkAllocateMemory)fnProcLoad(device, "vkAllocateMemory");
  p_table->FreeMemory                                             = (PFN_vkFreeMemory)fnProcLoad(device, "vkFreeMemory");
  p_table->MapMemory                                              = (PFN_vkMapMemory)fnProcLoad(device, "vkMapMemory");
  p_table->UnmapMemory                                            = (PFN_vkUnmapMemory)fnProcLoad(device, "vkUnmapMemory");
  p_table->FlushMappedMemoryRanges                                = (PFN_vkFlushMappedMemoryRanges)fnProcLoad(device, "vkFlushMappedMemoryRanges");
  p_table->InvalidateMappedMemoryRanges                           = (PFN_vkInvalidateMappedMemoryRanges)fnProcLoad(device, "vkInvalidateMappedMemoryRanges");
  p_table->GetDeviceMemoryCommitment                              = (PFN_vkGetDeviceMemoryCommitment)fnProcLoad(device, "vkGetDeviceMemoryCommitment");
  p_table->BindBufferMemory                                       = (PFN_vkBindBufferMemory)fnProcLoad(device, "vkBindBufferMemory");
  p_table->BindImageMemory                                        = (PFN_vkBindImageMemory)fnProcLoad(device, "vkBindImageMemory");
  p_table->GetBufferMemoryRequirements                            = (PFN_vkGetBufferMemoryRequirements)fnProcLoad(device, "vkGetBufferMemoryRequirements");
  p_table->GetImageMemoryRequirements                             = (PFN_vkGetImageMemoryRequirements)fnProcLoad(device, "vkGetImageMemoryRequirements");
  p_table->GetImageSparseMemoryRequirements                       = (PFN_vkGetImageSparseMemoryRequirements)fnProcLoad(device, "vkGetImageSparseMemoryRequirements");
  p_table->QueueBindSparse                                        = (PFN_vkQueueBindSparse)fnProcLoad(device, "vkQueueBindSparse");
  p_table->CreateFence                                            = (PFN_vkCreateFence)fnProcLoad(device, "vkCreateFence");
  p_table->DestroyFence                                           = (PFN_vkDestroyFence)fnProcLoad(device, "vkDestroyFence");
  p_table->ResetFences                                            = (PFN_vkResetFences)fnProcLoad(device, "vkResetFences");
  p_table->GetFenceStatus                                         = (PFN_vkGetFenceStatus)fnProcLoad(device, "vkGetFenceStatus");
  p_table->WaitForFences                                          = (PFN_vkWaitForFences)fnProcLoad(device, "vkWaitForFences");
  p_table->CreateSemaphore                                        = (PFN_vkCreateSemaphore)fnProcLoad(device, "vkCreateSemaphore");
  p_table->DestroySemaphore                                       = (PFN_vkDestroySemaphore)fnProcLoad(device, "vkDestroySemaphore");
  p_table->CreateEvent                                            = (PFN_vkCreateEvent)fnProcLoad(device, "vkCreateEvent");
  p_table->DestroyEvent                                           = (PFN_vkDestroyEvent)fnProcLoad(device, "vkDestroyEvent");
  p_table->GetEventStatus                                         = (PFN_vkGetEventStatus)fnProcLoad(device, "vkGetEventStatus");
  p_table->SetEvent                                               = (PFN_vkSetEvent)fnProcLoad(device, "vkSetEvent");
  p_table->ResetEvent                                             = (PFN_vkResetEvent)fnProcLoad(device, "vkResetEvent");
  p_table->CreateQueryPool                                        = (PFN_vkCreateQueryPool)fnProcLoad(device, "vkCreateQueryPool");
  p_table->DestroyQueryPool                                       = (PFN_vkDestroyQueryPool)fnProcLoad(device, "vkDestroyQueryPool");
  p_table->GetQueryPoolResults                                    = (PFN_vkGetQueryPoolResults)fnProcLoad(device, "vkGetQueryPoolResults");
  p_table->CreateBuffer                                           = (PFN_vkCreateBuffer)fnProcLoad(device, "vkCreateBuffer");
  p_table->DestroyBuffer                                          = (PFN_vkDestroyBuffer)fnProcLoad(device, "vkDestroyBuffer");
  p_table->CreateBufferView                                       = (PFN_vkCreateBufferView)fnProcLoad(device, "vkCreateBufferView");
  p_table->DestroyBufferView                                      = (PFN_vkDestroyBufferView)fnProcLoad(device, "vkDestroyBufferView");
  p_table->CreateImage                                            = (PFN_vkCreateImage)fnProcLoad(device, "vkCreateImage");
  p_table->DestroyImage                                           = (PFN_vkDestroyImage)fnProcLoad(device, "vkDestroyImage");
  p_table->GetImageSubresourceLayout                              = (PFN_vkGetImageSubresourceLayout)fnProcLoad(device, "vkGetImageSubresourceLayout");
  p_table->CreateImageView                                        = (PFN_vkCreateImageView)fnProcLoad(device, "vkCreateImageView");
  p_table->DestroyImageView                                       = (PFN_vkDestroyImageView)fnProcLoad(device, "vkDestroyImageView");
  p_table->CreateShaderModule                                     = (PFN_vkCreateShaderModule)fnProcLoad(device, "vkCreateShaderModule");
  p_table->DestroyShaderModule                                    = (PFN_vkDestroyShaderModule)fnProcLoad(device, "vkDestroyShaderModule");
  p_table->CreatePipelineCache                                    = (PFN_vkCreatePipelineCache)fnProcLoad(device, "vkCreatePipelineCache");
  p_table->DestroyPipelineCache                                   = (PFN_vkDestroyPipelineCache)fnProcLoad(device, "vkDestroyPipelineCache");
  p_table->GetPipelineCacheData                                   = (PFN_vkGetPipelineCacheData)fnProcLoad(device, "vkGetPipelineCacheData");
  p_table->MergePipelineCaches                                    = (PFN_vkMergePipelineCaches)fnProcLoad(device, "vkMergePipelineCaches");
  p_table->CreateGraphicsPipelines                                = (PFN_vkCreateGraphicsPipelines)fnProcLoad(device, "vkCreateGraphicsPipelines");
  p_table->CreateComputePipelines                                 = (PFN_vkCreateComputePipelines)fnProcLoad(device, "vkCreateComputePipelines");
  p_table->DestroyPipeline                                        = (PFN_vkDestroyPipeline)fnProcLoad(device, "vkDestroyPipeline");
  p_table->CreatePipelineLayout                                   = (PFN_vkCreatePipelineLayout)fnProcLoad(device, "vkCreatePipelineLayout");
  p_table->DestroyPipelineLayout                                  = (PFN_vkDestroyPipelineLayout)fnProcLoad(device, "vkDestroyPipelineLayout");
  p_table->CreateSampler                                          = (PFN_vkCreateSampler)fnProcLoad(device, "vkCreateSampler");
  p_table->DestroySampler                                         = (PFN_vkDestroySampler)fnProcLoad(device, "vkDestroySampler");
  p_table->CreateDescriptorSetLayout                              = (PFN_vkCreateDescriptorSetLayout)fnProcLoad(device, "vkCreateDescriptorSetLayout");
  p_table->DestroyDescriptorSetLayout                             = (PFN_vkDestroyDescriptorSetLayout)fnProcLoad(device, "vkDestroyDescriptorSetLayout");
  p_table->CreateDescriptorPool                                   = (PFN_vkCreateDescriptorPool)fnProcLoad(device, "vkCreateDescriptorPool");
  p_table->DestroyDescriptorPool                                  = (PFN_vkDestroyDescriptorPool)fnProcLoad(device, "vkDestroyDescriptorPool");
  p_table->ResetDescriptorPool                                    = (PFN_vkResetDescriptorPool)fnProcLoad(device, "vkResetDescriptorPool");
  p_table->AllocateDescriptorSets                                 = (PFN_vkAllocateDescriptorSets)fnProcLoad(device, "vkAllocateDescriptorSets");
  p_table->FreeDescriptorSets                                     = (PFN_vkFreeDescriptorSets)fnProcLoad(device, "vkFreeDescriptorSets");
  p_table->UpdateDescriptorSets                                   = (PFN_vkUpdateDescriptorSets)fnProcLoad(device, "vkUpdateDescriptorSets");
  p_table->CreateFramebuffer                                      = (PFN_vkCreateFramebuffer)fnProcLoad(device, "vkCreateFramebuffer");
  p_table->DestroyFramebuffer                                     = (PFN_vkDestroyFramebuffer)fnProcLoad(device, "vkDestroyFramebuffer");
  p_table->CreateRenderPass                                       = (PFN_vkCreateRenderPass)fnProcLoad(device, "vkCreateRenderPass");
  p_table->DestroyRenderPass                                      = (PFN_vkDestroyRenderPass)fnProcLoad(device, "vkDestroyRenderPass");
  p_table->GetRenderAreaGranularity                               = (PFN_vkGetRenderAreaGranularity)fnProcLoad(device, "vkGetRenderAreaGranularity");
  p_table->CreateCommandPool                                      = (PFN_vkCreateCommandPool)fnProcLoad(device, "vkCreateCommandPool");
  p_table->DestroyCommandPool                                     = (PFN_vkDestroyCommandPool)fnProcLoad(device, "vkDestroyCommandPool");
  p_table->ResetCommandPool                                       = (PFN_vkResetCommandPool)fnProcLoad(device, "vkResetCommandPool");
  p_table->AllocateCommandBuffers                                 = (PFN_vkAllocateCommandBuffers)fnProcLoad(device, "vkAllocateCommandBuffers");
  p_table->FreeCommandBuffers                                     = (PFN_vkFreeCommandBuffers)fnProcLoad(device, "vkFreeCommandBuffers");
  p_table->BeginCommandBuffer                                     = (PFN_vkBeginCommandBuffer)fnProcLoad(device, "vkBeginCommandBuffer");
  p_table->EndCommandBuffer                                       = (PFN_vkEndCommandBuffer)fnProcLoad(device, "vkEndCommandBuffer");
  p_table->ResetCommandBuffer                                     = (PFN_vkResetCommandBuffer)fnProcLoad(device, "vkResetCommandBuffer");
  p_table->CmdBindPipeline                                        = (PFN_vkCmdBindPipeline)fnProcLoad(device, "vkCmdBindPipeline");
  p_table->CmdSetViewport                                         = (PFN_vkCmdSetViewport)fnProcLoad(device, "vkCmdSetViewport");
  p_table->CmdSetScissor                                          = (PFN_vkCmdSetScissor)fnProcLoad(device, "vkCmdSetScissor");
  p_table->CmdSetLineWidth                                        = (PFN_vkCmdSetLineWidth)fnProcLoad(device, "vkCmdSetLineWidth");
  p_table->CmdSetDepthBias                                        = (PFN_vkCmdSetDepthBias)fnProcLoad(device, "vkCmdSetDepthBias");
  p_table->CmdSetBlendConstants                                   = (PFN_vkCmdSetBlendConstants)fnProcLoad(device, "vkCmdSetBlendConstants");
  p_table->CmdSetDepthBounds                                      = (PFN_vkCmdSetDepthBounds)fnProcLoad(device, "vkCmdSetDepthBounds");
  p_table->CmdSetStencilCompareMask                               = (PFN_vkCmdSetStencilCompareMask)fnProcLoad(device, "vkCmdSetStencilCompareMask");
  p_table->CmdSetStencilWriteMask                                 = (PFN_vkCmdSetStencilWriteMask)fnProcLoad(device, "vkCmdSetStencilWriteMask");
  p_table->CmdSetStencilReference                                 = (PFN_vkCmdSetStencilReference)fnProcLoad(device, "vkCmdSetStencilReference");
  p_table->CmdBindDescriptorSets                                  = (PFN_vkCmdBindDescriptorSets)fnProcLoad(device, "vkCmdBindDescriptorSets");
  p_table->CmdBindIndexBuffer                                     = (PFN_vkCmdBindIndexBuffer)fnProcLoad(device, "vkCmdBindIndexBuffer");
  p_table->CmdBindVertexBuffers                                   = (PFN_vkCmdBindVertexBuffers)fnProcLoad(device, "vkCmdBindVertexBuffers");
  p_table->CmdDraw                                                = (PFN_vkCmdDraw)fnProcLoad(device, "vkCmdDraw");
  p_table->CmdDrawIndexed                                         = (PFN_vkCmdDrawIndexed)fnProcLoad(device, "vkCmdDrawIndexed");
  p_table->CmdDrawIndirect                                        = (PFN_vkCmdDrawIndirect)fnProcLoad(device, "vkCmdDrawIndirect");
  p_table->CmdDrawIndexedIndirect                                 = (PFN_vkCmdDrawIndexedIndirect)fnProcLoad(device, "vkCmdDrawIndexedIndirect");
  p_table->CmdDispatch                                            = (PFN_vkCmdDispatch)fnProcLoad(device, "vkCmdDispatch");
  p_table->CmdDispatchIndirect                                    = (PFN_vkCmdDispatchIndirect)fnProcLoad(device, "vkCmdDispatchIndirect");
  p_table->CmdCopyBuffer                                          = (PFN_vkCmdCopyBuffer)fnProcLoad(device, "vkCmdCopyBuffer");
  p_table->CmdCopyImage                                           = (PFN_vkCmdCopyImage)fnProcLoad(device, "vkCmdCopyImage");
  p_table->CmdBlitImage                                           = (PFN_vkCmdBlitImage)fnProcLoad(device, "vkCmdBlitImage");
  p_table->CmdCopyBufferToImage                                   = (PFN_vkCmdCopyBufferToImage)fnProcLoad(device, "vkCmdCopyBufferToImage");
  p_table->CmdCopyImageToBuffer                                   = (PFN_vkCmdCopyImageToBuffer)fnProcLoad(device, "vkCmdCopyImageToBuffer");
  p_table->CmdUpdateBuffer                                        = (PFN_vkCmdUpdateBuffer)fnProcLoad(device, "vkCmdUpdateBuffer");
  p_table->CmdFillBuffer                                          = (PFN_vkCmdFillBuffer)fnProcLoad(device, "vkCmdFillBuffer");
  p_table->CmdClearColorImage                                     = (PFN_vkCmdClearColorImage)fnProcLoad(device, "vkCmdClearColorImage");
  p_table->CmdClearDepthStencilImage                              = (PFN_vkCmdClearDepthStencilImage)fnProcLoad(device, "vkCmdClearDepthStencilImage");
  p_table->CmdClearAttachments                                    = (PFN_vkCmdClearAttachments)fnProcLoad(device, "vkCmdClearAttachments");
  p_table->CmdResolveImage                                        = (PFN_vkCmdResolveImage)fnProcLoad(device, "vkCmdResolveImage");
  p_table->CmdSetEvent                                            = (PFN_vkCmdSetEvent)fnProcLoad(device, "vkCmdSetEvent");
  p_table->CmdResetEvent                                          = (PFN_vkCmdResetEvent)fnProcLoad(device, "vkCmdResetEvent");
  p_table->CmdWaitEvents                                          = (PFN_vkCmdWaitEvents)fnProcLoad(device, "vkCmdWaitEvents");
  p_table->CmdPipelineBarrier                                     = (PFN_vkCmdPipelineBarrier)fnProcLoad(device, "vkCmdPipelineBarrier");
  p_table->CmdBeginQuery                                          = (PFN_vkCmdBeginQuery)fnProcLoad(device, "vkCmdBeginQuery");
  p_table->CmdEndQuery                                            = (PFN_vkCmdEndQuery)fnProcLoad(device, "vkCmdEndQuery");
  p_table->CmdResetQueryPool                                      = (PFN_vkCmdResetQueryPool)fnProcLoad(device, "vkCmdResetQueryPool");
  p_table->CmdWriteTimestamp                                      = (PFN_vkCmdWriteTimestamp)fnProcLoad(device, "vkCmdWriteTimestamp");
  p_table->CmdCopyQueryPoolResults                                = (PFN_vkCmdCopyQueryPoolResults)fnProcLoad(device, "vkCmdCopyQueryPoolResults");
  p_table->CmdPushConstants                                       = (PFN_vkCmdPushConstants)fnProcLoad(device, "vkCmdPushConstants");
  p_table->CmdBeginRenderPass                                     = (PFN_vkCmdBeginRenderPass)fnProcLoad(device, "vkCmdBeginRenderPass");
  p_table->CmdNextSubpass                                         = (PFN_vkCmdNextSubpass)fnProcLoad(device, "vkCmdNextSubpass");
  p_table->CmdEndRenderPass                                       = (PFN_vkCmdEndRenderPass)fnProcLoad(device, "vkCmdEndRenderPass");
  p_table->CmdExecuteCommands                                     = (PFN_vkCmdExecuteCommands)fnProcLoad(device, "vkCmdExecuteCommands");
  p_table->BindBufferMemory2                                      = (PFN_vkBindBufferMemory2)fnProcLoad(device, "vkBindBufferMemory2");
  p_table->BindImageMemory2                                       = (PFN_vkBindImageMemory2)fnProcLoad(device, "vkBindImageMemory2");
  p_table->GetDeviceGroupPeerMemoryFeatures                       = (PFN_vkGetDeviceGroupPeerMemoryFeatures)fnProcLoad(device, "vkGetDeviceGroupPeerMemoryFeatures");
  p_table->CmdSetDeviceMask                                       = (PFN_vkCmdSetDeviceMask)fnProcLoad(device, "vkCmdSetDeviceMask");
  p_table->CmdDispatchBase                                        = (PFN_vkCmdDispatchBase)fnProcLoad(device, "vkCmdDispatchBase");
  p_table->GetImageMemoryRequirements2                            = (PFN_vkGetImageMemoryRequirements2)fnProcLoad(device, "vkGetImageMemoryRequirements2");
  p_table->GetBufferMemoryRequirements2                           = (PFN_vkGetBufferMemoryRequirements2)fnProcLoad(device, "vkGetBufferMemoryRequirements2");
  p_table->GetImageSparseMemoryRequirements2                      = (PFN_vkGetImageSparseMemoryRequirements2)fnProcLoad(device, "vkGetImageSparseMemoryRequirements2");
  p_table->TrimCommandPool                                        = (PFN_vkTrimCommandPool)fnProcLoad(device, "vkTrimCommandPool");
  p_table->GetDeviceQueue2                                        = (PFN_vkGetDeviceQueue2)fnProcLoad(device, "vkGetDeviceQueue2");
  p_table->CreateSamplerYcbcrConversion                           = (PFN_vkCreateSamplerYcbcrConversion)fnProcLoad(device, "vkCreateSamplerYcbcrConversion");
  p_table->DestroySamplerYcbcrConversion                          = (PFN_vkDestroySamplerYcbcrConversion)fnProcLoad(device, "vkDestroySamplerYcbcrConversion");
  p_table->CreateDescriptorUpdateTemplate                         = (PFN_vkCreateDescriptorUpdateTemplate)fnProcLoad(device, "vkCreateDescriptorUpdateTemplate");
  p_table->DestroyDescriptorUpdateTemplate                        = (PFN_vkDestroyDescriptorUpdateTemplate)fnProcLoad(device, "vkDestroyDescriptorUpdateTemplate");
  p_table->UpdateDescriptorSetWithTemplate                        = (PFN_vkUpdateDescriptorSetWithTemplate)fnProcLoad(device, "vkUpdateDescriptorSetWithTemplate");
  p_table->GetDescriptorSetLayoutSupport                          = (PFN_vkGetDescriptorSetLayoutSupport)fnProcLoad(device, "vkGetDescriptorSetLayoutSupport");
  p_table->CreateSwapchainKHR                                     = (PFN_vkCreateSwapchainKHR)fnProcLoad(device, "vkCreateSwapchainKHR");
  p_table->DestroySwapchainKHR                                    = (PFN_vkDestroySwapchainKHR)fnProcLoad(device, "vkDestroySwapchainKHR");
  p_table->GetSwapchainImagesKHR                                  = (PFN_vkGetSwapchainImagesKHR)fnProcLoad(device, "vkGetSwapchainImagesKHR");
  p_table->AcquireNextImageKHR                                    = (PFN_vkAcquireNextImageKHR)fnProcLoad(device, "vkAcquireNextImageKHR");
  p_table->QueuePresentKHR                                        = (PFN_vkQueuePresentKHR)fnProcLoad(device, "vkQueuePresentKHR");
  p_table->GetDeviceGroupPresentCapabilitiesKHR                   = (PFN_vkGetDeviceGroupPresentCapabilitiesKHR)fnProcLoad(device, "vkGetDeviceGroupPresentCapabilitiesKHR");
  p_table->GetDeviceGroupSurfacePresentModesKHR                   = (PFN_vkGetDeviceGroupSurfacePresentModesKHR)fnProcLoad(device, "vkGetDeviceGroupSurfacePresentModesKHR");
  p_table->AcquireNextImage2KHR                                   = (PFN_vkAcquireNextImage2KHR)fnProcLoad(device, "vkAcquireNextImage2KHR");
  p_table->CreateSharedSwapchainsKHR                              = (PFN_vkCreateSharedSwapchainsKHR)fnProcLoad(device, "vkCreateSharedSwapchainsKHR");
  p_table->GetDeviceGroupPeerMemoryFeaturesKHR                    = (PFN_vkGetDeviceGroupPeerMemoryFeaturesKHR)fnProcLoad(device, "vkGetDeviceGroupPeerMemoryFeaturesKHR");
  p_table->CmdSetDeviceMaskKHR                                    = (PFN_vkCmdSetDeviceMaskKHR)fnProcLoad(device, "vkCmdSetDeviceMaskKHR");
  p_table->CmdDispatchBaseKHR                                     = (PFN_vkCmdDispatchBaseKHR)fnProcLoad(device, "vkCmdDispatchBaseKHR");
  p_table->TrimCommandPoolKHR                                     = (PFN_vkTrimCommandPoolKHR)fnProcLoad(device, "vkTrimCommandPoolKHR");
  p_table->GetMemoryFdKHR                                         = (PFN_vkGetMemoryFdKHR)fnProcLoad(device, "vkGetMemoryFdKHR");
  p_table->GetMemoryFdPropertiesKHR                               = (PFN_vkGetMemoryFdPropertiesKHR)fnProcLoad(device, "vkGetMemoryFdPropertiesKHR");
  p_table->ImportSemaphoreFdKHR                                   = (PFN_vkImportSemaphoreFdKHR)fnProcLoad(device, "vkImportSemaphoreFdKHR");
  p_table->GetSemaphoreFdKHR                                      = (PFN_vkGetSemaphoreFdKHR)fnProcLoad(device, "vkGetSemaphoreFdKHR");
  p_table->CmdPushDescriptorSetKHR                                = (PFN_vkCmdPushDescriptorSetKHR)fnProcLoad(device, "vkCmdPushDescriptorSetKHR");
  p_table->CmdPushDescriptorSetWithTemplateKHR                    = (PFN_vkCmdPushDescriptorSetWithTemplateKHR)fnProcLoad(device, "vkCmdPushDescriptorSetWithTemplateKHR");
  p_table->CreateDescriptorUpdateTemplateKHR                      = (PFN_vkCreateDescriptorUpdateTemplateKHR)fnProcLoad(device, "vkCreateDescriptorUpdateTemplateKHR");
  p_table->DestroyDescriptorUpdateTemplateKHR                     = (PFN_vkDestroyDescriptorUpdateTemplateKHR)fnProcLoad(device, "vkDestroyDescriptorUpdateTemplateKHR");
  p_table->UpdateDescriptorSetWithTemplateKHR                     = (PFN_vkUpdateDescriptorSetWithTemplateKHR)fnProcLoad(device, "vkUpdateDescriptorSetWithTemplateKHR");
  p_table->CreateRenderPass2KHR                                   = (PFN_vkCreateRenderPass2KHR)fnProcLoad(device, "vkCreateRenderPass2KHR");
  p_table->CmdBeginRenderPass2KHR                                 = (PFN_vkCmdBeginRenderPass2KHR)fnProcLoad(device, "vkCmdBeginRenderPass2KHR");
  p_table->CmdNextSubpass2KHR                                     = (PFN_vkCmdNextSubpass2KHR)fnProcLoad(device, "vkCmdNextSubpass2KHR");
  p_table->CmdEndRenderPass2KHR                                   = (PFN_vkCmdEndRenderPass2KHR)fnProcLoad(device, "vkCmdEndRenderPass2KHR");
  p_table->GetSwapchainStatusKHR                                  = (PFN_vkGetSwapchainStatusKHR)fnProcLoad(device, "vkGetSwapchainStatusKHR");
  p_table->ImportFenceFdKHR                                       = (PFN_vkImportFenceFdKHR)fnProcLoad(device, "vkImportFenceFdKHR");
  p_table->GetFenceFdKHR                                          = (PFN_vkGetFenceFdKHR)fnProcLoad(device, "vkGetFenceFdKHR");
  p_table->GetImageMemoryRequirements2KHR                         = (PFN_vkGetImageMemoryRequirements2KHR)fnProcLoad(device, "vkGetImageMemoryRequirements2KHR");
  p_table->GetBufferMemoryRequirements2KHR                        = (PFN_vkGetBufferMemoryRequirements2KHR)fnProcLoad(device, "vkGetBufferMemoryRequirements2KHR");
  p_table->GetImageSparseMemoryRequirements2KHR                   = (PFN_vkGetImageSparseMemoryRequirements2KHR)fnProcLoad(device, "vkGetImageSparseMemoryRequirements2KHR");
  p_table->CreateSamplerYcbcrConversionKHR                        = (PFN_vkCreateSamplerYcbcrConversionKHR)fnProcLoad(device, "vkCreateSamplerYcbcrConversionKHR");
  p_table->DestroySamplerYcbcrConversionKHR                       = (PFN_vkDestroySamplerYcbcrConversionKHR)fnProcLoad(device, "vkDestroySamplerYcbcrConversionKHR");
  p_table->BindBufferMemory2KHR                                   = (PFN_vkBindBufferMemory2KHR)fnProcLoad(device, "vkBindBufferMemory2KHR");
  p_table->BindImageMemory2KHR                                    = (PFN_vkBindImageMemory2KHR)fnProcLoad(device, "vkBindImageMemory2KHR");
  p_table->GetDescriptorSetLayoutSupportKHR                       = (PFN_vkGetDescriptorSetLayoutSupportKHR)fnProcLoad(device, "vkGetDescriptorSetLayoutSupportKHR");
  p_table->CmdDrawIndirectCountKHR                                = (PFN_vkCmdDrawIndirectCountKHR)fnProcLoad(device, "vkCmdDrawIndirectCountKHR");
  p_table->CmdDrawIndexedIndirectCountKHR                         = (PFN_vkCmdDrawIndexedIndirectCountKHR)fnProcLoad(device, "vkCmdDrawIndexedIndirectCountKHR");
  p_table->DebugReportCallbackEXT                                 = (PFN_vkDebugReportCallbackEXT)fnProcLoad(device, "vkDebugReportCallbackEXT");
  p_table->DebugMarkerSetObjectTagEXT                             = (PFN_vkDebugMarkerSetObjectTagEXT)fnProcLoad(device, "vkDebugMarkerSetObjectTagEXT");
  p_table->DebugMarkerSetObjectNameEXT                            = (PFN_vkDebugMarkerSetObjectNameEXT)fnProcLoad(device, "vkDebugMarkerSetObjectNameEXT");
  p_table->CmdDebugMarkerBeginEXT                                 = (PFN_vkCmdDebugMarkerBeginEXT)fnProcLoad(device, "vkCmdDebugMarkerBeginEXT");
  p_table->CmdDebugMarkerEndEXT                                   = (PFN_vkCmdDebugMarkerEndEXT)fnProcLoad(device, "vkCmdDebugMarkerEndEXT");
  p_table->CmdDebugMarkerInsertEXT                                = (PFN_vkCmdDebugMarkerInsertEXT)fnProcLoad(device, "vkCmdDebugMarkerInsertEXT");
  p_table->CmdDrawIndirectCountAMD                                = (PFN_vkCmdDrawIndirectCountAMD)fnProcLoad(device, "vkCmdDrawIndirectCountAMD");
  p_table->CmdDrawIndexedIndirectCountAMD                         = (PFN_vkCmdDrawIndexedIndirectCountAMD)fnProcLoad(device, "vkCmdDrawIndexedIndirectCountAMD");
  p_table->GetShaderInfoAMD                                       = (PFN_vkGetShaderInfoAMD)fnProcLoad(device, "vkGetShaderInfoAMD");
  p_table->CmdBeginConditionalRenderingEXT                        = (PFN_vkCmdBeginConditionalRenderingEXT)fnProcLoad(device, "vkCmdBeginConditionalRenderingEXT");
  p_table->CmdEndConditionalRenderingEXT                          = (PFN_vkCmdEndConditionalRenderingEXT)fnProcLoad(device, "vkCmdEndConditionalRenderingEXT");
  p_table->CmdProcessCommandsNVX                                  = (PFN_vkCmdProcessCommandsNVX)fnProcLoad(device, "vkCmdProcessCommandsNVX");
  p_table->CmdReserveSpaceForCommandsNVX                          = (PFN_vkCmdReserveSpaceForCommandsNVX)fnProcLoad(device, "vkCmdReserveSpaceForCommandsNVX");
  p_table->CreateIndirectCommandsLayoutNVX                        = (PFN_vkCreateIndirectCommandsLayoutNVX)fnProcLoad(device, "vkCreateIndirectCommandsLayoutNVX");
  p_table->DestroyIndirectCommandsLayoutNVX                       = (PFN_vkDestroyIndirectCommandsLayoutNVX)fnProcLoad(device, "vkDestroyIndirectCommandsLayoutNVX");
  p_table->CreateObjectTableNVX                                   = (PFN_vkCreateObjectTableNVX)fnProcLoad(device, "vkCreateObjectTableNVX");
  p_table->DestroyObjectTableNVX                                  = (PFN_vkDestroyObjectTableNVX)fnProcLoad(device, "vkDestroyObjectTableNVX");
  p_table->RegisterObjectsNVX                                     = (PFN_vkRegisterObjectsNVX)fnProcLoad(device, "vkRegisterObjectsNVX");
  p_table->UnregisterObjectsNVX                                   = (PFN_vkUnregisterObjectsNVX)fnProcLoad(device, "vkUnregisterObjectsNVX");
  p_table->CmdSetViewportWScalingNV                               = (PFN_vkCmdSetViewportWScalingNV)fnProcLoad(device, "vkCmdSetViewportWScalingNV");
  p_table->DisplayPowerControlEXT                                 = (PFN_vkDisplayPowerControlEXT)fnProcLoad(device, "vkDisplayPowerControlEXT");
  p_table->RegisterDeviceEventEXT                                 = (PFN_vkRegisterDeviceEventEXT)fnProcLoad(device, "vkRegisterDeviceEventEXT");
  p_table->RegisterDisplayEventEXT                                = (PFN_vkRegisterDisplayEventEXT)fnProcLoad(device, "vkRegisterDisplayEventEXT");
  p_table->GetSwapchainCounterEXT                                 = (PFN_vkGetSwapchainCounterEXT)fnProcLoad(device, "vkGetSwapchainCounterEXT");
  p_table->GetRefreshCycleDurationGOOGLE                          = (PFN_vkGetRefreshCycleDurationGOOGLE)fnProcLoad(device, "vkGetRefreshCycleDurationGOOGLE");
  p_table->GetPastPresentationTimingGOOGLE                        = (PFN_vkGetPastPresentationTimingGOOGLE)fnProcLoad(device, "vkGetPastPresentationTimingGOOGLE");
  p_table->CmdSetDiscardRectangleEXT                              = (PFN_vkCmdSetDiscardRectangleEXT)fnProcLoad(device, "vkCmdSetDiscardRectangleEXT");
  p_table->SetHdrMetadataEXT                                      = (PFN_vkSetHdrMetadataEXT)fnProcLoad(device, "vkSetHdrMetadataEXT");
  p_table->DebugUtilsMessengerCallbackEXT                         = (PFN_vkDebugUtilsMessengerCallbackEXT)fnProcLoad(device, "vkDebugUtilsMessengerCallbackEXT");
  p_table->SetDebugUtilsObjectNameEXT                             = (PFN_vkSetDebugUtilsObjectNameEXT)fnProcLoad(device, "vkSetDebugUtilsObjectNameEXT");
  p_table->SetDebugUtilsObjectTagEXT                              = (PFN_vkSetDebugUtilsObjectTagEXT)fnProcLoad(device, "vkSetDebugUtilsObjectTagEXT");
  p_table->QueueBeginDebugUtilsLabelEXT                           = (PFN_vkQueueBeginDebugUtilsLabelEXT)fnProcLoad(device, "vkQueueBeginDebugUtilsLabelEXT");
  p_table->QueueEndDebugUtilsLabelEXT                             = (PFN_vkQueueEndDebugUtilsLabelEXT)fnProcLoad(device, "vkQueueEndDebugUtilsLabelEXT");
  p_table->QueueInsertDebugUtilsLabelEXT                          = (PFN_vkQueueInsertDebugUtilsLabelEXT)fnProcLoad(device, "vkQueueInsertDebugUtilsLabelEXT");
  p_table->CmdBeginDebugUtilsLabelEXT                             = (PFN_vkCmdBeginDebugUtilsLabelEXT)fnProcLoad(device, "vkCmdBeginDebugUtilsLabelEXT");
  p_table->CmdEndDebugUtilsLabelEXT                               = (PFN_vkCmdEndDebugUtilsLabelEXT)fnProcLoad(device, "vkCmdEndDebugUtilsLabelEXT");
  p_table->CmdInsertDebugUtilsLabelEXT                            = (PFN_vkCmdInsertDebugUtilsLabelEXT)fnProcLoad(device, "vkCmdInsertDebugUtilsLabelEXT");
  p_table->CmdSetSampleLocationsEXT                               = (PFN_vkCmdSetSampleLocationsEXT)fnProcLoad(device, "vkCmdSetSampleLocationsEXT");
  p_table->CreateValidationCacheEXT                               = (PFN_vkCreateValidationCacheEXT)fnProcLoad(device, "vkCreateValidationCacheEXT");
  p_table->DestroyValidationCacheEXT                              = (PFN_vkDestroyValidationCacheEXT)fnProcLoad(device, "vkDestroyValidationCacheEXT");
  p_table->MergeValidationCachesEXT                               = (PFN_vkMergeValidationCachesEXT)fnProcLoad(device, "vkMergeValidationCachesEXT");
  p_table->GetValidationCacheDataEXT                              = (PFN_vkGetValidationCacheDataEXT)fnProcLoad(device, "vkGetValidationCacheDataEXT");
  p_table->GetMemoryHostPointerPropertiesEXT                      = (PFN_vkGetMemoryHostPointerPropertiesEXT)fnProcLoad(device, "vkGetMemoryHostPointerPropertiesEXT");
  p_table->CmdWriteBufferMarkerAMD                                = (PFN_vkCmdWriteBufferMarkerAMD)fnProcLoad(device, "vkCmdWriteBufferMarkerAMD");
  p_table->CmdSetCheckpointNV                                     = (PFN_vkCmdSetCheckpointNV)fnProcLoad(device, "vkCmdSetCheckpointNV");
  p_table->GetQueueCheckpointDataNV                               = (PFN_vkGetQueueCheckpointDataNV)fnProcLoad(device, "vkGetQueueCheckpointDataNV");

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
  p_table->GetAndroidHardwareBufferPropertiesANDROID              = (PFN_vkGetAndroidHardwareBufferPropertiesANDROID)fnProcLoad(device, "vkGetAndroidHardwareBufferPropertiesANDROID");
  p_table->GetMemoryAndroidHardwareBufferANDROID                  = (PFN_vkGetMemoryAndroidHardwareBufferANDROID)fnProcLoad(device, "vkGetMemoryAndroidHardwareBufferANDROID");
#endif // defined(VK_USE_PLATFORM_ANDROID_KHR)

#if defined(VK_USE_PLATFORM_IOS_MVK)
#endif // defined(VK_USE_PLATFORM_IOS_MVK)

#if defined(VK_USE_PLATFORM_MACOS_MVK)
#endif // defined(VK_USE_PLATFORM_MACOS_MVK)

#if defined(VK_USE_PLATFORM_MIR_KHR)
#endif // defined(VK_USE_PLATFORM_MIR_KHR)

#if defined(VK_USE_PLATFORM_VI_NN)
#endif // defined(VK_USE_PLATFORM_VI_NN)

#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
#endif // defined(VK_USE_PLATFORM_WAYLAND_KHR)

#if defined(VK_USE_PLATFORM_WIN32_KHR)
  p_table->GetMemoryWin32HandleKHR                                = (PFN_vkGetMemoryWin32HandleKHR)fnProcLoad(device, "vkGetMemoryWin32HandleKHR");
  p_table->GetMemoryWin32HandlePropertiesKHR                      = (PFN_vkGetMemoryWin32HandlePropertiesKHR)fnProcLoad(device, "vkGetMemoryWin32HandlePropertiesKHR");
  p_table->ImportSemaphoreWin32HandleKHR                          = (PFN_vkImportSemaphoreWin32HandleKHR)fnProcLoad(device, "vkImportSemaphoreWin32HandleKHR");
  p_table->GetSemaphoreWin32HandleKHR                             = (PFN_vkGetSemaphoreWin32HandleKHR)fnProcLoad(device, "vkGetSemaphoreWin32HandleKHR");
  p_table->ImportFenceWin32HandleKHR                              = (PFN_vkImportFenceWin32HandleKHR)fnProcLoad(device, "vkImportFenceWin32HandleKHR");
  p_table->GetFenceWin32HandleKHR                                 = (PFN_vkGetFenceWin32HandleKHR)fnProcLoad(device, "vkGetFenceWin32HandleKHR");
  p_table->GetMemoryWin32HandleNV                                 = (PFN_vkGetMemoryWin32HandleNV)fnProcLoad(device, "vkGetMemoryWin32HandleNV");
#endif // defined(VK_USE_PLATFORM_WIN32_KHR)

#if defined(VK_USE_PLATFORM_XCB_KHR)
#endif // defined(VK_USE_PLATFORM_XCB_KHR)

#if defined(VK_USE_PLATFORM_XLIB_KHR)
#endif // defined(VK_USE_PLATFORM_XLIB_KHR)

#if defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
#endif // defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
}

// =================================================================================================
// Assignment functions
// =================================================================================================
static void AssignProcsInstance(const vkex::InstanceFunctionTable* p_table)
{
  vkex::CreateInstance                                         = p_table->CreateInstance;
  vkex::DestroyInstance                                        = p_table->DestroyInstance;
  vkex::EnumeratePhysicalDevices                               = p_table->EnumeratePhysicalDevices;
  vkex::GetPhysicalDeviceFeatures                              = p_table->GetPhysicalDeviceFeatures;
  vkex::GetPhysicalDeviceFormatProperties                      = p_table->GetPhysicalDeviceFormatProperties;
  vkex::GetPhysicalDeviceImageFormatProperties                 = p_table->GetPhysicalDeviceImageFormatProperties;
  vkex::GetPhysicalDeviceProperties                            = p_table->GetPhysicalDeviceProperties;
  vkex::GetPhysicalDeviceQueueFamilyProperties                 = p_table->GetPhysicalDeviceQueueFamilyProperties;
  vkex::GetPhysicalDeviceMemoryProperties                      = p_table->GetPhysicalDeviceMemoryProperties;
  vkex::GetInstanceProcAddr                                    = p_table->GetInstanceProcAddr;
  vkex::CreateDevice                                           = p_table->CreateDevice;
  vkex::EnumerateInstanceExtensionProperties                   = p_table->EnumerateInstanceExtensionProperties;
  vkex::EnumerateDeviceExtensionProperties                     = p_table->EnumerateDeviceExtensionProperties;
  vkex::EnumerateInstanceLayerProperties                       = p_table->EnumerateInstanceLayerProperties;
  vkex::EnumerateDeviceLayerProperties                         = p_table->EnumerateDeviceLayerProperties;
  vkex::GetPhysicalDeviceSparseImageFormatProperties           = p_table->GetPhysicalDeviceSparseImageFormatProperties;
  vkex::EnumerateInstanceVersion                               = p_table->EnumerateInstanceVersion;
  vkex::EnumeratePhysicalDeviceGroups                          = p_table->EnumeratePhysicalDeviceGroups;
  vkex::GetPhysicalDeviceFeatures2                             = p_table->GetPhysicalDeviceFeatures2;
  vkex::GetPhysicalDeviceProperties2                           = p_table->GetPhysicalDeviceProperties2;
  vkex::GetPhysicalDeviceFormatProperties2                     = p_table->GetPhysicalDeviceFormatProperties2;
  vkex::GetPhysicalDeviceImageFormatProperties2                = p_table->GetPhysicalDeviceImageFormatProperties2;
  vkex::GetPhysicalDeviceQueueFamilyProperties2                = p_table->GetPhysicalDeviceQueueFamilyProperties2;
  vkex::GetPhysicalDeviceMemoryProperties2                     = p_table->GetPhysicalDeviceMemoryProperties2;
  vkex::GetPhysicalDeviceSparseImageFormatProperties2          = p_table->GetPhysicalDeviceSparseImageFormatProperties2;
  vkex::GetPhysicalDeviceExternalBufferProperties              = p_table->GetPhysicalDeviceExternalBufferProperties;
  vkex::GetPhysicalDeviceExternalFenceProperties               = p_table->GetPhysicalDeviceExternalFenceProperties;
  vkex::GetPhysicalDeviceExternalSemaphoreProperties           = p_table->GetPhysicalDeviceExternalSemaphoreProperties;
  vkex::DestroySurfaceKHR                                      = p_table->DestroySurfaceKHR;
  vkex::GetPhysicalDeviceSurfaceSupportKHR                     = p_table->GetPhysicalDeviceSurfaceSupportKHR;
  vkex::GetPhysicalDeviceSurfaceCapabilitiesKHR                = p_table->GetPhysicalDeviceSurfaceCapabilitiesKHR;
  vkex::GetPhysicalDeviceSurfaceFormatsKHR                     = p_table->GetPhysicalDeviceSurfaceFormatsKHR;
  vkex::GetPhysicalDeviceSurfacePresentModesKHR                = p_table->GetPhysicalDeviceSurfacePresentModesKHR;
  vkex::GetPhysicalDevicePresentRectanglesKHR                  = p_table->GetPhysicalDevicePresentRectanglesKHR;
  vkex::GetPhysicalDeviceDisplayPropertiesKHR                  = p_table->GetPhysicalDeviceDisplayPropertiesKHR;
  vkex::GetPhysicalDeviceDisplayPlanePropertiesKHR             = p_table->GetPhysicalDeviceDisplayPlanePropertiesKHR;
  vkex::GetDisplayPlaneSupportedDisplaysKHR                    = p_table->GetDisplayPlaneSupportedDisplaysKHR;
  vkex::GetDisplayModePropertiesKHR                            = p_table->GetDisplayModePropertiesKHR;
  vkex::CreateDisplayModeKHR                                   = p_table->CreateDisplayModeKHR;
  vkex::GetDisplayPlaneCapabilitiesKHR                         = p_table->GetDisplayPlaneCapabilitiesKHR;
  vkex::CreateDisplayPlaneSurfaceKHR                           = p_table->CreateDisplayPlaneSurfaceKHR;
  vkex::GetPhysicalDeviceFeatures2KHR                          = p_table->GetPhysicalDeviceFeatures2KHR;
  vkex::GetPhysicalDeviceProperties2KHR                        = p_table->GetPhysicalDeviceProperties2KHR;
  vkex::GetPhysicalDeviceFormatProperties2KHR                  = p_table->GetPhysicalDeviceFormatProperties2KHR;
  vkex::GetPhysicalDeviceImageFormatProperties2KHR             = p_table->GetPhysicalDeviceImageFormatProperties2KHR;
  vkex::GetPhysicalDeviceQueueFamilyProperties2KHR             = p_table->GetPhysicalDeviceQueueFamilyProperties2KHR;
  vkex::GetPhysicalDeviceMemoryProperties2KHR                  = p_table->GetPhysicalDeviceMemoryProperties2KHR;
  vkex::GetPhysicalDeviceSparseImageFormatProperties2KHR       = p_table->GetPhysicalDeviceSparseImageFormatProperties2KHR;
  vkex::EnumeratePhysicalDeviceGroupsKHR                       = p_table->EnumeratePhysicalDeviceGroupsKHR;
  vkex::GetPhysicalDeviceExternalBufferPropertiesKHR           = p_table->GetPhysicalDeviceExternalBufferPropertiesKHR;
  vkex::GetPhysicalDeviceExternalSemaphorePropertiesKHR        = p_table->GetPhysicalDeviceExternalSemaphorePropertiesKHR;
  vkex::GetPhysicalDeviceExternalFencePropertiesKHR            = p_table->GetPhysicalDeviceExternalFencePropertiesKHR;
  vkex::GetPhysicalDeviceSurfaceCapabilities2KHR               = p_table->GetPhysicalDeviceSurfaceCapabilities2KHR;
  vkex::GetPhysicalDeviceSurfaceFormats2KHR                    = p_table->GetPhysicalDeviceSurfaceFormats2KHR;
  vkex::GetPhysicalDeviceDisplayProperties2KHR                 = p_table->GetPhysicalDeviceDisplayProperties2KHR;
  vkex::GetPhysicalDeviceDisplayPlaneProperties2KHR            = p_table->GetPhysicalDeviceDisplayPlaneProperties2KHR;
  vkex::GetDisplayModeProperties2KHR                           = p_table->GetDisplayModeProperties2KHR;
  vkex::GetDisplayPlaneCapabilities2KHR                        = p_table->GetDisplayPlaneCapabilities2KHR;
  vkex::CreateDebugReportCallbackEXT                           = p_table->CreateDebugReportCallbackEXT;
  vkex::DestroyDebugReportCallbackEXT                          = p_table->DestroyDebugReportCallbackEXT;
  vkex::DebugReportMessageEXT                                  = p_table->DebugReportMessageEXT;
  vkex::GetPhysicalDeviceExternalImageFormatPropertiesNV       = p_table->GetPhysicalDeviceExternalImageFormatPropertiesNV;
  vkex::GetPhysicalDeviceGeneratedCommandsPropertiesNVX        = p_table->GetPhysicalDeviceGeneratedCommandsPropertiesNVX;
  vkex::ReleaseDisplayEXT                                      = p_table->ReleaseDisplayEXT;
  vkex::GetPhysicalDeviceSurfaceCapabilities2EXT               = p_table->GetPhysicalDeviceSurfaceCapabilities2EXT;
  vkex::CreateDebugUtilsMessengerEXT                           = p_table->CreateDebugUtilsMessengerEXT;
  vkex::DestroyDebugUtilsMessengerEXT                          = p_table->DestroyDebugUtilsMessengerEXT;
  vkex::SubmitDebugUtilsMessageEXT                             = p_table->SubmitDebugUtilsMessageEXT;
  vkex::GetPhysicalDeviceMultisamplePropertiesEXT              = p_table->GetPhysicalDeviceMultisamplePropertiesEXT;

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
  vkex::CreateAndroidSurfaceKHR                                = p_table->CreateAndroidSurfaceKHR;
#endif // defined(VK_USE_PLATFORM_ANDROID_KHR)

#if defined(VK_USE_PLATFORM_IOS_MVK)
  vkex::CreateIOSSurfaceMVK                                    = p_table->CreateIOSSurfaceMVK;
#endif // defined(VK_USE_PLATFORM_IOS_MVK)

#if defined(VK_USE_PLATFORM_MACOS_MVK)
  vkex::CreateMacOSSurfaceMVK                                  = p_table->CreateMacOSSurfaceMVK;
#endif // defined(VK_USE_PLATFORM_MACOS_MVK)

#if defined(VK_USE_PLATFORM_MIR_KHR)
  vkex::CreateMirSurfaceKHR                                    = p_table->CreateMirSurfaceKHR;
  vkex::GetPhysicalDeviceMirPresentationSupportKHR             = p_table->GetPhysicalDeviceMirPresentationSupportKHR;
#endif // defined(VK_USE_PLATFORM_MIR_KHR)

#if defined(VK_USE_PLATFORM_VI_NN)
  vkex::CreateViSurfaceNN                                      = p_table->CreateViSurfaceNN;
#endif // defined(VK_USE_PLATFORM_VI_NN)

#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
  vkex::CreateWaylandSurfaceKHR                                = p_table->CreateWaylandSurfaceKHR;
  vkex::GetPhysicalDeviceWaylandPresentationSupportKHR         = p_table->GetPhysicalDeviceWaylandPresentationSupportKHR;
#endif // defined(VK_USE_PLATFORM_WAYLAND_KHR)

#if defined(VK_USE_PLATFORM_WIN32_KHR)
  vkex::CreateWin32SurfaceKHR                                  = p_table->CreateWin32SurfaceKHR;
  vkex::GetPhysicalDeviceWin32PresentationSupportKHR           = p_table->GetPhysicalDeviceWin32PresentationSupportKHR;
#endif // defined(VK_USE_PLATFORM_WIN32_KHR)

#if defined(VK_USE_PLATFORM_XCB_KHR)
  vkex::CreateXcbSurfaceKHR                                    = p_table->CreateXcbSurfaceKHR;
  vkex::GetPhysicalDeviceXcbPresentationSupportKHR             = p_table->GetPhysicalDeviceXcbPresentationSupportKHR;
#endif // defined(VK_USE_PLATFORM_XCB_KHR)

#if defined(VK_USE_PLATFORM_XLIB_KHR)
  vkex::CreateXlibSurfaceKHR                                   = p_table->CreateXlibSurfaceKHR;
  vkex::GetPhysicalDeviceXlibPresentationSupportKHR            = p_table->GetPhysicalDeviceXlibPresentationSupportKHR;
#endif // defined(VK_USE_PLATFORM_XLIB_KHR)

#if defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
  vkex::AcquireXlibDisplayEXT                                  = p_table->AcquireXlibDisplayEXT;
  vkex::GetRandROutputDisplayEXT                               = p_table->GetRandROutputDisplayEXT;
#endif // defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)

#if defined(VK_USE_PLATFORM_GGP_KHR)
  vkex::CreateYetiSurfaceGOOGLE                                = p_table->CreateYetiSurfaceGOOGLE;
  vkex::GetPhysicalDeviceYetiPresentationSupportGOOGLE         = p_table->GetPhysicalDeviceYetiPresentationSupportGOOGLE;
#endif // defined(VK_USE_PLATFORM_GGP_KHR)
}

static void AssignProcsDevice(const vkex::DeviceFunctionTable* p_table)
{
  vkex::GetDeviceProcAddr                                      = p_table->GetDeviceProcAddr;
  vkex::DestroyDevice                                          = p_table->DestroyDevice;
  vkex::GetDeviceQueue                                         = p_table->GetDeviceQueue;
  vkex::QueueSubmit                                            = p_table->QueueSubmit;
  vkex::QueueWaitIdle                                          = p_table->QueueWaitIdle;
  vkex::DeviceWaitIdle                                         = p_table->DeviceWaitIdle;
  vkex::AllocateMemory                                         = p_table->AllocateMemory;
  vkex::FreeMemory                                             = p_table->FreeMemory;
  vkex::MapMemory                                              = p_table->MapMemory;
  vkex::UnmapMemory                                            = p_table->UnmapMemory;
  vkex::FlushMappedMemoryRanges                                = p_table->FlushMappedMemoryRanges;
  vkex::InvalidateMappedMemoryRanges                           = p_table->InvalidateMappedMemoryRanges;
  vkex::GetDeviceMemoryCommitment                              = p_table->GetDeviceMemoryCommitment;
  vkex::BindBufferMemory                                       = p_table->BindBufferMemory;
  vkex::BindImageMemory                                        = p_table->BindImageMemory;
  vkex::GetBufferMemoryRequirements                            = p_table->GetBufferMemoryRequirements;
  vkex::GetImageMemoryRequirements                             = p_table->GetImageMemoryRequirements;
  vkex::GetImageSparseMemoryRequirements                       = p_table->GetImageSparseMemoryRequirements;
  vkex::QueueBindSparse                                        = p_table->QueueBindSparse;
  vkex::CreateFence                                            = p_table->CreateFence;
  vkex::DestroyFence                                           = p_table->DestroyFence;
  vkex::ResetFences                                            = p_table->ResetFences;
  vkex::GetFenceStatus                                         = p_table->GetFenceStatus;
  vkex::WaitForFences                                          = p_table->WaitForFences;
  vkex::CreateSemaphore                                        = p_table->CreateSemaphore;
  vkex::DestroySemaphore                                       = p_table->DestroySemaphore;
  vkex::CreateEvent                                            = p_table->CreateEvent;
  vkex::DestroyEvent                                           = p_table->DestroyEvent;
  vkex::GetEventStatus                                         = p_table->GetEventStatus;
  vkex::SetEvent                                               = p_table->SetEvent;
  vkex::ResetEvent                                             = p_table->ResetEvent;
  vkex::CreateQueryPool                                        = p_table->CreateQueryPool;
  vkex::DestroyQueryPool                                       = p_table->DestroyQueryPool;
  vkex::GetQueryPoolResults                                    = p_table->GetQueryPoolResults;
  vkex::CreateBuffer                                           = p_table->CreateBuffer;
  vkex::DestroyBuffer                                          = p_table->DestroyBuffer;
  vkex::CreateBufferView                                       = p_table->CreateBufferView;
  vkex::DestroyBufferView                                      = p_table->DestroyBufferView;
  vkex::CreateImage                                            = p_table->CreateImage;
  vkex::DestroyImage                                           = p_table->DestroyImage;
  vkex::GetImageSubresourceLayout                              = p_table->GetImageSubresourceLayout;
  vkex::CreateImageView                                        = p_table->CreateImageView;
  vkex::DestroyImageView                                       = p_table->DestroyImageView;
  vkex::CreateShaderModule                                     = p_table->CreateShaderModule;
  vkex::DestroyShaderModule                                    = p_table->DestroyShaderModule;
  vkex::CreatePipelineCache                                    = p_table->CreatePipelineCache;
  vkex::DestroyPipelineCache                                   = p_table->DestroyPipelineCache;
  vkex::GetPipelineCacheData                                   = p_table->GetPipelineCacheData;
  vkex::MergePipelineCaches                                    = p_table->MergePipelineCaches;
  vkex::CreateGraphicsPipelines                                = p_table->CreateGraphicsPipelines;
  vkex::CreateComputePipelines                                 = p_table->CreateComputePipelines;
  vkex::DestroyPipeline                                        = p_table->DestroyPipeline;
  vkex::CreatePipelineLayout                                   = p_table->CreatePipelineLayout;
  vkex::DestroyPipelineLayout                                  = p_table->DestroyPipelineLayout;
  vkex::CreateSampler                                          = p_table->CreateSampler;
  vkex::DestroySampler                                         = p_table->DestroySampler;
  vkex::CreateDescriptorSetLayout                              = p_table->CreateDescriptorSetLayout;
  vkex::DestroyDescriptorSetLayout                             = p_table->DestroyDescriptorSetLayout;
  vkex::CreateDescriptorPool                                   = p_table->CreateDescriptorPool;
  vkex::DestroyDescriptorPool                                  = p_table->DestroyDescriptorPool;
  vkex::ResetDescriptorPool                                    = p_table->ResetDescriptorPool;
  vkex::AllocateDescriptorSets                                 = p_table->AllocateDescriptorSets;
  vkex::FreeDescriptorSets                                     = p_table->FreeDescriptorSets;
  vkex::UpdateDescriptorSets                                   = p_table->UpdateDescriptorSets;
  vkex::CreateFramebuffer                                      = p_table->CreateFramebuffer;
  vkex::DestroyFramebuffer                                     = p_table->DestroyFramebuffer;
  vkex::CreateRenderPass                                       = p_table->CreateRenderPass;
  vkex::DestroyRenderPass                                      = p_table->DestroyRenderPass;
  vkex::GetRenderAreaGranularity                               = p_table->GetRenderAreaGranularity;
  vkex::CreateCommandPool                                      = p_table->CreateCommandPool;
  vkex::DestroyCommandPool                                     = p_table->DestroyCommandPool;
  vkex::ResetCommandPool                                       = p_table->ResetCommandPool;
  vkex::AllocateCommandBuffers                                 = p_table->AllocateCommandBuffers;
  vkex::FreeCommandBuffers                                     = p_table->FreeCommandBuffers;
  vkex::BeginCommandBuffer                                     = p_table->BeginCommandBuffer;
  vkex::EndCommandBuffer                                       = p_table->EndCommandBuffer;
  vkex::ResetCommandBuffer                                     = p_table->ResetCommandBuffer;
  vkex::CmdBindPipeline                                        = p_table->CmdBindPipeline;
  vkex::CmdSetViewport                                         = p_table->CmdSetViewport;
  vkex::CmdSetScissor                                          = p_table->CmdSetScissor;
  vkex::CmdSetLineWidth                                        = p_table->CmdSetLineWidth;
  vkex::CmdSetDepthBias                                        = p_table->CmdSetDepthBias;
  vkex::CmdSetBlendConstants                                   = p_table->CmdSetBlendConstants;
  vkex::CmdSetDepthBounds                                      = p_table->CmdSetDepthBounds;
  vkex::CmdSetStencilCompareMask                               = p_table->CmdSetStencilCompareMask;
  vkex::CmdSetStencilWriteMask                                 = p_table->CmdSetStencilWriteMask;
  vkex::CmdSetStencilReference                                 = p_table->CmdSetStencilReference;
  vkex::CmdBindDescriptorSets                                  = p_table->CmdBindDescriptorSets;
  vkex::CmdBindIndexBuffer                                     = p_table->CmdBindIndexBuffer;
  vkex::CmdBindVertexBuffers                                   = p_table->CmdBindVertexBuffers;
  vkex::CmdDraw                                                = p_table->CmdDraw;
  vkex::CmdDrawIndexed                                         = p_table->CmdDrawIndexed;
  vkex::CmdDrawIndirect                                        = p_table->CmdDrawIndirect;
  vkex::CmdDrawIndexedIndirect                                 = p_table->CmdDrawIndexedIndirect;
  vkex::CmdDispatch                                            = p_table->CmdDispatch;
  vkex::CmdDispatchIndirect                                    = p_table->CmdDispatchIndirect;
  vkex::CmdCopyBuffer                                          = p_table->CmdCopyBuffer;
  vkex::CmdCopyImage                                           = p_table->CmdCopyImage;
  vkex::CmdBlitImage                                           = p_table->CmdBlitImage;
  vkex::CmdCopyBufferToImage                                   = p_table->CmdCopyBufferToImage;
  vkex::CmdCopyImageToBuffer                                   = p_table->CmdCopyImageToBuffer;
  vkex::CmdUpdateBuffer                                        = p_table->CmdUpdateBuffer;
  vkex::CmdFillBuffer                                          = p_table->CmdFillBuffer;
  vkex::CmdClearColorImage                                     = p_table->CmdClearColorImage;
  vkex::CmdClearDepthStencilImage                              = p_table->CmdClearDepthStencilImage;
  vkex::CmdClearAttachments                                    = p_table->CmdClearAttachments;
  vkex::CmdResolveImage                                        = p_table->CmdResolveImage;
  vkex::CmdSetEvent                                            = p_table->CmdSetEvent;
  vkex::CmdResetEvent                                          = p_table->CmdResetEvent;
  vkex::CmdWaitEvents                                          = p_table->CmdWaitEvents;
  vkex::CmdPipelineBarrier                                     = p_table->CmdPipelineBarrier;
  vkex::CmdBeginQuery                                          = p_table->CmdBeginQuery;
  vkex::CmdEndQuery                                            = p_table->CmdEndQuery;
  vkex::CmdResetQueryPool                                      = p_table->CmdResetQueryPool;
  vkex::CmdWriteTimestamp                                      = p_table->CmdWriteTimestamp;
  vkex::CmdCopyQueryPoolResults                                = p_table->CmdCopyQueryPoolResults;
  vkex::CmdPushConstants                                       = p_table->CmdPushConstants;
  vkex::CmdBeginRenderPass                                     = p_table->CmdBeginRenderPass;
  vkex::CmdNextSubpass                                         = p_table->CmdNextSubpass;
  vkex::CmdEndRenderPass                                       = p_table->CmdEndRenderPass;
  vkex::CmdExecuteCommands                                     = p_table->CmdExecuteCommands;
  vkex::BindBufferMemory2                                      = p_table->BindBufferMemory2;
  vkex::BindImageMemory2                                       = p_table->BindImageMemory2;
  vkex::GetDeviceGroupPeerMemoryFeatures                       = p_table->GetDeviceGroupPeerMemoryFeatures;
  vkex::CmdSetDeviceMask                                       = p_table->CmdSetDeviceMask;
  vkex::CmdDispatchBase                                        = p_table->CmdDispatchBase;
  vkex::GetImageMemoryRequirements2                            = p_table->GetImageMemoryRequirements2;
  vkex::GetBufferMemoryRequirements2                           = p_table->GetBufferMemoryRequirements2;
  vkex::GetImageSparseMemoryRequirements2                      = p_table->GetImageSparseMemoryRequirements2;
  vkex::TrimCommandPool                                        = p_table->TrimCommandPool;
  vkex::GetDeviceQueue2                                        = p_table->GetDeviceQueue2;
  vkex::CreateSamplerYcbcrConversion                           = p_table->CreateSamplerYcbcrConversion;
  vkex::DestroySamplerYcbcrConversion                          = p_table->DestroySamplerYcbcrConversion;
  vkex::CreateDescriptorUpdateTemplate                         = p_table->CreateDescriptorUpdateTemplate;
  vkex::DestroyDescriptorUpdateTemplate                        = p_table->DestroyDescriptorUpdateTemplate;
  vkex::UpdateDescriptorSetWithTemplate                        = p_table->UpdateDescriptorSetWithTemplate;
  vkex::GetDescriptorSetLayoutSupport                          = p_table->GetDescriptorSetLayoutSupport;
  vkex::CreateSwapchainKHR                                     = p_table->CreateSwapchainKHR;
  vkex::DestroySwapchainKHR                                    = p_table->DestroySwapchainKHR;
  vkex::GetSwapchainImagesKHR                                  = p_table->GetSwapchainImagesKHR;
  vkex::AcquireNextImageKHR                                    = p_table->AcquireNextImageKHR;
  vkex::QueuePresentKHR                                        = p_table->QueuePresentKHR;
  vkex::GetDeviceGroupPresentCapabilitiesKHR                   = p_table->GetDeviceGroupPresentCapabilitiesKHR;
  vkex::GetDeviceGroupSurfacePresentModesKHR                   = p_table->GetDeviceGroupSurfacePresentModesKHR;
  vkex::AcquireNextImage2KHR                                   = p_table->AcquireNextImage2KHR;
  vkex::CreateSharedSwapchainsKHR                              = p_table->CreateSharedSwapchainsKHR;
  vkex::GetDeviceGroupPeerMemoryFeaturesKHR                    = p_table->GetDeviceGroupPeerMemoryFeaturesKHR;
  vkex::CmdSetDeviceMaskKHR                                    = p_table->CmdSetDeviceMaskKHR;
  vkex::CmdDispatchBaseKHR                                     = p_table->CmdDispatchBaseKHR;
  vkex::TrimCommandPoolKHR                                     = p_table->TrimCommandPoolKHR;
  vkex::GetMemoryFdKHR                                         = p_table->GetMemoryFdKHR;
  vkex::GetMemoryFdPropertiesKHR                               = p_table->GetMemoryFdPropertiesKHR;
  vkex::ImportSemaphoreFdKHR                                   = p_table->ImportSemaphoreFdKHR;
  vkex::GetSemaphoreFdKHR                                      = p_table->GetSemaphoreFdKHR;
  vkex::CmdPushDescriptorSetKHR                                = p_table->CmdPushDescriptorSetKHR;
  vkex::CmdPushDescriptorSetWithTemplateKHR                    = p_table->CmdPushDescriptorSetWithTemplateKHR;
  vkex::CreateDescriptorUpdateTemplateKHR                      = p_table->CreateDescriptorUpdateTemplateKHR;
  vkex::DestroyDescriptorUpdateTemplateKHR                     = p_table->DestroyDescriptorUpdateTemplateKHR;
  vkex::UpdateDescriptorSetWithTemplateKHR                     = p_table->UpdateDescriptorSetWithTemplateKHR;
  vkex::CreateRenderPass2KHR                                   = p_table->CreateRenderPass2KHR;
  vkex::CmdBeginRenderPass2KHR                                 = p_table->CmdBeginRenderPass2KHR;
  vkex::CmdNextSubpass2KHR                                     = p_table->CmdNextSubpass2KHR;
  vkex::CmdEndRenderPass2KHR                                   = p_table->CmdEndRenderPass2KHR;
  vkex::GetSwapchainStatusKHR                                  = p_table->GetSwapchainStatusKHR;
  vkex::ImportFenceFdKHR                                       = p_table->ImportFenceFdKHR;
  vkex::GetFenceFdKHR                                          = p_table->GetFenceFdKHR;
  vkex::GetImageMemoryRequirements2KHR                         = p_table->GetImageMemoryRequirements2KHR;
  vkex::GetBufferMemoryRequirements2KHR                        = p_table->GetBufferMemoryRequirements2KHR;
  vkex::GetImageSparseMemoryRequirements2KHR                   = p_table->GetImageSparseMemoryRequirements2KHR;
  vkex::CreateSamplerYcbcrConversionKHR                        = p_table->CreateSamplerYcbcrConversionKHR;
  vkex::DestroySamplerYcbcrConversionKHR                       = p_table->DestroySamplerYcbcrConversionKHR;
  vkex::BindBufferMemory2KHR                                   = p_table->BindBufferMemory2KHR;
  vkex::BindImageMemory2KHR                                    = p_table->BindImageMemory2KHR;
  vkex::GetDescriptorSetLayoutSupportKHR                       = p_table->GetDescriptorSetLayoutSupportKHR;
  vkex::CmdDrawIndirectCountKHR                                = p_table->CmdDrawIndirectCountKHR;
  vkex::CmdDrawIndexedIndirectCountKHR                         = p_table->CmdDrawIndexedIndirectCountKHR;
  vkex::DebugReportCallbackEXT                                 = p_table->DebugReportCallbackEXT;
  vkex::DebugMarkerSetObjectTagEXT                             = p_table->DebugMarkerSetObjectTagEXT;
  vkex::DebugMarkerSetObjectNameEXT                            = p_table->DebugMarkerSetObjectNameEXT;
  vkex::CmdDebugMarkerBeginEXT                                 = p_table->CmdDebugMarkerBeginEXT;
  vkex::CmdDebugMarkerEndEXT                                   = p_table->CmdDebugMarkerEndEXT;
  vkex::CmdDebugMarkerInsertEXT                                = p_table->CmdDebugMarkerInsertEXT;
  vkex::CmdDrawIndirectCountAMD                                = p_table->CmdDrawIndirectCountAMD;
  vkex::CmdDrawIndexedIndirectCountAMD                         = p_table->CmdDrawIndexedIndirectCountAMD;
  vkex::GetShaderInfoAMD                                       = p_table->GetShaderInfoAMD;
  vkex::CmdBeginConditionalRenderingEXT                        = p_table->CmdBeginConditionalRenderingEXT;
  vkex::CmdEndConditionalRenderingEXT                          = p_table->CmdEndConditionalRenderingEXT;
  vkex::CmdProcessCommandsNVX                                  = p_table->CmdProcessCommandsNVX;
  vkex::CmdReserveSpaceForCommandsNVX                          = p_table->CmdReserveSpaceForCommandsNVX;
  vkex::CreateIndirectCommandsLayoutNVX                        = p_table->CreateIndirectCommandsLayoutNVX;
  vkex::DestroyIndirectCommandsLayoutNVX                       = p_table->DestroyIndirectCommandsLayoutNVX;
  vkex::CreateObjectTableNVX                                   = p_table->CreateObjectTableNVX;
  vkex::DestroyObjectTableNVX                                  = p_table->DestroyObjectTableNVX;
  vkex::RegisterObjectsNVX                                     = p_table->RegisterObjectsNVX;
  vkex::UnregisterObjectsNVX                                   = p_table->UnregisterObjectsNVX;
  vkex::CmdSetViewportWScalingNV                               = p_table->CmdSetViewportWScalingNV;
  vkex::DisplayPowerControlEXT                                 = p_table->DisplayPowerControlEXT;
  vkex::RegisterDeviceEventEXT                                 = p_table->RegisterDeviceEventEXT;
  vkex::RegisterDisplayEventEXT                                = p_table->RegisterDisplayEventEXT;
  vkex::GetSwapchainCounterEXT                                 = p_table->GetSwapchainCounterEXT;
  vkex::GetRefreshCycleDurationGOOGLE                          = p_table->GetRefreshCycleDurationGOOGLE;
  vkex::GetPastPresentationTimingGOOGLE                        = p_table->GetPastPresentationTimingGOOGLE;
  vkex::CmdSetDiscardRectangleEXT                              = p_table->CmdSetDiscardRectangleEXT;
  vkex::SetHdrMetadataEXT                                      = p_table->SetHdrMetadataEXT;
  vkex::DebugUtilsMessengerCallbackEXT                         = p_table->DebugUtilsMessengerCallbackEXT;
  vkex::SetDebugUtilsObjectNameEXT                             = p_table->SetDebugUtilsObjectNameEXT;
  vkex::SetDebugUtilsObjectTagEXT                              = p_table->SetDebugUtilsObjectTagEXT;
  vkex::QueueBeginDebugUtilsLabelEXT                           = p_table->QueueBeginDebugUtilsLabelEXT;
  vkex::QueueEndDebugUtilsLabelEXT                             = p_table->QueueEndDebugUtilsLabelEXT;
  vkex::QueueInsertDebugUtilsLabelEXT                          = p_table->QueueInsertDebugUtilsLabelEXT;
  vkex::CmdBeginDebugUtilsLabelEXT                             = p_table->CmdBeginDebugUtilsLabelEXT;
  vkex::CmdEndDebugUtilsLabelEXT                               = p_table->CmdEndDebugUtilsLabelEXT;
  vkex::CmdInsertDebugUtilsLabelEXT                            = p_table->CmdInsertDebugUtilsLabelEXT;
  vkex::CmdSetSampleLocationsEXT                               = p_table->CmdSetSampleLocationsEXT;
  vkex::CreateValidationCacheEXT                               = p_table->CreateValidationCacheEXT;
  vkex::DestroyValidationCacheEXT                              = p_table->DestroyValidationCacheEXT;
  vkex::MergeValidationCachesEXT                               = p_table->MergeValidationCachesEXT;
  vkex::GetValidationCacheDataEXT                              = p_table->GetValidationCacheDataEXT;
  vkex::GetMemoryHostPointerPropertiesEXT                      = p_table->GetMemoryHostPointerPropertiesEXT;
  vkex::CmdWriteBufferMarkerAMD                                = p_table->CmdWriteBufferMarkerAMD;
  vkex::CmdSetCheckpointNV                                     = p_table->CmdSetCheckpointNV;
  vkex::GetQueueCheckpointDataNV                               = p_table->GetQueueCheckpointDataNV;

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
  vkex::GetAndroidHardwareBufferPropertiesANDROID              = p_table->GetAndroidHardwareBufferPropertiesANDROID;
  vkex::GetMemoryAndroidHardwareBufferANDROID                  = p_table->GetMemoryAndroidHardwareBufferANDROID;
#endif // defined(VK_USE_PLATFORM_ANDROID_KHR)

#if defined(VK_USE_PLATFORM_IOS_MVK)
#endif // defined(VK_USE_PLATFORM_IOS_MVK)

#if defined(VK_USE_PLATFORM_MACOS_MVK)
#endif // defined(VK_USE_PLATFORM_MACOS_MVK)

#if defined(VK_USE_PLATFORM_MIR_KHR)
#endif // defined(VK_USE_PLATFORM_MIR_KHR)

#if defined(VK_USE_PLATFORM_VI_NN)
#endif // defined(VK_USE_PLATFORM_VI_NN)

#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
#endif // defined(VK_USE_PLATFORM_WAYLAND_KHR)

#if defined(VK_USE_PLATFORM_WIN32_KHR)
  vkex::GetMemoryWin32HandleKHR                                = p_table->GetMemoryWin32HandleKHR;
  vkex::GetMemoryWin32HandlePropertiesKHR                      = p_table->GetMemoryWin32HandlePropertiesKHR;
  vkex::ImportSemaphoreWin32HandleKHR                          = p_table->ImportSemaphoreWin32HandleKHR;
  vkex::GetSemaphoreWin32HandleKHR                             = p_table->GetSemaphoreWin32HandleKHR;
  vkex::ImportFenceWin32HandleKHR                              = p_table->ImportFenceWin32HandleKHR;
  vkex::GetFenceWin32HandleKHR                                 = p_table->GetFenceWin32HandleKHR;
  vkex::GetMemoryWin32HandleNV                                 = p_table->GetMemoryWin32HandleNV;
#endif // defined(VK_USE_PLATFORM_WIN32_KHR)

#if defined(VK_USE_PLATFORM_XCB_KHR)
#endif // defined(VK_USE_PLATFORM_XCB_KHR)

#if defined(VK_USE_PLATFORM_XLIB_KHR)
#endif // defined(VK_USE_PLATFORM_XLIB_KHR)

#if defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
#endif // defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
}

// =================================================================================================
// VKEX Vulkan API symbols
// =================================================================================================
PFN_vkCreateInstance                                   CreateInstance = nullptr;
PFN_vkDestroyInstance                                  DestroyInstance = nullptr;
PFN_vkEnumeratePhysicalDevices                         EnumeratePhysicalDevices = nullptr;
PFN_vkGetPhysicalDeviceFeatures                        GetPhysicalDeviceFeatures = nullptr;
PFN_vkGetPhysicalDeviceFormatProperties                GetPhysicalDeviceFormatProperties = nullptr;
PFN_vkGetPhysicalDeviceImageFormatProperties           GetPhysicalDeviceImageFormatProperties = nullptr;
PFN_vkGetPhysicalDeviceProperties                      GetPhysicalDeviceProperties = nullptr;
PFN_vkGetPhysicalDeviceQueueFamilyProperties           GetPhysicalDeviceQueueFamilyProperties = nullptr;
PFN_vkGetPhysicalDeviceMemoryProperties                GetPhysicalDeviceMemoryProperties = nullptr;
PFN_vkGetInstanceProcAddr                              GetInstanceProcAddr = nullptr;
PFN_vkGetDeviceProcAddr                                GetDeviceProcAddr = nullptr;
PFN_vkCreateDevice                                     CreateDevice = nullptr;
PFN_vkDestroyDevice                                    DestroyDevice = nullptr;
PFN_vkEnumerateInstanceExtensionProperties             EnumerateInstanceExtensionProperties = nullptr;
PFN_vkEnumerateDeviceExtensionProperties               EnumerateDeviceExtensionProperties = nullptr;
PFN_vkEnumerateInstanceLayerProperties                 EnumerateInstanceLayerProperties = nullptr;
PFN_vkEnumerateDeviceLayerProperties                   EnumerateDeviceLayerProperties = nullptr;
PFN_vkGetDeviceQueue                                   GetDeviceQueue = nullptr;
PFN_vkQueueSubmit                                      QueueSubmit = nullptr;
PFN_vkQueueWaitIdle                                    QueueWaitIdle = nullptr;
PFN_vkDeviceWaitIdle                                   DeviceWaitIdle = nullptr;
PFN_vkAllocateMemory                                   AllocateMemory = nullptr;
PFN_vkFreeMemory                                       FreeMemory = nullptr;
PFN_vkMapMemory                                        MapMemory = nullptr;
PFN_vkUnmapMemory                                      UnmapMemory = nullptr;
PFN_vkFlushMappedMemoryRanges                          FlushMappedMemoryRanges = nullptr;
PFN_vkInvalidateMappedMemoryRanges                     InvalidateMappedMemoryRanges = nullptr;
PFN_vkGetDeviceMemoryCommitment                        GetDeviceMemoryCommitment = nullptr;
PFN_vkBindBufferMemory                                 BindBufferMemory = nullptr;
PFN_vkBindImageMemory                                  BindImageMemory = nullptr;
PFN_vkGetBufferMemoryRequirements                      GetBufferMemoryRequirements = nullptr;
PFN_vkGetImageMemoryRequirements                       GetImageMemoryRequirements = nullptr;
PFN_vkGetImageSparseMemoryRequirements                 GetImageSparseMemoryRequirements = nullptr;
PFN_vkGetPhysicalDeviceSparseImageFormatProperties     GetPhysicalDeviceSparseImageFormatProperties = nullptr;
PFN_vkQueueBindSparse                                  QueueBindSparse = nullptr;
PFN_vkCreateFence                                      CreateFence = nullptr;
PFN_vkDestroyFence                                     DestroyFence = nullptr;
PFN_vkResetFences                                      ResetFences = nullptr;
PFN_vkGetFenceStatus                                   GetFenceStatus = nullptr;
PFN_vkWaitForFences                                    WaitForFences = nullptr;
PFN_vkCreateSemaphore                                  CreateSemaphore = nullptr;
PFN_vkDestroySemaphore                                 DestroySemaphore = nullptr;
PFN_vkCreateEvent                                      CreateEvent = nullptr;
PFN_vkDestroyEvent                                     DestroyEvent = nullptr;
PFN_vkGetEventStatus                                   GetEventStatus = nullptr;
PFN_vkSetEvent                                         SetEvent = nullptr;
PFN_vkResetEvent                                       ResetEvent = nullptr;
PFN_vkCreateQueryPool                                  CreateQueryPool = nullptr;
PFN_vkDestroyQueryPool                                 DestroyQueryPool = nullptr;
PFN_vkGetQueryPoolResults                              GetQueryPoolResults = nullptr;
PFN_vkCreateBuffer                                     CreateBuffer = nullptr;
PFN_vkDestroyBuffer                                    DestroyBuffer = nullptr;
PFN_vkCreateBufferView                                 CreateBufferView = nullptr;
PFN_vkDestroyBufferView                                DestroyBufferView = nullptr;
PFN_vkCreateImage                                      CreateImage = nullptr;
PFN_vkDestroyImage                                     DestroyImage = nullptr;
PFN_vkGetImageSubresourceLayout                        GetImageSubresourceLayout = nullptr;
PFN_vkCreateImageView                                  CreateImageView = nullptr;
PFN_vkDestroyImageView                                 DestroyImageView = nullptr;
PFN_vkCreateShaderModule                               CreateShaderModule = nullptr;
PFN_vkDestroyShaderModule                              DestroyShaderModule = nullptr;
PFN_vkCreatePipelineCache                              CreatePipelineCache = nullptr;
PFN_vkDestroyPipelineCache                             DestroyPipelineCache = nullptr;
PFN_vkGetPipelineCacheData                             GetPipelineCacheData = nullptr;
PFN_vkMergePipelineCaches                              MergePipelineCaches = nullptr;
PFN_vkCreateGraphicsPipelines                          CreateGraphicsPipelines = nullptr;
PFN_vkCreateComputePipelines                           CreateComputePipelines = nullptr;
PFN_vkDestroyPipeline                                  DestroyPipeline = nullptr;
PFN_vkCreatePipelineLayout                             CreatePipelineLayout = nullptr;
PFN_vkDestroyPipelineLayout                            DestroyPipelineLayout = nullptr;
PFN_vkCreateSampler                                    CreateSampler = nullptr;
PFN_vkDestroySampler                                   DestroySampler = nullptr;
PFN_vkCreateDescriptorSetLayout                        CreateDescriptorSetLayout = nullptr;
PFN_vkDestroyDescriptorSetLayout                       DestroyDescriptorSetLayout = nullptr;
PFN_vkCreateDescriptorPool                             CreateDescriptorPool = nullptr;
PFN_vkDestroyDescriptorPool                            DestroyDescriptorPool = nullptr;
PFN_vkResetDescriptorPool                              ResetDescriptorPool = nullptr;
PFN_vkAllocateDescriptorSets                           AllocateDescriptorSets = nullptr;
PFN_vkFreeDescriptorSets                               FreeDescriptorSets = nullptr;
PFN_vkUpdateDescriptorSets                             UpdateDescriptorSets = nullptr;
PFN_vkCreateFramebuffer                                CreateFramebuffer = nullptr;
PFN_vkDestroyFramebuffer                               DestroyFramebuffer = nullptr;
PFN_vkCreateRenderPass                                 CreateRenderPass = nullptr;
PFN_vkDestroyRenderPass                                DestroyRenderPass = nullptr;
PFN_vkGetRenderAreaGranularity                         GetRenderAreaGranularity = nullptr;
PFN_vkCreateCommandPool                                CreateCommandPool = nullptr;
PFN_vkDestroyCommandPool                               DestroyCommandPool = nullptr;
PFN_vkResetCommandPool                                 ResetCommandPool = nullptr;
PFN_vkAllocateCommandBuffers                           AllocateCommandBuffers = nullptr;
PFN_vkFreeCommandBuffers                               FreeCommandBuffers = nullptr;
PFN_vkBeginCommandBuffer                               BeginCommandBuffer = nullptr;
PFN_vkEndCommandBuffer                                 EndCommandBuffer = nullptr;
PFN_vkResetCommandBuffer                               ResetCommandBuffer = nullptr;
PFN_vkCmdBindPipeline                                  CmdBindPipeline = nullptr;
PFN_vkCmdSetViewport                                   CmdSetViewport = nullptr;
PFN_vkCmdSetScissor                                    CmdSetScissor = nullptr;
PFN_vkCmdSetLineWidth                                  CmdSetLineWidth = nullptr;
PFN_vkCmdSetDepthBias                                  CmdSetDepthBias = nullptr;
PFN_vkCmdSetBlendConstants                             CmdSetBlendConstants = nullptr;
PFN_vkCmdSetDepthBounds                                CmdSetDepthBounds = nullptr;
PFN_vkCmdSetStencilCompareMask                         CmdSetStencilCompareMask = nullptr;
PFN_vkCmdSetStencilWriteMask                           CmdSetStencilWriteMask = nullptr;
PFN_vkCmdSetStencilReference                           CmdSetStencilReference = nullptr;
PFN_vkCmdBindDescriptorSets                            CmdBindDescriptorSets = nullptr;
PFN_vkCmdBindIndexBuffer                               CmdBindIndexBuffer = nullptr;
PFN_vkCmdBindVertexBuffers                             CmdBindVertexBuffers = nullptr;
PFN_vkCmdDraw                                          CmdDraw = nullptr;
PFN_vkCmdDrawIndexed                                   CmdDrawIndexed = nullptr;
PFN_vkCmdDrawIndirect                                  CmdDrawIndirect = nullptr;
PFN_vkCmdDrawIndexedIndirect                           CmdDrawIndexedIndirect = nullptr;
PFN_vkCmdDispatch                                      CmdDispatch = nullptr;
PFN_vkCmdDispatchIndirect                              CmdDispatchIndirect = nullptr;
PFN_vkCmdCopyBuffer                                    CmdCopyBuffer = nullptr;
PFN_vkCmdCopyImage                                     CmdCopyImage = nullptr;
PFN_vkCmdBlitImage                                     CmdBlitImage = nullptr;
PFN_vkCmdCopyBufferToImage                             CmdCopyBufferToImage = nullptr;
PFN_vkCmdCopyImageToBuffer                             CmdCopyImageToBuffer = nullptr;
PFN_vkCmdUpdateBuffer                                  CmdUpdateBuffer = nullptr;
PFN_vkCmdFillBuffer                                    CmdFillBuffer = nullptr;
PFN_vkCmdClearColorImage                               CmdClearColorImage = nullptr;
PFN_vkCmdClearDepthStencilImage                        CmdClearDepthStencilImage = nullptr;
PFN_vkCmdClearAttachments                              CmdClearAttachments = nullptr;
PFN_vkCmdResolveImage                                  CmdResolveImage = nullptr;
PFN_vkCmdSetEvent                                      CmdSetEvent = nullptr;
PFN_vkCmdResetEvent                                    CmdResetEvent = nullptr;
PFN_vkCmdWaitEvents                                    CmdWaitEvents = nullptr;
PFN_vkCmdPipelineBarrier                               CmdPipelineBarrier = nullptr;
PFN_vkCmdBeginQuery                                    CmdBeginQuery = nullptr;
PFN_vkCmdEndQuery                                      CmdEndQuery = nullptr;
PFN_vkCmdResetQueryPool                                CmdResetQueryPool = nullptr;
PFN_vkCmdWriteTimestamp                                CmdWriteTimestamp = nullptr;
PFN_vkCmdCopyQueryPoolResults                          CmdCopyQueryPoolResults = nullptr;
PFN_vkCmdPushConstants                                 CmdPushConstants = nullptr;
PFN_vkCmdBeginRenderPass                               CmdBeginRenderPass = nullptr;
PFN_vkCmdNextSubpass                                   CmdNextSubpass = nullptr;
PFN_vkCmdEndRenderPass                                 CmdEndRenderPass = nullptr;
PFN_vkCmdExecuteCommands                               CmdExecuteCommands = nullptr;
PFN_vkEnumerateInstanceVersion                         EnumerateInstanceVersion = nullptr;
PFN_vkBindBufferMemory2                                BindBufferMemory2 = nullptr;
PFN_vkBindImageMemory2                                 BindImageMemory2 = nullptr;
PFN_vkGetDeviceGroupPeerMemoryFeatures                 GetDeviceGroupPeerMemoryFeatures = nullptr;
PFN_vkCmdSetDeviceMask                                 CmdSetDeviceMask = nullptr;
PFN_vkCmdDispatchBase                                  CmdDispatchBase = nullptr;
PFN_vkEnumeratePhysicalDeviceGroups                    EnumeratePhysicalDeviceGroups = nullptr;
PFN_vkGetImageMemoryRequirements2                      GetImageMemoryRequirements2 = nullptr;
PFN_vkGetBufferMemoryRequirements2                     GetBufferMemoryRequirements2 = nullptr;
PFN_vkGetImageSparseMemoryRequirements2                GetImageSparseMemoryRequirements2 = nullptr;
PFN_vkGetPhysicalDeviceFeatures2                       GetPhysicalDeviceFeatures2 = nullptr;
PFN_vkGetPhysicalDeviceProperties2                     GetPhysicalDeviceProperties2 = nullptr;
PFN_vkGetPhysicalDeviceFormatProperties2               GetPhysicalDeviceFormatProperties2 = nullptr;
PFN_vkGetPhysicalDeviceImageFormatProperties2          GetPhysicalDeviceImageFormatProperties2 = nullptr;
PFN_vkGetPhysicalDeviceQueueFamilyProperties2          GetPhysicalDeviceQueueFamilyProperties2 = nullptr;
PFN_vkGetPhysicalDeviceMemoryProperties2               GetPhysicalDeviceMemoryProperties2 = nullptr;
PFN_vkGetPhysicalDeviceSparseImageFormatProperties2    GetPhysicalDeviceSparseImageFormatProperties2 = nullptr;
PFN_vkTrimCommandPool                                  TrimCommandPool = nullptr;
PFN_vkGetDeviceQueue2                                  GetDeviceQueue2 = nullptr;
PFN_vkCreateSamplerYcbcrConversion                     CreateSamplerYcbcrConversion = nullptr;
PFN_vkDestroySamplerYcbcrConversion                    DestroySamplerYcbcrConversion = nullptr;
PFN_vkCreateDescriptorUpdateTemplate                   CreateDescriptorUpdateTemplate = nullptr;
PFN_vkDestroyDescriptorUpdateTemplate                  DestroyDescriptorUpdateTemplate = nullptr;
PFN_vkUpdateDescriptorSetWithTemplate                  UpdateDescriptorSetWithTemplate = nullptr;
PFN_vkGetPhysicalDeviceExternalBufferProperties        GetPhysicalDeviceExternalBufferProperties = nullptr;
PFN_vkGetPhysicalDeviceExternalFenceProperties         GetPhysicalDeviceExternalFenceProperties = nullptr;
PFN_vkGetPhysicalDeviceExternalSemaphoreProperties     GetPhysicalDeviceExternalSemaphoreProperties = nullptr;
PFN_vkGetDescriptorSetLayoutSupport                    GetDescriptorSetLayoutSupport = nullptr;
PFN_vkDestroySurfaceKHR                                DestroySurfaceKHR = nullptr;
PFN_vkGetPhysicalDeviceSurfaceSupportKHR               GetPhysicalDeviceSurfaceSupportKHR = nullptr;
PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR          GetPhysicalDeviceSurfaceCapabilitiesKHR = nullptr;
PFN_vkGetPhysicalDeviceSurfaceFormatsKHR               GetPhysicalDeviceSurfaceFormatsKHR = nullptr;
PFN_vkGetPhysicalDeviceSurfacePresentModesKHR          GetPhysicalDeviceSurfacePresentModesKHR = nullptr;
PFN_vkCreateSwapchainKHR                               CreateSwapchainKHR = nullptr;
PFN_vkDestroySwapchainKHR                              DestroySwapchainKHR = nullptr;
PFN_vkGetSwapchainImagesKHR                            GetSwapchainImagesKHR = nullptr;
PFN_vkAcquireNextImageKHR                              AcquireNextImageKHR = nullptr;
PFN_vkQueuePresentKHR                                  QueuePresentKHR = nullptr;
PFN_vkGetDeviceGroupPresentCapabilitiesKHR             GetDeviceGroupPresentCapabilitiesKHR = nullptr;
PFN_vkGetDeviceGroupSurfacePresentModesKHR             GetDeviceGroupSurfacePresentModesKHR = nullptr;
PFN_vkGetPhysicalDevicePresentRectanglesKHR            GetPhysicalDevicePresentRectanglesKHR = nullptr;
PFN_vkAcquireNextImage2KHR                             AcquireNextImage2KHR = nullptr;
PFN_vkGetPhysicalDeviceDisplayPropertiesKHR            GetPhysicalDeviceDisplayPropertiesKHR = nullptr;
PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR       GetPhysicalDeviceDisplayPlanePropertiesKHR = nullptr;
PFN_vkGetDisplayPlaneSupportedDisplaysKHR              GetDisplayPlaneSupportedDisplaysKHR = nullptr;
PFN_vkGetDisplayModePropertiesKHR                      GetDisplayModePropertiesKHR = nullptr;
PFN_vkCreateDisplayModeKHR                             CreateDisplayModeKHR = nullptr;
PFN_vkGetDisplayPlaneCapabilitiesKHR                   GetDisplayPlaneCapabilitiesKHR = nullptr;
PFN_vkCreateDisplayPlaneSurfaceKHR                     CreateDisplayPlaneSurfaceKHR = nullptr;
PFN_vkCreateSharedSwapchainsKHR                        CreateSharedSwapchainsKHR = nullptr;
PFN_vkGetPhysicalDeviceFeatures2KHR                    GetPhysicalDeviceFeatures2KHR = nullptr;
PFN_vkGetPhysicalDeviceProperties2KHR                  GetPhysicalDeviceProperties2KHR = nullptr;
PFN_vkGetPhysicalDeviceFormatProperties2KHR            GetPhysicalDeviceFormatProperties2KHR = nullptr;
PFN_vkGetPhysicalDeviceImageFormatProperties2KHR       GetPhysicalDeviceImageFormatProperties2KHR = nullptr;
PFN_vkGetPhysicalDeviceQueueFamilyProperties2KHR       GetPhysicalDeviceQueueFamilyProperties2KHR = nullptr;
PFN_vkGetPhysicalDeviceMemoryProperties2KHR            GetPhysicalDeviceMemoryProperties2KHR = nullptr;
PFN_vkGetPhysicalDeviceSparseImageFormatProperties2KHR GetPhysicalDeviceSparseImageFormatProperties2KHR = nullptr;
PFN_vkGetDeviceGroupPeerMemoryFeaturesKHR              GetDeviceGroupPeerMemoryFeaturesKHR = nullptr;
PFN_vkCmdSetDeviceMaskKHR                              CmdSetDeviceMaskKHR = nullptr;
PFN_vkCmdDispatchBaseKHR                               CmdDispatchBaseKHR = nullptr;
PFN_vkTrimCommandPoolKHR                               TrimCommandPoolKHR = nullptr;
PFN_vkEnumeratePhysicalDeviceGroupsKHR                 EnumeratePhysicalDeviceGroupsKHR = nullptr;
PFN_vkGetPhysicalDeviceExternalBufferPropertiesKHR     GetPhysicalDeviceExternalBufferPropertiesKHR = nullptr;
PFN_vkGetMemoryFdKHR                                   GetMemoryFdKHR = nullptr;
PFN_vkGetMemoryFdPropertiesKHR                         GetMemoryFdPropertiesKHR = nullptr;
PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR  GetPhysicalDeviceExternalSemaphorePropertiesKHR = nullptr;
PFN_vkImportSemaphoreFdKHR                             ImportSemaphoreFdKHR = nullptr;
PFN_vkGetSemaphoreFdKHR                                GetSemaphoreFdKHR = nullptr;
PFN_vkCmdPushDescriptorSetKHR                          CmdPushDescriptorSetKHR = nullptr;
PFN_vkCmdPushDescriptorSetWithTemplateKHR              CmdPushDescriptorSetWithTemplateKHR = nullptr;
PFN_vkCreateDescriptorUpdateTemplateKHR                CreateDescriptorUpdateTemplateKHR = nullptr;
PFN_vkDestroyDescriptorUpdateTemplateKHR               DestroyDescriptorUpdateTemplateKHR = nullptr;
PFN_vkUpdateDescriptorSetWithTemplateKHR               UpdateDescriptorSetWithTemplateKHR = nullptr;
PFN_vkCreateRenderPass2KHR                             CreateRenderPass2KHR = nullptr;
PFN_vkCmdBeginRenderPass2KHR                           CmdBeginRenderPass2KHR = nullptr;
PFN_vkCmdNextSubpass2KHR                               CmdNextSubpass2KHR = nullptr;
PFN_vkCmdEndRenderPass2KHR                             CmdEndRenderPass2KHR = nullptr;
PFN_vkGetSwapchainStatusKHR                            GetSwapchainStatusKHR = nullptr;
PFN_vkGetPhysicalDeviceExternalFencePropertiesKHR      GetPhysicalDeviceExternalFencePropertiesKHR = nullptr;
PFN_vkImportFenceFdKHR                                 ImportFenceFdKHR = nullptr;
PFN_vkGetFenceFdKHR                                    GetFenceFdKHR = nullptr;
PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR         GetPhysicalDeviceSurfaceCapabilities2KHR = nullptr;
PFN_vkGetPhysicalDeviceSurfaceFormats2KHR              GetPhysicalDeviceSurfaceFormats2KHR = nullptr;
PFN_vkGetPhysicalDeviceDisplayProperties2KHR           GetPhysicalDeviceDisplayProperties2KHR = nullptr;
PFN_vkGetPhysicalDeviceDisplayPlaneProperties2KHR      GetPhysicalDeviceDisplayPlaneProperties2KHR = nullptr;
PFN_vkGetDisplayModeProperties2KHR                     GetDisplayModeProperties2KHR = nullptr;
PFN_vkGetDisplayPlaneCapabilities2KHR                  GetDisplayPlaneCapabilities2KHR = nullptr;
PFN_vkGetImageMemoryRequirements2KHR                   GetImageMemoryRequirements2KHR = nullptr;
PFN_vkGetBufferMemoryRequirements2KHR                  GetBufferMemoryRequirements2KHR = nullptr;
PFN_vkGetImageSparseMemoryRequirements2KHR             GetImageSparseMemoryRequirements2KHR = nullptr;
PFN_vkCreateSamplerYcbcrConversionKHR                  CreateSamplerYcbcrConversionKHR = nullptr;
PFN_vkDestroySamplerYcbcrConversionKHR                 DestroySamplerYcbcrConversionKHR = nullptr;
PFN_vkBindBufferMemory2KHR                             BindBufferMemory2KHR = nullptr;
PFN_vkBindImageMemory2KHR                              BindImageMemory2KHR = nullptr;
PFN_vkGetDescriptorSetLayoutSupportKHR                 GetDescriptorSetLayoutSupportKHR = nullptr;
PFN_vkCmdDrawIndirectCountKHR                          CmdDrawIndirectCountKHR = nullptr;
PFN_vkCmdDrawIndexedIndirectCountKHR                   CmdDrawIndexedIndirectCountKHR = nullptr;
PFN_vkDebugReportCallbackEXT                           DebugReportCallbackEXT = nullptr;
PFN_vkCreateDebugReportCallbackEXT                     CreateDebugReportCallbackEXT = nullptr;
PFN_vkDestroyDebugReportCallbackEXT                    DestroyDebugReportCallbackEXT = nullptr;
PFN_vkDebugReportMessageEXT                            DebugReportMessageEXT = nullptr;
PFN_vkDebugMarkerSetObjectTagEXT                       DebugMarkerSetObjectTagEXT = nullptr;
PFN_vkDebugMarkerSetObjectNameEXT                      DebugMarkerSetObjectNameEXT = nullptr;
PFN_vkCmdDebugMarkerBeginEXT                           CmdDebugMarkerBeginEXT = nullptr;
PFN_vkCmdDebugMarkerEndEXT                             CmdDebugMarkerEndEXT = nullptr;
PFN_vkCmdDebugMarkerInsertEXT                          CmdDebugMarkerInsertEXT = nullptr;
PFN_vkCmdDrawIndirectCountAMD                          CmdDrawIndirectCountAMD = nullptr;
PFN_vkCmdDrawIndexedIndirectCountAMD                   CmdDrawIndexedIndirectCountAMD = nullptr;
PFN_vkGetShaderInfoAMD                                 GetShaderInfoAMD = nullptr;
PFN_vkGetPhysicalDeviceExternalImageFormatPropertiesNV GetPhysicalDeviceExternalImageFormatPropertiesNV = nullptr;
PFN_vkCmdBeginConditionalRenderingEXT                  CmdBeginConditionalRenderingEXT = nullptr;
PFN_vkCmdEndConditionalRenderingEXT                    CmdEndConditionalRenderingEXT = nullptr;
PFN_vkCmdProcessCommandsNVX                            CmdProcessCommandsNVX = nullptr;
PFN_vkCmdReserveSpaceForCommandsNVX                    CmdReserveSpaceForCommandsNVX = nullptr;
PFN_vkCreateIndirectCommandsLayoutNVX                  CreateIndirectCommandsLayoutNVX = nullptr;
PFN_vkDestroyIndirectCommandsLayoutNVX                 DestroyIndirectCommandsLayoutNVX = nullptr;
PFN_vkCreateObjectTableNVX                             CreateObjectTableNVX = nullptr;
PFN_vkDestroyObjectTableNVX                            DestroyObjectTableNVX = nullptr;
PFN_vkRegisterObjectsNVX                               RegisterObjectsNVX = nullptr;
PFN_vkUnregisterObjectsNVX                             UnregisterObjectsNVX = nullptr;
PFN_vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX  GetPhysicalDeviceGeneratedCommandsPropertiesNVX = nullptr;
PFN_vkCmdSetViewportWScalingNV                         CmdSetViewportWScalingNV = nullptr;
PFN_vkReleaseDisplayEXT                                ReleaseDisplayEXT = nullptr;
PFN_vkGetPhysicalDeviceSurfaceCapabilities2EXT         GetPhysicalDeviceSurfaceCapabilities2EXT = nullptr;
PFN_vkDisplayPowerControlEXT                           DisplayPowerControlEXT = nullptr;
PFN_vkRegisterDeviceEventEXT                           RegisterDeviceEventEXT = nullptr;
PFN_vkRegisterDisplayEventEXT                          RegisterDisplayEventEXT = nullptr;
PFN_vkGetSwapchainCounterEXT                           GetSwapchainCounterEXT = nullptr;
PFN_vkGetRefreshCycleDurationGOOGLE                    GetRefreshCycleDurationGOOGLE = nullptr;
PFN_vkGetPastPresentationTimingGOOGLE                  GetPastPresentationTimingGOOGLE = nullptr;
PFN_vkCmdSetDiscardRectangleEXT                        CmdSetDiscardRectangleEXT = nullptr;
PFN_vkSetHdrMetadataEXT                                SetHdrMetadataEXT = nullptr;
PFN_vkDebugUtilsMessengerCallbackEXT                   DebugUtilsMessengerCallbackEXT = nullptr;
PFN_vkSetDebugUtilsObjectNameEXT                       SetDebugUtilsObjectNameEXT = nullptr;
PFN_vkSetDebugUtilsObjectTagEXT                        SetDebugUtilsObjectTagEXT = nullptr;
PFN_vkQueueBeginDebugUtilsLabelEXT                     QueueBeginDebugUtilsLabelEXT = nullptr;
PFN_vkQueueEndDebugUtilsLabelEXT                       QueueEndDebugUtilsLabelEXT = nullptr;
PFN_vkQueueInsertDebugUtilsLabelEXT                    QueueInsertDebugUtilsLabelEXT = nullptr;
PFN_vkCmdBeginDebugUtilsLabelEXT                       CmdBeginDebugUtilsLabelEXT = nullptr;
PFN_vkCmdEndDebugUtilsLabelEXT                         CmdEndDebugUtilsLabelEXT = nullptr;
PFN_vkCmdInsertDebugUtilsLabelEXT                      CmdInsertDebugUtilsLabelEXT = nullptr;
PFN_vkCreateDebugUtilsMessengerEXT                     CreateDebugUtilsMessengerEXT = nullptr;
PFN_vkDestroyDebugUtilsMessengerEXT                    DestroyDebugUtilsMessengerEXT = nullptr;
PFN_vkSubmitDebugUtilsMessageEXT                       SubmitDebugUtilsMessageEXT = nullptr;
PFN_vkCmdSetSampleLocationsEXT                         CmdSetSampleLocationsEXT = nullptr;
PFN_vkGetPhysicalDeviceMultisamplePropertiesEXT        GetPhysicalDeviceMultisamplePropertiesEXT = nullptr;
PFN_vkCreateValidationCacheEXT                         CreateValidationCacheEXT = nullptr;
PFN_vkDestroyValidationCacheEXT                        DestroyValidationCacheEXT = nullptr;
PFN_vkMergeValidationCachesEXT                         MergeValidationCachesEXT = nullptr;
PFN_vkGetValidationCacheDataEXT                        GetValidationCacheDataEXT = nullptr;
PFN_vkGetMemoryHostPointerPropertiesEXT                GetMemoryHostPointerPropertiesEXT = nullptr;
PFN_vkCmdWriteBufferMarkerAMD                          CmdWriteBufferMarkerAMD = nullptr;
PFN_vkCmdSetCheckpointNV                               CmdSetCheckpointNV = nullptr;
PFN_vkGetQueueCheckpointDataNV                         GetQueueCheckpointDataNV = nullptr;

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
PFN_vkCreateAndroidSurfaceKHR                          CreateAndroidSurfaceKHR = nullptr;
PFN_vkGetAndroidHardwareBufferPropertiesANDROID        GetAndroidHardwareBufferPropertiesANDROID = nullptr;
PFN_vkGetMemoryAndroidHardwareBufferANDROID            GetMemoryAndroidHardwareBufferANDROID = nullptr;
#endif // defined(VK_USE_PLATFORM_ANDROID_KHR)

#if defined(VK_USE_PLATFORM_IOS_MVK)
PFN_vkCreateIOSSurfaceMVK                              CreateIOSSurfaceMVK = nullptr;
#endif // defined(VK_USE_PLATFORM_IOS_MVK)

#if defined(VK_USE_PLATFORM_MACOS_MVK)
PFN_vkCreateMacOSSurfaceMVK                            CreateMacOSSurfaceMVK = nullptr;
#endif // defined(VK_USE_PLATFORM_MACOS_MVK)

#if defined(VK_USE_PLATFORM_MIR_KHR)
PFN_vkCreateMirSurfaceKHR                              CreateMirSurfaceKHR = nullptr;
PFN_vkGetPhysicalDeviceMirPresentationSupportKHR       GetPhysicalDeviceMirPresentationSupportKHR = nullptr;
#endif // defined(VK_USE_PLATFORM_MIR_KHR)

#if defined(VK_USE_PLATFORM_VI_NN)
PFN_vkCreateViSurfaceNN                                CreateViSurfaceNN = nullptr;
#endif // defined(VK_USE_PLATFORM_VI_NN)

#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
PFN_vkCreateWaylandSurfaceKHR                          CreateWaylandSurfaceKHR = nullptr;
PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR   GetPhysicalDeviceWaylandPresentationSupportKHR = nullptr;
#endif // defined(VK_USE_PLATFORM_WAYLAND_KHR)

#if defined(VK_USE_PLATFORM_WIN32_KHR)
PFN_vkCreateWin32SurfaceKHR                            CreateWin32SurfaceKHR = nullptr;
PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR     GetPhysicalDeviceWin32PresentationSupportKHR = nullptr;
PFN_vkGetMemoryWin32HandleKHR                          GetMemoryWin32HandleKHR = nullptr;
PFN_vkGetMemoryWin32HandlePropertiesKHR                GetMemoryWin32HandlePropertiesKHR = nullptr;
PFN_vkImportSemaphoreWin32HandleKHR                    ImportSemaphoreWin32HandleKHR = nullptr;
PFN_vkGetSemaphoreWin32HandleKHR                       GetSemaphoreWin32HandleKHR = nullptr;
PFN_vkImportFenceWin32HandleKHR                        ImportFenceWin32HandleKHR = nullptr;
PFN_vkGetFenceWin32HandleKHR                           GetFenceWin32HandleKHR = nullptr;
PFN_vkGetMemoryWin32HandleNV                           GetMemoryWin32HandleNV = nullptr;
#endif // defined(VK_USE_PLATFORM_WIN32_KHR)

#if defined(VK_USE_PLATFORM_XCB_KHR)
PFN_vkCreateXcbSurfaceKHR                              CreateXcbSurfaceKHR = nullptr;
PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR       GetPhysicalDeviceXcbPresentationSupportKHR = nullptr;
#endif // defined(VK_USE_PLATFORM_XCB_KHR)

#if defined(VK_USE_PLATFORM_XLIB_KHR)
PFN_vkCreateXlibSurfaceKHR                             CreateXlibSurfaceKHR = nullptr;
PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR      GetPhysicalDeviceXlibPresentationSupportKHR = nullptr;
#endif // defined(VK_USE_PLATFORM_XLIB_KHR)

#if defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
PFN_vkAcquireXlibDisplayEXT                            AcquireXlibDisplayEXT = nullptr;
PFN_vkGetRandROutputDisplayEXT                         GetRandROutputDisplayEXT = nullptr;
#endif // defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)

#if defined(VK_USE_PLATFORM_GGP_KHR)
PFN_vkCreateYetiSurfaceGOOGLE                          CreateYetiSurfaceGOOGLE = nullptr;
PFN_vkGetPhysicalDeviceYetiPresentationSupportGOOGLE   GetPhysicalDeviceYetiPresentationSupportGOOGLE = nullptr;
#endif // defined(VK_USE_PLATFORM_GGP_KHR)

} // namespace vkex

