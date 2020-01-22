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

void VkexInfoApp::RenderInternalAndTarget(vkex::CommandBuffer cmd, uint32_t frame_index)
{
    // TODO: These constant buffer updates probably happen in-line with the renders
    // in the future
    VKEX_CALL(m_simple_draw_constant_buffers[frame_index]->Copy(m_simple_draw_view_transform_constants.size, &m_simple_draw_view_transform_constants.data));

    {
        auto& scaled_tex_copy_cb = m_internal_to_target_scaled_copy_constant_buffers[frame_index];
        VKEX_CALL(scaled_tex_copy_cb->Copy(
            m_internal_to_target_scaled_copy_constants.size,
            &m_internal_to_target_scaled_copy_constants.data));
    }

    auto& per_frame_data = m_per_frame_datas[frame_index];

    cmd->Begin();

    {
        cmd->CmdResetQueryPool(per_frame_data.timer_query_pool, 0, TimerTag::kTimerQueryCount);
        per_frame_data.timestamps_issued = true;
    }

    IssueGpuTimeStart(cmd, per_frame_data, TimerTag::kTotalInternal);
    {

        IssueGpuTimeStart(cmd, per_frame_data, TimerTag::kSceneRenderInternal);
        {

            auto render_pass = m_internal_draw_simple_render_pass.render_pass;

            VkClearValue rtv_clear = {};
            VkClearValue dsv_clear = {};
            dsv_clear.depthStencil.depth = 1.0f;
            dsv_clear.depthStencil.stencil = 0xFF;
            std::vector<VkClearValue> clear_values = { rtv_clear, dsv_clear };
            cmd->CmdBeginRenderPass(render_pass, &clear_values);

            cmd->CmdSetViewport(m_internal_render_area);
            cmd->CmdSetScissor(m_internal_render_area);

            cmd->CmdBindPipeline(m_generated_shader_states[AppShaderList::Geometry].graphics_pipeline);
            cmd->CmdBindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS,
                *(m_generated_shader_states[AppShaderList::Geometry].pipeline_layout),
                0,
                { *(m_generated_shader_states[AppShaderList::Geometry].descriptor_sets[frame_index]) });

            DrawModel(cmd);

            cmd->CmdEndRenderPass();
        }
        IssueGpuTimeEnd(cmd, per_frame_data, TimerTag::kSceneRenderInternal);

        UpscaleInternalToTarget(cmd, frame_index);
    }
    IssueGpuTimeEnd(cmd, per_frame_data, TimerTag::kTotalInternal);

    VisualizeInternalTargetDelta(cmd, frame_index);

    cmd->End();
}

