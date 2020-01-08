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

#include "vkex/Application.h"

#include "AssetUtil.h"

#include "AppCore.h"

// TODO: Simplify shader management
// * Move constant buffer structs to another file?
//   * Possibly share structs with shaders?
// * Shared header for stuff like TG dims

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

    configuration.graphics_debug.enable = false;
    configuration.graphics_debug.message_severity.info = false;
    configuration.graphics_debug.message_severity.warning = false;
    configuration.graphics_debug.message_severity.error = false;
    configuration.graphics_debug.message_type.validation = false;

    auto present_res_key = FindPresentResolutionKey(configuration.window.width);
    SetPresentResolution(present_res_key);
}

void VkexInfoApp::Setup()
{
    // Geometry data
    vkex::PlatonicSolid::Options cube_options = {};
    cube_options.vertex_colors = true;
    vkex::PlatonicSolid cube = vkex::PlatonicSolid::Cube(cube_options);
    const vkex::VertexBufferData* p_vertex_buffer_data = cube.GetVertexBufferByIndex(0);

    // Geometry draw renderpasses at different resolutions
    {
        // TODO: Maybe this should just get the Present resolution extent? I guess this is always
        // the same...
        auto target_res = GetTargetResolutionExtent();

        vkex::Result vkex_result = vkex::Result::Undefined;
        VKEX_CALL(CreateSimpleRenderPass(GetDevice(),
            target_res.width, target_res.height,
            GetConfiguration().swapchain.color_format,
            VK_FORMAT_D32_SFLOAT,
            &m_draw_simple_render_pass));
    }

    // Build pipelines + related state
    {
        // TODO: Move this shader/pipeline generation into another file where it's
        // simple + isolated to add in new shaders and related info

        // TODO: Descriptor set multipliers needed for multiple usage of pipelines!
        std::vector<ShaderProgramInputs> shader_inputs(AppShaderList::NumTypes);

        {
            shader_inputs[AppShaderList::Geometry].pipeline_type = ShaderPipelineType::Graphics;

            shader_inputs[AppShaderList::Geometry].shader_paths.resize(2);
            shader_inputs[AppShaderList::Geometry].shader_paths[0] = GetAssetPath("shaders/draw_vertex.vs.spv");
            shader_inputs[AppShaderList::Geometry].shader_paths[1] = GetAssetPath("shaders/draw_vertex.ps.spv");

            vkex::VertexBindingDescription vertex_binding_descriptions = p_vertex_buffer_data->GetVertexBindingDescription();

            vkex::GraphicsPipelineCreateInfo create_info = {};
            create_info.vertex_binding_descriptions = { vertex_binding_descriptions };
            create_info.samples = VK_SAMPLE_COUNT_1_BIT;
            create_info.depth_test_enable = true;
            create_info.depth_write_enable = true;
            create_info.rtv_formats = { m_draw_simple_render_pass.rtv->GetFormat() };
            create_info.dsv_format = m_draw_simple_render_pass.dsv->GetFormat();
            create_info.render_pass = m_draw_simple_render_pass.render_pass;
            shader_inputs[AppShaderList::Geometry].graphics_pipeline_create_info = create_info;
        }
        {
            shader_inputs[AppShaderList::ScaledTexCopy].pipeline_type = ShaderPipelineType::Compute;

            shader_inputs[AppShaderList::ScaledTexCopy].shader_paths.resize(1);
            shader_inputs[AppShaderList::ScaledTexCopy].shader_paths[0] = GetAssetPath("shaders/copy_texture.cs.spv");
        }

        SetupShaders(shader_inputs, m_generated_shader_states);
    }

    // Draw constants + vertex buffers
    {
        VKEX_CALL(asset_util::CreateVertexBuffer(
            p_vertex_buffer_data->GetDataSize(),
            p_vertex_buffer_data->GetData(),
            GetGraphicsQueue(),
            asset_util::MEMORY_USAGE_GPU_ONLY,
            &m_simple_draw_vertex_buffer));
    }
    {

        m_simple_draw_constant_buffers.resize(GetConfiguration().frame_count);
        for (auto& cb : m_simple_draw_constant_buffers) {
            VKEX_CALL(asset_util::CreateConstantBuffer(
                m_simple_draw_view_transform_constants.size,
                nullptr,
                GetGraphicsQueue(),
                asset_util::MEMORY_USAGE_CPU_TO_GPU,
                &cb));
        }
    }

    // Update draw descriptors
    {
        auto frame_count = GetConfiguration().frame_count;
        for (uint32_t frame_index = 0; frame_index < frame_count; frame_index++) {
            m_generated_shader_states[AppShaderList::Geometry].descriptor_sets[frame_index]->UpdateDescriptor(0, m_simple_draw_constant_buffers[frame_index]);
        }
    }

    // Scaled tex copy constants + descriptor updates
    {
        m_scaled_tex_copy_constant_buffers.resize(GetConfiguration().frame_count);

        for (auto& cb : m_scaled_tex_copy_constant_buffers) {
            VKEX_CALL(asset_util::CreateConstantBuffer(
                m_scaled_tex_copy_dims_constants.size,
                nullptr,
                GetGraphicsQueue(),
                asset_util::MEMORY_USAGE_CPU_TO_GPU,
                &cb));
        }
    }

    {
        auto internal_res_extent = GetInternalResolutionExtent();

        m_internal_width = internal_res_extent.width;
        m_internal_height = internal_res_extent.height;

        // TODO: Drive these from the internal renderpass? Or fixed options?
        m_internal_render_area = m_draw_simple_render_pass.render_pass->GetFullRenderArea();
        m_internal_render_area.extent.width = m_internal_width;
        m_internal_render_area.extent.height = m_internal_height;

        m_scaled_tex_copy_dims_constants.data.srcWidth = m_internal_width;
        m_scaled_tex_copy_dims_constants.data.srcHeight = m_internal_height;

        // TODO: Target res for now...
        auto target_res_extent = GetTargetResolutionExtent();
        m_scaled_tex_copy_dims_constants.data.dstWidth = target_res_extent.width;
        m_scaled_tex_copy_dims_constants.data.dstHeight = target_res_extent.height;
    }

    // TODO: Set up a copy from present target to swapchain

    {
        // TODO: It would be nice to have this grab the binding via the name instead of magically knowing
        // the binding here :p (TBH, all of that could be done offline as well, but whatever)

        auto frame_count = GetConfiguration().frame_count;
        for (uint32_t frame_index = 0; frame_index < frame_count; frame_index++) {
            m_generated_shader_states[AppShaderList::ScaledTexCopy].descriptor_sets[frame_index]->UpdateDescriptor(0, m_scaled_tex_copy_constant_buffers[frame_index]);
            m_generated_shader_states[AppShaderList::ScaledTexCopy].descriptor_sets[frame_index]->UpdateDescriptor(1, m_draw_simple_render_pass.rtv_texture);
        }
    }

    // TODO: Create separate internal + target images

    // Image transitions
    // TODO: When we create textures, can we specify an initial layout?
    // Like...do we really need to see this code here?
    {
        vkex::Result vkex_result = vkex::Result::Undefined;
        VKEX_CALL(vkex::TransitionImageLayout(GetGraphicsQueue(), 
            m_draw_simple_render_pass.rtv_texture,
            VK_IMAGE_LAYOUT_UNDEFINED, 
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT));

        vkex_result = vkex::Result::Undefined;
        VKEX_CALL(vkex::TransitionImageLayout(GetGraphicsQueue(),
            m_draw_simple_render_pass.dsv_texture,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            (VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT)));
    }
}

