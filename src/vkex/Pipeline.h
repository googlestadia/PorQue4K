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

#ifndef __VKEX_PIPELINE_H__
#define __VKEX_PIPELINE_H__

#include <vkex/Config.h>
#include <vkex/Buffer.h>
#include <vkex/Traits.h>

namespace vkex {

// =================================================================================================
// PipelineLayout
// =================================================================================================

/** @struct PipelineLayoutCreateInfo 
 *
 */
struct PipelineLayoutCreateInfo {
  std::vector<VkDescriptorSetLayout>  descriptor_set_layouts;
  std::vector<VkPushConstantRange>    push_constant_ranges;
};

/** @class IPipelineLayout
 *
 */ 
class CPipelineLayout : public IDeviceObject {
public:
  CPipelineLayout();
  ~CPipelineLayout();

  /** @fn operator VkPipelineLayout()
   *
   */
  operator VkPipelineLayout() const { 
    return m_vk_object; 
  }

  /** @fn GetVkObject
   *
   */
  VkPipelineLayout GetVkObject() const { 
    return m_vk_object; 
  }

private:
  friend class CDevice;
  friend class IObjectStorageFunctions;

  /** @fn InternalCreate
   *
   */
  vkex::Result InternalCreate(
    const vkex::PipelineLayoutCreateInfo& create_info,
    const VkAllocationCallbacks*          p_allocator
  );

  /** @fn InternalDestroy
   *
   */
  vkex::Result InternalDestroy(const VkAllocationCallbacks* p_allocator);

private:
  vkex::PipelineLayoutCreateInfo  m_create_info = {};
  VkPipelineLayoutCreateInfo      m_vk_create_info = {};
  VkPipelineLayout                m_vk_object = VK_NULL_HANDLE;
};

// =================================================================================================
// PipelineCache
// =================================================================================================

/** @struct PipelineCacheCreateInfo 
 *
 */
struct PipelineCacheCreateInfo {
  size_t      initial_data_size;
  const void* initial_data;
};

/** @class IPipelineCache
 *
 */ 
class CPipelineCache : public IDeviceObject {
public:
  CPipelineCache();
  ~CPipelineCache();

  /** @fn operator VkPipelineCache()
   *
   */
  operator VkPipelineCache() const { 
    return m_vk_object; 
  }

  /** @fn GetVkObject
   *
   */
  VkPipelineCache GetVkObject() const { 
    return m_vk_object; 
  }

private:
  friend class CDevice;
  friend class IObjectStorageFunctions;

  /** @fn InternalCreate
   *
   */
  vkex::Result InternalCreate(
    const vkex::PipelineCacheCreateInfo&  create_info,
    const VkAllocationCallbacks*          p_allocator
  );

  /** @fn InternalDestroy
   *
   */
  vkex::Result InternalDestroy(const VkAllocationCallbacks* p_allocator);

private:
  vkex::PipelineCacheCreateInfo m_create_info = {};
  VkPipelineCacheCreateInfo     m_vk_create_info = {};
  VkPipelineCache               m_vk_object = VK_NULL_HANDLE;
};

// =================================================================================================
// ComputePipeline
// =================================================================================================

/** @struct ComputePipelineCreateInfo 
 *
 */
struct ComputePipelineCreateInfo {
  vkex::ShaderProgram   shader_program;
  vkex::PipelineLayout  pipeline_layout;
  vkex::PipelineCache   pipeline_cache;
};

/** @class IComputePipeline
 *
 */ 
class CComputePipeline : public IDeviceObject {
public:
  CComputePipeline();
  ~CComputePipeline();

  /** @fn operator VkPipeline()
   *
   */
  operator VkPipeline() const { 
    return m_vk_object; 
  }

  /** @fn GetVkObject
   *
   */
  VkPipeline GetVkObject() const { 
    return m_vk_object; 
  }

private:
  friend class CDevice;
  friend class IObjectStorageFunctions;

  /** @fn InternalCreate
   *
   */
  vkex::Result InternalCreate(
    const vkex::ComputePipelineCreateInfo&  create_info,
    const VkAllocationCallbacks*            p_allocator
  );

  /** @fn InternalDestroy
   *
   */
  vkex::Result InternalDestroy(const VkAllocationCallbacks* p_allocator);

private:
  vkex::ComputePipelineCreateInfo m_create_info = {};
  VkComputePipelineCreateInfo     m_vk_create_info = {};
  VkPipeline                      m_vk_object = VK_NULL_HANDLE;
  std::string                     m_cs_entry_point;
};

// =================================================================================================
// GraphicsPipeline
// =================================================================================================

/** @class ColorBlendStates
 *
 */
class ColorBlendAttachmentStates {
public:
  ColorBlendAttachmentStates() {}
  ~ColorBlendAttachmentStates() {}

  static ColorBlendAttachmentStates CreateDefault();

  const std::vector<VkPipelineColorBlendAttachmentState>& GetStates() const {
    return m_states;
  }

