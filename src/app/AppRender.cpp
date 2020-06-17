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

void VkexInfoApp::RenderInternalAndTarget(vkex::CommandBuffer cmd,
                                          uint32_t frame_index) {
  auto& per_frame_data = m_per_frame_datas[frame_index];

  auto per_frame_dynamic_offset =
      m_constant_buffer_manager.UploadConstantsToDynamicBuffer(
          m_per_frame_constants);
  auto per_object_dynamic_offset =
      m_constant_buffer_manager.UploadConstantsToDynamicBuffer(
          m_per_object_constants);

  cmd->Begin();

  {
    cmd->CmdResetQueryPool(per_frame_data.timer_query_pool, 0,
                           TimerTag::kTimerQueryCount);
    per_frame_data.timestamps_issued = true;
  }

  IssueGpuTimeStart(cmd, per_frame_data, TimerTag::kTotalInternal);
  {
    IssueGpuTimeStart(cmd, per_frame_data, TimerTag::kSceneRenderInternal);
    {
      vkex::RenderPass render_pass;
      GeneratedShaderState* pipeline;
      VkViewport viewport;
      void* render_pass_begin_pNext = nullptr;

      switch (GetUpscalingTechnique()) {
        case UpscalingTechniqueKey::kuNone:
        case UpscalingTechniqueKey::CAS: {
          render_pass = m_internal_draw_simple_render_pass.render_pass;
          pipeline = &m_generated_shader_states[AppShaderList::Geometry];
          viewport = vkex::BuildInvertedYViewport(m_internal_render_area);
          break;
        }
        case UpscalingTechniqueKey::Checkerboard: {
          render_pass =
              m_checkerboard_simple_render_pass[per_frame_data.cb_frame_index]
                  .render_pass;
          pipeline = &m_generated_shader_states[AppShaderList::GeometryCB];
          viewport = m_cb_viewport;
          if (m_sample_locations_enabled) {
            // Technically, we don't need this on platforms that support
            // variableSampleLocations and our particular management
            // of depth attachments, but it doesn't _hurt_
            render_pass_begin_pNext =
                static_cast<void*>(&m_rp_sample_locations);

            // The spec is unclear whether this state is latched, or attaches
            // to the previously bound pipeline. But same with the other
            // dynamic state so...
            vkex::CmdSetSampleLocationsEXT(cmd->GetVkObject(),
                                           &m_current_sample_locations_info);
          }
          break;
        }
        default:
          VKEX_LOG_ERROR(
              "Internal resolution render failure due to unknown upscaling "
              "technique.");
          break;
      }

      VkClearValue rtv_clear = {};
      VkClearValue dsv_clear = {};
      dsv_clear.depthStencil.depth = 1.0f;
      dsv_clear.depthStencil.stencil = 0xFF;
      std::vector<VkClearValue> clear_values = {rtv_clear, rtv_clear,
                                                dsv_clear};
      cmd->CmdBeginRenderPass(render_pass, &clear_values,
                              VK_SUBPASS_CONTENTS_INLINE,
                              render_pass_begin_pNext);

      std::vector<VkViewport> vps = {viewport};
      cmd->CmdSetViewport(0, &vps);

      cmd->CmdSetScissor(m_internal_render_area);

      cmd->CmdBindPipeline(pipeline->graphics_pipeline);

      std::vector<uint32_t> dynamic_offsets = {per_frame_dynamic_offset,
                                               per_object_dynamic_offset};
      cmd->CmdBindDescriptorSets(
          VK_PIPELINE_BIND_POINT_GRAPHICS, *(pipeline->pipeline_layout), 0,
          {*(pipeline->descriptor_sets[frame_index])}, &dynamic_offsets);

      DrawModel(cmd);

      cmd->CmdEndRenderPass();
    }
    IssueGpuTimeEnd(cmd, per_frame_data, TimerTag::kSceneRenderInternal);

    UpscaleInternalToTarget(cmd, frame_index);
  }
  IssueGpuTimeEnd(cmd, per_frame_data, TimerTag::kTotalInternal);

  RenderSceneTargetResolution(cmd, frame_index);
  VisualizeInternalTargetDelta(cmd, frame_index);

  cmd->End();
}