void VkexInfoApp::Update(double frame_elapsed_time)
{
    auto internal_res_extent = GetInternalResolutionExtent();
    auto target_res_extent = GetTargetResolutionExtent();

    m_internal_width = internal_res_extent.width;
    m_internal_height = internal_res_extent.height;

    m_internal_render_area.extent.width = m_internal_width;
    m_internal_render_area.extent.height = m_internal_height;

    m_scaled_tex_copy_dims_constants.data.srcWidth = m_internal_width;
    m_scaled_tex_copy_dims_constants.data.srcHeight = m_internal_height;
    m_scaled_tex_copy_dims_constants.data.dstWidth = target_res_extent.width;
    m_scaled_tex_copy_dims_constants.data.dstHeight = target_res_extent.height;

    // TODO: Update target + swapchain constants

    float3 eye = float3(0, 0, 2);
    float3 center = float3(0, 0, 0);
    float3 up = float3(0, 1, 0);
    float aspect = GetWindowAspect();
    vkex::PerspCamera camera(eye, center, up, 60.0f, aspect);

    float t = GetFrameStartTime();
    float4x4 M = glm::translate(float3(0, 0, 0)) * glm::rotate(t / 2.0f, float3(0, 1, 0)) * glm::rotate(t / 4.0f, float3(1, 0, 0));
    float4x4 V = camera.GetViewMatrix();
    float4x4 P = camera.GetProjectionMatrix();

    m_simple_draw_view_transform_constants.data.ModelViewProjectionMatrix = P * V*M;
}

