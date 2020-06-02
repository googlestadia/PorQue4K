#include "AppCore.h"

void VkexInfoApp::UpdateCheckerboardConstants(const VkExtent2D& srcExtent,
                                              CBUpscalingConstants& constants) {
  constants.data.srcWidth = srcExtent.width;
  constants.data.srcHeight = srcExtent.height;

  // checkerboard mode state
  if (!m_sample_locations_enabled) {
    m_checkerboard_samples_mode = CheckerboardSampleMode::kViewportJitter;
  }

  constants.data.sampleGenMode = static_cast<uint>(m_checkerboard_samples_mode);
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
}

void VkexInfoApp::CheckerboardUpscale(vkex::CommandBuffer cmd,
                                      uint32_t frame_index) {
  auto& per_frame_data = m_per_frame_datas[frame_index];

  uint32_t cb_frame_index = per_frame_data.cb_frame_index;
  uint32_t other_cb_index = cb_frame_index ^ 1;

  const uint32_t kLeftCBIndex = 0;
  const uint32_t kRightCBIndex = 1;

  m_generated_shader_states[AppShaderList::CheckerboardUpscale]
      .descriptor_sets[frame_index]
      ->UpdateDescriptor(
          1, m_checkerboard_simple_render_pass[kLeftCBIndex].color_texture);
  m_generated_shader_states[AppShaderList::CheckerboardUpscale]
      .descriptor_sets[frame_index]
      ->UpdateDescriptor(
          2, m_checkerboard_simple_render_pass[kRightCBIndex].color_texture);

  cmd->CmdTransitionImageLayout(
      m_checkerboard_simple_render_pass[cb_frame_index].color_texture,
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
  cmd->CmdTransitionImageLayout(
      m_checkerboard_simple_render_pass[other_cb_index].color_texture,
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
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

  cmd->CmdBindPipeline(
      m_generated_shader_states[AppShaderList::CheckerboardUpscale]
          .compute_pipeline);

  std::vector<uint32_t> dynamic_offsets = {
      checkerboard_constants_dynamic_offset};
  cmd->CmdBindDescriptorSets(
      VK_PIPELINE_BIND_POINT_COMPUTE,
      *(m_generated_shader_states[AppShaderList::CheckerboardUpscale]
            .pipeline_layout),
      0,
      {*(m_generated_shader_states[AppShaderList::CheckerboardUpscale]
             .descriptor_sets[frame_index])},
      &dynamic_offsets);

  IssueGpuTimeStart(cmd, per_frame_data, TimerTag::kUpscaleInternal);
  {
    vkex::uint3 dispatchDims = CalculateSimpleDispatchDimensions(
        m_generated_shader_states[AppShaderList::CheckerboardUpscale],
        GetInternalResolutionExtent());
    cmd->CmdDispatch(dispatchDims.x, dispatchDims.y, dispatchDims.z);
  }
  IssueGpuTimeEnd(cmd, per_frame_data, TimerTag::kUpscaleInternal);

  cmd->CmdTransitionImageLayout(
      m_checkerboard_simple_render_pass[cb_frame_index].color_texture,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
  cmd->CmdTransitionImageLayout(
      m_checkerboard_simple_render_pass[other_cb_index].color_texture,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
}