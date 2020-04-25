#include "AppCore.h"

void VkexInfoApp::UpdateCheckerboardConstants(const VkExtent2D& srcExtent,
                                              CBUpscalingConstants& constants) {
  constants.data.srcWidth = srcExtent.width;
  constants.data.srcHeight = srcExtent.height;
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
          1, m_checkerboard_simple_render_pass[kLeftCBIndex].rtv_texture);
  m_generated_shader_states[AppShaderList::CheckerboardUpscale]
      .descriptor_sets[frame_index]
      ->UpdateDescriptor(
          2, m_checkerboard_simple_render_pass[kRightCBIndex].rtv_texture);

  cmd->CmdTransitionImageLayout(
      m_checkerboard_simple_render_pass[cb_frame_index].rtv_texture,
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
  cmd->CmdTransitionImageLayout(
      m_checkerboard_simple_render_pass[other_cb_index].rtv_texture,
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);

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
      m_checkerboard_simple_render_pass[cb_frame_index].rtv_texture,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
  cmd->CmdTransitionImageLayout(
      m_checkerboard_simple_render_pass[other_cb_index].rtv_texture,
      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
}