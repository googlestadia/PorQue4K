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

#include "DebugUi.h"

void AddRowBool(const std::string& label, VkBool32 value)
{
  ImGui::Text("%s", label.c_str());
  ImGui::NextColumn();
  ImGui::Text(value == VK_TRUE ? "1" : "0");
  ImGui::NextColumn();
}

void AddRowDeviceType(const std::string& label, const VkPhysicalDeviceType& value)
{
  ImGui::Text("%s", label.c_str());
  ImGui::NextColumn(); 
  ImGui::Text("%s", vkex::ToStringShort(value).c_str());
  ImGui::NextColumn();
}

void AddRowQueueFlags(const std::string& label, const VkQueueFlags& value)
{
  std::string str_value;
  uint32_t count = 0;
  for (uint32_t i = 0; i < 8; ++i) {
    uint32_t mask = 1 << i;
    std::string s;
    if (value & mask) {
      switch (mask) {
        case VK_QUEUE_GRAPHICS_BIT       : s += "GRAPHICS";  break;
        case VK_QUEUE_COMPUTE_BIT        : s += "COMPUTE";   break;
        case VK_QUEUE_TRANSFER_BIT       : s += "TRANSFER";  break;
        case VK_QUEUE_SPARSE_BINDING_BIT : s += "SPARSE";    break;
      }
    }
    if (!s.empty()) {
      if (count > 0) {
        str_value += ", ";
      }
      str_value += s;
      ++count;
    }
  }
  ImGui::Text("%s", label.c_str());
  ImGui::NextColumn(); 
  ImGui::Text("%s", str_value.c_str());
  ImGui::NextColumn();
}

void AddRowMemoryPropertyFlags(const std::string& label, const VkMemoryPropertyFlags& value)
{
  std::string str_value;
  uint32_t count = 0;
  for (uint32_t i = 0; i < 8; ++i) {
    uint32_t mask = 1 << i;
    std::string s;
    if (value & mask) {
      switch (mask) {
        case VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT     : s = "DEVICE_LOCAL";     break;
        case VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT     : s = "HOST_VISIBLE";     break;
        case VK_MEMORY_PROPERTY_HOST_COHERENT_BIT    : s = "HOST_COHERENT";    break;
        case VK_MEMORY_PROPERTY_HOST_CACHED_BIT      : s = "HOST_CACHED";      break;
        case VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT : s = "LAZILY_ALLOCATED"; break;
        case VK_MEMORY_PROPERTY_PROTECTED_BIT        : s = "PROTECTED";        break;
      }
    }
    if (!s.empty()) {
      if (count > 0) {
        str_value += ", ";
      }
      str_value += s;
      ++count;
    }
  }
  ImGui::Text("%s", label.c_str());
  ImGui::NextColumn(); 
  ImGui::Text("%s", str_value.c_str());
  ImGui::NextColumn();
}

void AddRowMemoryHeapFlags(const std::string& label, const VkMemoryHeapFlags& value)
{
  std::string str_value;
  uint32_t count = 0;
  for (uint32_t i = 0; i < 8; ++i) {
    uint32_t mask = 1 << i;
    std::string s;
    if (value & mask) {
      switch (mask) {
        case VK_MEMORY_HEAP_DEVICE_LOCAL_BIT    : s = "DEVICE_LOCAL";   break;
        case VK_MEMORY_HEAP_MULTI_INSTANCE_BIT  : s = "MULTI_INSTANCE"; break;
      }
    }
    if (!s.empty()) {
      if (count > 0) {
        str_value += ", ";
      }
      str_value += s;
      ++count;
    }
  }
  ImGui::Text("%s", label.c_str());
  ImGui::NextColumn(); 
  ImGui::Text("%s", str_value.c_str());
  ImGui::NextColumn();
}