void VkexInfoApp::NaiveUpscale(vkex::CommandBuffer cmd, uint32_t frame_index) {
  auto& per_frame_data = m_per_frame_datas[frame_index];

  auto& naive_upscale_shader_state =
      m_generated_shader_states[AppShaderList::InternalToTargetScaledCopy];
  auto& naive_upscale_descriptor_set =
      naive_upscale_shader_state
          .descriptor_sets[frame_index];

  naive_upscale_descriptor_set->UpdateDescriptor(2, m_current_target_texture);

  auto scaled_copy_dynamic_offset =
      m_constant_buffer_manager.UploadConstantsToDynamicBuffer(
          m_internal_to_target_scaled_copy_constants);

  cmd->CmdBindPipeline(naive_upscale_shader_state.compute_pipeline);

  std::vector<uint32_t> dynamic_offsets = {scaled_copy_dynamic_offset};
  cmd->CmdBindDescriptorSets(
      VK_PIPELINE_BIND_POINT_COMPUTE,
      *(naive_upscale_shader_state.pipeline_layout),
      0, {*(naive_upscale_descriptor_set)},
      &dynamic_offsets);

  IssueGpuTimeStart(cmd, per_frame_data, TimerTag::kUpscaleInternal);
  {
    vkex::uint3 dispatchDims = CalculateSimpleDispatchDimensions(
        naive_upscale_shader_state, GetTargetResolutionExtent());
    cmd->CmdDispatch(dispatchDims.x, dispatchDims.y, dispatchDims.z);
  }
  IssueGpuTimeEnd(cmd, per_frame_data, TimerTag::kUpscaleInternal);
}

