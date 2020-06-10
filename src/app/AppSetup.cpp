/*
 Copyright 2020 Google Inc.

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

#include "AppCore.h"

#include "AssetUtil.h"

void VkexInfoApp::SetupImagesAndRenderPasses(const VkExtent2D present_extent,
                                             const VkFormat color_format,
                                             const VkFormat depth_format) {
  {
    VKEX_CALL(CreateSimpleRenderPass(
        GetDevice(), GetGraphicsQueue(), present_extent.width,
        present_extent.height, color_format, depth_format,
        &m_internal_draw_simple_render_pass));
  }

  {
    VKEX_CALL(CreateSimpleRenderPass(
        GetDevice(), GetGraphicsQueue(), present_extent.width,
        present_extent.height, color_format, depth_format,
        &m_internal_as_target_draw_simple_render_pass));
  }

  {
    vkex::TextureCreateInfo create_info = {};
    create_info.image.image_type = VK_IMAGE_TYPE_2D;
    create_info.image.format = color_format;
    create_info.image.extent = {present_extent.width, present_extent.height, 1};
    create_info.image.mip_levels = 1;
    create_info.image.array_layers = 1;
    create_info.image.samples = VK_SAMPLE_COUNT_1_BIT;
    create_info.image.tiling = VK_IMAGE_TILING_OPTIMAL;
    create_info.image.usage_flags.bits.color_attachment = false;
    create_info.image.usage_flags.bits.transfer_src =
        true;  // Maybe if we use vkCmdBlitImage?
    create_info.image.usage_flags.bits.sampled = true;
    create_info.image.usage_flags.bits.storage = true;
    create_info.image.sharing_mode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.image.initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;
    create_info.image.committed = true;
    create_info.image.host_visible = false;
    create_info.image.device_local = true;
    create_info.view.derive_from_image = true;

    {
      for (uint32_t target_texture_index = 0;
           target_texture_index < kNumHistoryImages; target_texture_index++) {
        VKEX_CALL(GetDevice()->CreateTexture(
            create_info, &m_target_texture_list[target_texture_index]));
      }

      VKEX_CALL(
          GetDevice()->CreateTexture(create_info, &m_visualization_texture));
    }
  }

  {
    for (uint32_t target_texture_index = 0;
         target_texture_index < kNumHistoryImages; target_texture_index++) {
      VKEX_CALL(vkex::TransitionImageLayout(
          GetGraphicsQueue(), m_target_texture_list[target_texture_index],
          VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL,
          VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT));
    }

    VKEX_CALL(vkex::TransitionImageLayout(
        GetGraphicsQueue(), m_visualization_texture, VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT));
  }

  {
    auto checkerboard_width = present_extent.width / 2;
    auto checkerboard_height = present_extent.height / 2;

    VkImageCreateFlags extra_depth_usage_flags = 0;
    {
      if (m_sample_locations_enabled) {
        extra_depth_usage_flags =
            VK_IMAGE_CREATE_SAMPLE_LOCATIONS_COMPATIBLE_DEPTH_BIT_EXT;
      }
    }

    for (uint32_t checkerboard_index = 0;
         checkerboard_index < kNumHistoryImages; checkerboard_index++) {
      VKEX_CALL(CreateSimpleMSRenderPass(
          GetDevice(), GetGraphicsQueue(), checkerboard_width,
          checkerboard_height, color_format, depth_format,
          VK_SAMPLE_COUNT_2_BIT, extra_depth_usage_flags,
          &m_checkerboard_simple_render_pass[checkerboard_index]));
    }
  }
}

void VkexInfoApp::BuildCheckerboardMaterialSampler() {
  const GLTFModel::Material& mat = m_helmet_model.GetMaterialInfo(0, 0);
  const GLTFModel::Texture& tex = m_helmet_model.GetTextureInfo(
      mat.textureIndices[GLTFModel::MaterialTextureType::BaseColor]);
  const GLTFModel::Sampler original_sampler =
      m_helmet_model.GetSamplerInfo(tex.samplerIndex);

  vkex::SamplerCreateInfo checkerboard_sampler_ci = {};
  checkerboard_sampler_ci.min_filter = original_sampler.minFilter;
  checkerboard_sampler_ci.mag_filter = original_sampler.magFilter;
  checkerboard_sampler_ci.mipmap_mode = original_sampler.mipmapMode;
  checkerboard_sampler_ci.min_lod = 0.0f;
  checkerboard_sampler_ci.max_lod = 15.0f;
  checkerboard_sampler_ci.address_mode_u = original_sampler.wrapS;
  checkerboard_sampler_ci.address_mode_v = original_sampler.wrapT;
  checkerboard_sampler_ci.address_mode_w = original_sampler.wrapR;
  checkerboard_sampler_ci.mip_lod_bias = -1.f;

  VKEX_CALL(GetGraphicsQueue()->GetDevice()->CreateSampler(
      checkerboard_sampler_ci, &m_cb_grad_adj_sampler));
}

void ConfigureDynamicUbos(vkex::DescriptorSetLayoutCreateInfo& create_info) {
  for (auto& binding : create_info.bindings) {
    if (binding.descriptorType ==
        VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
      binding.descriptorType =
          VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    }
  }
}

void VkexInfoApp::SetupShaders(
    const std::vector<ShaderProgramInputs>& shader_inputs,
    std::vector<GeneratedShaderState>& generated_shader_states) {
  generated_shader_states.reserve(shader_inputs.size());

  vkex::DescriptorPoolCreateInfo descriptor_pool_create_info = {};

  for (auto& shader_input : shader_inputs) {
    GeneratedShaderState gen_shader_state = {};

    {
      gen_shader_state.pipeline_type = shader_input.pipeline_type;

      // TODO: Check shader_paths length?
      if (gen_shader_state.pipeline_type == ShaderPipelineType::Compute) {
        VKEX_CALL(asset_util::CreateShaderProgramCompute(
            GetDevice(), shader_input.shader_paths[0],
            &gen_shader_state.program));
      } else {
        VKEX_CALL(asset_util::CreateShaderProgram(
            GetDevice(), shader_input.shader_paths[0],
            shader_input.shader_paths[1], &gen_shader_state.program));
      }
    }

    {
      const vkex::ShaderInterface& shader_interface =
          gen_shader_state.program->GetInterface();
      vkex::DescriptorSetLayoutCreateInfo create_info =
          ToVkexCreateInfo(shader_interface.GetSet(0));
      ConfigureDynamicUbos(create_info);
      VKEX_CALL(GetDevice()->CreateDescriptorSetLayout(
          create_info, &gen_shader_state.descriptor_set_layout));

      descriptor_pool_create_info.pool_sizes +=
          shader_interface.GetDescriptorPoolSizes();
    }

    {
      vkex::PipelineLayoutCreateInfo create_info = {};
      create_info.descriptor_set_layouts.push_back(
          vkex::ToVulkan(gen_shader_state.descriptor_set_layout));
      VKEX_CALL(GetDevice()->CreatePipelineLayout(
          create_info, &gen_shader_state.pipeline_layout));
    }

    {
      if (gen_shader_state.pipeline_type == ShaderPipelineType::Compute) {
        vkex::ComputePipelineCreateInfo create_info = {};
        create_info.shader_program = gen_shader_state.program;
        create_info.pipeline_layout = gen_shader_state.pipeline_layout;

        VKEX_CALL(GetDevice()->CreateComputePipeline(
            create_info, &gen_shader_state.compute_pipeline));
      } else {
        vkex::GraphicsPipelineCreateInfo gfx_create_info =
            shader_input.graphics_pipeline_create_info;
        gfx_create_info.shader_program = gen_shader_state.program;
        gfx_create_info.pipeline_layout = gen_shader_state.pipeline_layout;

        VKEX_CALL(GetDevice()->CreateGraphicsPipeline(
            gfx_create_info, &gen_shader_state.graphics_pipeline));
      }
    }

    generated_shader_states.push_back(gen_shader_state);
  }

  const uint32_t frame_count = GetConfiguration().frame_count;

  {
    // TODO: DescriptorPoolCreateInfo does have a size_multiplier, but it isn't
    // used yet...
    descriptor_pool_create_info.pool_sizes *= frame_count;
    VKEX_CALL(GetDevice()->CreateDescriptorPool(descriptor_pool_create_info,
                                                &m_shared_descriptor_pool));
  }

  for (auto& gen_shader_state : generated_shader_states) {
    vkex::DescriptorSetAllocateInfo allocate_info = {};
    allocate_info.layouts.push_back(gen_shader_state.descriptor_set_layout);

    gen_shader_state.descriptor_sets.resize(frame_count);
    for (uint32_t frame_index = 0; frame_index < frame_count; frame_index++) {
      VKEX_CALL(m_shared_descriptor_pool->AllocateDescriptorSets(
          allocate_info, &gen_shader_state.descriptor_sets[frame_index]));
    }
  }
}

void VkexInfoApp::CheckVulkanFeaturesForPipelines() {
  if (GetDevice()
          ->GetPhysicalDevice()
          ->GetPhysicalDeviceFeatures()
          .features.sampleRateShading == VK_FALSE) {
    VKEX_LOG_ERROR(
        "sampleRateShading is missing from VkPhysicalDeviceFeatures, "
        "Checkerboard Rendering will fail!");
  }

  {
    std::string sample_locations_name = VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME;
    if (vkex::Contains(GetDevice()->GetLoadedExtensions(),
                       sample_locations_name)) {
      m_sample_locations_enabled = true;
    } else {
      // We could use viewport jitter, but it complicates the solution.
      // We could consider re-adding support in the future.
      VKEX_LOG_ERROR(VK_EXT_SAMPLE_LOCATIONS_EXTENSION_NAME
                     " not enabled, checkerboard rendering currently requires "
                     "the extension.");
    }

    // If variableSampleLocations is supported, we aren't obliged
    // to use the render pass structures associated with
    // VK_EXT_sample_locations for the purposes of validating consistent
    // sample locations.
    if (m_sample_locations_enabled) {
      m_variable_sample_locations_available =
          (GetDevice()
               ->GetPhysicalDevice()
               ->GetSampleLocationsPropertiesEXT()
               .variableSampleLocations == VK_TRUE);
    }
  }
}

void VkexInfoApp::ConfigureCustomSampleLocationsState() {
  if (m_sample_locations_enabled) {
    m_current_sample_locations_info = {
        VK_STRUCTURE_TYPE_SAMPLE_LOCATIONS_INFO_EXT};
    m_current_sample_locations_info.sampleLocationsPerPixel =
        VK_SAMPLE_COUNT_2_BIT;
    m_current_sample_locations_info.sampleLocationGridSize = {1, 1};

    // Every client that needs VkSampleLocationsInfoEXT will
    // use this vector of sample locations. Since we keep them
    // unified across the lifetime of a frame, it's ok to point
    // to the same backing data. Makes it easy to toggle between
    // frames without having to replicate data.
    m_current_sample_locations.push_back({0.75, 0.75});
    m_current_sample_locations.push_back({0.25, 0.25});

    m_current_sample_locations_info.sampleLocationsCount =
        vkex::CountU32(m_current_sample_locations);
    m_current_sample_locations_info.pSampleLocations =
        vkex::DataPtr(m_current_sample_locations);

    // It's really not clear how much info is _actually_
    // needed in this structure, mostly because information on
    // how custom sample locations affect render pass/render
    // target/pipeline functionality is scattered all over the spec.
    // The spec seems to indicate you only need VkAttachmentSampleLocationsEXT
    // in order to perform implicit transition layouts of depth images.
    // But it seems if it needs the information to do layout transitions,
    // maybe it also needs the info to do render pass clears.
    // VkSubpassSampleLocationsEXT seems to be needed for the depth layout
    // transitions or to validate that pipelines don't change in a subpass
    // if VkPhysicalDeviceSampleLocationsPropertiesEXT::variableSampleLocations
    // is VK_FALSE.
    // It doesn't seem to hurt if I have this information, and since it's one
    // subpass in the sample, I don't see the harm. If I had a full-featured
    // engine, I could see being pretty irritated to add this functionality.
    // I guess the need for the structure is omitted as long as I don't
    // need implicit layout transitions or ship on a platform that forbids
    // variableSampleLocations.
    m_rp_sample_locations = {
        VK_STRUCTURE_TYPE_RENDER_PASS_SAMPLE_LOCATIONS_BEGIN_INFO_EXT};

    VkAttachmentSampleLocationsEXT attachment_sample_locations = {};
    attachment_sample_locations.attachmentIndex =
        m_checkerboard_simple_render_pass[0]
            .render_pass->GetDepthStencilAttachmentReference()
            .attachment;
    attachment_sample_locations.sampleLocationsInfo =
        m_current_sample_locations_info;
    m_attachment_sample_locations.push_back(attachment_sample_locations);

    VkSubpassSampleLocationsEXT subpass_sample_locations = {};
    subpass_sample_locations.subpassIndex = 0;
    subpass_sample_locations.sampleLocationsInfo =
        m_current_sample_locations_info;
    m_subpass_sample_locations.push_back(subpass_sample_locations);

    m_rp_sample_locations.attachmentInitialSampleLocationsCount =
        vkex::CountU32(m_attachment_sample_locations);
    m_rp_sample_locations.pAttachmentInitialSampleLocations =
        vkex::DataPtr(m_attachment_sample_locations);
    m_rp_sample_locations.postSubpassSampleLocationsCount =
        vkex::CountU32(m_subpass_sample_locations);
    m_rp_sample_locations.pPostSubpassSampleLocations =
        vkex::DataPtr(m_subpass_sample_locations);
  }
}

void VkexInfoApp::SetupInitialConstantBufferValues() {
  // Plumb some dummy values for the transformation matrices to prevent
  // degenerate cases in the shader
  m_per_object_constants.data.worldMatrix = float4x4(1.f);
  m_per_object_constants.data.prevWorldMatrix = float4x4(1.f);
  m_per_frame_constants.data.viewProjectionMatrix = float4x4(1.f);
  m_per_frame_constants.data.prevViewProjectionMatrix = float4x4(1.f);
}
