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

#include "vkex/Pipeline.h"
#include "vkex/Device.h"
#include "vkex/RenderPass.h"
#include "vkex/Shader.h"
#include "vkex/ToString.h"

namespace vkex {

// =================================================================================================
// PipelineLayout
// =================================================================================================
CPipelineLayout::CPipelineLayout()
{
}

CPipelineLayout::~CPipelineLayout()
{
}

vkex::Result CPipelineLayout::InternalCreate(
  const vkex::PipelineLayoutCreateInfo& create_info,
  const VkAllocationCallbacks*          p_allocator
)
{
  // Copy create info
  m_create_info = create_info;

  // Vulkan create info
  m_vk_create_info = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
  m_vk_create_info.flags                  = 0;
  m_vk_create_info.setLayoutCount         = CountU32(m_create_info.descriptor_set_layouts);
  m_vk_create_info.pSetLayouts            = DataPtr(m_create_info.descriptor_set_layouts);
  m_vk_create_info.pushConstantRangeCount = CountU32(m_create_info.push_constant_ranges);
  m_vk_create_info.pPushConstantRanges    = DataPtr(m_create_info.push_constant_ranges);
  VkResult vk_result = InvalidValue<VkResult>::Value;
  VKEX_VULKAN_RESULT_CALL(
    vk_result,
    vkex::CreatePipelineLayout(
      *m_device,
      &m_vk_create_info,
      p_allocator,
      &m_vk_object)
  );
  if (vk_result != VK_SUCCESS) {
    return vkex::Result(vk_result);
  }

  return vkex::Result::Success;
}

vkex::Result CPipelineLayout::InternalDestroy(const VkAllocationCallbacks* p_allocator)
{
  if (m_vk_object) {
    vkex::DestroyPipelineLayout(
      *m_device,
      m_vk_object,
      p_allocator);
    
    m_vk_object = VK_NULL_HANDLE;
  }

  return vkex::Result::Success;
}

// =================================================================================================
// PipelineCache
// =================================================================================================
CPipelineCache::CPipelineCache()
{
}

CPipelineCache::~CPipelineCache()
{
}

vkex::Result CPipelineCache::InternalCreate(
  const vkex::PipelineCacheCreateInfo&  create_info,
  const VkAllocationCallbacks*          p_allocator
)
{
  // Copy create info
  m_create_info = create_info;

  // Vulkan create info
  m_vk_create_info = { VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO };
  m_vk_create_info.flags            = 0;
  m_vk_create_info.initialDataSize  = m_create_info.initial_data_size;
  m_vk_create_info.pInitialData     = m_create_info.initial_data;
  VkResult vk_result = InvalidValue<VkResult>::Value;
  // Call create
  VKEX_VULKAN_RESULT_CALL(
    vk_result,
    vkex::CreatePipelineCache(
      *m_device,
      &m_vk_create_info,
      p_allocator,
      &m_vk_object)
  );
  if (vk_result != VK_SUCCESS) {
    return vkex::Result(vk_result);
  }

  return vkex::Result::Success;
}

vkex::Result CPipelineCache::InternalDestroy(const VkAllocationCallbacks* p_allocator)
{
  if (m_vk_object) {
    vkex::DestroyPipelineCache(
      *m_device,
      m_vk_object,
      p_allocator);
    
    m_vk_object = VK_NULL_HANDLE;
  }

  return vkex::Result::Success;
}

// =================================================================================================
// ComputePipeline
// =================================================================================================
CComputePipeline::CComputePipeline()
{
}

CComputePipeline::~CComputePipeline()
{
}

vkex::Result CComputePipeline::InternalCreate(
  const vkex::ComputePipelineCreateInfo&  create_info,
  const VkAllocationCallbacks*            p_allocator
)
{
  // Copy create info
  m_create_info = create_info;

  // Shader stage
  VkPipelineShaderStageCreateInfo vk_shader_stage = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
  {
    vkex::ShaderModule module = m_create_info.shader_program->GetCS();
    if (module == nullptr) {
      return vkex::Result::ErrorPipelineMissingRequiredShaderStage;
    }

    m_cs_entry_point = module->GetEntryPoint();

    vk_shader_stage.flags               = 0;
    vk_shader_stage.pSpecializationInfo = nullptr;
    vk_shader_stage.pName               = m_cs_entry_point.c_str();
    vk_shader_stage.stage               = VK_SHADER_STAGE_COMPUTE_BIT; 
    vk_shader_stage.module              = *(module);
  }

  // Pipeline cache
  VkPipelineCache vk_pipeline_cache = VK_NULL_HANDLE;
  if (m_create_info.pipeline_cache != nullptr) {
    vk_pipeline_cache = *(m_create_info.pipeline_cache);
  }

  // Vulkan create info
  m_vk_create_info = { VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO };
  m_vk_create_info.flags              = 0;
  m_vk_create_info.stage              = vk_shader_stage;
  m_vk_create_info.layout             = *(m_create_info.pipeline_layout);
  m_vk_create_info.basePipelineHandle = VK_NULL_HANDLE;
  m_vk_create_info.basePipelineIndex  = 0;
// Call create
  VkResult vk_result = InvalidValue<VkResult>::Value;
  VKEX_VULKAN_RESULT_CALL(
    vk_result,
    vkex::CreateComputePipelines(
      *m_device,
      vk_pipeline_cache,
      1,
      &m_vk_create_info, 
      p_allocator,
      &m_vk_object)
  );

  if (vk_result != VK_SUCCESS) {
    return vkex::Result(vk_result);
  }

  return vkex::Result::Success;
}

vkex::Result CComputePipeline::InternalDestroy(const VkAllocationCallbacks* p_allocator)
{
  return vkex::Result::ErrorFailed;
}

// =================================================================================================
// GraphicsPipeline
// =================================================================================================
vkex::ColorBlendAttachmentStates ColorBlendAttachmentStates::CreateDefault()
{
  ColorBlendAttachmentStates cbas;
  VkPipelineColorBlendAttachmentState vk_state = {};
  vk_state.colorWriteMask 
    = VK_COLOR_COMPONENT_R_BIT |
      VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT |
      VK_COLOR_COMPONENT_A_BIT;
  cbas.AddState(vk_state);
  return cbas;
}

CGraphicsPipeline::CGraphicsPipeline()
{
}

CGraphicsPipeline::~CGraphicsPipeline()
{
}

vkex::Result CGraphicsPipeline::InitializeShaderStages()
{
  // VS
  {
    vkex::ShaderModule module = m_create_info.shader_program->GetVS();
    if (module != nullptr) {
      m_vs_entry_point = module->GetEntryPoint();

      VkPipelineShaderStageCreateInfo vk_shader_stage = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
      vk_shader_stage.flags               = 0;
      vk_shader_stage.pSpecializationInfo = nullptr;
      vk_shader_stage.pName               = m_vs_entry_point.c_str();
      vk_shader_stage.stage               = VK_SHADER_STAGE_VERTEX_BIT; 
      vk_shader_stage.module              = *(module);
      m_vk_shader_stages.push_back(vk_shader_stage);
    }
  }

  // HS
  {
    vkex::ShaderModule module = m_create_info.shader_program->GetHS();
    if (module != nullptr) {
      m_hs_entry_point = module->GetEntryPoint();

      VkPipelineShaderStageCreateInfo vk_shader_stage = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
      vk_shader_stage.flags               = 0;
      vk_shader_stage.pSpecializationInfo = nullptr;
      vk_shader_stage.pName               = m_hs_entry_point.c_str();
      vk_shader_stage.stage               = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
      vk_shader_stage.module              = *(module);
      m_vk_shader_stages.push_back(vk_shader_stage);
    }
  }

  // DS
  {
    vkex::ShaderModule module = m_create_info.shader_program->GetDS();
    if (module != nullptr) {
      m_ds_entry_point = module->GetEntryPoint();

      VkPipelineShaderStageCreateInfo vk_shader_stage = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
      vk_shader_stage.flags               = 0;
      vk_shader_stage.pSpecializationInfo = nullptr;
      vk_shader_stage.pName               = m_ds_entry_point.c_str();
      vk_shader_stage.stage               = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
      vk_shader_stage.module              = *(module);
      m_vk_shader_stages.push_back(vk_shader_stage);
    }
  }

  // GS
  {
    vkex::ShaderModule module = m_create_info.shader_program->GetGS();
    if (module != nullptr) {
      m_gs_entry_point = module->GetEntryPoint();

      VkPipelineShaderStageCreateInfo vk_shader_stage = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
      vk_shader_stage.flags               = 0;
      vk_shader_stage.pSpecializationInfo = nullptr;
      vk_shader_stage.pName               = m_gs_entry_point.c_str();
      vk_shader_stage.stage               = VK_SHADER_STAGE_GEOMETRY_BIT;
      vk_shader_stage.module              = *(module);
      m_vk_shader_stages.push_back(vk_shader_stage);
    }
  }

  // PS
  {
    vkex::ShaderModule module = m_create_info.shader_program->GetPS();
    if (module != nullptr) {
      m_ps_entry_point = module->GetEntryPoint();

      VkPipelineShaderStageCreateInfo vk_shader_stage = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
      vk_shader_stage.flags               = 0;
      vk_shader_stage.pSpecializationInfo = nullptr;
      vk_shader_stage.pName               = m_ps_entry_point.c_str();
      vk_shader_stage.stage               = VK_SHADER_STAGE_FRAGMENT_BIT;
      vk_shader_stage.module              = *(module);
      m_vk_shader_stages.push_back(vk_shader_stage);
    }
  }

  return vkex::Result::Success;
}

vkex::Result CGraphicsPipeline::InitializeVertexInput()
{
  uint32_t binding_count = CountU32(m_create_info.vertex_binding_descriptions);
  for (uint32_t binding_index = 0; binding_index < binding_count; ++binding_index) {
    const auto& binding = m_create_info.vertex_binding_descriptions[binding_index];
    // Building bindings
    {
      VkVertexInputBindingDescription description = binding.GetDescription();
      m_vk_vertex_input_bindings.push_back(description);
    }
    // Build attributes
    {
      const auto& attributes = binding.GetAttributes();
      uint32_t attribute_count = CountU32(attributes);
      for (uint32_t attribute_index = 0; attribute_index < attribute_count; ++attribute_index) {
        const auto& attribute = attributes[attribute_index];
        VkVertexInputAttributeDescription description = attribute.GetDescription();
        m_vk_vertex_input_attributes.push_back(description);
      }
    }
  }

  m_vk_pipeline_vertex_input.flags                            = 0;
  m_vk_pipeline_vertex_input.vertexBindingDescriptionCount    = CountU32(m_vk_vertex_input_bindings);
  m_vk_pipeline_vertex_input.pVertexBindingDescriptions       = DataPtr(m_vk_vertex_input_bindings);
  m_vk_pipeline_vertex_input.vertexAttributeDescriptionCount  = CountU32(m_vk_vertex_input_attributes);
  m_vk_pipeline_vertex_input.pVertexAttributeDescriptions     = DataPtr(m_vk_vertex_input_attributes);

  return vkex::Result::Success;
}

vkex::Result CGraphicsPipeline::InitializeInputAssembly()
{
  m_vk_pipeline_input_assembly.flags                  = 0;
  m_vk_pipeline_input_assembly.topology               = m_create_info.topology;
  m_vk_pipeline_input_assembly.primitiveRestartEnable = VK_FALSE;

  return vkex::Result::Success;
}

vkex::Result CGraphicsPipeline::InitializeTessellation()
{
  m_vk_tessellation_domain_origin.domainOrigin  = m_create_info.tessellation_domain_origin;

  m_vk_pipeline_tessellation.pNext  = (m_vk_pipeline_tessellation.patchControlPoints >= 1) ? &m_vk_tessellation_domain_origin : nullptr;
  m_vk_pipeline_tessellation.flags  = 0;
  m_vk_pipeline_tessellation.patchControlPoints = m_create_info.patch_control_points;

  return vkex::Result::Success;
}

vkex::Result CGraphicsPipeline::InitializeViewports()
{
  m_vk_pipeline_viewport.flags          = 0;
  m_vk_pipeline_viewport.viewportCount  = 1;
  m_vk_pipeline_viewport.pViewports     = nullptr;
  m_vk_pipeline_viewport.scissorCount   = 1;
  m_vk_pipeline_viewport.pScissors      = nullptr;

  return vkex::Result::Success;
}

vkex::Result CGraphicsPipeline::InitializeRasterization()
{
  m_vk_pipeline_rasterization.flags                    = 0;
  m_vk_pipeline_rasterization.depthClampEnable         = VK_FALSE;
  m_vk_pipeline_rasterization.rasterizerDiscardEnable  = VK_FALSE;
  m_vk_pipeline_rasterization.polygonMode              = VK_POLYGON_MODE_FILL;
  m_vk_pipeline_rasterization.cullMode                 = m_create_info.cull_mode;
  m_vk_pipeline_rasterization.frontFace                = m_create_info.front_face;
  m_vk_pipeline_rasterization.depthBiasEnable          = VK_FALSE;
  m_vk_pipeline_rasterization.depthBiasConstantFactor  = 0.0f;
  m_vk_pipeline_rasterization.depthBiasClamp           = 0.0f;
  m_vk_pipeline_rasterization.depthBiasSlopeFactor     = 0.0f;
  m_vk_pipeline_rasterization.lineWidth                = 1.0f;

  return vkex::Result::Success;
}

vkex::Result CGraphicsPipeline::InitializeMultisample()
{
  m_vk_pipeline_multisample.flags                  = 0;
  m_vk_pipeline_multisample.rasterizationSamples   = m_create_info.samples;
  m_vk_pipeline_multisample.sampleShadingEnable    = VK_FALSE;
  m_vk_pipeline_multisample.minSampleShading       = 0.0f;
  m_vk_pipeline_multisample.pSampleMask            = 0;
  m_vk_pipeline_multisample.alphaToCoverageEnable  = VK_FALSE;
  m_vk_pipeline_multisample.alphaToOneEnable       = VK_FALSE;

  return vkex::Result::Success;
}

vkex::Result CGraphicsPipeline::InitializeDepthStencil()
{
  m_vk_pipeline_depth_stencil.flags                 = 0;
  m_vk_pipeline_depth_stencil.depthTestEnable       = m_create_info.depth_test_enable ? VK_TRUE : VK_FALSE;
  m_vk_pipeline_depth_stencil.depthWriteEnable      = m_create_info.depth_write_enable ? VK_TRUE : VK_FALSE;
  m_vk_pipeline_depth_stencil.depthCompareOp        = VK_COMPARE_OP_LESS;
  m_vk_pipeline_depth_stencil.depthBoundsTestEnable = m_create_info.depth_bounds_test_enable ? VK_TRUE : VK_FALSE;
  m_vk_pipeline_depth_stencil.stencilTestEnable     = VK_FALSE;
  m_vk_pipeline_depth_stencil.front.failOp          = VK_STENCIL_OP_KEEP;
  m_vk_pipeline_depth_stencil.front.passOp          = VK_STENCIL_OP_KEEP;
  m_vk_pipeline_depth_stencil.front.depthFailOp     = VK_STENCIL_OP_KEEP;
  m_vk_pipeline_depth_stencil.front.compareOp       = VK_COMPARE_OP_NEVER; 
  m_vk_pipeline_depth_stencil.front.compareMask     = 0;
  m_vk_pipeline_depth_stencil.front.writeMask       = 0;
  m_vk_pipeline_depth_stencil.front.reference       = 0;
  m_vk_pipeline_depth_stencil.back.failOp           = VK_STENCIL_OP_KEEP;
  m_vk_pipeline_depth_stencil.back.passOp           = VK_STENCIL_OP_KEEP;
  m_vk_pipeline_depth_stencil.back.depthFailOp      = VK_STENCIL_OP_KEEP;
  m_vk_pipeline_depth_stencil.back.compareOp        = VK_COMPARE_OP_NEVER; 
  m_vk_pipeline_depth_stencil.back.compareMask      = 0;
  m_vk_pipeline_depth_stencil.back.writeMask        = 0;
  m_vk_pipeline_depth_stencil.back.reference        = 0;
  m_vk_pipeline_depth_stencil.minDepthBounds        = 0.0f;
  m_vk_pipeline_depth_stencil.maxDepthBounds        = 0.0f;

  return vkex::Result::Success;
}

vkex::Result CGraphicsPipeline::InitializeBlending()
{
  auto& attachemnts = m_create_info.color_blend_attachment_states.GetStates();
  // Warn if colorWriteMask is zero
  {
    uint32_t count = CountU32(attachemnts);
    for (uint32_t i = 0; i < count; ++i) {
      auto& attachment = attachemnts[i];
      if (attachment.colorWriteMask == 0) {
        std::string name = "<UNAMED>";
        VKEX_LOG_RAW("\n*** VKEX WARNING: Graphics Pipeline Warning! ***");
        VKEX_LOG_WARN("Function : " << __FUNCTION__);
        VKEX_LOG_WARN("Mesage   : Color blend attachment state " << i << " has colorWriteMask=0x0, is this what you want?");
        VKEX_LOG_RAW("");
      }
    }
  }
  
  m_vk_pipeline_color_blend.flags             = 0;
  m_vk_pipeline_color_blend.logicOpEnable     = m_create_info.color_blend_logic_op_enable ? VK_TRUE : VK_FALSE;
  m_vk_pipeline_color_blend.logicOp           = m_create_info.color_blend_logic_op;
  m_vk_pipeline_color_blend.attachmentCount   = CountU32(attachemnts);
  m_vk_pipeline_color_blend.pAttachments      = DataPtr(attachemnts);
  m_vk_pipeline_color_blend.blendConstants[0] = m_create_info.blend_constants[0];
  m_vk_pipeline_color_blend.blendConstants[1] = m_create_info.blend_constants[1];
  m_vk_pipeline_color_blend.blendConstants[2] = m_create_info.blend_constants[2];
  m_vk_pipeline_color_blend.blendConstants[3] = m_create_info.blend_constants[3];

  return vkex::Result::Success;
}

vkex::Result CGraphicsPipeline::InitializeDynamicState()
{
  m_vk_dynamic_states.push_back(VK_DYNAMIC_STATE_VIEWPORT);
  m_vk_dynamic_states.push_back(VK_DYNAMIC_STATE_SCISSOR);
  m_vk_dynamic_states.push_back(VK_DYNAMIC_STATE_LINE_WIDTH);
  m_vk_dynamic_states.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);
  m_vk_dynamic_states.push_back(VK_DYNAMIC_STATE_BLEND_CONSTANTS);
  m_vk_dynamic_states.push_back(VK_DYNAMIC_STATE_DEPTH_BOUNDS);
  m_vk_dynamic_states.push_back(VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK);
  m_vk_dynamic_states.push_back(VK_DYNAMIC_STATE_STENCIL_WRITE_MASK);
  m_vk_dynamic_states.push_back(VK_DYNAMIC_STATE_STENCIL_REFERENCE);
  
