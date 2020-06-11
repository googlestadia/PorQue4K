#ifndef __VKEX_LOADER_H__
#define __VKEX_LOADER_H__

#if defined(VULKAN_H_)
# error "vulkan.h must come after VkexLoader.h"
#endif

#if defined(VKEX_WIN32)
  #define VK_USE_PLATFORM_WIN32_KHR
#elif defined(VKEX_LINUX)
 #if defined(VKEX_LINUX_WAYLAND)
  #define VK_USE_PLATFORM_WAYLAND_KHR
 #elif defined(VKEX_LINUX_XCB)
  #define VK_USE_PLATFORM_XCB_KHR
 #elif defined(VKEX_LINUX_XLIB)
  #define VK_USE_PLATFORM_XLIB_KHR
 #elif defined(VKEX_LINUX_GGP)
  #define VK_USE_PLATFORM_GGP_KHR
 #endif
#endif

#if ! defined(VULKAN_H_)
# include <vulkan/vulkan.h>
#ifdef VK_USE_PLATFORM_GGP_KHR
# include <vulkan/vulkan_ggp.h>
#endif
#endif

#include <functional>

namespace vkex {

// =================================================================================================
// Forward declarations
// =================================================================================================
struct InstanceFunctionTable;
struct DeviceFunctionTable;

// =================================================================================================
// Enums
// =================================================================================================
enum LoadMode {
  LOAD_MODE_VK_API_FN,
  LOAD_MODE_PROC_ADDR,
  LOAD_MODE_SO_DIRECT,
};

// =================================================================================================
// Load function types
// =================================================================================================
using GetInstanceProcType = std::function<PFN_vkVoidFunction(VkInstance, const char*)>;
using GetDeviceProcType = std::function<PFN_vkVoidFunction(VkDevice, const char*)>;

// =================================================================================================
// Functions
// =================================================================================================
bool VkexLoaderInitialize(LoadMode mode);
void VkexLoaderLoadInstance(VkInstance instance, vkex::GetInstanceProcType fnProcLoad, vkex::InstanceFunctionTable* p_table = nullptr);
void VkexLoaderLoadDevice(VkDevice device, vkex::GetDeviceProcType fnProcLoad, vkex::DeviceFunctionTable* p_table = nullptr);

// =================================================================================================
// Function tables
// =================================================================================================
struct InstanceFunctionTable {
  PFN_vkCreateInstance                                   CreateInstance;
  PFN_vkDestroyInstance                                  DestroyInstance;
  PFN_vkEnumeratePhysicalDevices                         EnumeratePhysicalDevices;
  PFN_vkGetPhysicalDeviceFeatures                        GetPhysicalDeviceFeatures;
  PFN_vkGetPhysicalDeviceFormatProperties                GetPhysicalDeviceFormatProperties;
  PFN_vkGetPhysicalDeviceImageFormatProperties           GetPhysicalDeviceImageFormatProperties;
  PFN_vkGetPhysicalDeviceProperties                      GetPhysicalDeviceProperties;
  PFN_vkGetPhysicalDeviceQueueFamilyProperties           GetPhysicalDeviceQueueFamilyProperties;
  PFN_vkGetPhysicalDeviceMemoryProperties                GetPhysicalDeviceMemoryProperties;
  PFN_vkGetInstanceProcAddr                              GetInstanceProcAddr;
  PFN_vkCreateDevice                                     CreateDevice;
  PFN_vkEnumerateInstanceExtensionProperties             EnumerateInstanceExtensionProperties;
  PFN_vkEnumerateDeviceExtensionProperties               EnumerateDeviceExtensionProperties;
  PFN_vkEnumerateInstanceLayerProperties                 EnumerateInstanceLayerProperties;
  PFN_vkEnumerateDeviceLayerProperties                   EnumerateDeviceLayerProperties;
  PFN_vkGetPhysicalDeviceSparseImageFormatProperties     GetPhysicalDeviceSparseImageFormatProperties;
  PFN_vkEnumerateInstanceVersion                         EnumerateInstanceVersion;
  PFN_vkEnumeratePhysicalDeviceGroups                    EnumeratePhysicalDeviceGroups;
  PFN_vkGetPhysicalDeviceFeatures2                       GetPhysicalDeviceFeatures2;
  PFN_vkGetPhysicalDeviceProperties2                     GetPhysicalDeviceProperties2;
  PFN_vkGetPhysicalDeviceFormatProperties2               GetPhysicalDeviceFormatProperties2;
  PFN_vkGetPhysicalDeviceImageFormatProperties2          GetPhysicalDeviceImageFormatProperties2;
  PFN_vkGetPhysicalDeviceQueueFamilyProperties2          GetPhysicalDeviceQueueFamilyProperties2;
  PFN_vkGetPhysicalDeviceMemoryProperties2               GetPhysicalDeviceMemoryProperties2;
  PFN_vkGetPhysicalDeviceSparseImageFormatProperties2    GetPhysicalDeviceSparseImageFormatProperties2;
  PFN_vkGetPhysicalDeviceExternalBufferProperties        GetPhysicalDeviceExternalBufferProperties;
  PFN_vkGetPhysicalDeviceExternalFenceProperties         GetPhysicalDeviceExternalFenceProperties;
  PFN_vkGetPhysicalDeviceExternalSemaphoreProperties     GetPhysicalDeviceExternalSemaphoreProperties;
  PFN_vkDestroySurfaceKHR                                DestroySurfaceKHR;
  PFN_vkGetPhysicalDeviceSurfaceSupportKHR               GetPhysicalDeviceSurfaceSupportKHR;
  PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR          GetPhysicalDeviceSurfaceCapabilitiesKHR;
  PFN_vkGetPhysicalDeviceSurfaceFormatsKHR               GetPhysicalDeviceSurfaceFormatsKHR;
  PFN_vkGetPhysicalDeviceSurfacePresentModesKHR          GetPhysicalDeviceSurfacePresentModesKHR;
  PFN_vkGetPhysicalDevicePresentRectanglesKHR            GetPhysicalDevicePresentRectanglesKHR;
  PFN_vkGetPhysicalDeviceDisplayPropertiesKHR            GetPhysicalDeviceDisplayPropertiesKHR;
  PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR       GetPhysicalDeviceDisplayPlanePropertiesKHR;
  PFN_vkGetDisplayPlaneSupportedDisplaysKHR              GetDisplayPlaneSupportedDisplaysKHR;
  PFN_vkGetDisplayModePropertiesKHR                      GetDisplayModePropertiesKHR;
  PFN_vkCreateDisplayModeKHR                             CreateDisplayModeKHR;
  PFN_vkGetDisplayPlaneCapabilitiesKHR                   GetDisplayPlaneCapabilitiesKHR;
  PFN_vkCreateDisplayPlaneSurfaceKHR                     CreateDisplayPlaneSurfaceKHR;
  PFN_vkGetPhysicalDeviceFeatures2KHR                    GetPhysicalDeviceFeatures2KHR;
  PFN_vkGetPhysicalDeviceProperties2KHR                  GetPhysicalDeviceProperties2KHR;
  PFN_vkGetPhysicalDeviceFormatProperties2KHR            GetPhysicalDeviceFormatProperties2KHR;
  PFN_vkGetPhysicalDeviceImageFormatProperties2KHR       GetPhysicalDeviceImageFormatProperties2KHR;
  PFN_vkGetPhysicalDeviceQueueFamilyProperties2KHR       GetPhysicalDeviceQueueFamilyProperties2KHR;
  PFN_vkGetPhysicalDeviceMemoryProperties2KHR            GetPhysicalDeviceMemoryProperties2KHR;
  PFN_vkGetPhysicalDeviceSparseImageFormatProperties2KHR GetPhysicalDeviceSparseImageFormatProperties2KHR;
  PFN_vkEnumeratePhysicalDeviceGroupsKHR                 EnumeratePhysicalDeviceGroupsKHR;
  PFN_vkGetPhysicalDeviceExternalBufferPropertiesKHR     GetPhysicalDeviceExternalBufferPropertiesKHR;
  PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR  GetPhysicalDeviceExternalSemaphorePropertiesKHR;
  PFN_vkGetPhysicalDeviceExternalFencePropertiesKHR      GetPhysicalDeviceExternalFencePropertiesKHR;
  PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR         GetPhysicalDeviceSurfaceCapabilities2KHR;
  PFN_vkGetPhysicalDeviceSurfaceFormats2KHR              GetPhysicalDeviceSurfaceFormats2KHR;
  PFN_vkGetPhysicalDeviceDisplayProperties2KHR           GetPhysicalDeviceDisplayProperties2KHR;
  PFN_vkGetPhysicalDeviceDisplayPlaneProperties2KHR      GetPhysicalDeviceDisplayPlaneProperties2KHR;
  PFN_vkGetDisplayModeProperties2KHR                     GetDisplayModeProperties2KHR;
  PFN_vkGetDisplayPlaneCapabilities2KHR                  GetDisplayPlaneCapabilities2KHR;
  PFN_vkCreateDebugReportCallbackEXT                     CreateDebugReportCallbackEXT;
  PFN_vkDestroyDebugReportCallbackEXT                    DestroyDebugReportCallbackEXT;
  PFN_vkDebugReportMessageEXT                            DebugReportMessageEXT;
  PFN_vkGetPhysicalDeviceExternalImageFormatPropertiesNV GetPhysicalDeviceExternalImageFormatPropertiesNV;
  PFN_vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX  GetPhysicalDeviceGeneratedCommandsPropertiesNVX;
  PFN_vkReleaseDisplayEXT                                ReleaseDisplayEXT;
  PFN_vkGetPhysicalDeviceSurfaceCapabilities2EXT         GetPhysicalDeviceSurfaceCapabilities2EXT;
  PFN_vkCreateDebugUtilsMessengerEXT                     CreateDebugUtilsMessengerEXT;
  PFN_vkDestroyDebugUtilsMessengerEXT                    DestroyDebugUtilsMessengerEXT;
  PFN_vkSubmitDebugUtilsMessageEXT                       SubmitDebugUtilsMessageEXT;
  PFN_vkGetPhysicalDeviceMultisamplePropertiesEXT        GetPhysicalDeviceMultisamplePropertiesEXT;

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
  PFN_vkCreateAndroidSurfaceKHR                          CreateAndroidSurfaceKHR;
#endif // defined(VK_USE_PLATFORM_ANDROID_KHR)

#if defined(VK_USE_PLATFORM_IOS_MVK)
  PFN_vkCreateIOSSurfaceMVK                              CreateIOSSurfaceMVK;
#endif // defined(VK_USE_PLATFORM_IOS_MVK)

#if defined(VK_USE_PLATFORM_MACOS_MVK)
  PFN_vkCreateMacOSSurfaceMVK                            CreateMacOSSurfaceMVK;
#endif // defined(VK_USE_PLATFORM_MACOS_MVK)

#if defined(VK_USE_PLATFORM_MIR_KHR)
  PFN_vkCreateMirSurfaceKHR                              CreateMirSurfaceKHR;
  PFN_vkGetPhysicalDeviceMirPresentationSupportKHR       GetPhysicalDeviceMirPresentationSupportKHR;
#endif // defined(VK_USE_PLATFORM_MIR_KHR)

#if defined(VK_USE_PLATFORM_VI_NN)
  PFN_vkCreateViSurfaceNN                                CreateViSurfaceNN;
#endif // defined(VK_USE_PLATFORM_VI_NN)

#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
  PFN_vkCreateWaylandSurfaceKHR                          CreateWaylandSurfaceKHR;
  PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR   GetPhysicalDeviceWaylandPresentationSupportKHR;
#endif // defined(VK_USE_PLATFORM_WAYLAND_KHR)

#if defined(VK_USE_PLATFORM_WIN32_KHR)
  PFN_vkCreateWin32SurfaceKHR                            CreateWin32SurfaceKHR;
  PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR     GetPhysicalDeviceWin32PresentationSupportKHR;
#endif // defined(VK_USE_PLATFORM_WIN32_KHR)

#if defined(VK_USE_PLATFORM_XCB_KHR)
  PFN_vkCreateXcbSurfaceKHR                              CreateXcbSurfaceKHR;
  PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR       GetPhysicalDeviceXcbPresentationSupportKHR;
#endif // defined(VK_USE_PLATFORM_XCB_KHR)

#if defined(VK_USE_PLATFORM_XLIB_KHR)
  PFN_vkCreateXlibSurfaceKHR                             CreateXlibSurfaceKHR;
  PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR      GetPhysicalDeviceXlibPresentationSupportKHR;
#endif // defined(VK_USE_PLATFORM_XLIB_KHR)

#if defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
  PFN_vkAcquireXlibDisplayEXT                            AcquireXlibDisplayEXT;
  PFN_vkGetRandROutputDisplayEXT                         GetRandROutputDisplayEXT;
#endif // defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)

#if defined(VK_USE_PLATFORM_GGP_KHR)
  PFN_vkCreateStreamDescriptorSurfaceGGP                 CreateStreamDescriptorSurfaceGGP;
#endif // defined(VK_USE_PLATFORM_GGP_KHR)
};

struct DeviceFunctionTable {
  PFN_vkGetDeviceProcAddr                                GetDeviceProcAddr;
  PFN_vkDestroyDevice                                    DestroyDevice;
  PFN_vkGetDeviceQueue                                   GetDeviceQueue;
  PFN_vkQueueSubmit                                      QueueSubmit;
  PFN_vkQueueWaitIdle                                    QueueWaitIdle;
  PFN_vkDeviceWaitIdle                                   DeviceWaitIdle;
  PFN_vkAllocateMemory                                   AllocateMemory;
  PFN_vkFreeMemory                                       FreeMemory;
  PFN_vkMapMemory                                        MapMemory;
  PFN_vkUnmapMemory                                      UnmapMemory;
  PFN_vkFlushMappedMemoryRanges                          FlushMappedMemoryRanges;
  PFN_vkInvalidateMappedMemoryRanges                     InvalidateMappedMemoryRanges;
  PFN_vkGetDeviceMemoryCommitment                        GetDeviceMemoryCommitment;
  PFN_vkBindBufferMemory                                 BindBufferMemory;
  PFN_vkBindImageMemory                                  BindImageMemory;
  PFN_vkGetBufferMemoryRequirements                      GetBufferMemoryRequirements;
  PFN_vkGetImageMemoryRequirements                       GetImageMemoryRequirements;
  PFN_vkGetImageSparseMemoryRequirements                 GetImageSparseMemoryRequirements;
  PFN_vkQueueBindSparse                                  QueueBindSparse;
  PFN_vkCreateFence                                      CreateFence;
  PFN_vkDestroyFence                                     DestroyFence;
  PFN_vkResetFences                                      ResetFences;
  PFN_vkGetFenceStatus                                   GetFenceStatus;
  PFN_vkWaitForFences                                    WaitForFences;
  PFN_vkCreateSemaphore                                  CreateSemaphore;
  PFN_vkDestroySemaphore                                 DestroySemaphore;
  PFN_vkCreateEvent                                      CreateEvent;
  PFN_vkDestroyEvent                                     DestroyEvent;
  PFN_vkGetEventStatus                                   GetEventStatus;
  PFN_vkSetEvent                                         SetEvent;
  PFN_vkResetEvent                                       ResetEvent;
  PFN_vkCreateQueryPool                                  CreateQueryPool;
  PFN_vkDestroyQueryPool                                 DestroyQueryPool;
  PFN_vkGetQueryPoolResults                              GetQueryPoolResults;
  PFN_vkCreateBuffer                                     CreateBuffer;
  PFN_vkDestroyBuffer                                    DestroyBuffer;
  PFN_vkCreateBufferView                                 CreateBufferView;
  PFN_vkDestroyBufferView                                DestroyBufferView;
  PFN_vkCreateImage                                      CreateImage;
  PFN_vkDestroyImage                                     DestroyImage;
  PFN_vkGetImageSubresourceLayout                        GetImageSubresourceLayout;
  PFN_vkCreateImageView                                  CreateImageView;
  PFN_vkDestroyImageView                                 DestroyImageView;
  PFN_vkCreateShaderModule                               CreateShaderModule;
  PFN_vkDestroyShaderModule                              DestroyShaderModule;
  PFN_vkCreatePipelineCache                              CreatePipelineCache;
  PFN_vkDestroyPipelineCache                             DestroyPipelineCache;
  PFN_vkGetPipelineCacheData                             GetPipelineCacheData;
  PFN_vkMergePipelineCaches                              MergePipelineCaches;
  PFN_vkCreateGraphicsPipelines                          CreateGraphicsPipelines;
  PFN_vkCreateComputePipelines                           CreateComputePipelines;
  PFN_vkDestroyPipeline                                  DestroyPipeline;
  PFN_vkCreatePipelineLayout                             CreatePipelineLayout;
  PFN_vkDestroyPipelineLayout                            DestroyPipelineLayout;
  PFN_vkCreateSampler                                    CreateSampler;
  PFN_vkDestroySampler                                   DestroySampler;
  PFN_vkCreateDescriptorSetLayout                        CreateDescriptorSetLayout;
  PFN_vkDestroyDescriptorSetLayout                       DestroyDescriptorSetLayout;
  PFN_vkCreateDescriptorPool                             CreateDescriptorPool;
  PFN_vkDestroyDescriptorPool                            DestroyDescriptorPool;
  PFN_vkResetDescriptorPool                              ResetDescriptorPool;
  PFN_vkAllocateDescriptorSets                           AllocateDescriptorSets;
  PFN_vkFreeDescriptorSets                               FreeDescriptorSets;
  PFN_vkUpdateDescriptorSets                             UpdateDescriptorSets;
  PFN_vkCreateFramebuffer                                CreateFramebuffer;
  PFN_vkDestroyFramebuffer                               DestroyFramebuffer;
  PFN_vkCreateRenderPass                                 CreateRenderPass;
  PFN_vkDestroyRenderPass                                DestroyRenderPass;
  PFN_vkGetRenderAreaGranularity                         GetRenderAreaGranularity;
  PFN_vkCreateCommandPool                                CreateCommandPool;
  PFN_vkDestroyCommandPool                               DestroyCommandPool;
  PFN_vkResetCommandPool                                 ResetCommandPool;
  PFN_vkAllocateCommandBuffers                           AllocateCommandBuffers;
  PFN_vkFreeCommandBuffers                               FreeCommandBuffers;
  PFN_vkBeginCommandBuffer                               BeginCommandBuffer;
  PFN_vkEndCommandBuffer                                 EndCommandBuffer;
  PFN_vkResetCommandBuffer                               ResetCommandBuffer;
  PFN_vkCmdBindPipeline                                  CmdBindPipeline;
  PFN_vkCmdSetViewport                                   CmdSetViewport;
  PFN_vkCmdSetScissor                                    CmdSetScissor;
  PFN_vkCmdSetLineWidth                                  CmdSetLineWidth;
  PFN_vkCmdSetDepthBias                                  CmdSetDepthBias;
  PFN_vkCmdSetBlendConstants                             CmdSetBlendConstants;
  PFN_vkCmdSetDepthBounds                                CmdSetDepthBounds;
  PFN_vkCmdSetStencilCompareMask                         CmdSetStencilCompareMask;
  PFN_vkCmdSetStencilWriteMask                           CmdSetStencilWriteMask;
  PFN_vkCmdSetStencilReference                           CmdSetStencilReference;
  PFN_vkCmdBindDescriptorSets                            CmdBindDescriptorSets;
  PFN_vkCmdBindIndexBuffer                               CmdBindIndexBuffer;
  PFN_vkCmdBindVertexBuffers                             CmdBindVertexBuffers;
  PFN_vkCmdDraw                                          CmdDraw;
  PFN_vkCmdDrawIndexed                                   CmdDrawIndexed;
  PFN_vkCmdDrawIndirect                                  CmdDrawIndirect;
  PFN_vkCmdDrawIndexedIndirect                           CmdDrawIndexedIndirect;
  PFN_vkCmdDispatch                                      CmdDispatch;
  PFN_vkCmdDispatchIndirect                              CmdDispatchIndirect;
  PFN_vkCmdCopyBuffer                                    CmdCopyBuffer;
  PFN_vkCmdCopyImage                                     CmdCopyImage;
  PFN_vkCmdBlitImage                                     CmdBlitImage;
  PFN_vkCmdCopyBufferToImage                             CmdCopyBufferToImage;
  PFN_vkCmdCopyImageToBuffer                             CmdCopyImageToBuffer;
  PFN_vkCmdUpdateBuffer                                  CmdUpdateBuffer;
  PFN_vkCmdFillBuffer                                    CmdFillBuffer;
  PFN_vkCmdClearColorImage                               CmdClearColorImage;
  PFN_vkCmdClearDepthStencilImage                        CmdClearDepthStencilImage;
  PFN_vkCmdClearAttachments                              CmdClearAttachments;
  PFN_vkCmdResolveImage                                  CmdResolveImage;
  PFN_vkCmdSetEvent                                      CmdSetEvent;
  PFN_vkCmdResetEvent                                    CmdResetEvent;
  PFN_vkCmdWaitEvents                                    CmdWaitEvents;
  PFN_vkCmdPipelineBarrier                               CmdPipelineBarrier;
  PFN_vkCmdBeginQuery                                    CmdBeginQuery;
  PFN_vkCmdEndQuery                                      CmdEndQuery;
  PFN_vkCmdResetQueryPool                                CmdResetQueryPool;
  PFN_vkCmdWriteTimestamp                                CmdWriteTimestamp;
  PFN_vkCmdCopyQueryPoolResults                          CmdCopyQueryPoolResults;
  PFN_vkCmdPushConstants                                 CmdPushConstants;
  PFN_vkCmdBeginRenderPass                               CmdBeginRenderPass;
  PFN_vkCmdNextSubpass                                   CmdNextSubpass;
  PFN_vkCmdEndRenderPass                                 CmdEndRenderPass;
  PFN_vkCmdExecuteCommands                               CmdExecuteCommands;
  PFN_vkBindBufferMemory2                                BindBufferMemory2;
  PFN_vkBindImageMemory2                                 BindImageMemory2;
  PFN_vkGetDeviceGroupPeerMemoryFeatures                 GetDeviceGroupPeerMemoryFeatures;
  PFN_vkCmdSetDeviceMask                                 CmdSetDeviceMask;
  PFN_vkCmdDispatchBase                                  CmdDispatchBase;
  PFN_vkGetImageMemoryRequirements2                      GetImageMemoryRequirements2;
  PFN_vkGetBufferMemoryRequirements2                     GetBufferMemoryRequirements2;
  PFN_vkGetImageSparseMemoryRequirements2                GetImageSparseMemoryRequirements2;
  PFN_vkTrimCommandPool                                  TrimCommandPool;
  PFN_vkGetDeviceQueue2                                  GetDeviceQueue2;
  PFN_vkCreateSamplerYcbcrConversion                     CreateSamplerYcbcrConversion;
  PFN_vkDestroySamplerYcbcrConversion                    DestroySamplerYcbcrConversion;
  PFN_vkCreateDescriptorUpdateTemplate                   CreateDescriptorUpdateTemplate;
  PFN_vkDestroyDescriptorUpdateTemplate                  DestroyDescriptorUpdateTemplate;
  PFN_vkUpdateDescriptorSetWithTemplate                  UpdateDescriptorSetWithTemplate;
  PFN_vkGetDescriptorSetLayoutSupport                    GetDescriptorSetLayoutSupport;
  PFN_vkCreateSwapchainKHR                               CreateSwapchainKHR;
  PFN_vkDestroySwapchainKHR                              DestroySwapchainKHR;
  PFN_vkGetSwapchainImagesKHR                            GetSwapchainImagesKHR;
  PFN_vkAcquireNextImageKHR                              AcquireNextImageKHR;
  PFN_vkQueuePresentKHR                                  QueuePresentKHR;
  PFN_vkGetDeviceGroupPresentCapabilitiesKHR             GetDeviceGroupPresentCapabilitiesKHR;
  PFN_vkGetDeviceGroupSurfacePresentModesKHR             GetDeviceGroupSurfacePresentModesKHR;
  PFN_vkAcquireNextImage2KHR                             AcquireNextImage2KHR;
  PFN_vkCreateSharedSwapchainsKHR                        CreateSharedSwapchainsKHR;
  PFN_vkGetDeviceGroupPeerMemoryFeaturesKHR              GetDeviceGroupPeerMemoryFeaturesKHR;
  PFN_vkCmdSetDeviceMaskKHR                              CmdSetDeviceMaskKHR;
  PFN_vkCmdDispatchBaseKHR                               CmdDispatchBaseKHR;
  PFN_vkTrimCommandPoolKHR                               TrimCommandPoolKHR;
  PFN_vkGetMemoryFdKHR                                   GetMemoryFdKHR;
  PFN_vkGetMemoryFdPropertiesKHR                         GetMemoryFdPropertiesKHR;
  PFN_vkImportSemaphoreFdKHR                             ImportSemaphoreFdKHR;
  PFN_vkGetSemaphoreFdKHR                                GetSemaphoreFdKHR;
  PFN_vkCmdPushDescriptorSetKHR                          CmdPushDescriptorSetKHR;
  PFN_vkCmdPushDescriptorSetWithTemplateKHR              CmdPushDescriptorSetWithTemplateKHR;
  PFN_vkCreateDescriptorUpdateTemplateKHR                CreateDescriptorUpdateTemplateKHR;
  PFN_vkDestroyDescriptorUpdateTemplateKHR               DestroyDescriptorUpdateTemplateKHR;
  PFN_vkUpdateDescriptorSetWithTemplateKHR               UpdateDescriptorSetWithTemplateKHR;
  PFN_vkCreateRenderPass2KHR                             CreateRenderPass2KHR;
  PFN_vkCmdBeginRenderPass2KHR                           CmdBeginRenderPass2KHR;
  PFN_vkCmdNextSubpass2KHR                               CmdNextSubpass2KHR;
  PFN_vkCmdEndRenderPass2KHR                             CmdEndRenderPass2KHR;
  PFN_vkGetSwapchainStatusKHR                            GetSwapchainStatusKHR;
  PFN_vkImportFenceFdKHR                                 ImportFenceFdKHR;
  PFN_vkGetFenceFdKHR                                    GetFenceFdKHR;
  PFN_vkGetImageMemoryRequirements2KHR                   GetImageMemoryRequirements2KHR;
  PFN_vkGetBufferMemoryRequirements2KHR                  GetBufferMemoryRequirements2KHR;
  PFN_vkGetImageSparseMemoryRequirements2KHR             GetImageSparseMemoryRequirements2KHR;
  PFN_vkCreateSamplerYcbcrConversionKHR                  CreateSamplerYcbcrConversionKHR;
  PFN_vkDestroySamplerYcbcrConversionKHR                 DestroySamplerYcbcrConversionKHR;
  PFN_vkBindBufferMemory2KHR                             BindBufferMemory2KHR;
  PFN_vkBindImageMemory2KHR                              BindImageMemory2KHR;
  PFN_vkGetDescriptorSetLayoutSupportKHR                 GetDescriptorSetLayoutSupportKHR;
  PFN_vkCmdDrawIndirectCountKHR                          CmdDrawIndirectCountKHR;
  PFN_vkCmdDrawIndexedIndirectCountKHR                   CmdDrawIndexedIndirectCountKHR;
  PFN_vkDebugReportCallbackEXT                           DebugReportCallbackEXT;
  PFN_vkDebugMarkerSetObjectTagEXT                       DebugMarkerSetObjectTagEXT;
  PFN_vkDebugMarkerSetObjectNameEXT                      DebugMarkerSetObjectNameEXT;
  PFN_vkCmdDebugMarkerBeginEXT                           CmdDebugMarkerBeginEXT;
  PFN_vkCmdDebugMarkerEndEXT                             CmdDebugMarkerEndEXT;
  PFN_vkCmdDebugMarkerInsertEXT                          CmdDebugMarkerInsertEXT;
  PFN_vkCmdDrawIndirectCountAMD                          CmdDrawIndirectCountAMD;
  PFN_vkCmdDrawIndexedIndirectCountAMD                   CmdDrawIndexedIndirectCountAMD;
  PFN_vkGetShaderInfoAMD                                 GetShaderInfoAMD;
  PFN_vkCmdBeginConditionalRenderingEXT                  CmdBeginConditionalRenderingEXT;
  PFN_vkCmdEndConditionalRenderingEXT                    CmdEndConditionalRenderingEXT;
  PFN_vkCmdProcessCommandsNVX                            CmdProcessCommandsNVX;
  PFN_vkCmdReserveSpaceForCommandsNVX                    CmdReserveSpaceForCommandsNVX;
  PFN_vkCreateIndirectCommandsLayoutNVX                  CreateIndirectCommandsLayoutNVX;
  PFN_vkDestroyIndirectCommandsLayoutNVX                 DestroyIndirectCommandsLayoutNVX;
  PFN_vkCreateObjectTableNVX                             CreateObjectTableNVX;
  PFN_vkDestroyObjectTableNVX                            DestroyObjectTableNVX;
  PFN_vkRegisterObjectsNVX                               RegisterObjectsNVX;
  PFN_vkUnregisterObjectsNVX                             UnregisterObjectsNVX;
  PFN_vkCmdSetViewportWScalingNV                         CmdSetViewportWScalingNV;
  PFN_vkDisplayPowerControlEXT                           DisplayPowerControlEXT;
  PFN_vkRegisterDeviceEventEXT                           RegisterDeviceEventEXT;
  PFN_vkRegisterDisplayEventEXT                          RegisterDisplayEventEXT;
  PFN_vkGetSwapchainCounterEXT                           GetSwapchainCounterEXT;
  PFN_vkGetRefreshCycleDurationGOOGLE                    GetRefreshCycleDurationGOOGLE;
  PFN_vkGetPastPresentationTimingGOOGLE                  GetPastPresentationTimingGOOGLE;
  PFN_vkCmdSetDiscardRectangleEXT                        CmdSetDiscardRectangleEXT;
  PFN_vkSetHdrMetadataEXT                                SetHdrMetadataEXT;
  PFN_vkDebugUtilsMessengerCallbackEXT                   DebugUtilsMessengerCallbackEXT;
  PFN_vkSetDebugUtilsObjectNameEXT                       SetDebugUtilsObjectNameEXT;
  PFN_vkSetDebugUtilsObjectTagEXT                        SetDebugUtilsObjectTagEXT;
  PFN_vkQueueBeginDebugUtilsLabelEXT                     QueueBeginDebugUtilsLabelEXT;
  PFN_vkQueueEndDebugUtilsLabelEXT                       QueueEndDebugUtilsLabelEXT;
  PFN_vkQueueInsertDebugUtilsLabelEXT                    QueueInsertDebugUtilsLabelEXT;
  PFN_vkCmdBeginDebugUtilsLabelEXT                       CmdBeginDebugUtilsLabelEXT;
  PFN_vkCmdEndDebugUtilsLabelEXT                         CmdEndDebugUtilsLabelEXT;
  PFN_vkCmdInsertDebugUtilsLabelEXT                      CmdInsertDebugUtilsLabelEXT;
  PFN_vkCmdSetSampleLocationsEXT                         CmdSetSampleLocationsEXT;
  PFN_vkCreateValidationCacheEXT                         CreateValidationCacheEXT;
  PFN_vkDestroyValidationCacheEXT                        DestroyValidationCacheEXT;
  PFN_vkMergeValidationCachesEXT                         MergeValidationCachesEXT;
  PFN_vkGetValidationCacheDataEXT                        GetValidationCacheDataEXT;
  PFN_vkGetMemoryHostPointerPropertiesEXT                GetMemoryHostPointerPropertiesEXT;
  PFN_vkCmdWriteBufferMarkerAMD                          CmdWriteBufferMarkerAMD;
  PFN_vkCmdSetCheckpointNV                               CmdSetCheckpointNV;
  PFN_vkGetQueueCheckpointDataNV                         GetQueueCheckpointDataNV;

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
  PFN_vkGetAndroidHardwareBufferPropertiesANDROID        GetAndroidHardwareBufferPropertiesANDROID;
  PFN_vkGetMemoryAndroidHardwareBufferANDROID            GetMemoryAndroidHardwareBufferANDROID;
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
  PFN_vkGetMemoryWin32HandleKHR                          GetMemoryWin32HandleKHR;
  PFN_vkGetMemoryWin32HandlePropertiesKHR                GetMemoryWin32HandlePropertiesKHR;
  PFN_vkImportSemaphoreWin32HandleKHR                    ImportSemaphoreWin32HandleKHR;
  PFN_vkGetSemaphoreWin32HandleKHR                       GetSemaphoreWin32HandleKHR;
  PFN_vkImportFenceWin32HandleKHR                        ImportFenceWin32HandleKHR;
  PFN_vkGetFenceWin32HandleKHR                           GetFenceWin32HandleKHR;
  PFN_vkGetMemoryWin32HandleNV                           GetMemoryWin32HandleNV;
#endif // defined(VK_USE_PLATFORM_WIN32_KHR)

#if defined(VK_USE_PLATFORM_XCB_KHR)
#endif // defined(VK_USE_PLATFORM_XCB_KHR)

#if defined(VK_USE_PLATFORM_XLIB_KHR)
#endif // defined(VK_USE_PLATFORM_XLIB_KHR)

#if defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
#endif // defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)

#if defined(VK_USE_PLATFORM_GGP_KHR)
#endif // defined(VK_USE_PLATFORM_GGP_KHR)
};

// =================================================================================================
// VKEX Vulkan API symbols
// =================================================================================================
extern PFN_vkCreateInstance                                   CreateInstance;
extern PFN_vkDestroyInstance                                  DestroyInstance;
extern PFN_vkEnumeratePhysicalDevices                         EnumeratePhysicalDevices;
extern PFN_vkGetPhysicalDeviceFeatures                        GetPhysicalDeviceFeatures;
extern PFN_vkGetPhysicalDeviceFormatProperties                GetPhysicalDeviceFormatProperties;
extern PFN_vkGetPhysicalDeviceImageFormatProperties           GetPhysicalDeviceImageFormatProperties;
extern PFN_vkGetPhysicalDeviceProperties                      GetPhysicalDeviceProperties;
extern PFN_vkGetPhysicalDeviceQueueFamilyProperties           GetPhysicalDeviceQueueFamilyProperties;
extern PFN_vkGetPhysicalDeviceMemoryProperties                GetPhysicalDeviceMemoryProperties;
extern PFN_vkGetInstanceProcAddr                              GetInstanceProcAddr;
extern PFN_vkGetDeviceProcAddr                                GetDeviceProcAddr;
extern PFN_vkCreateDevice                                     CreateDevice;
extern PFN_vkDestroyDevice                                    DestroyDevice;
extern PFN_vkEnumerateInstanceExtensionProperties             EnumerateInstanceExtensionProperties;
extern PFN_vkEnumerateDeviceExtensionProperties               EnumerateDeviceExtensionProperties;
extern PFN_vkEnumerateInstanceLayerProperties                 EnumerateInstanceLayerProperties;
extern PFN_vkEnumerateDeviceLayerProperties                   EnumerateDeviceLayerProperties;
extern PFN_vkGetDeviceQueue                                   GetDeviceQueue;
extern PFN_vkQueueSubmit                                      QueueSubmit;
extern PFN_vkQueueWaitIdle                                    QueueWaitIdle;
extern PFN_vkDeviceWaitIdle                                   DeviceWaitIdle;
extern PFN_vkAllocateMemory                                   AllocateMemory;
extern PFN_vkFreeMemory                                       FreeMemory;
extern PFN_vkMapMemory                                        MapMemory;
extern PFN_vkUnmapMemory                                      UnmapMemory;
extern PFN_vkFlushMappedMemoryRanges                          FlushMappedMemoryRanges;
extern PFN_vkInvalidateMappedMemoryRanges                     InvalidateMappedMemoryRanges;
extern PFN_vkGetDeviceMemoryCommitment                        GetDeviceMemoryCommitment;
extern PFN_vkBindBufferMemory                                 BindBufferMemory;
extern PFN_vkBindImageMemory                                  BindImageMemory;
extern PFN_vkGetBufferMemoryRequirements                      GetBufferMemoryRequirements;
extern PFN_vkGetImageMemoryRequirements                       GetImageMemoryRequirements;
extern PFN_vkGetImageSparseMemoryRequirements                 GetImageSparseMemoryRequirements;
extern PFN_vkGetPhysicalDeviceSparseImageFormatProperties     GetPhysicalDeviceSparseImageFormatProperties;
extern PFN_vkQueueBindSparse                                  QueueBindSparse;
extern PFN_vkCreateFence                                      CreateFence;
extern PFN_vkDestroyFence                                     DestroyFence;
extern PFN_vkResetFences                                      ResetFences;
extern PFN_vkGetFenceStatus                                   GetFenceStatus;
extern PFN_vkWaitForFences                                    WaitForFences;
extern PFN_vkCreateSemaphore                                  CreateSemaphore;
extern PFN_vkDestroySemaphore                                 DestroySemaphore;
extern PFN_vkCreateEvent                                      CreateEvent;
extern PFN_vkDestroyEvent                                     DestroyEvent;
extern PFN_vkGetEventStatus                                   GetEventStatus;
extern PFN_vkSetEvent                                         SetEvent;
extern PFN_vkResetEvent                                       ResetEvent;
extern PFN_vkCreateQueryPool                                  CreateQueryPool;
extern PFN_vkDestroyQueryPool                                 DestroyQueryPool;
extern PFN_vkGetQueryPoolResults                              GetQueryPoolResults;
extern PFN_vkCreateBuffer                                     CreateBuffer;
extern PFN_vkDestroyBuffer                                    DestroyBuffer;
extern PFN_vkCreateBufferView                                 CreateBufferView;
extern PFN_vkDestroyBufferView                                DestroyBufferView;
extern PFN_vkCreateImage                                      CreateImage;
extern PFN_vkDestroyImage                                     DestroyImage;
extern PFN_vkGetImageSubresourceLayout                        GetImageSubresourceLayout;
extern PFN_vkCreateImageView                                  CreateImageView;
extern PFN_vkDestroyImageView                                 DestroyImageView;
extern PFN_vkCreateShaderModule                               CreateShaderModule;
extern PFN_vkDestroyShaderModule                              DestroyShaderModule;
extern PFN_vkCreatePipelineCache                              CreatePipelineCache;
extern PFN_vkDestroyPipelineCache                             DestroyPipelineCache;
extern PFN_vkGetPipelineCacheData                             GetPipelineCacheData;
extern PFN_vkMergePipelineCaches                              MergePipelineCaches;
extern PFN_vkCreateGraphicsPipelines                          CreateGraphicsPipelines;
extern PFN_vkCreateComputePipelines                           CreateComputePipelines;
extern PFN_vkDestroyPipeline                                  DestroyPipeline;
extern PFN_vkCreatePipelineLayout                             CreatePipelineLayout;
extern PFN_vkDestroyPipelineLayout                            DestroyPipelineLayout;
extern PFN_vkCreateSampler                                    CreateSampler;
extern PFN_vkDestroySampler                                   DestroySampler;
extern PFN_vkCreateDescriptorSetLayout                        CreateDescriptorSetLayout;
extern PFN_vkDestroyDescriptorSetLayout                       DestroyDescriptorSetLayout;
extern PFN_vkCreateDescriptorPool                             CreateDescriptorPool;
extern PFN_vkDestroyDescriptorPool                            DestroyDescriptorPool;
extern PFN_vkResetDescriptorPool                              ResetDescriptorPool;
extern PFN_vkAllocateDescriptorSets                           AllocateDescriptorSets;
extern PFN_vkFreeDescriptorSets                               FreeDescriptorSets;
extern PFN_vkUpdateDescriptorSets                             UpdateDescriptorSets;
extern PFN_vkCreateFramebuffer                                CreateFramebuffer;
extern PFN_vkDestroyFramebuffer                               DestroyFramebuffer;
extern PFN_vkCreateRenderPass                                 CreateRenderPass;
extern PFN_vkDestroyRenderPass                                DestroyRenderPass;
extern PFN_vkGetRenderAreaGranularity                         GetRenderAreaGranularity;
extern PFN_vkCreateCommandPool                                CreateCommandPool;
extern PFN_vkDestroyCommandPool                               DestroyCommandPool;
extern PFN_vkResetCommandPool                                 ResetCommandPool;
extern PFN_vkAllocateCommandBuffers                           AllocateCommandBuffers;
extern PFN_vkFreeCommandBuffers                               FreeCommandBuffers;
extern PFN_vkBeginCommandBuffer                               BeginCommandBuffer;
extern PFN_vkEndCommandBuffer                                 EndCommandBuffer;
extern PFN_vkResetCommandBuffer                               ResetCommandBuffer;
extern PFN_vkCmdBindPipeline                                  CmdBindPipeline;
extern PFN_vkCmdSetViewport                                   CmdSetViewport;
extern PFN_vkCmdSetScissor                                    CmdSetScissor;
extern PFN_vkCmdSetLineWidth                                  CmdSetLineWidth;
extern PFN_vkCmdSetDepthBias                                  CmdSetDepthBias;
extern PFN_vkCmdSetBlendConstants                             CmdSetBlendConstants;
extern PFN_vkCmdSetDepthBounds                                CmdSetDepthBounds;
extern PFN_vkCmdSetStencilCompareMask                         CmdSetStencilCompareMask;
extern PFN_vkCmdSetStencilWriteMask                           CmdSetStencilWriteMask;
extern PFN_vkCmdSetStencilReference                           CmdSetStencilReference;
extern PFN_vkCmdBindDescriptorSets                            CmdBindDescriptorSets;
extern PFN_vkCmdBindIndexBuffer                               CmdBindIndexBuffer;
extern PFN_vkCmdBindVertexBuffers                             CmdBindVertexBuffers;
extern PFN_vkCmdDraw                                          CmdDraw;
extern PFN_vkCmdDrawIndexed                                   CmdDrawIndexed;
extern PFN_vkCmdDrawIndirect                                  CmdDrawIndirect;
extern PFN_vkCmdDrawIndexedIndirect                           CmdDrawIndexedIndirect;
extern PFN_vkCmdDispatch                                      CmdDispatch;
extern PFN_vkCmdDispatchIndirect                              CmdDispatchIndirect;
extern PFN_vkCmdCopyBuffer                                    CmdCopyBuffer;
extern PFN_vkCmdCopyImage                                     CmdCopyImage;
extern PFN_vkCmdBlitImage                                     CmdBlitImage;
extern PFN_vkCmdCopyBufferToImage                             CmdCopyBufferToImage;
extern PFN_vkCmdCopyImageToBuffer                             CmdCopyImageToBuffer;
extern PFN_vkCmdUpdateBuffer                                  CmdUpdateBuffer;
extern PFN_vkCmdFillBuffer                                    CmdFillBuffer;
extern PFN_vkCmdClearColorImage                               CmdClearColorImage;
extern PFN_vkCmdClearDepthStencilImage                        CmdClearDepthStencilImage;
extern PFN_vkCmdClearAttachments                              CmdClearAttachments;
extern PFN_vkCmdResolveImage                                  CmdResolveImage;
extern PFN_vkCmdSetEvent                                      CmdSetEvent;
extern PFN_vkCmdResetEvent                                    CmdResetEvent;
extern PFN_vkCmdWaitEvents                                    CmdWaitEvents;
extern PFN_vkCmdPipelineBarrier                               CmdPipelineBarrier;
extern PFN_vkCmdBeginQuery                                    CmdBeginQuery;
extern PFN_vkCmdEndQuery                                      CmdEndQuery;
extern PFN_vkCmdResetQueryPool                                CmdResetQueryPool;
extern PFN_vkCmdWriteTimestamp                                CmdWriteTimestamp;
extern PFN_vkCmdCopyQueryPoolResults                          CmdCopyQueryPoolResults;
extern PFN_vkCmdPushConstants                                 CmdPushConstants;
extern PFN_vkCmdBeginRenderPass                               CmdBeginRenderPass;
extern PFN_vkCmdNextSubpass                                   CmdNextSubpass;
extern PFN_vkCmdEndRenderPass                                 CmdEndRenderPass;
extern PFN_vkCmdExecuteCommands                               CmdExecuteCommands;
extern PFN_vkEnumerateInstanceVersion                         EnumerateInstanceVersion;
extern PFN_vkBindBufferMemory2                                BindBufferMemory2;
extern PFN_vkBindImageMemory2                                 BindImageMemory2;
extern PFN_vkGetDeviceGroupPeerMemoryFeatures                 GetDeviceGroupPeerMemoryFeatures;
extern PFN_vkCmdSetDeviceMask                                 CmdSetDeviceMask;
extern PFN_vkCmdDispatchBase                                  CmdDispatchBase;
extern PFN_vkEnumeratePhysicalDeviceGroups                    EnumeratePhysicalDeviceGroups;
extern PFN_vkGetImageMemoryRequirements2                      GetImageMemoryRequirements2;
extern PFN_vkGetBufferMemoryRequirements2                     GetBufferMemoryRequirements2;
extern PFN_vkGetImageSparseMemoryRequirements2                GetImageSparseMemoryRequirements2;
extern PFN_vkGetPhysicalDeviceFeatures2                       GetPhysicalDeviceFeatures2;
extern PFN_vkGetPhysicalDeviceProperties2                     GetPhysicalDeviceProperties2;
extern PFN_vkGetPhysicalDeviceFormatProperties2               GetPhysicalDeviceFormatProperties2;
extern PFN_vkGetPhysicalDeviceImageFormatProperties2          GetPhysicalDeviceImageFormatProperties2;
extern PFN_vkGetPhysicalDeviceQueueFamilyProperties2          GetPhysicalDeviceQueueFamilyProperties2;
extern PFN_vkGetPhysicalDeviceMemoryProperties2               GetPhysicalDeviceMemoryProperties2;
extern PFN_vkGetPhysicalDeviceSparseImageFormatProperties2    GetPhysicalDeviceSparseImageFormatProperties2;
extern PFN_vkTrimCommandPool                                  TrimCommandPool;
extern PFN_vkGetDeviceQueue2                                  GetDeviceQueue2;
extern PFN_vkCreateSamplerYcbcrConversion                     CreateSamplerYcbcrConversion;
extern PFN_vkDestroySamplerYcbcrConversion                    DestroySamplerYcbcrConversion;
extern PFN_vkCreateDescriptorUpdateTemplate                   CreateDescriptorUpdateTemplate;
extern PFN_vkDestroyDescriptorUpdateTemplate                  DestroyDescriptorUpdateTemplate;
extern PFN_vkUpdateDescriptorSetWithTemplate                  UpdateDescriptorSetWithTemplate;
extern PFN_vkGetPhysicalDeviceExternalBufferProperties        GetPhysicalDeviceExternalBufferProperties;
extern PFN_vkGetPhysicalDeviceExternalFenceProperties         GetPhysicalDeviceExternalFenceProperties;
extern PFN_vkGetPhysicalDeviceExternalSemaphoreProperties     GetPhysicalDeviceExternalSemaphoreProperties;
extern PFN_vkGetDescriptorSetLayoutSupport                    GetDescriptorSetLayoutSupport;
extern PFN_vkDestroySurfaceKHR                                DestroySurfaceKHR;
extern PFN_vkGetPhysicalDeviceSurfaceSupportKHR               GetPhysicalDeviceSurfaceSupportKHR;
extern PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR          GetPhysicalDeviceSurfaceCapabilitiesKHR;
extern PFN_vkGetPhysicalDeviceSurfaceFormatsKHR               GetPhysicalDeviceSurfaceFormatsKHR;
extern PFN_vkGetPhysicalDeviceSurfacePresentModesKHR          GetPhysicalDeviceSurfacePresentModesKHR;
extern PFN_vkCreateSwapchainKHR                               CreateSwapchainKHR;
extern PFN_vkDestroySwapchainKHR                              DestroySwapchainKHR;
extern PFN_vkGetSwapchainImagesKHR                            GetSwapchainImagesKHR;
extern PFN_vkAcquireNextImageKHR                              AcquireNextImageKHR;
extern PFN_vkQueuePresentKHR                                  QueuePresentKHR;
extern PFN_vkGetDeviceGroupPresentCapabilitiesKHR             GetDeviceGroupPresentCapabilitiesKHR;
extern PFN_vkGetDeviceGroupSurfacePresentModesKHR             GetDeviceGroupSurfacePresentModesKHR;
extern PFN_vkGetPhysicalDevicePresentRectanglesKHR            GetPhysicalDevicePresentRectanglesKHR;
extern PFN_vkAcquireNextImage2KHR                             AcquireNextImage2KHR;
extern PFN_vkGetPhysicalDeviceDisplayPropertiesKHR            GetPhysicalDeviceDisplayPropertiesKHR;
extern PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR       GetPhysicalDeviceDisplayPlanePropertiesKHR;
extern PFN_vkGetDisplayPlaneSupportedDisplaysKHR              GetDisplayPlaneSupportedDisplaysKHR;
extern PFN_vkGetDisplayModePropertiesKHR                      GetDisplayModePropertiesKHR;
extern PFN_vkCreateDisplayModeKHR                             CreateDisplayModeKHR;
extern PFN_vkGetDisplayPlaneCapabilitiesKHR                   GetDisplayPlaneCapabilitiesKHR;
extern PFN_vkCreateDisplayPlaneSurfaceKHR                     CreateDisplayPlaneSurfaceKHR;
extern PFN_vkCreateSharedSwapchainsKHR                        CreateSharedSwapchainsKHR;
extern PFN_vkGetPhysicalDeviceFeatures2KHR                    GetPhysicalDeviceFeatures2KHR;
extern PFN_vkGetPhysicalDeviceProperties2KHR                  GetPhysicalDeviceProperties2KHR;
extern PFN_vkGetPhysicalDeviceFormatProperties2KHR            GetPhysicalDeviceFormatProperties2KHR;
extern PFN_vkGetPhysicalDeviceImageFormatProperties2KHR       GetPhysicalDeviceImageFormatProperties2KHR;
extern PFN_vkGetPhysicalDeviceQueueFamilyProperties2KHR       GetPhysicalDeviceQueueFamilyProperties2KHR;
extern PFN_vkGetPhysicalDeviceMemoryProperties2KHR            GetPhysicalDeviceMemoryProperties2KHR;
extern PFN_vkGetPhysicalDeviceSparseImageFormatProperties2KHR GetPhysicalDeviceSparseImageFormatProperties2KHR;
extern PFN_vkGetDeviceGroupPeerMemoryFeaturesKHR              GetDeviceGroupPeerMemoryFeaturesKHR;
extern PFN_vkCmdSetDeviceMaskKHR                              CmdSetDeviceMaskKHR;
extern PFN_vkCmdDispatchBaseKHR                               CmdDispatchBaseKHR;
extern PFN_vkTrimCommandPoolKHR                               TrimCommandPoolKHR;
extern PFN_vkEnumeratePhysicalDeviceGroupsKHR                 EnumeratePhysicalDeviceGroupsKHR;
extern PFN_vkGetPhysicalDeviceExternalBufferPropertiesKHR     GetPhysicalDeviceExternalBufferPropertiesKHR;
extern PFN_vkGetMemoryFdKHR                                   GetMemoryFdKHR;
extern PFN_vkGetMemoryFdPropertiesKHR                         GetMemoryFdPropertiesKHR;
extern PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR  GetPhysicalDeviceExternalSemaphorePropertiesKHR;
extern PFN_vkImportSemaphoreFdKHR                             ImportSemaphoreFdKHR;
extern PFN_vkGetSemaphoreFdKHR                                GetSemaphoreFdKHR;
extern PFN_vkCmdPushDescriptorSetKHR                          CmdPushDescriptorSetKHR;
extern PFN_vkCmdPushDescriptorSetWithTemplateKHR              CmdPushDescriptorSetWithTemplateKHR;
extern PFN_vkCreateDescriptorUpdateTemplateKHR                CreateDescriptorUpdateTemplateKHR;
extern PFN_vkDestroyDescriptorUpdateTemplateKHR               DestroyDescriptorUpdateTemplateKHR;
extern PFN_vkUpdateDescriptorSetWithTemplateKHR               UpdateDescriptorSetWithTemplateKHR;
extern PFN_vkCreateRenderPass2KHR                             CreateRenderPass2KHR;
extern PFN_vkCmdBeginRenderPass2KHR                           CmdBeginRenderPass2KHR;
extern PFN_vkCmdNextSubpass2KHR                               CmdNextSubpass2KHR;
extern PFN_vkCmdEndRenderPass2KHR                             CmdEndRenderPass2KHR;
extern PFN_vkGetSwapchainStatusKHR                            GetSwapchainStatusKHR;
extern PFN_vkGetPhysicalDeviceExternalFencePropertiesKHR      GetPhysicalDeviceExternalFencePropertiesKHR;
extern PFN_vkImportFenceFdKHR                                 ImportFenceFdKHR;
extern PFN_vkGetFenceFdKHR                                    GetFenceFdKHR;
extern PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR         GetPhysicalDeviceSurfaceCapabilities2KHR;
extern PFN_vkGetPhysicalDeviceSurfaceFormats2KHR              GetPhysicalDeviceSurfaceFormats2KHR;
extern PFN_vkGetPhysicalDeviceDisplayProperties2KHR           GetPhysicalDeviceDisplayProperties2KHR;
extern PFN_vkGetPhysicalDeviceDisplayPlaneProperties2KHR      GetPhysicalDeviceDisplayPlaneProperties2KHR;
extern PFN_vkGetDisplayModeProperties2KHR                     GetDisplayModeProperties2KHR;
extern PFN_vkGetDisplayPlaneCapabilities2KHR                  GetDisplayPlaneCapabilities2KHR;
extern PFN_vkGetImageMemoryRequirements2KHR                   GetImageMemoryRequirements2KHR;
extern PFN_vkGetBufferMemoryRequirements2KHR                  GetBufferMemoryRequirements2KHR;
extern PFN_vkGetImageSparseMemoryRequirements2KHR             GetImageSparseMemoryRequirements2KHR;
extern PFN_vkCreateSamplerYcbcrConversionKHR                  CreateSamplerYcbcrConversionKHR;
extern PFN_vkDestroySamplerYcbcrConversionKHR                 DestroySamplerYcbcrConversionKHR;
extern PFN_vkBindBufferMemory2KHR                             BindBufferMemory2KHR;
extern PFN_vkBindImageMemory2KHR                              BindImageMemory2KHR;
extern PFN_vkGetDescriptorSetLayoutSupportKHR                 GetDescriptorSetLayoutSupportKHR;
extern PFN_vkCmdDrawIndirectCountKHR                          CmdDrawIndirectCountKHR;
extern PFN_vkCmdDrawIndexedIndirectCountKHR                   CmdDrawIndexedIndirectCountKHR;
extern PFN_vkDebugReportCallbackEXT                           DebugReportCallbackEXT;
extern PFN_vkCreateDebugReportCallbackEXT                     CreateDebugReportCallbackEXT;
extern PFN_vkDestroyDebugReportCallbackEXT                    DestroyDebugReportCallbackEXT;
extern PFN_vkDebugReportMessageEXT                            DebugReportMessageEXT;
extern PFN_vkDebugMarkerSetObjectTagEXT                       DebugMarkerSetObjectTagEXT;
extern PFN_vkDebugMarkerSetObjectNameEXT                      DebugMarkerSetObjectNameEXT;
extern PFN_vkCmdDebugMarkerBeginEXT                           CmdDebugMarkerBeginEXT;
extern PFN_vkCmdDebugMarkerEndEXT                             CmdDebugMarkerEndEXT;
extern PFN_vkCmdDebugMarkerInsertEXT                          CmdDebugMarkerInsertEXT;
extern PFN_vkCmdDrawIndirectCountAMD                          CmdDrawIndirectCountAMD;
extern PFN_vkCmdDrawIndexedIndirectCountAMD                   CmdDrawIndexedIndirectCountAMD;
extern PFN_vkGetShaderInfoAMD                                 GetShaderInfoAMD;
extern PFN_vkGetPhysicalDeviceExternalImageFormatPropertiesNV GetPhysicalDeviceExternalImageFormatPropertiesNV;
extern PFN_vkCmdBeginConditionalRenderingEXT                  CmdBeginConditionalRenderingEXT;
extern PFN_vkCmdEndConditionalRenderingEXT                    CmdEndConditionalRenderingEXT;
extern PFN_vkCmdProcessCommandsNVX                            CmdProcessCommandsNVX;
extern PFN_vkCmdReserveSpaceForCommandsNVX                    CmdReserveSpaceForCommandsNVX;
extern PFN_vkCreateIndirectCommandsLayoutNVX                  CreateIndirectCommandsLayoutNVX;
extern PFN_vkDestroyIndirectCommandsLayoutNVX                 DestroyIndirectCommandsLayoutNVX;
extern PFN_vkCreateObjectTableNVX                             CreateObjectTableNVX;
extern PFN_vkDestroyObjectTableNVX                            DestroyObjectTableNVX;
extern PFN_vkRegisterObjectsNVX                               RegisterObjectsNVX;
extern PFN_vkUnregisterObjectsNVX                             UnregisterObjectsNVX;
extern PFN_vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX  GetPhysicalDeviceGeneratedCommandsPropertiesNVX;
extern PFN_vkCmdSetViewportWScalingNV                         CmdSetViewportWScalingNV;
extern PFN_vkReleaseDisplayEXT                                ReleaseDisplayEXT;
extern PFN_vkGetPhysicalDeviceSurfaceCapabilities2EXT         GetPhysicalDeviceSurfaceCapabilities2EXT;
extern PFN_vkDisplayPowerControlEXT                           DisplayPowerControlEXT;
extern PFN_vkRegisterDeviceEventEXT                           RegisterDeviceEventEXT;
extern PFN_vkRegisterDisplayEventEXT                          RegisterDisplayEventEXT;
extern PFN_vkGetSwapchainCounterEXT                           GetSwapchainCounterEXT;
extern PFN_vkGetRefreshCycleDurationGOOGLE                    GetRefreshCycleDurationGOOGLE;
extern PFN_vkGetPastPresentationTimingGOOGLE                  GetPastPresentationTimingGOOGLE;
extern PFN_vkCmdSetDiscardRectangleEXT                        CmdSetDiscardRectangleEXT;
extern PFN_vkSetHdrMetadataEXT                                SetHdrMetadataEXT;
extern PFN_vkDebugUtilsMessengerCallbackEXT                   DebugUtilsMessengerCallbackEXT;
extern PFN_vkSetDebugUtilsObjectNameEXT                       SetDebugUtilsObjectNameEXT;
extern PFN_vkSetDebugUtilsObjectTagEXT                        SetDebugUtilsObjectTagEXT;
extern PFN_vkQueueBeginDebugUtilsLabelEXT                     QueueBeginDebugUtilsLabelEXT;
extern PFN_vkQueueEndDebugUtilsLabelEXT                       QueueEndDebugUtilsLabelEXT;
extern PFN_vkQueueInsertDebugUtilsLabelEXT                    QueueInsertDebugUtilsLabelEXT;
extern PFN_vkCmdBeginDebugUtilsLabelEXT                       CmdBeginDebugUtilsLabelEXT;
extern PFN_vkCmdEndDebugUtilsLabelEXT                         CmdEndDebugUtilsLabelEXT;
extern PFN_vkCmdInsertDebugUtilsLabelEXT                      CmdInsertDebugUtilsLabelEXT;
extern PFN_vkCreateDebugUtilsMessengerEXT                     CreateDebugUtilsMessengerEXT;
extern PFN_vkDestroyDebugUtilsMessengerEXT                    DestroyDebugUtilsMessengerEXT;
extern PFN_vkSubmitDebugUtilsMessageEXT                       SubmitDebugUtilsMessageEXT;
extern PFN_vkCmdSetSampleLocationsEXT                         CmdSetSampleLocationsEXT;
extern PFN_vkGetPhysicalDeviceMultisamplePropertiesEXT        GetPhysicalDeviceMultisamplePropertiesEXT;
extern PFN_vkCreateValidationCacheEXT                         CreateValidationCacheEXT;
extern PFN_vkDestroyValidationCacheEXT                        DestroyValidationCacheEXT;
extern PFN_vkMergeValidationCachesEXT                         MergeValidationCachesEXT;
extern PFN_vkGetValidationCacheDataEXT                        GetValidationCacheDataEXT;
extern PFN_vkGetMemoryHostPointerPropertiesEXT                GetMemoryHostPointerPropertiesEXT;
extern PFN_vkCmdWriteBufferMarkerAMD                          CmdWriteBufferMarkerAMD;
extern PFN_vkCmdSetCheckpointNV                               CmdSetCheckpointNV;
extern PFN_vkGetQueueCheckpointDataNV                         GetQueueCheckpointDataNV;

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
extern PFN_vkCreateAndroidSurfaceKHR                          CreateAndroidSurfaceKHR;
extern PFN_vkGetAndroidHardwareBufferPropertiesANDROID        GetAndroidHardwareBufferPropertiesANDROID;
extern PFN_vkGetMemoryAndroidHardwareBufferANDROID            GetMemoryAndroidHardwareBufferANDROID;
#endif // defined(VK_USE_PLATFORM_ANDROID_KHR)

#if defined(VK_USE_PLATFORM_IOS_MVK)
extern PFN_vkCreateIOSSurfaceMVK                              CreateIOSSurfaceMVK;
#endif // defined(VK_USE_PLATFORM_IOS_MVK)

#if defined(VK_USE_PLATFORM_MACOS_MVK)
extern PFN_vkCreateMacOSSurfaceMVK                            CreateMacOSSurfaceMVK;
#endif // defined(VK_USE_PLATFORM_MACOS_MVK)

#if defined(VK_USE_PLATFORM_MIR_KHR)
extern PFN_vkCreateMirSurfaceKHR                              CreateMirSurfaceKHR;
extern PFN_vkGetPhysicalDeviceMirPresentationSupportKHR       GetPhysicalDeviceMirPresentationSupportKHR;
#endif // defined(VK_USE_PLATFORM_MIR_KHR)

#if defined(VK_USE_PLATFORM_VI_NN)
extern PFN_vkCreateViSurfaceNN                                CreateViSurfaceNN;
#endif // defined(VK_USE_PLATFORM_VI_NN)

#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
extern PFN_vkCreateWaylandSurfaceKHR                          CreateWaylandSurfaceKHR;
extern PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR   GetPhysicalDeviceWaylandPresentationSupportKHR;
#endif // defined(VK_USE_PLATFORM_WAYLAND_KHR)

#if defined(VK_USE_PLATFORM_WIN32_KHR)
extern PFN_vkCreateWin32SurfaceKHR                            CreateWin32SurfaceKHR;
extern PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR     GetPhysicalDeviceWin32PresentationSupportKHR;
extern PFN_vkGetMemoryWin32HandleKHR                          GetMemoryWin32HandleKHR;
extern PFN_vkGetMemoryWin32HandlePropertiesKHR                GetMemoryWin32HandlePropertiesKHR;
extern PFN_vkImportSemaphoreWin32HandleKHR                    ImportSemaphoreWin32HandleKHR;
extern PFN_vkGetSemaphoreWin32HandleKHR                       GetSemaphoreWin32HandleKHR;
extern PFN_vkImportFenceWin32HandleKHR                        ImportFenceWin32HandleKHR;
extern PFN_vkGetFenceWin32HandleKHR                           GetFenceWin32HandleKHR;
extern PFN_vkGetMemoryWin32HandleNV                           GetMemoryWin32HandleNV;
#endif // defined(VK_USE_PLATFORM_WIN32_KHR)

#if defined(VK_USE_PLATFORM_XCB_KHR)
extern PFN_vkCreateXcbSurfaceKHR                              CreateXcbSurfaceKHR;
extern PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR       GetPhysicalDeviceXcbPresentationSupportKHR;
#endif // defined(VK_USE_PLATFORM_XCB_KHR)

#if defined(VK_USE_PLATFORM_XLIB_KHR)
extern PFN_vkCreateXlibSurfaceKHR                             CreateXlibSurfaceKHR;
extern PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR      GetPhysicalDeviceXlibPresentationSupportKHR;
#endif // defined(VK_USE_PLATFORM_XLIB_KHR)

#if defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)
extern PFN_vkAcquireXlibDisplayEXT                            AcquireXlibDisplayEXT;
extern PFN_vkGetRandROutputDisplayEXT                         GetRandROutputDisplayEXT;
#endif // defined(VK_USE_PLATFORM_XLIB_XRANDR_EXT)

#if defined(VK_USE_PLATFORM_GGP_KHR)
extern PFN_vkCreateStreamDescriptorSurfaceGGP                 CreateStreamDescriptorSurfaceGGP;
#endif // defined(VK_USE_PLATFORM_GGP_KHR)

} // namespace vkex

#endif // __VKEX_LOADER_H__