void AddRowVersion(const std::string& label, uint32_t version)
{
  uint32_t major = VK_VERSION_MAJOR(version);
  uint32_t minor = VK_VERSION_MINOR(version);
  uint32_t patch = VK_VERSION_PATCH(version);
  ImGui::Text("%s", label.c_str());
  ImGui::NextColumn();
  ImGui::Text("%u.%u.%u", major, minor, patch);
  ImGui::NextColumn();
}

void AddRow(const std::string& label, const char* value)
{
  ImGui::Text("%s", label.c_str());
  ImGui::NextColumn();
  ImGui::Text("%s", value);
  ImGui::NextColumn();
}

void AddRow(const std::string& label, int32_t value)
{
  ImGui::Text("%s", label.c_str());
  ImGui::NextColumn();
  ImGui::Text("%d", value);
  ImGui::NextColumn();
}

void AddRow(const std::string& label, uint32_t value)
{
  ImGui::Text("%s", label.c_str());
  ImGui::NextColumn();
  ImGui::Text("%u", value);
  ImGui::NextColumn();
}

void AddRow(const std::string& label, uint32_t value0, uint32_t value1)
{
  ImGui::Text("%s", label.c_str());
  ImGui::NextColumn();
  ImGui::Text("[%u, %u]", value0, value1);
  ImGui::NextColumn();
}

void AddRow(const std::string& label, uint64_t value)
{
  ImGui::Text("%s", label.c_str());
  ImGui::NextColumn();
  ImGui::Text("%llu", static_cast<unsigned long long>(value));
  ImGui::NextColumn();
}

void AddRow(const std::string& label, float value, const std::string& suffix)
{
  ImGui::Text("%s", label.c_str());
  ImGui::NextColumn();
  ImGui::Text("%f%s", value, suffix.c_str());
  ImGui::NextColumn();
}

void AddRow(const std::string& label, double value, const std::string& suffix)
{
  ImGui::Text("%s", label.c_str());
  ImGui::NextColumn();
  ImGui::Text("%f%s", value, suffix.c_str());
  ImGui::NextColumn();
}

void AddRow(const std::string& label, float value0, float value1)
{
  ImGui::Text("%s", label.c_str());
  ImGui::NextColumn();
  ImGui::Text("[%f, %f]", value0, value1);
  ImGui::NextColumn();
}

void AddRow(const std::string& label, const VkExtent3D& value)
{
  ImGui::Text("%s", label.c_str());
  ImGui::NextColumn();
  ImGui::Text("[%f, %f, %f]", static_cast<double>(value.width), static_cast<double>(value.height), static_cast<double>(value.depth));
  ImGui::NextColumn();
}