  void AddState(const VkPipelineColorBlendAttachmentState& state) {
    m_states.push_back(state);
  }
  
private:
  std::vector<VkPipelineColorBlendAttachmentState> m_states;
};

/** @struct GraphicsPipelineCreateInfo 
 *
 */
struct GraphicsPipelineCreateInfo {
  vkex::ShaderProgram                   shader_program;
  std::vector<VertexBindingDescription> vertex_binding_descriptions;
  VkPrimitiveTopology                   topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  VkTessellationDomainOrigin            tessellation_domain_origin;
  uint32_t                              patch_control_points;
  VkSampleCountFlagBits                 samples = VK_SAMPLE_COUNT_1_BIT;
  VkCullModeFlags                       cull_mode;
  VkFrontFace                           front_face;
  VkBool32                              depth_test_enable;
  VkBool32                              depth_write_enable;
  VkBool32                              depth_bounds_test_enable;
  ColorBlendAttachmentStates            color_blend_attachment_states = ColorBlendAttachmentStates::CreateDefault();
  VkBool32                              color_blend_logic_op_enable;
  VkLogicOp                             color_blend_logic_op;
  float                                 blend_constants[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
  vkex::PipelineLayout                  pipeline_layout;
  RenderPass                            render_pass;
  uint32_t                              subpass;
  vkex::PipelineCache                   pipeline_cache;

  std::vector<VkFormat>                 rtv_formats;
  VkFormat                              dsv_format;

  VkBool32                              sample_shading_enable = false;
  float                                 min_sample_shading_factor = 0.0f;

  VkBool32 sample_locations_enable = VK_FALSE;
  VkSampleLocationsInfoEXT default_sample_locations_info = {};
};

/** @class IGraphicsPipeline
 *
 */ 
class CGraphicsPipeline : public IDeviceObject {
public:
  CGraphicsPipeline();
  ~CGraphicsPipeline();

  /** @fn operator VkPipeline()
   *
   */
  operator VkPipeline() const { 
    return m_vk_object; 
  }

  /** @fn GetVkObject
   *
   */
  VkPipeline GetVkObject() const { 
    return m_vk_object; 
  }

private:
  friend class CDevice;
  friend class IObjectStorageFunctions;

  vkex::Result InitializeShaderStages();
  vkex::Result InitializeVertexInput();
  vkex::Result InitializeInputAssembly();
  vkex::Result InitializeTessellation();
  vkex::Result InitializeViewports();
  vkex::Result InitializeRasterization();
  vkex::Result InitializeMultisample();
  vkex::Result InitializeDepthStencil();
  vkex::Result InitializeBlending();
  vkex::Result InitializeDynamicState();

  /** @fn InternalCreate
   *
   */
  vkex::Result InternalCreate(
    const vkex::GraphicsPipelineCreateInfo& create_info,
    const VkAllocationCallbacks*            p_allocator
  );

  /** @fn InternalDestroy
   *
   */
  vkex::Result InternalDestroy(const VkAllocationCallbacks* p_allocator);

private:
  // clang-format off
  vkex::GraphicsPipelineCreateInfo                      m_create_info = {};
  VkGraphicsPipelineCreateInfo                          m_vk_create_info = {};
  VkPipeline                                            m_vk_object = VK_NULL_HANDLE;
  std::string                                           m_vs_entry_point;
  std::string                                           m_hs_entry_point;
  std::string                                           m_ds_entry_point;
  std::string                                           m_gs_entry_point;
  std::string                                           m_ps_entry_point;
  std::vector<VkPipelineShaderStageCreateInfo>          m_vk_shader_stages;
  std::vector<VkVertexInputAttributeDescription>        m_vk_vertex_input_attributes;
  std::vector<VkVertexInputBindingDescription>          m_vk_vertex_input_bindings;
  VkPipelineVertexInputStateCreateInfo                  m_vk_pipeline_vertex_input =  { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
  VkPrimitiveTopology                                   m_vk_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  VkPipelineInputAssemblyStateCreateInfo                m_vk_pipeline_input_assembly = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
  VkPipelineTessellationDomainOriginStateCreateInfoKHR  m_vk_tessellation_domain_origin = { VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_DOMAIN_ORIGIN_STATE_CREATE_INFO_KHR };
  VkPipelineTessellationStateCreateInfo                 m_vk_pipeline_tessellation = { VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO };
  VkPipelineViewportStateCreateInfo                     m_vk_pipeline_viewport = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
  VkCullModeFlags                                       m_vk_cull_mode = VK_CULL_MODE_NONE;
  VkFrontFace                                           m_vk_front_face = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  VkPipelineRasterizationStateCreateInfo                m_vk_pipeline_rasterization = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
  VkPipelineMultisampleStateCreateInfo                  m_vk_pipeline_multisample = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
  VkPipelineSampleLocationsStateCreateInfoEXT           m_vk_pipeline_sample_locations_ext = { VK_STRUCTURE_TYPE_PIPELINE_SAMPLE_LOCATIONS_STATE_CREATE_INFO_EXT };
  std::vector<VkSampleLocationEXT>                      m_vk_pipeline_sample_locations_list_ext;
  VkPipelineDepthStencilStateCreateInfo                 m_vk_pipeline_depth_stencil = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
  VkPipelineColorBlendStateCreateInfo                   m_vk_pipeline_color_blend = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
  std::vector<VkDynamicState>                           m_vk_dynamic_states;
  VkPipelineDynamicStateCreateInfo                      m_vk_pipeline_dynamic_state = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
  // clang-format on
};

} // namespace vkex

#endif // __VKEX_PIPELINE_H__