void VkexInfoApp::UpscaleInternalToTarget(vkex::CommandBuffer cmd, uint32_t frame_index)
{
    auto& per_frame_data = m_per_frame_datas[frame_index];

    cmd->CmdTransitionImageLayout(m_internal_draw_simple_render_pass.rtv_texture,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

    cmd->CmdBindPipeline(m_generated_shader_states[AppShaderList::InternalToTargetScaledCopy].compute_pipeline);
    cmd->CmdBindDescriptorSets(VK_PIPELINE_BIND_POINT_COMPUTE,
        *(m_generated_shader_states[AppShaderList::InternalToTargetScaledCopy].pipeline_layout),
        0,
        { *(m_generated_shader_states[AppShaderList::InternalToTargetScaledCopy].descriptor_sets[frame_index]) });

    IssueGpuTimeStart(cmd, per_frame_data, TimerTag::kUpscaleInternal);
    {
        vkex::uint3 dispatchDims = CalculateSimpleDispatchDimensions(
            m_generated_shader_states[AppShaderList::InternalToTargetScaledCopy],
            GetTargetResolutionExtent());
        cmd->CmdDispatch(dispatchDims.x, dispatchDims.y, dispatchDims.z);
    }
    IssueGpuTimeEnd(cmd, per_frame_data, TimerTag::kUpscaleInternal);

    cmd->CmdTransitionImageLayout(m_internal_draw_simple_render_pass.rtv_texture,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
}

void VkexInfoApp::VisualizeInternalTargetDelta(vkex::CommandBuffer cmd, uint32_t frame_index)
{
    auto& per_frame_data = m_per_frame_datas[frame_index];

    {
        auto& image_delta_options_cb = m_image_delta_options_constant_buffers[frame_index];
        VKEX_CALL(image_delta_options_cb->Copy(
            m_image_delta_options_constants.size,
            &m_image_delta_options_constants.data));
    }

    // Target resolution draw
    IssueGpuTimeStart(cmd, per_frame_data, TimerTag::kSceneRenderTarget);
    {

        auto render_pass = m_internal_as_target_draw_simple_render_pass.render_pass;

        VkClearValue rtv_clear = {};
        VkClearValue dsv_clear = {};
        dsv_clear.depthStencil.depth = 1.0f;
        dsv_clear.depthStencil.stencil = 0xFF;
        std::vector<VkClearValue> clear_values = { rtv_clear, dsv_clear };
        cmd->CmdBeginRenderPass(render_pass, &clear_values);
        cmd->CmdSetViewport(m_target_render_area);
        cmd->CmdSetScissor(m_target_render_area);
        cmd->CmdBindPipeline(m_generated_shader_states[AppShaderList::Geometry].graphics_pipeline);
        cmd->CmdBindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS,
            *(m_generated_shader_states[AppShaderList::Geometry].pipeline_layout),
            0,
            { *(m_generated_shader_states[AppShaderList::Geometry].descriptor_sets[frame_index]) });

        DrawModel(cmd);

        cmd->CmdEndRenderPass();
    }
    IssueGpuTimeEnd(cmd, per_frame_data, TimerTag::kSceneRenderTarget);

    // Run delta visualizer
    cmd->CmdTransitionImageLayout(m_target_texture,
        VK_IMAGE_LAYOUT_GENERAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
    cmd->CmdTransitionImageLayout(m_internal_as_target_draw_simple_render_pass.rtv_texture,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

    cmd->CmdBindPipeline(m_generated_shader_states[AppShaderList::InternalTargetImageDelta].compute_pipeline);
    cmd->CmdBindDescriptorSets(VK_PIPELINE_BIND_POINT_COMPUTE,
        *(m_generated_shader_states[AppShaderList::InternalTargetImageDelta].pipeline_layout),
        0,
        { *(m_generated_shader_states[AppShaderList::InternalTargetImageDelta].descriptor_sets[frame_index]) });

    vkex::uint3 dispatchDims = CalculateSimpleDispatchDimensions(
        m_generated_shader_states[AppShaderList::InternalTargetImageDelta],
        GetTargetResolutionExtent());
    cmd->CmdDispatch(dispatchDims.x, dispatchDims.y, dispatchDims.z);

    cmd->CmdTransitionImageLayout(m_target_texture,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_GENERAL,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    cmd->CmdTransitionImageLayout(m_internal_as_target_draw_simple_render_pass.rtv_texture,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
}

void VkexInfoApp::DrawModel(vkex::CommandBuffer cmd)
{
    // TODO: In the future, we'll want to formally loop over multiple nodes/meshes/prims
    // TODO: Once we toggle models, we'll need to bind pre-generated descriptor
    //       sets, or build the constant buffers/descriptors live

    uint32_t node_index = 0;
    uint32_t primitive_index = 0;

    auto index_buffer = m_helmet_model.GetIndexBuffer(node_index, primitive_index);
    auto index_type = m_helmet_model.GetIndexType(node_index, primitive_index);

    cmd->CmdBindIndexBuffer(index_buffer, 0, index_type);

    const VkDeviceSize offsets[] = { 0, 0, 0 };
    std::vector<VkBuffer> vertex_buffers;
    m_helmet_model.GetVertexBuffers(node_index, primitive_index, vertex_buffers);
    cmd->CmdBindVertexBuffers(0, uint32_t(vertex_buffers.size()), vertex_buffers.data(), offsets);

    auto index_count = m_helmet_model.GetIndexCount(node_index, primitive_index);
    cmd->CmdDrawIndexed(index_count, 1, 0, 0, 0);
}