void DrawDebugUiPhyiscalDevice(vkex::PhysicalDevice physical_device)
{
  if (ImGui::TreeNode("Physical Device Properties")) {
    auto& properties = physical_device->GetPhysicalDeviceProperties().properties;
    ImGui::Columns(2);

    AddRowVersion("apiVersion",     properties.apiVersion);
    AddRow("driverVersion",         properties.driverVersion);
    AddRow("vendorID",              properties.vendorID);
    AddRow("deviceID",              properties.deviceID);
    AddRowDeviceType("deviceType",  properties.deviceType);
    AddRow("deviceName",            reinterpret_cast<const char*>(properties.deviceName));
    std::string s;
    for (uint32_t i = 0; i < VK_UUID_SIZE; ++i) {
      uint8_t u8 = properties.pipelineCacheUUID[i];
      s += vkex::ToHexString(u8, true, true, false);
    }
    AddRow("pipelineCacheUUID", s.c_str());
 
    ImGui::Columns(1);
    ImGui::TreePop();
  }

  if (physical_device->IsAMD()) {
    if (ImGui::TreeNode("AMD Shader Core Properties")) {
      auto& properties = physical_device->GetShaderCorePropertiesAMD();
      ImGui::Columns(2);

      AddRow("shaderEngineCount",           properties.shaderEngineCount );
      AddRow("shaderArraysPerEngineCount",  properties.shaderArraysPerEngineCount);
      AddRow("computeUnitsPerShaderArray",  properties.computeUnitsPerShaderArray);
      AddRow("simdPerComputeUnit",          properties.simdPerComputeUnit );
      AddRow("wavefrontsPerSimd",           properties.wavefrontsPerSimd );
      AddRow("wavefrontSize",               properties.wavefrontSize);
      AddRow("sgprsPerSimd",                properties.sgprsPerSimd);
      AddRow("minSgprAllocation",           properties.minSgprAllocation);
      AddRow("maxSgprAllocation",           properties.maxSgprAllocation);
      AddRow("sgprAllocationGranularity",   properties.sgprAllocationGranularity);
      AddRow("vgprsPerSimd",                properties.vgprsPerSimd);
      AddRow("minVgprAllocation",           properties.minVgprAllocation);
      AddRow("maxVgprAllocation",           properties.maxVgprAllocation);
      AddRow("vgprAllocationGranularity",   properties.vgprAllocationGranularity);
 
      ImGui::Columns(1);
      ImGui::TreePop();
    }  
  }

  if (ImGui::TreeNode("Physical Device Features")) {
    auto& features = physical_device->GetPhysicalDeviceFeatures().features;
    ImGui::Columns(2);
    AddRowBool("robustBufferAccess",                       features.robustBufferAccess);
    AddRowBool("fullDrawIndexUint32",                      features.fullDrawIndexUint32);
    AddRowBool("imageCubeArray",                           features.imageCubeArray);
    AddRowBool("independentBlend",                         features.independentBlend);
    AddRowBool("geometryShader",                           features.geometryShader);
    AddRowBool("tessellationShader",                       features.tessellationShader);
    AddRowBool("sampleRateShading",                        features.sampleRateShading);
    AddRowBool("dualSrcBlend",                             features.dualSrcBlend);
    AddRowBool("logicOp",                                  features.logicOp);
    AddRowBool("multiDrawIndirect",                        features.multiDrawIndirect);
    AddRowBool("drawIndirectFirstInstance",                features.drawIndirectFirstInstance);
    AddRowBool("depthClamp",                               features.depthClamp);
    AddRowBool("depthBiasClamp",                           features.depthBiasClamp);
    AddRowBool("fillModeNonSolid",                         features.fillModeNonSolid);
    AddRowBool("depthBounds",                              features.depthBounds);
    AddRowBool("wideLines",                                features.wideLines);
    AddRowBool("largePoints",                              features.largePoints);
    AddRowBool("alphaToOne",                               features.alphaToOne);
    AddRowBool("multiViewport",                            features.multiViewport);
    AddRowBool("samplerAnisotropy",                        features.samplerAnisotropy);
    AddRowBool("textureCompressionETC2",                   features.textureCompressionETC2);
    AddRowBool("textureCompressionASTC_LDR",               features.textureCompressionASTC_LDR);
    AddRowBool("textureCompressionBC",                     features.textureCompressionBC);
    AddRowBool("occlusionQueryPrecise",                    features.occlusionQueryPrecise);
    AddRowBool("pipelineStatisticsQuery",                  features.pipelineStatisticsQuery);
    AddRowBool("vertexPipelineStoresAndAtomics",           features.vertexPipelineStoresAndAtomics);
    AddRowBool("fragmentStoresAndAtomics",                 features.fragmentStoresAndAtomics);
    AddRowBool("shaderTessellationAndGeometryPointSize",   features.shaderTessellationAndGeometryPointSize);
    AddRowBool("shaderImageGatherExtended",                features.shaderImageGatherExtended);
    AddRowBool("shaderStorageImageExtendedFormats",        features.shaderStorageImageExtendedFormats);
    AddRowBool("shaderStorageImageMultisample",            features.shaderStorageImageMultisample);
    AddRowBool("shaderStorageImageReadWithoutFormat",      features.shaderStorageImageReadWithoutFormat);
    AddRowBool("shaderStorageImageWriteWithoutFormat",     features.shaderStorageImageWriteWithoutFormat);
    AddRowBool("shaderUniformBufferArrayDynamicIndexing",  features.shaderUniformBufferArrayDynamicIndexing);
    AddRowBool("shaderSampledImageArrayDynamicIndexing",   features.shaderSampledImageArrayDynamicIndexing);
    AddRowBool("shaderStorageBufferArrayDynamicIndexing",  features.shaderStorageBufferArrayDynamicIndexing);
    AddRowBool("shaderStorageImageArrayDynamicIndexing",   features.shaderStorageImageArrayDynamicIndexing);
    AddRowBool("shaderClipDistance",                       features.shaderClipDistance);
    AddRowBool("shaderCullDistance",                       features.shaderCullDistance);
    AddRowBool("shaderFloat64",                            features.shaderFloat64);
    AddRowBool("shaderInt64",                              features.shaderInt64);
    AddRowBool("shaderInt16",                              features.shaderInt16);
    AddRowBool("shaderResourceResidency",                  features.shaderResourceResidency);
    AddRowBool("shaderResourceMinLod",                     features.shaderResourceMinLod);
    AddRowBool("sparseBinding",                            features.sparseBinding);
    AddRowBool("sparseResidencyBuffer",                    features.sparseResidencyBuffer);
    AddRowBool("sparseResidencyImage2D",                   features.sparseResidencyImage2D);
    AddRowBool("sparseResidencyImage3D",                   features.sparseResidencyImage3D);
    AddRowBool("sparseResidency2Samples",                  features.sparseResidency2Samples);
    AddRowBool("sparseResidency4Samples",                  features.sparseResidency4Samples);
    AddRowBool("sparseResidency8Samples",                  features.sparseResidency8Samples);
    AddRowBool("sparseResidency16Samples",                 features.sparseResidency16Samples);
    AddRowBool("sparseResidencyAliased",                   features.sparseResidencyAliased);
    AddRowBool("variableMultisampleRate",                  features.variableMultisampleRate);
    AddRowBool("inheritedQueries",                         features.inheritedQueries);
    ImGui::Columns(1);
    ImGui::TreePop();
  }

  if (ImGui::TreeNode("Physical Device Limits")) {
    auto& limits = physical_device->GetPhysicalDeviceLimits();
    ImGui::Columns(2);
    AddRow("maxImageDimension1D",                             limits.maxImageDimension1D);
    AddRow("maxImageDimension2D",                             limits.maxImageDimension2D);
    AddRow("maxImageDimension3D",                             limits.maxImageDimension3D);
    AddRow("maxImageDimensionCube",                           limits.maxImageDimensionCube);
    AddRow("maxImageArrayLayers",                             limits.maxImageArrayLayers);
    AddRow("maxTexelBufferElements",                          limits.maxTexelBufferElements);
    AddRow("maxUniformBufferRange",                           limits.maxUniformBufferRange);
    AddRow("maxStorageBufferRange",                           limits.maxStorageBufferRange);
    AddRow("maxPushConstantsSize",                            limits.maxPushConstantsSize);
    AddRow("maxMemoryAllocationCount",                        limits.maxMemoryAllocationCount);
    AddRow("maxSamplerAllocationCount",                       limits.maxSamplerAllocationCount);
    AddRow("bufferImageGranularity",                          limits.bufferImageGranularity);
    AddRow("sparseAddressSpaceSize",                          limits.sparseAddressSpaceSize);
    AddRow("maxBoundDescriptorSets",                          limits.maxBoundDescriptorSets);
    AddRow("maxPerStageDescriptorSamplers",                   limits.maxPerStageDescriptorSamplers);
    AddRow("maxPerStageDescriptorUniformBuffers",             limits.maxPerStageDescriptorUniformBuffers);
    AddRow("maxPerStageDescriptorStorageBuffers",             limits.maxPerStageDescriptorStorageBuffers);
    AddRow("maxPerStageDescriptorSampledImages",              limits.maxPerStageDescriptorSampledImages);
    AddRow("maxPerStageDescriptorStorageImages",              limits.maxPerStageDescriptorStorageImages);
    AddRow("maxPerStageDescriptorInputAttachments",           limits.maxPerStageDescriptorInputAttachments);
    AddRow("maxPerStageResources",                            limits.maxPerStageResources);
    AddRow("maxDescriptorSetSamplers",                        limits.maxDescriptorSetSamplers);
    AddRow("maxDescriptorSetUniformBuffers",                  limits.maxDescriptorSetUniformBuffers);
    AddRow("maxDescriptorSetUniformBuffersDynamic",           limits.maxDescriptorSetUniformBuffersDynamic);
    AddRow("maxDescriptorSetStorageBuffers",                  limits.maxDescriptorSetStorageBuffers);
    AddRow("maxDescriptorSetStorageBuffersDynamic",           limits.maxDescriptorSetStorageBuffersDynamic);
    AddRow("maxDescriptorSetSampledImages",                   limits.maxDescriptorSetSampledImages);
    AddRow("maxDescriptorSetStorageImages",                   limits.maxDescriptorSetStorageImages);
    AddRow("maxDescriptorSetInputAttachments",                limits.maxDescriptorSetInputAttachments);
    AddRow("maxVertexInputAttributes",                        limits.maxVertexInputAttributes);
    AddRow("maxVertexInputBindings",                          limits.maxVertexInputBindings);
    AddRow("maxVertexInputAttributeOffset",                   limits.maxVertexInputAttributeOffset);
    AddRow("maxVertexInputBindingStride",                     limits.maxVertexInputBindingStride);
    AddRow("maxVertexOutputComponents",                       limits.maxVertexOutputComponents);
    AddRow("maxTessellationGenerationLevel",                  limits.maxTessellationGenerationLevel);
    AddRow("maxTessellationPatchSize",                        limits.maxTessellationPatchSize);
    AddRow("maxTessellationControlPerVertexInputComponents",  limits.maxTessellationControlPerVertexInputComponents);
    AddRow("maxTessellationControlPerVertexOutputComponents", limits.maxTessellationControlPerVertexOutputComponents);
    AddRow("maxTessellationControlPerPatchOutputComponents",  limits.maxTessellationControlPerPatchOutputComponents);
    AddRow("maxTessellationControlTotalOutputComponents",     limits.maxTessellationControlTotalOutputComponents);
    AddRow("maxTessellationEvaluationInputComponents",        limits.maxTessellationEvaluationInputComponents);
    AddRow("maxTessellationEvaluationOutputComponents",       limits.maxTessellationEvaluationOutputComponents);
    AddRow("maxGeometryShaderInvocations",                    limits.maxGeometryShaderInvocations);
    AddRow("maxGeometryInputComponents",                      limits.maxGeometryInputComponents);
    AddRow("maxGeometryOutputComponents",                     limits.maxGeometryOutputComponents);
    AddRow("maxGeometryOutputVertices",                       limits.maxGeometryOutputVertices);
    AddRow("maxGeometryTotalOutputComponents",                limits.maxGeometryTotalOutputComponents);
    AddRow("maxFragmentInputComponents",                      limits.maxFragmentInputComponents);
    AddRow("maxFragmentOutputAttachments",                    limits.maxFragmentOutputAttachments);
    AddRow("maxFragmentDualSrcAttachments",                   limits.maxFragmentDualSrcAttachments);
    AddRow("maxFragmentCombinedOutputResources",              limits.maxFragmentCombinedOutputResources);
    AddRow("maxComputeSharedMemorySize",                      limits.maxComputeSharedMemorySize);
    AddRow("maxComputeWorkGroupCount[0]",                     limits.maxComputeWorkGroupCount[0]);
    AddRow("maxComputeWorkGroupCount[1]",                     limits.maxComputeWorkGroupCount[1]);
    AddRow("maxComputeWorkGroupCount[2]",                     limits.maxComputeWorkGroupCount[2]);
    AddRow("maxComputeWorkGroupInvocations",                  limits.maxComputeWorkGroupInvocations);
    AddRow("maxComputeWorkGroupSize[0]",                      limits.maxComputeWorkGroupSize[0]);
    AddRow("maxComputeWorkGroupSize[1]",                      limits.maxComputeWorkGroupSize[1]);
    AddRow("maxComputeWorkGroupSize[2]",                      limits.maxComputeWorkGroupSize[2]);
    AddRow("subPixelPrecisionBits",                           limits.subPixelPrecisionBits);
    AddRow("subTexelPrecisionBits",                           limits.subTexelPrecisionBits);
    AddRow("mipmapPrecisionBits",                             limits.mipmapPrecisionBits);
    AddRow("maxDrawIndexedIndexValue",                        limits.maxDrawIndexedIndexValue);
    AddRow("maxDrawIndirectCount",                            limits.maxDrawIndirectCount);
    AddRow("maxSamplerLodBias",                               limits.maxSamplerLodBias);
    AddRow("maxSamplerAnisotropy",                            limits.maxSamplerAnisotropy);
    AddRow("maxViewports",                                    limits.maxViewports);
    AddRow("maxViewportDimensions",                           limits.maxViewportDimensions[0], limits.maxViewportDimensions[1]);
    AddRow("viewportBoundsRange",                             limits.viewportBoundsRange[0], limits.viewportBoundsRange[1]);
    AddRow("viewportSubPixelBits",                            limits.viewportSubPixelBits);
    AddRow("minMemoryMapAlignment",                           limits.minMemoryMapAlignment);
    AddRow("minTexelBufferOffsetAlignment",                   limits.minTexelBufferOffsetAlignment);
    AddRow("minUniformBufferOffsetAlignment",                 limits.minUniformBufferOffsetAlignment);
    AddRow("minStorageBufferOffsetAlignment",                 limits.minStorageBufferOffsetAlignment);
    AddRow("minTexelOffset",                                  limits.minTexelOffset);
    AddRow("maxTexelOffset",                                  limits.maxTexelOffset);
    AddRow("minTexelGatherOffset",                            limits.minTexelGatherOffset);
    AddRow("maxTexelGatherOffset",                            limits.maxTexelGatherOffset);
    AddRow("minInterpolationOffset",                          limits.minInterpolationOffset);
    AddRow("maxInterpolationOffset",                          limits.maxInterpolationOffset);
    AddRow("subPixelInterpolationOffsetBits",                 limits.subPixelInterpolationOffsetBits);
    AddRow("maxFramebufferWidth",                             limits.maxFramebufferWidth);
    AddRow("maxFramebufferHeight",                            limits.maxFramebufferHeight);
    AddRow("maxFramebufferLayers",                            limits.maxFramebufferLayers);
    AddRow("framebufferColorSampleCounts",                    limits.framebufferColorSampleCounts);
    AddRow("framebufferDepthSampleCounts",                    limits.framebufferDepthSampleCounts);
    AddRow("framebufferStencilSampleCounts",                  limits.framebufferStencilSampleCounts);
    AddRow("framebufferNoAttachmentsSampleCounts",            limits.framebufferNoAttachmentsSampleCounts);
    AddRow("maxColorAttachments",                             limits.maxColorAttachments);
    AddRow("sampledImageColorSampleCounts",                   limits.sampledImageColorSampleCounts);
    AddRow("sampledImageIntegerSampleCounts",                 limits.sampledImageIntegerSampleCounts);
    AddRow("sampledImageDepthSampleCounts",                   limits.sampledImageDepthSampleCounts);
    AddRow("sampledImageStencilSampleCounts",                 limits.sampledImageStencilSampleCounts);
    AddRow("storageImageSampleCounts",                        limits.storageImageSampleCounts);
    AddRow("maxSampleMaskWords",                              limits.maxSampleMaskWords);
    AddRow("timestampComputeAndGraphics",                     limits.timestampComputeAndGraphics);
    AddRow("timestampPeriod",                                 limits.timestampPeriod);
    AddRow("maxClipDistances",                                limits.maxClipDistances);
    AddRow("maxCullDistances",                                limits.maxCullDistances);
    AddRow("maxCombinedClipAndCullDistances",                 limits.maxCombinedClipAndCullDistances);
    AddRow("discreteQueuePriorities",                         limits.discreteQueuePriorities);
    AddRow("pointSizeRange",                                  limits.pointSizeRange[0], limits.pointSizeRange[1]);
    AddRow("lineWidthRange",                                  limits.lineWidthRange[0], limits.lineWidthRange[1]);
    AddRow("pointSizeGranularity",                            limits.pointSizeGranularity);
    AddRow("lineWidthGranularity",                            limits.lineWidthGranularity);
    AddRow("strictLines",                                     limits.strictLines);
    AddRow("standardSampleLocations",                         limits.standardSampleLocations);
    AddRow("optimalBufferCopyOffsetAlignment",                limits.optimalBufferCopyOffsetAlignment);
    AddRow("optimalBufferCopyRowPitchAlignment",              limits.optimalBufferCopyRowPitchAlignment);
    AddRow("nonCoherentAtomSize",                             limits.nonCoherentAtomSize);
    ImGui::Columns(1);
    ImGui::TreePop();
  }

  if (ImGui::TreeNode("Physical Device Memory Properties")) {
    auto& properties = physical_device->GetPhysicalDeviceMemoryProperties();
    if (ImGui::TreeNode("Memory Types")) {
      for (uint32_t i = 0; i < properties.memoryTypeCount; ++i) {
        std::string s = "Memory Type " + std::to_string(i);
        if (ImGui::TreeNode(s.c_str())) {
          ImGui::Columns(2);
          const VkMemoryType& type = properties.memoryTypes[i];
          AddRowMemoryPropertyFlags("propertyFlags",  type.propertyFlags);
          AddRow("heapIndex",                         type.heapIndex);
          ImGui::Columns(1);
          ImGui::TreePop();
        }
      }
      ImGui::TreePop();
    }
    if (ImGui::TreeNode("Memory Heaps")) {
      for (uint32_t i = 0; i < properties.memoryHeapCount; ++i) {
        std::string s = "Memory Heap " + std::to_string(i);
        if (ImGui::TreeNode(s.c_str())) {
          ImGui::Columns(2);
          const VkMemoryHeap& heap = properties.memoryHeaps[i];
          AddRow("size",                         heap.size);
          AddRowMemoryHeapFlags("propertyFlags", heap.flags);
          ImGui::Columns(1);
          ImGui::TreePop();
        }
      }
      ImGui::TreePop();
    }
    ImGui::TreePop();
  }

  if (ImGui::TreeNode("Physical Device Queue Families")) {
    auto& queue_families = physical_device->GetQueueFamilyProperties();
    for (uint32_t i = 0; i < queue_families.size(); ++i) {
      std::string s = "Queue Family " + std::to_string(i);
      if (ImGui::TreeNode(s.c_str())) {
        auto& properties = queue_families[i].queueFamilyProperties;
        ImGui::Columns(2);
        AddRowQueueFlags("queueFlags",        properties.queueFlags);
        AddRow("queueCount",                  properties.queueCount);
        AddRow("timestampValidBits",          properties.timestampValidBits);
        AddRow("minImageTransferGranularity", properties.minImageTransferGranularity);
        ImGui::Columns(1);
        ImGui::TreePop();
      }
    }
    ImGui::TreePop();
  }
}