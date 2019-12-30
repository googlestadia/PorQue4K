/*
 Copyright 2019 Google Inc.
 
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

void VkexInfoApp::Configure(const vkex::ArgParser& args, vkex::Configuration& configuration)
{
    configuration.window.resizeable = false;

    configuration.swapchain.color_format = VK_FORMAT_B8G8R8A8_SRGB;
    configuration.swapchain.paced_frame_rate = 60;

    configuration.graphics_debug.enable = false;
    configuration.graphics_debug.message_severity.info = false;
    configuration.graphics_debug.message_severity.warning = false;
    configuration.graphics_debug.message_severity.error = false;
    configuration.graphics_debug.message_type.validation = false;
}

void VkexInfoApp::Setup()
{
    // Geometry data
    vkex::PlatonicSolid::Options cube_options = {};
    cube_options.vertex_colors = true;
    vkex::PlatonicSolid cube = vkex::PlatonicSolid::Cube(cube_options);
    const vkex::VertexBufferData* p_vertex_buffer_data = cube.GetVertexBufferByIndex(0);

    // Shader program
    {
        VKEX_CALL(asset_util::CreateShaderProgram(
            GetDevice(),
            GetAssetPath("shaders/draw_vertex.vs.spv"),
            GetAssetPath("shaders/draw_vertex.ps.spv"),
            &m_shader_program));
    }

    // Descriptor set layouts
    {
        const vkex::ShaderInterface& shader_interface = m_shader_program->GetInterface();
        vkex::DescriptorSetLayoutCreateInfo create_info = ToVkexCreateInfo(shader_interface.GetSet(0));
        VKEX_CALL(GetDevice()->CreateDescriptorSetLayout(create_info, &m_descriptor_set_layout));
    }

    // Descriptor pool
    {
        const vkex::ShaderInterface& shader_interface = m_shader_program->GetInterface();
        vkex::DescriptorPoolCreateInfo create_info = {};
        create_info.pool_sizes = shader_interface.GetDescriptorPoolSizes();
        VKEX_CALL(GetDevice()->CreateDescriptorPool(create_info, &m_descriptor_pool));
    }

    // Descriptor sets
    {
        vkex::DescriptorSetAllocateInfo allocate_info = {};
        allocate_info.layouts.push_back(m_descriptor_set_layout);
        VKEX_CALL(m_descriptor_pool->AllocateDescriptorSets(allocate_info, &m_descriptor_set));
    }

    // Pipeline layout
    {
        vkex::PipelineLayoutCreateInfo create_info = {};
        create_info.descriptor_set_layouts.push_back(vkex::ToVulkan(m_descriptor_set_layout));
        vkex::Result vkex_result = vkex::Result::Undefined;
        VKEX_CALL(GetDevice()->CreatePipelineLayout(create_info, &m_pipeline_layout));
    }

    // Pipeline
    {
        vkex::VertexBindingDescription vertex_binding_descriptions = p_vertex_buffer_data->GetVertexBindingDescription();

        vkex::GraphicsPipelineCreateInfo create_info = {};
        create_info.shader_program = m_shader_program;
        create_info.vertex_binding_descriptions = { vertex_binding_descriptions };
        create_info.samples = VK_SAMPLE_COUNT_1_BIT;
        create_info.depth_test_enable = true;
        create_info.depth_write_enable = true;
        create_info.pipeline_layout = m_pipeline_layout;
        create_info.rtv_formats = { GetConfiguration().swapchain.color_format };
        create_info.dsv_format = GetConfiguration().swapchain.depth_stencil_format;
        vkex::Result vkex_result = vkex::Result::Undefined;
        VKEX_CALL(GetDevice()->CreateGraphicsPipeline(create_info, &m_pipeline));
    }

    // Constant buffer
    {
        VKEX_CALL(asset_util::CreateConstantBuffer(
            m_view_transform_constants.size,
            nullptr,
            GetGraphicsQueue(),
            asset_util::MEMORY_USAGE_CPU_TO_GPU,
            &m_constant_buffer));
    }

    // Vertex buffer
    {
        VKEX_CALL(asset_util::CreateVertexBuffer(
            p_vertex_buffer_data->GetDataSize(),
            p_vertex_buffer_data->GetData(),
            GetGraphicsQueue(),
            asset_util::MEMORY_USAGE_GPU_ONLY,
            &m_vertex_buffer));
    }

    // Update descriptors
    {
        m_descriptor_set->UpdateDescriptor(0, m_constant_buffer);
    }

    // TODO: Create internal color image + view
    // TODO: Create internal depth image + view
    // TODO: Consider having full res internal image, and use 
    // scissor to restrict rendered area. Fixed internal size
    // is fine for now, but not really realistic

    {
        vkex::Result vkex_result = vkex::Result::Undefined;
        VKEX_CALL(CreateSimpleRenderPass(GetDevice(),
            GetConfiguration().window.width, GetConfiguration().window.height,
            GetConfiguration().swapchain.color_format,
            VK_FORMAT_D32_SFLOAT,
            &m_target_res_simple_render_pass));
    }

    {
        vkex::Result vkex_result = vkex::Result::Undefined;
        VKEX_CALL(vkex::TransitionImageLayout(GetGraphicsQueue(), 
            m_target_res_simple_render_pass.rtv_texture, 
            VK_IMAGE_LAYOUT_UNDEFINED, 
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT));

        vkex_result = vkex::Result::Undefined;
        VKEX_CALL(vkex::TransitionImageLayout(GetGraphicsQueue(),
            m_target_res_simple_render_pass.dsv_texture,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            (VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT)));
    }
}

void VkexInfoApp::Render(vkex::Application::RenderData* p_data)
{
    auto cmd = p_data->GetCommandBuffer();
    auto render_pass = m_target_res_simple_render_pass.render_pass;

    float3 eye = float3(0, 0, 2);
    float3 center = float3(0, 0, 0);
    float3 up = float3(0, 1, 0);
    float aspect = GetWindowAspect();
    vkex::PerspCamera camera(eye, center, up, 60.0f, aspect);

    float t = GetFrameStartTime();
    float4x4 M = glm::translate(float3(0, 0, 0)) * glm::rotate(t / 2.0f, float3(0, 1, 0)) * glm::rotate(t / 4.0f, float3(1, 0, 0));
    float4x4 V = camera.GetViewMatrix();
    float4x4 P = camera.GetProjectionMatrix();

    m_view_transform_constants.data.ModelViewProjectionMatrix = P * V*M;

    VKEX_CALL(m_constant_buffer->Copy(m_view_transform_constants.size, &m_view_transform_constants.data));

    VkClearValue rtv_clear = {};
    VkClearValue dsv_clear = {};
    dsv_clear.depthStencil.depth = 1.0f;
    dsv_clear.depthStencil.stencil = 0xFF;
    std::vector<VkClearValue> clear_values = { rtv_clear, dsv_clear };
    cmd->Begin();
    cmd->CmdBeginRenderPass(render_pass, &clear_values);
    cmd->CmdSetViewport(render_pass->GetFullRenderArea());
    cmd->CmdSetScissor(render_pass->GetFullRenderArea());
    cmd->CmdBindPipeline(m_pipeline);
    cmd->CmdBindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, *m_pipeline_layout, 0, { *m_descriptor_set });
    cmd->CmdBindVertexBuffers(m_vertex_buffer);
    cmd->CmdDraw(36, 1, 0, 0);

    // TODO: Ensure that GUI composited onto final swapchain target
    // TODO: Make sure this size is multiplied for current target resolution
    // TODO: Figure out how to change FontSize?

    ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_Once);
    this->DrawAppInfoGUI();
    this->DrawImGui(cmd);

    cmd->CmdEndRenderPass();

    cmd->End();

    SubmitRender(p_data);
}

void VkexInfoApp::Present(vkex::Application::PresentData* p_data)
{
    auto cmd = p_data->GetCommandBuffer();

    auto present_render_pass = p_data->GetRenderPass();
    auto swapchain_image = present_render_pass->GetRtvs()[0]->GetResource()->GetImage();
    auto source_image = m_target_res_simple_render_pass.rtv_texture->GetImage();
    
    cmd->Begin();

    // TODO: Build out helper function to merge these barriers into one
    cmd->CmdTransitionImageLayout(m_target_res_simple_render_pass.rtv_texture, 
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        VK_PIPELINE_STAGE_TRANSFER_BIT);

    cmd->CmdTransitionImageLayout(swapchain_image->GetVkObject(), 
        swapchain_image->GetAspectFlags(),
        0, swapchain_image->GetMipLevels(),
        0, swapchain_image->GetArrayLayers(),
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_PIPELINE_STAGE_TRANSFER_BIT);

    VkImageCopy image_copy_region;
    image_copy_region.srcSubresource.aspectMask = source_image->GetAspectFlags();
    image_copy_region.srcSubresource.mipLevel = 0;
    image_copy_region.srcSubresource.baseArrayLayer = 0;
    image_copy_region.srcSubresource.layerCount = 1;
    image_copy_region.srcOffset = { 0, 0, 0 };
    image_copy_region.dstSubresource.aspectMask = swapchain_image->GetAspectFlags();
    image_copy_region.dstSubresource.mipLevel = 0;
    image_copy_region.dstSubresource.baseArrayLayer = 0;
    image_copy_region.dstSubresource.layerCount = 1;
    image_copy_region.dstOffset = { 0, 0, 0 };
    image_copy_region.extent = source_image->GetExtent();

    // TODO: Port to resolution-independent copy? Probably dispatch is easiest
    cmd->CmdCopyImage(source_image->GetVkObject(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                      swapchain_image->GetVkObject(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                      1, &image_copy_region);

    cmd->CmdTransitionImageLayout(m_target_res_simple_render_pass.rtv_texture,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

    cmd->CmdTransitionImageLayout(swapchain_image->GetVkObject(),
        swapchain_image->GetAspectFlags(),
        0, swapchain_image->GetMipLevels(),
        0, swapchain_image->GetArrayLayers(),
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);


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