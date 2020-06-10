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

void VkexInfoApp::UpdateCheckerboardConstants(const VkExtent2D& srcExtent,
                                              CBUpscalingConstants& constants) {
  constants.data.srcWidth = srcExtent.width;
  constants.data.srcHeight = srcExtent.height;

  // Fixing to this mode for now
  m_checkerboard_samples_mode = CheckerboardSampleMode::kCustomSampleLocs;
  if (!m_sample_locations_enabled) {
    m_checkerboard_samples_mode = CheckerboardSampleMode::kViewportJitter;
  }
}

void VkexInfoApp::UpdateCheckerboardRenderState(uint32_t cb_frame_index) {
  m_cb_viewport = vkex::BuildInvertedYViewport(m_internal_render_area);

  if (m_checkerboard_samples_mode ==
      CheckerboardSampleMode::kCustomSampleLocs) {
    if (cb_frame_index == 0) {
      m_current_sample_locations[0] = {0.75, 0.75};
      m_current_sample_locations[1] = {0.25, 0.25};
    } else {
      // Toggle sample positions horizontally, preserving their vertical
      // positioning.
      m_current_sample_locations[0] = {0.25, 0.75};
      m_current_sample_locations[1] = {0.75, 0.25};
    }
  } else {
    // If we don't toggle sample locations, we need to offset
    // the viewport.
    // Add half pixel offset between frames, since half-pixel in
    // half-res equals full pixel in full-res.
    // 'odd' frames will be offset
    m_cb_viewport.x += 0.5f * cb_frame_index;

    if (m_sample_locations_enabled) {
      m_current_sample_locations[0] = {0.75, 0.75};
      m_current_sample_locations[1] = {0.25, 0.25};
    }
  }

   m_cb_upscaling_constants.data.cbIndex = cb_frame_index;
}

void VkexInfoApp::CheckerboardUpscale(vkex::CommandBuffer cmd,
                                      uint32_t frame_index) {
  auto& per_frame_data = m_per_frame_datas[frame_index];

  const uint32_t cb_frame_index = per_frame_data.cb_frame_index;

  auto& cb_shader_state =
      m_generated_shader_states[AppShaderList::CheckerboardUpscale];
  auto& cb_render_pass =
      m_checkerboard_simple_render_pass[cb_frame_index];

  cb_shader_state.descriptor_sets[frame_index]->UpdateDescriptor(
      1, cb_render_pass.color_texture);
  cb_shader_state.descriptor_sets[frame_index]->UpdateDescriptor(
          2, cb_render_pass.velocity_texture);
  cb_shader_state.descriptor_sets[frame_index]
      ->UpdateDescriptor(3, m_previous_target_texture);
  cb_shader_state.descriptor_sets[frame_index]
      ->UpdateDescriptor(4, m_current_target_texture);

  cmd->CmdTransitionImageLayout(cb_render_pass.color_texture,
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
  cmd->CmdTransitionImageLayout(cb_render_pass.velocity_texture,
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
  cmd->CmdTransitionImageLayout(m_previous_target_texture,
                                VK_IMAGE_LAYOUT_GENERAL,
                                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

  // TODO: In the future, we'll use depth in the custom resolve.
  // If we're using custom sample locations, we have to
  // make sure we include VkSampleLocationsInfoEXT when we transition
  // our depth attachments into shader_read layout. The reason is
  // that the underlying implementation might have to decompress
  // the depth resource, and it can't decompress unless it knows
  // the sample locations (which might not be the 'standard' locations).
  // https://vulkan.lunarg.com/doc/view/1.2.135.0/windows/chunked_spec/chap6.html#synchronization-image-memory-barriers

  auto checkerboard_constants_dynamic_offset =
      m_constant_buffer_manager.UploadConstantsToDynamicBuffer(
          m_cb_upscaling_constants);

  cmd->CmdBindPipeline(cb_shader_state.compute_pipeline);

  std::vector<uint32_t> dynamic_offsets = {
      checkerboard_constants_dynamic_offset};
  cmd->CmdBindDescriptorSets(
      VK_PIPELINE_BIND_POINT_COMPUTE, *(cb_shader_state.pipeline_layout),
      0,
      {*(cb_shader_state.descriptor_sets[frame_index])},
      &dynamic_offsets);

  IssueGpuTimeStart(cmd, per_frame_data, TimerTag::kUpscaleInternal);
  {
    vkex::uint3 dispatchDims = CalculateSimpleDispatchDimensions(
        cb_shader_state,
        GetInternalResolutionExtent());
    cmd->CmdDispatch(dispatchDims.x, dispatchDims.y, dispatchDims.z);
  }
  IssueGpuTimeEnd(cmd, per_frame_data, TimerTag::kUpscaleInternal);

  cmd->CmdTransitionImageLayout(cb_render_pass.color_texture,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
  cmd->CmdTransitionImageLayout(cb_render_pass.velocity_texture,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
  cmd->CmdTransitionImageLayout(
      m_previous_target_texture, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
}