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

#if ! defined(__VKEX_CONFIG_H__)
 #error "vkex/Forward.h cannot be used by itself, include it using vkex/Config.h"
#endif

#ifndef __VKEX_FORWARD_H__
#define __VKEX_FORWARD_H__

#include <type_traits>

namespace vkex {

/** Forward declares
 * 
 */
class CBuffer;
class CCommandBuffer;
class CCommandPool;
class CComputePipeline;
class CDepthStencilView;
class CDescriptorPool;
class CDescriptorSetLayout;
class CDescriptorSet;
class CDevice;
class CDeviceMemory;
class CEvent;
class CFence;
class CGpuBufferResource;
class CGpuTextureResource;
class CGraphicsPipeline;
class CImage;
class CImageView;
class CInstance;
class CPhysicalDevice;
class CPipelineCache;
class CPipelineLayout;
class CQueryPool;
class CQueue;
class CRenderPass;
class CRenderTargetView;
class CSampledTexture;
class CSampler;
class CSemaphore;
class CShaderModule;
class CShaderProgram;
class CSurface;
class CSwapchain;
class CTexture;

struct DescriptorSetLayoutCreateInfo;

/** Handles
 * 
 */
using Buffer = typename std::add_pointer<CBuffer>::type;
using CommandBuffer = typename std::add_pointer<CCommandBuffer>::type;
using CommandPool = typename std::add_pointer<CCommandPool>::type;
using ComputePipeline = typename std::add_pointer<CComputePipeline>::type;
using DepthStencilView = typename std::add_pointer<CDepthStencilView>::type;
using DescriptorPool = typename std::add_pointer<CDescriptorPool>::type;
using DescriptorSetLayout = typename std::add_pointer<CDescriptorSetLayout>::type;
using DescriptorSet = typename std::add_pointer<CDescriptorSet>::type;
using Device = typename std::add_pointer<CDevice>::type;
using DeviceMemory = typename std::add_pointer<CDeviceMemory>::type;
using Event = typename std::add_pointer<CEvent>::type;
using Fence = typename std::add_pointer<CFence>::type;
using GpuBufferResource = typename std::add_pointer<CGpuBufferResource>::type;
using GpuTextureResource = typename std::add_pointer<CGpuTextureResource>::type;
using GraphicsPipeline = typename std::add_pointer<CGraphicsPipeline>::type;
using Image = typename std::add_pointer<CImage>::type;
using ImageView = typename std::add_pointer<CImageView>::type;
using Instance = typename std::add_pointer<CInstance>::type;
using PhysicalDevice = typename std::add_pointer<CPhysicalDevice>::type;
using PipelineCache = typename std::add_pointer<CPipelineCache>::type;
using PipelineLayout = typename std::add_pointer<CPipelineLayout>::type;
using QueryPool = typename std::add_pointer<CQueryPool>::type;
using Queue = typename std::add_pointer<CQueue>::type;
using RenderPass = typename std::add_pointer<CRenderPass>::type;
using RenderTargetView = typename std::add_pointer<CRenderTargetView>::type;
using SampledTexture = typename std::add_pointer<CSampledTexture>::type;
using Sampler = typename std::add_pointer<CSampler>::type;
using Semaphore = typename std::add_pointer<CSemaphore>::type;
using ShaderModule = typename std::add_pointer<CShaderModule>::type;
using ShaderProgram = typename std::add_pointer<CShaderProgram>::type;
using Surface = typename std::add_pointer<CSurface>::type;
using Swapchain = typename std::add_pointer<CSwapchain>::type;
using Texture = typename std::add_pointer<CTexture>::type;

} // namespace vkex

#endif // __VKEX_FORWARD_H__