void VkexInfoApp::UpscaleInternalToTarget(vkex::CommandBuffer cmd,
                                          uint32_t frame_index) {
  auto& per_frame_data = m_per_frame_datas[frame_index];

  // TODO: These barriers are bogus for the checkerboard upscale...
  cmd->CmdTransitionImageLayout(
      m_internal_draw_simple_render_pass.color_texture,
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

  switch (GetUpscalingTechnique()) {
    case UpscalingTechniqueKey::kuNone: {
      NaiveUpscale(cmd, frame_index);
      break;
    }
    case UpscalingTechniqueKey::CAS: {
      CASUpscale(cmd, frame_index);
      break;
    }
    case UpscalingTechniqueKey::Checkerboard: {
      CheckerboardUpscale(cmd, frame_index);
      break;
    }
    default:
      VKEX_LOG_ERROR("Upscaling failure due to unknown upscaling technique.");
      break;
  }

  // TODO: These barriers are bogus for the checkerboard upscale...
  cmd->CmdTransitionImageLayout(
      m_internal_draw_simple_render_pass.color_texture,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
}

void VkexInfoApp::VisualizeInternalTargetDelta(vkex::CommandBuffer cmd,
                                               uint32_t frame_index) {
  auto& per_frame_data = m_per_frame_datas[frame_index];

  auto scaled_copy_dynamic_offset =
      m_constant_buffer_manager.UploadConstantsToDynamicBuffer(
          m_internal_to_target_scaled_copy_constants);
  auto image_delta_dynamic_offset =
      m_constant_buffer_manager.UploadConstantsToDynamicBuffer(
          m_image_delta_options_constants);

  cmd->CmdTransitionImageLayout(m_current_target_texture, VK_IMAGE_LAYOUT_GENERAL,
                                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
  cmd->CmdTransitionImageLayout(
      m_internal_as_target_draw_simple_render_pass.color_texture,
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

  auto& delta_shader_state =
      m_generated_shader_states[AppShaderList::InternalTargetImageDelta];

  delta_shader_state.descriptor_sets[frame_index]
      ->UpdateDescriptor(2, m_current_target_texture);

  cmd->CmdBindPipeline(delta_shader_state.compute_pipeline);

  std::vector<uint32_t> dynamic_offsets = {scaled_copy_dynamic_offset,
                                           image_delta_dynamic_offset};
  cmd->CmdBindDescriptorSets(
      VK_PIPELINE_BIND_POINT_COMPUTE, *(delta_shader_state.pipeline_layout),
      0,
      {*(delta_shader_state.descriptor_sets[frame_index])},
      &dynamic_offsets);

  vkex::uint3 dispatchDims = CalculateSimpleDispatchDimensions(
      delta_shader_state, GetTargetResolutionExtent());
  cmd->CmdDispatch(dispatchDims.x, dispatchDims.y, dispatchDims.z);

  cmd->CmdTransitionImageLayout(
      m_current_target_texture, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
  cmd->CmdTransitionImageLayout(
      m_internal_as_target_draw_simple_render_pass.color_texture,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
}

void VkexInfoApp::RenderSceneTargetResolution(vkex::CommandBuffer cmd,
                                              uint32_t frame_index) {
  auto& per_frame_data = m_per_frame_datas[frame_index];

  auto per_frame_dynamic_offset =
      m_constant_buffer_manager.UploadConstantsToDynamicBuffer(
          m_per_frame_constants);
  auto per_object_dynamic_offset =
      m_constant_buffer_manager.UploadConstantsToDynamicBuffer(
          m_per_object_constants);

  IssueGpuTimeStart(cmd, per_frame_data, TimerTag::kSceneRenderTarget);
  {
    auto render_pass = m_internal_as_target_draw_simple_render_pass.render_pass;
    const auto& scene_shader_state =
        m_generated_shader_states[AppShaderList::Geometry];

    VkClearValue rtv_clear = {};
    VkClearValue dsv_clear = {};
    dsv_clear.depthStencil.depth = 1.0f;
    dsv_clear.depthStencil.stencil = 0xFF;
    std::vector<VkClearValue> clear_values = {rtv_clear, rtv_clear, dsv_clear};
    cmd->CmdBeginRenderPass(render_pass, &clear_values);
    cmd->CmdSetViewport(m_target_render_area);
    cmd->CmdSetScissor(m_target_render_area);
    cmd->CmdBindPipeline(scene_shader_state.graphics_pipeline);

    std::vector<uint32_t> dynamic_offsets = {per_frame_dynamic_offset,
                                             per_object_dynamic_offset};
    cmd->CmdBindDescriptorSets(
        VK_PIPELINE_BIND_POINT_GRAPHICS, *(scene_shader_state.pipeline_layout),
        0, {*(scene_shader_state.descriptor_sets[frame_index])},
        &dynamic_offsets);

    DrawModel(cmd);

    cmd->CmdEndRenderPass();
  }
  IssueGpuTimeEnd(cmd, per_frame_data, TimerTag::kSceneRenderTarget);
}

void VkexInfoApp::DrawModel(vkex::CommandBuffer cmd) {
  // TODO: In the future, we'll want to formally loop over multiple
  // nodes/meshes/prims
  // TODO: Once we toggle models, we'll need to bind pre-generated descriptor
  //       sets, or build the constant buffers/descriptors live

  uint32_t node_index = 0;
  uint32_t primitive_index = 0;

  auto index_buffer =
      m_helmet_model.GetIndexBuffer(node_index, primitive_index);
  auto index_type = m_helmet_model.GetIndexType(node_index, primitive_index);

  cmd->CmdBindIndexBuffer(index_buffer, 0, index_type);

  const VkDeviceSize offsets[] = {0, 0, 0};
  std::vector<VkBuffer> vertex_buffers;
  m_helmet_model.GetVertexBuffers(node_index, primitive_index, vertex_buffers);
  cmd->CmdBindVertexBuffers(0, uint32_t(vertex_buffers.size()),
                            vertex_buffers.data(), offsets);

  auto index_count = m_helmet_model.GetIndexCount(node_index, primitive_index);
  cmd->CmdDrawIndexed(index_count, 1, 0, 0, 0);
}