void VkexInfoApp::Render(vkex::Application::RenderData* p_data)
{
    const auto frame_index = p_data->GetFrameIndex();
    VKEX_CALL(m_simple_draw_constant_buffers[frame_index]->Copy(m_simple_draw_view_transform_constants.size, &m_simple_draw_view_transform_constants.data));

    auto cmd = p_data->GetCommandBuffer();

    // TODO: Render to internal resolution and target resolutions
    // in order to have delta visualization
    auto render_pass = m_draw_simple_render_pass.render_pass;

    VkClearValue rtv_clear = {};
    VkClearValue dsv_clear = {};
    dsv_clear.depthStencil.depth = 1.0f;
    dsv_clear.depthStencil.stencil = 0xFF;
    std::vector<VkClearValue> clear_values = { rtv_clear, dsv_clear };
    cmd->Begin();
    cmd->CmdBeginRenderPass(render_pass, &clear_values);
    cmd->CmdSetViewport(m_internal_render_area);
    cmd->CmdSetScissor(m_internal_render_area);
    cmd->CmdBindPipeline(m_generated_shader_states[AppShaderList::Geometry].graphics_pipeline);
    cmd->CmdBindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, 
                               *(m_generated_shader_states[AppShaderList::Geometry].pipeline_layout),
                               0, 
                               { *(m_generated_shader_states[AppShaderList::Geometry].descriptor_sets[frame_index]) });
    cmd->CmdBindVertexBuffers(m_simple_draw_vertex_buffer);
    cmd->CmdDraw(36, 1, 0, 0);

    cmd->CmdEndRenderPass();

    // TODO: Perform upscale or visualization to 'final' target here
    // Final target will then be copied to swapchain in Present

    cmd->End();

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

    {
        auto& scaled_tex_copy_cb = m_scaled_tex_copy_constant_buffers[frame_index];
        VKEX_CALL(scaled_tex_copy_cb->Copy(
            m_scaled_tex_copy_dims_constants.size,
            &m_scaled_tex_copy_dims_constants.data));
        // Each per-frame descriptor set already points to the correct per-frame constant buffer
    }

    {
        VkDescriptorImageInfo info = {};
        info.sampler = VK_NULL_HANDLE;
        info.imageView = *(present_render_pass->GetRtvs()[0]->GetResource());
        info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

        m_generated_shader_states[AppShaderList::ScaledTexCopy].descriptor_sets[frame_index]->UpdateDescriptors(2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0, 1, &info);

        // TODO: Add UpdateDescriptor helper for ImageViews?
    }
    
    cmd->Begin();

    {
        cmd->CmdTransitionImageLayout(m_draw_simple_render_pass.rtv_texture,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

        cmd->CmdTransitionImageLayout(swapchain_image->GetVkObject(),
            swapchain_image->GetAspectFlags(),
            0, swapchain_image->GetMipLevels(),
            0, swapchain_image->GetArrayLayers(),
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            VK_IMAGE_LAYOUT_GENERAL,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

        cmd->CmdBindPipeline(m_generated_shader_states[AppShaderList::ScaledTexCopy].compute_pipeline);
        cmd->CmdBindDescriptorSets(VK_PIPELINE_BIND_POINT_COMPUTE, 
            *(m_generated_shader_states[AppShaderList::ScaledTexCopy].pipeline_layout),
            0, 
            { *(m_generated_shader_states[AppShaderList::ScaledTexCopy].descriptor_sets[frame_index]) });
        
        // TODO: Select between visualizations or draws
        // TODO: Perhaps we should do the upscale or visualization to another internal
        //       full-res texture, and then do the copy from that to the swapchain.
        //       The reason being that the descriptor set complexity is greatly
        //       simplified if we don't have to worry about the swapchain images
        //       for all dispatches.

        // TODO: Automate (image size / thread group size)
        vkex::uint3 dispatchDims = { 120, 68, 1 };
        cmd->CmdDispatch(dispatchDims.x, dispatchDims.y, dispatchDims.z);

        cmd->CmdTransitionImageLayout(m_draw_simple_render_pass.rtv_texture,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

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
        
        // TODO: Make sure this size is multiplied for current target resolution
        // TODO: Figure out how to change FontSize?

        ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_Once);
        this->DrawAppInfoGUI();
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
    // TODO: Drive resolution from input
    VkexInfoApp app;
    vkex::Result vkex_result = app.Run(argc, argv);
    if (!vkex_result) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}