  m_vk_pipeline_dynamic_state.flags             = 0;
  m_vk_pipeline_dynamic_state.dynamicStateCount = CountU32(m_vk_dynamic_states);
  m_vk_pipeline_dynamic_state.pDynamicStates    = DataPtr(m_vk_dynamic_states);

  return vkex::Result::Success;
}

vkex::Result CGraphicsPipeline::InternalCreate(
  const vkex::GraphicsPipelineCreateInfo& create_info,
  const VkAllocationCallbacks*            p_allocator
)
{
  // Copy create info
  m_create_info = create_info;

  vkex::Result htk_result = InitializeShaderStages();
  if (!htk_result) {
    return htk_result;
  }

  htk_result = InitializeVertexInput();
  if (!htk_result) {
    return htk_result;
  }

  htk_result = InitializeInputAssembly();
  if (!htk_result) {
    return htk_result;
  }
 
  htk_result = InitializeTessellation();
  if (!htk_result) {
    return htk_result;
  }

  htk_result = InitializeViewports();
  if (!htk_result) {
    return htk_result;
  }

  htk_result = InitializeRasterization();
  if (!htk_result) {
    return htk_result;
  }

  htk_result = InitializeMultisample();
  if (!htk_result) {
    return htk_result;
  }

  htk_result = InitializeDepthStencil();
  if (!htk_result) {
    return htk_result;
  }

  htk_result = InitializeBlending();
  if (!htk_result) {
    return htk_result;
  }

  htk_result = InitializeDynamicState();
  if (!htk_result) {
    return htk_result;
  }

  // Pipeline cache
  VkPipelineCache vk_pipeline_cache = VK_NULL_HANDLE;
  if (m_create_info.pipeline_cache != nullptr) {
    vk_pipeline_cache = *(m_create_info.pipeline_cache);
  }

  // Render pass
  VkRenderPass vk_render_pass = (m_create_info.render_pass != nullptr)
                                ? *(m_create_info.render_pass)
                                : static_cast<VkRenderPass>(VK_NULL_HANDLE);
  //
  // Create a transient render pass using RTV/DSV formats and 
  // sample count if render pass object is not specified.
  //
  vkex::RenderPass transient_render_pass = nullptr;  
  if (vk_render_pass == VK_NULL_HANDLE) {
    // Create info
    vkex::RenderPassCreateInfo render_pass_create_info = {};
    render_pass_create_info.transient.rtv_formats = m_create_info.rtv_formats;
    render_pass_create_info.transient.dsv_format  = m_create_info.dsv_format;
    render_pass_create_info.transient.samples     = m_create_info.samples;
    // Render pass
    vkex::Result vkex_result = vkex::Result::Undefined;
    VKEX_RESULT_CALL(
      vkex_result,
      m_device->CreateRenderPass(
        render_pass_create_info, 
        &transient_render_pass)
    );
    if (!vkex_result) {
      return vkex_result;
    }

    vk_render_pass = *transient_render_pass;
  }

  // Vulkan create info
  m_vk_create_info = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
  m_vk_create_info.flags                = 0;
  m_vk_create_info.stageCount           = CountU32(m_vk_shader_stages);
  m_vk_create_info.pStages              = DataPtr(m_vk_shader_stages);
  m_vk_create_info.pVertexInputState    = &m_vk_pipeline_vertex_input;
  m_vk_create_info.pInputAssemblyState  = &m_vk_pipeline_input_assembly;
  m_vk_create_info.pTessellationState   = &m_vk_pipeline_tessellation;
  m_vk_create_info.pViewportState       = &m_vk_pipeline_viewport;
  m_vk_create_info.pRasterizationState  = &m_vk_pipeline_rasterization;
  m_vk_create_info.pMultisampleState    = &m_vk_pipeline_multisample;
  m_vk_create_info.pDepthStencilState   = &m_vk_pipeline_depth_stencil;
  m_vk_create_info.pColorBlendState     = &m_vk_pipeline_color_blend;
  m_vk_create_info.pDynamicState        = &m_vk_pipeline_dynamic_state;
  m_vk_create_info.layout               = *(m_create_info.pipeline_layout);
  m_vk_create_info.renderPass           = vk_render_pass;
  m_vk_create_info.subpass              = m_create_info.subpass;
  m_vk_create_info.basePipelineHandle   = VK_NULL_HANDLE;
  m_vk_create_info.basePipelineIndex    = -1;
  // Call create
  VkResult vk_result = InvalidValue<VkResult>::Value;
  VKEX_VULKAN_RESULT_CALL(
    vk_result,
    vkex::CreateGraphicsPipelines(
      *m_device,
      vk_pipeline_cache,
      1,
      &m_vk_create_info, 
      p_allocator,
      &m_vk_object)
  );

  // Destroy temporary render pass
  if (transient_render_pass != nullptr) {
    vkex::Result vkex_result = vkex::Result::Undefined;
    VKEX_RESULT_CALL(
      vkex_result,
      m_device->DestroyRenderPass(transient_render_pass);
    );
    if (!vkex_result) {
      return vkex_result;
    }
    transient_render_pass = nullptr;
  }

  if (vk_result != VK_SUCCESS) {
    return vkex::Result(vk_result);
  }

  return vkex::Result::Success;
}

vkex::Result CGraphicsPipeline::InternalDestroy(const VkAllocationCallbacks* p_allocator)
{
  if (m_vk_object != VK_NULL_HANDLE) {
    vkex::DestroyPipeline(
      *m_device,
      m_vk_object,
      p_allocator);
  }

  return vkex::Result::Success;
}

} // namespace vkex