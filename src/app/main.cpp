/*
 Copyright 2019-2020 Google Inc.

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

void VkexInfoApp::AddArgs(vkex::ArgParser& args)
{
    args.AddOptionInt("h", "height", "Height of swapchain image (1080, 2160)", 1080);
}

void VkexInfoApp::Configure(const vkex::ArgParser& args, vkex::Configuration& configuration)
{
    configuration.window.resizeable = false;

    // TODO: We need to use UNORM because it supports storage on AMD,
    // but we need to make sure the rest of the render chain correctly
    // handle sRGB-ness through the chain, which they do not right now
    // Alternatively, the final copy could be a graphics blit?
    // Of course, on the Intel part I'm on doesn't support STORAGE on
    // this format. In the future, we'll support both compute and graphics.
    configuration.swapchain.color_format = VK_FORMAT_B8G8R8A8_UNORM;
    configuration.swapchain.paced_frame_rate = 60;

    // INFO: Right now, Application::InitializeVkexSwapchain() creates a renderpass
    // for the swapchain images based on some of the config bits. Right now, it's
    // hard-coded to load the existing contents, plus all layouts are VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL.
    // This really should be configurable, allowing for different uses without
    // touching the vkex lib code.

#if defined(ENABLE_VALIDATION)
    configuration.graphics_debug.enable = true;
    configuration.graphics_debug.message_severity.info = true;
    configuration.graphics_debug.message_severity.warning = true;
    configuration.graphics_debug.message_severity.error = true;
    configuration.graphics_debug.message_type.validation = true;
#else
    configuration.graphics_debug.enable = false;
    configuration.graphics_debug.message_severity.info = false;
    configuration.graphics_debug.message_severity.warning = false;
    configuration.graphics_debug.message_severity.error = false;
    configuration.graphics_debug.message_type.validation = false;
#endif // defined(ENABLE_VALIDATION)

    int32_t requested_height = 1080;
    args.GetInt("h", "height", &requested_height);

    // On platforms where the swapchain size is fixed, this will
    // be overridden when surface capabilities are obtained.
    if (requested_height == 2160) {
        configuration.window.width = 3840;
        configuration.window.height = 2160;
    } else {
        configuration.window.width = 1920;
        configuration.window.height = 1080;
    }

    if ((requested_height != 2160) && (requested_height != 1080)) {
        VKEX_LOG_WARN("Requested window height is unsupported: " << requested_height);
        VKEX_LOG_WARN("Window dimensions defaulting to 1920 x 1080");
    }
}

void VkexInfoApp::Setup()
{
    CheckVulkanFeaturesForPipelines();

    {
        auto present_res_key =
            FindPresentResolutionKey(GetConfiguration().window.width);
        SetPresentResolution(present_res_key);
    }

    {
        auto helmet_path = GetAssetPath("models/DamagedHelmet/glTF/DamagedHelmet.gltf");
        m_helmet_model.PopulateFromModel(helmet_path, GetGraphicsQueue());
    }

    // Render state managed from CPU side
    {
        m_animation_enabled = true;
        m_animation_progress = 0.0f;
        m_light_infos.resize(1); // TODO: Support more than one light?
        m_light_infos[0].lightType = LightType::kDirectional;
        m_light_infos[0].direction = float3(0.f, 0.f, 1.f);
        m_light_infos[0].color = float3(1.0f, 1.0f, 1.0f);
        m_light_infos[0].intensity = 10.f;
    }

    {
        auto frame_count = GetConfiguration().frame_count;
        m_per_frame_datas.resize(frame_count);
    }

    {
        m_selected_upscaling_technique_index = 0;
        m_upscaling_technique_key = UpscalingTechniqueKey::None;
    }

    SetupImagesAndRenderPasses(GetPresentResolutionExtent(), GetConfiguration().swapchain.color_format, VK_FORMAT_D32_SFLOAT);

    // Build pipelines + related state
    {
        // TODO: Move this shader/pipeline generation into another file where it's
        // simple + isolated to add in new shaders and related info

        // TODO: Material shaders probably have to be compiled separately to handle the different
        // possible combinations via specialization constants

        std::vector<ShaderProgramInputs> shader_inputs(AppShaderList::NumTypes);

        {
            shader_inputs[AppShaderList::Geometry].pipeline_type = ShaderPipelineType::Graphics;

            shader_inputs[AppShaderList::Geometry].shader_paths.resize(2);
            shader_inputs[AppShaderList::Geometry].shader_paths[0] = GetAssetPath("shaders/draw_standard.vs.spv");
            shader_inputs[AppShaderList::Geometry].shader_paths[1] = GetAssetPath("shaders/draw_standard.ps.spv");


            std::vector<vkex::VertexBindingDescription> vertex_buffer_bindings = m_helmet_model.GetVertexBindingDescriptions(0, 0);
            std::vector<VkFormat> vertex_buffer_formats = m_helmet_model.GetVertexBufferFormats(0, 0);

            // TODO: We could have shared constants/defines for the locations?
            vertex_buffer_bindings[GLTFModel::BufferType::Position].AddAttribute(0, vertex_buffer_formats[GLTFModel::BufferType::Position]);
            vertex_buffer_bindings[GLTFModel::BufferType::Normal].AddAttribute(1, vertex_buffer_formats[GLTFModel::BufferType::Normal]);
            vertex_buffer_bindings[GLTFModel::BufferType::TexCoord0].AddAttribute(2, vertex_buffer_formats[GLTFModel::BufferType::TexCoord0]);

            vkex::GraphicsPipelineCreateInfo create_info = {};
            create_info.vertex_binding_descriptions = vertex_buffer_bindings;
            create_info.samples = VK_SAMPLE_COUNT_1_BIT;
            create_info.depth_test_enable = true;
            create_info.depth_write_enable = true;
            create_info.rtv_formats = { m_internal_draw_simple_render_pass.rtv->GetFormat() };
            create_info.dsv_format = m_internal_draw_simple_render_pass.dsv->GetFormat();
            create_info.render_pass = m_internal_draw_simple_render_pass.render_pass;
            shader_inputs[AppShaderList::Geometry].graphics_pipeline_create_info = create_info;
        }
        {
            shader_inputs[AppShaderList::InternalToTargetScaledCopy].pipeline_type = ShaderPipelineType::Compute;

            shader_inputs[AppShaderList::InternalToTargetScaledCopy].shader_paths.resize(1);
            shader_inputs[AppShaderList::InternalToTargetScaledCopy].shader_paths[0] = GetAssetPath("shaders/copy_texture.cs.spv");
        }
        {
            shader_inputs[AppShaderList::InternalTargetImageDelta].pipeline_type = ShaderPipelineType::Compute;

            shader_inputs[AppShaderList::InternalTargetImageDelta].shader_paths.resize(1);
            shader_inputs[AppShaderList::InternalTargetImageDelta].shader_paths[0] = GetAssetPath("shaders/image_delta.cs.spv");
        }
        {
            // TODO: Right now, we'll re-use the previous shader, but this probably has to change to a graphics blit because of
            // swapchain + compute usage issues
            shader_inputs[AppShaderList::TargetToPresentScaledCopy].pipeline_type = ShaderPipelineType::Compute;

            shader_inputs[AppShaderList::TargetToPresentScaledCopy].shader_paths.resize(1);
            shader_inputs[AppShaderList::TargetToPresentScaledCopy].shader_paths[0] = GetAssetPath("shaders/copy_texture.cs.spv");
        }
        {
            shader_inputs[AppShaderList::UpscalingCAS].pipeline_type =
                ShaderPipelineType::Compute;

            shader_inputs[AppShaderList::UpscalingCAS].shader_paths.resize(1);
            shader_inputs[AppShaderList::UpscalingCAS].shader_paths[0] =
                GetAssetPath("shaders/cas.cs.spv");
        }
        {
            shader_inputs[AppShaderList::CheckerboardUpscale].pipeline_type = ShaderPipelineType::Compute;

            shader_inputs[AppShaderList::CheckerboardUpscale].shader_paths.resize(1);
            shader_inputs[AppShaderList::CheckerboardUpscale].shader_paths[0] = GetAssetPath("shaders/checkerboard_upscale.cs.spv");
        }
        {
            
            shader_inputs[AppShaderList::GeometryCB].pipeline_type = ShaderPipelineType::Graphics;

            shader_inputs[AppShaderList::GeometryCB].shader_paths.resize(2);
            shader_inputs[AppShaderList::GeometryCB].shader_paths[0] = GetAssetPath("shaders/draw_cb.vs.spv");
            shader_inputs[AppShaderList::GeometryCB].shader_paths[1] = GetAssetPath("shaders/draw_cb.ps.spv");


            std::vector<vkex::VertexBindingDescription> vertex_buffer_bindings = m_helmet_model.GetVertexBindingDescriptions(0, 0);
            std::vector<VkFormat> vertex_buffer_formats = m_helmet_model.GetVertexBufferFormats(0, 0);

            // TODO: We could have shared constants/defines for the locations?
            vertex_buffer_bindings[GLTFModel::BufferType::Position].AddAttribute(0, vertex_buffer_formats[GLTFModel::BufferType::Position]);
            vertex_buffer_bindings[GLTFModel::BufferType::Normal].AddAttribute(1, vertex_buffer_formats[GLTFModel::BufferType::Normal]);
            vertex_buffer_bindings[GLTFModel::BufferType::TexCoord0].AddAttribute(2, vertex_buffer_formats[GLTFModel::BufferType::TexCoord0]);

            vkex::GraphicsPipelineCreateInfo create_info = {};
            create_info.vertex_binding_descriptions = vertex_buffer_bindings;
            
            // We can enable sample location shading with this, but we still
            // need to manually enable sample location interpolation
            // in the PS input structure which makes this almost useless...
            create_info.samples = VK_SAMPLE_COUNT_2_BIT; 
            create_info.min_sample_shading_factor = 1.0f;
            create_info.sample_shading_enable = VK_TRUE;

            create_info.depth_test_enable = true;
            create_info.depth_write_enable = true;
            create_info.rtv_formats = { m_checkerboard_simple_render_pass[0].rtv->GetFormat() };
            create_info.dsv_format = m_checkerboard_simple_render_pass[0].dsv->GetFormat();
            create_info.render_pass = m_checkerboard_simple_render_pass[0].render_pass;
            shader_inputs[AppShaderList::GeometryCB].graphics_pipeline_create_info = create_info;
        }

        SetupShaders(shader_inputs, m_generated_shader_states);
    }

    // constant buffers
    {
        auto frame_count = GetConfiguration().frame_count;
        m_constant_buffer_manager.Initialize(GetDevice(), GetGraphicsQueue(), frame_count);
    }

    // Scene rendering descriptors
    {
        // TODO: There will probably only be one sampler to bind...

        auto& helmet_samplers = m_helmet_model.GetMaterialSamplers(0, 0);
        auto& helmet_textures = m_helmet_model.GetMaterialTextures(0, 0);
        // TODO: Don't hard code this...
        auto matComponent = GLTFModel::MaterialTextureType::BaseColor;

        auto frame_count = GetConfiguration().frame_count;
        for (uint32_t frame_index = 0; frame_index < frame_count; frame_index++) {
            auto& per_frame_data = m_per_frame_datas[frame_index];

            m_generated_shader_states[AppShaderList::Geometry].descriptor_sets[frame_index]->UpdateDescriptor(0,
                m_constant_buffer_manager.GetBuffer(frame_index),
                m_per_frame_constants.size);
            m_generated_shader_states[AppShaderList::Geometry].descriptor_sets[frame_index]->UpdateDescriptor(1,
                m_constant_buffer_manager.GetBuffer(frame_index),
                m_per_object_constants.size);

            // TODO: If models are switching, these updates will have to be per-frame
            m_generated_shader_states[AppShaderList::Geometry].descriptor_sets[frame_index]->UpdateDescriptor(2, helmet_samplers[matComponent]);
            m_generated_shader_states[AppShaderList::Geometry].descriptor_sets[frame_index]->UpdateDescriptor(3, helmet_textures[matComponent]);

            // TODO: I'm assuming the sampler, which is not correct
            m_generated_shader_states[AppShaderList::Geometry].descriptor_sets[frame_index]->UpdateDescriptor(4, helmet_textures[GLTFModel::MaterialTextureType::MetallicRoughness]);
            m_generated_shader_states[AppShaderList::Geometry].descriptor_sets[frame_index]->UpdateDescriptor(5, helmet_textures[GLTFModel::MaterialTextureType::Emissive]);
            m_generated_shader_states[AppShaderList::Geometry].descriptor_sets[frame_index]->UpdateDescriptor(6, helmet_textures[GLTFModel::MaterialTextureType::Occlusion]);
            m_generated_shader_states[AppShaderList::Geometry].descriptor_sets[frame_index]->UpdateDescriptor(7, helmet_textures[GLTFModel::MaterialTextureType::NormalTex]);

            {
                m_generated_shader_states[AppShaderList::GeometryCB].descriptor_sets[frame_index]->UpdateDescriptor(0,
                    m_constant_buffer_manager.GetBuffer(frame_index),
                    m_per_frame_constants.size);
                m_generated_shader_states[AppShaderList::GeometryCB].descriptor_sets[frame_index]->UpdateDescriptor(1,
                    m_constant_buffer_manager.GetBuffer(frame_index),
                    m_per_object_constants.size);

                // TODO: If models are switching, these updates will have to be per-frame
                m_generated_shader_states[AppShaderList::GeometryCB].descriptor_sets[frame_index]->UpdateDescriptor(2, helmet_samplers[matComponent]);
                m_generated_shader_states[AppShaderList::GeometryCB].descriptor_sets[frame_index]->UpdateDescriptor(3, helmet_textures[matComponent]);

                // TODO: I'm assuming the sampler, which is not correct
                m_generated_shader_states[AppShaderList::GeometryCB].descriptor_sets[frame_index]->UpdateDescriptor(4, helmet_textures[GLTFModel::MaterialTextureType::MetallicRoughness]);
                m_generated_shader_states[AppShaderList::GeometryCB].descriptor_sets[frame_index]->UpdateDescriptor(5, helmet_textures[GLTFModel::MaterialTextureType::Emissive]);
                m_generated_shader_states[AppShaderList::GeometryCB].descriptor_sets[frame_index]->UpdateDescriptor(6, helmet_textures[GLTFModel::MaterialTextureType::Occlusion]);
                m_generated_shader_states[AppShaderList::GeometryCB].descriptor_sets[frame_index]->UpdateDescriptor(7, helmet_textures[GLTFModel::MaterialTextureType::NormalTex]);
            }
        }
    }

    // Upscale + visualization descriptors
    {
        // TODO: It would be nice to have this grab the binding via the name instead of magically knowing
        // the binding here :p (TBH, all of that could be done offline as well, but whatever)

        auto frame_count = GetConfiguration().frame_count;
        for (uint32_t frame_index = 0; frame_index < frame_count; frame_index++) {
            auto& per_frame_data = m_per_frame_datas[frame_index];
            auto constant_buffer = m_constant_buffer_manager.GetBuffer(frame_index);

            m_generated_shader_states[AppShaderList::InternalToTargetScaledCopy].descriptor_sets[frame_index]->UpdateDescriptor(0,
                constant_buffer,
                m_internal_to_target_scaled_copy_constants.size);
            m_generated_shader_states[AppShaderList::InternalToTargetScaledCopy].descriptor_sets[frame_index]->UpdateDescriptor(1, m_internal_draw_simple_render_pass.rtv_texture);
            m_generated_shader_states[AppShaderList::InternalToTargetScaledCopy].descriptor_sets[frame_index]->UpdateDescriptor(2, m_target_texture);

            m_generated_shader_states[AppShaderList::InternalTargetImageDelta].descriptor_sets[frame_index]->UpdateDescriptor(0,
                constant_buffer,
                m_internal_to_target_scaled_copy_constants.size);
            m_generated_shader_states[AppShaderList::InternalTargetImageDelta].descriptor_sets[frame_index]->UpdateDescriptor(1,
                constant_buffer,
                m_image_delta_options_constants.size);
            m_generated_shader_states[AppShaderList::InternalTargetImageDelta].descriptor_sets[frame_index]->UpdateDescriptor(2, m_target_texture);
            m_generated_shader_states[AppShaderList::InternalTargetImageDelta].descriptor_sets[frame_index]->UpdateDescriptor(3, m_internal_as_target_draw_simple_render_pass.rtv_texture);
            m_generated_shader_states[AppShaderList::InternalTargetImageDelta].descriptor_sets[frame_index]->UpdateDescriptor(4, m_visualization_texture);

            m_generated_shader_states[AppShaderList::TargetToPresentScaledCopy].descriptor_sets[frame_index]->UpdateDescriptor(0,
                constant_buffer,
                m_target_to_present_scaled_copy_constants.size);
            m_generated_shader_states[AppShaderList::TargetToPresentScaledCopy].descriptor_sets[frame_index]->UpdateDescriptor(1, m_visualization_texture);

            m_generated_shader_states[AppShaderList::UpscalingCAS]
                .descriptor_sets[frame_index]
                ->UpdateDescriptor(0, constant_buffer,
                    m_cas_upscaling_constants.size);
            m_generated_shader_states[AppShaderList::UpscalingCAS]
                .descriptor_sets[frame_index]
                ->UpdateDescriptor(
                    1, m_internal_draw_simple_render_pass.rtv_texture);
            m_generated_shader_states[AppShaderList::UpscalingCAS]
                .descriptor_sets[frame_index]
                ->UpdateDescriptor(2, m_target_texture);

            {
                m_generated_shader_states[AppShaderList::CheckerboardUpscale].descriptor_sets[frame_index]->UpdateDescriptor(0,
                    constant_buffer,
                    m_cb_upscaling_constants.size);
                m_generated_shader_states[AppShaderList::CheckerboardUpscale].descriptor_sets[frame_index]->UpdateDescriptor(3, m_target_texture);
            }
        }
    }

    // Timer setup
    {
        auto frame_count = GetConfiguration().frame_count;

        for (uint32_t frame_index = 0; frame_index < frame_count; frame_index++) {
            auto& per_frame_data = m_per_frame_datas[frame_index];

            // TODO: Perhaps one pool for all frames?

            vkex::QueryPoolCreateInfo query_pool_create_info = {};
            query_pool_create_info.query_type = VkQueryType::VK_QUERY_TYPE_TIMESTAMP;
            query_pool_create_info.query_count = TimerTag::kTimerQueryCount;

            {
                VKEX_CALL(GetDevice()->CreateQueryPool(query_pool_create_info, &(per_frame_data.timer_query_pool)));
            }

            per_frame_data.issued_gpu_timers.resize(TimerTag::kTimerTagCount);
        }
    }
}

void VkexInfoApp::Update(double frame_elapsed_time)
{
    // TODO: Make this an update of CPU logic structures, not necessarily matching graphics stuff
    // Then do the graphics-based structure conversion at constant buffer update time

    // Almost entirely doing CPU-side updates of constant buffers

    float3 eye = float3(0, 0, 2);
    float3 center = float3(0, 0, 0);
    float3 up = float3(0, 1, 0);
    float aspect = GetWindowAspect();
    vkex::PerspCamera camera(eye, center, up, 60.0f, aspect);

    if (m_animation_enabled) {
        // TODO: Make the rate configurable?
        m_animation_progress += (1 / 60.f);
    }
    float4x4 M = glm::translate(float3(0, 0, 0)) *
        glm::rotate(m_animation_progress / 2.0f, float3(0, 1, 0)) *
        glm::rotate(m_animation_progress / 4.0f, float3(1, 0, 0));
    float4x4 V = camera.GetViewMatrix();
    float4x4 P = camera.GetProjectionMatrix();

    m_per_object_constants.data.worldMatrix = M;

    m_per_frame_constants.data.viewProjectionMatrix = P * V;
    m_per_frame_constants.data.cameraPos = eye;

    m_per_frame_constants.data.dirLight = ConvertCPULightInfoToGPULightInfo(m_light_infos[0]);

    UpdateMaterialConstants();

    UpdateTargetResolutionState();
    UpdateInternalResolutionState();
    UpdateUpscalingTechniqueState();

    auto internal_res_extent = GetInternalResolutionExtent();
    auto target_res_extent = GetTargetResolutionExtent();
    auto present_res_extent = GetPresentResolutionExtent();

    m_internal_render_area.offset.x = 0;
    m_internal_render_area.offset.x = 0;
    m_internal_render_area.extent = internal_res_extent;

    m_target_render_area.offset.x = 0;
    m_target_render_area.offset.x = 0;
    m_target_render_area.extent = target_res_extent;

    m_internal_to_target_scaled_copy_constants.data.srcWidth = internal_res_extent.width;
    m_internal_to_target_scaled_copy_constants.data.srcHeight = internal_res_extent.height;
    m_internal_to_target_scaled_copy_constants.data.dstWidth = target_res_extent.width;
    m_internal_to_target_scaled_copy_constants.data.dstHeight = target_res_extent.height;

    m_target_to_present_scaled_copy_constants.data.srcWidth = target_res_extent.width;
    m_target_to_present_scaled_copy_constants.data.srcHeight = target_res_extent.height;
    m_target_to_present_scaled_copy_constants.data.dstWidth = present_res_extent.width;
    m_target_to_present_scaled_copy_constants.data.dstHeight = present_res_extent.height;

    UpdateCASConstants(internal_res_extent, target_res_extent, 1.0f,
        m_cas_upscaling_constants);

    UpdateCheckerboardConstants(internal_res_extent, m_cb_upscaling_constants);

    UpdateImageDeltaConstants();

    UpdateDebugConstants();
}

void VkexInfoApp::Render(vkex::Application::RenderData* p_data)
{
    const auto frame_index = p_data->GetFrameIndex();

    uint32_t cb_frame_index = frame_index % 2;
    m_per_frame_datas[frame_index].cb_frame_index = cb_frame_index;

    // add half pixel offset between frames, since half-pixel in
    // half-res equals full pixel in full-res 
    // 'odd' frames will be offset
    m_cb_viewport = vkex::BuildInvertedYViewport(m_internal_render_area);
    m_cb_viewport.x += 0.5f * cb_frame_index;

    // TODO: In the future, if we have some temporal technique (TAA or DRS),
    // we might want to think about using the Viewport to generate the jitter.
    // Otherwise, the jitter can go into the projection matrix.

    m_constant_buffer_manager.NewFrame(frame_index);

    ReadbackGpuTimestamps(frame_index);

    RenderInternalAndTarget(p_data->GetCommandBuffer(), frame_index);

    SubmitRender(p_data);
}

void VkexInfoApp::Present(vkex::Application::PresentData* p_data)
{
    // TODO: Continue progress for multi-frame overlap by allowing multiple Presents to be in flight.
    // Currently, multiple RenderDatas in flight is implemented, but synchronized to a single PresentData
    // for now. Basically, this means pushing m_frame_fence into PresentData, and re-factoring how
    // ProcessFrameFence works (perhaps pushing it down to pre-DispatchCallPresent, assuming
    // there aren't other data dependencies)

    auto cmd = p_data->GetCommandBuffer();

    auto present_render_pass = p_data->GetRenderPass();
    auto swapchain_image = present_render_pass->GetRtvs()[0]->GetResource()->GetImage();

    const auto frame_index = p_data->GetFrameIndex();
    auto& per_frame_data = m_per_frame_datas[frame_index];

    uint32_t scaled_constants_dynamic_offset = m_constant_buffer_manager.UploadConstantsToDynamicBuffer(m_target_to_present_scaled_copy_constants);

    {
        VkDescriptorImageInfo info = {};
        info.sampler = VK_NULL_HANDLE;
        info.imageView = *(present_render_pass->GetRtvs()[0]->GetResource());
        info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

        m_generated_shader_states[AppShaderList::TargetToPresentScaledCopy].descriptor_sets[frame_index]->UpdateDescriptors(2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0, 1, &info);

        // TODO: Add UpdateDescriptor helper for ImageViews?
    }

    cmd->Begin();

    {
        cmd->CmdTransitionImageLayout(m_visualization_texture,
            VK_IMAGE_LAYOUT_GENERAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

        cmd->CmdTransitionImageLayout(swapchain_image->GetVkObject(),
            swapchain_image->GetAspectFlags(),
            0, swapchain_image->GetMipLevels(),
            0, swapchain_image->GetArrayLayers(),
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            VK_IMAGE_LAYOUT_GENERAL,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

        cmd->CmdBindPipeline(m_generated_shader_states[AppShaderList::TargetToPresentScaledCopy].compute_pipeline);

        std::vector<uint32_t> dynamic_offsets = { scaled_constants_dynamic_offset };
        cmd->CmdBindDescriptorSets(VK_PIPELINE_BIND_POINT_COMPUTE,
            *(m_generated_shader_states[AppShaderList::TargetToPresentScaledCopy].pipeline_layout),
            0,
            { *(m_generated_shader_states[AppShaderList::TargetToPresentScaledCopy].descriptor_sets[frame_index]) },
            &dynamic_offsets);

        vkex::uint3 dispatchDims = CalculateSimpleDispatchDimensions(
            m_generated_shader_states[AppShaderList::TargetToPresentScaledCopy],
            GetPresentResolutionExtent());
        cmd->CmdDispatch(dispatchDims.x, dispatchDims.y, dispatchDims.z);

        cmd->CmdTransitionImageLayout(m_visualization_texture,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_IMAGE_LAYOUT_GENERAL,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

        cmd->CmdTransitionImageLayout(swapchain_image->GetVkObject(),
            swapchain_image->GetAspectFlags(),
            0, swapchain_image->GetMipLevels(),
            0, swapchain_image->GetArrayLayers(),
            VK_IMAGE_LAYOUT_GENERAL,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

        auto swapchain_render_pass = p_data->GetRenderPass();

        VkClearValue rtv_clear = {};
        VkClearValue dsv_clear = {};
        dsv_clear.depthStencil.depth = 1.0f;
        dsv_clear.depthStencil.stencil = 0xFF;
        std::vector<VkClearValue> clear_values = { rtv_clear, dsv_clear };

        cmd->CmdBeginRenderPass(swapchain_render_pass, &clear_values);
        cmd->CmdSetViewport(swapchain_render_pass->GetFullRenderArea());
        cmd->CmdSetScissor(swapchain_render_pass->GetFullRenderArea());

        this->DrawAppInfoGUI(frame_index);
        this->DrawImGui(cmd);

        cmd->CmdEndRenderPass();

        cmd->CmdTransitionImageLayout(swapchain_image->GetVkObject(),
            swapchain_image->GetAspectFlags(),
            0, swapchain_image->GetMipLevels(),
            0, swapchain_image->GetArrayLayers(),
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    }

    cmd->End();

    SubmitPresent(p_data);
}

int main(int argc, char** argv)
{
    VkexInfoApp app;
    vkex::Result vkex_result = app.Run(argc, argv);
    if (!vkex_result) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}