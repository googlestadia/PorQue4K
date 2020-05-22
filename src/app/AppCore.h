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

#ifndef __APP_CORE_H__
#define __APP_CORE_H__

#include "vkex/Application.h"

#include "ConstantBufferManager.h"
#include "GLTFModel.h"
#include "SharedShaderConstants.h"
#include "SimpleRenderPass.h"

using float2 = vkex::float2;
using float3 = vkex::float3;
using float4 = vkex::float4;

using float2x2 = vkex::float2x2;
using float3x3 = vkex::float3x3;
using float4x4 = vkex::float4x4;

using uint = vkex::uint;
using uint2 = vkex::uint2;
using uint4 = vkex::uint4;

// Placed here to take advantage of the above 'using' directives
#include "ConstantBufferStructs.h"

using PerFrameConstants = vkex::ConstantBufferData<PerFrameConstantData>;
using PerObjectConstants = vkex::ConstantBufferData<PerObjectConstantData>;
using ScaledTexCopyDimsConstants =
    vkex::ConstantBufferData<ScaledTexCopyDimensionsData>;
using ImageDeltaOptionsConstants = vkex::ConstantBufferData<ImageDeltaOptions>;
using CASUpscalingConstants = vkex::ConstantBufferData<CASData>;
using CBUpscalingConstants = vkex::ConstantBufferData<CBResolveData>;

enum ShaderPipelineType {
  Graphics = 0,
  Compute = 1,
};

enum AppShaderList {
  Geometry = 0,
  InternalToTargetScaledCopy = 1,
  InternalTargetImageDelta = 2,
  TargetToPresentScaledCopy = 3,
  UpscalingCAS = 4,
  CheckerboardUpscale = 5,
  GeometryCB = 6,
  NumTypes,
};

struct ShaderProgramInputs {
  ShaderPipelineType pipeline_type;
  std::vector<vkex::fs::path> shader_paths;
  vkex::GraphicsPipelineCreateInfo graphics_pipeline_create_info;
};

// TODO: There's a chance we might have to tease apart the allocated
// descriptors from the shader, but...probably not

struct GeneratedShaderState {
  ShaderPipelineType pipeline_type;
  vkex::ShaderProgram program = nullptr;
  vkex::DescriptorSetLayout descriptor_set_layout = nullptr;
  vkex::PipelineLayout pipeline_layout = nullptr;
  union {
    vkex::GraphicsPipeline graphics_pipeline = nullptr;
    vkex::ComputePipeline compute_pipeline;
  };
  std::vector<vkex::DescriptorSet> descriptor_sets;
};

enum LightType {
  kDirectional = 0,
  kLightTypeCount,
};

struct CPULightInfo {
  LightType lightType;

  float3 direction;
  float3 color;
  float intensity;

  // TODO: Spotlight + Point light data
  // TODO: Sync with GPULightInfo
};

// TODO: Rename these enums into something more sane...

enum UpscalingTechniqueKey {
  None = 0,
  CAS = 1,
  Checkerboard = 2,
  kuCount,
};

enum ResolutionInfoKey {
  kr540p = 0,
  kr720p = 1,
  kr1080p = 2,
  kr1440p = 3,
  kr2160p = 4,
  krCount,
};

enum CBResolutionInfoKey {
  kcb1080CB = 0,
  kcb2160CB = 1,
  kcbCount,
};

enum TargetResolutionKey {
  kt1080p = 0,
  kt2160p = 1,
  ktCount,
};

enum PresentResolutionKey {
  kp1080p = 0,
  kp2160p = 1,
  kpCount,
};

enum DeltaVisualizerMode {
  kDisabled = 0,
  kLuminance = 1,
  kRGB = 2,
  kDeltaVizCount,
};

enum CheckerboardSampleMode {
  kViewportJitter = 0,
  kCustomSampleLocs = 1,
  kCBSampleModeCount,
};

enum TimerTag {
  kSceneRenderInternal = 0,
  kUpscaleInternal = 1,
  kTotalInternal = 2,
  kSceneRenderTarget = 3,
  kTimerTagCount,
  kTimerQueryCount = kTimerTagCount * 2,
};

struct GpuTimerInfo {
  uint64_t start_time;
  uint64_t end_time;
};

struct PerFrameData {
  vkex::QueryPool timer_query_pool;

  bool timestamps_issued = false;
  bool timestamps_readback = false;

  std::vector<GpuTimerInfo> issued_gpu_timers;

  uint32_t cb_frame_index;

  // TODO: Other stuff that might need to be inspected from previous
  // frames, such as targeted resolution or previous frame images
};

class VkexInfoApp : public vkex::Application {
 public:
  VkexInfoApp() : vkex::Application("PorQue4K") {}
  virtual ~VkexInfoApp() {}

  // main.cpp
  void AddArgs(vkex::ArgParser& args);
  void Configure(const vkex::ArgParser& args,
                 vkex::Configuration& configuration);
  void Setup();
  void Update(double frame_elapsed_time);
  void Render(vkex::Application::RenderData* p_data);
  void Present(vkex::Application::PresentData* p_data);

 protected:
  // AppUtil.cpp
  PresentResolutionKey FindPresentResolutionKey(const uint32_t width);
  void UpdateInternalResolutionState();
  void UpdateTargetResolutionState();
  void UpdateUpscalingTechniqueState();
  void SetPresentResolution(PresentResolutionKey new_present_resolution);
  VkExtent2D GetInternalResolutionExtent();
  VkExtent2D GetTargetResolutionExtent();
  VkExtent2D GetPresentResolutionExtent();
  UpscalingTechniqueKey GetUpscalingTechnique();

  const char* GetUpscalingTechniqueText();
  const char* GetTargetResolutionText();
  const char* GetPresentResolutionText();

  void BuildUpscalingTechniqueList(
      std::vector<const char*>& upscaling_technique_list);
  void BuildInternalResolutionTextList(
      std::vector<const char*>& internal_text_list);
  void BuildCBResolutionTextList(std::vector<const char*>& internal_text_list);
  void BuildTargetResolutionTextList(
      std::vector<const char*>& target_text_list);

  void IssueGpuTimeStart(vkex::CommandBuffer cmd, PerFrameData& per_frame_data,
                         TimerTag tag);
  void IssueGpuTimeEnd(vkex::CommandBuffer cmd, PerFrameData& per_frame_data,
                       TimerTag tag);
  void ReadbackGpuTimestamps(uint32_t frame_index);
  double CalculateGpuTimeRange(const PerFrameData& per_frame_data,
                               TimerTag requested_range, double nano_scaler);

  GPULightInfo ConvertCPULightInfoToGPULightInfo(CPULightInfo& cpuLight);
  void UpdateMaterialConstants();
  void UpdateImageDeltaConstants();
  void UpdateDebugConstants();

  vkex::uint3 CalculateSimpleDispatchDimensions(
      GeneratedShaderState& gen_shader_state, VkExtent2D image_extent);

  ImVec2 GetSuggestedGUISize();
  float GetSuggestedFontScale();
  void DrawAppInfoGUI(uint32_t frame_index);

  // AppRender.cpp
  void RenderInternalAndTarget(vkex::CommandBuffer cmd, uint32_t frame_index);
  void NaiveUpscale(vkex::CommandBuffer cmd, uint32_t frame_index);
  void CASUpscale(vkex::CommandBuffer cmd, uint32_t frame_index);
  void CheckerboardUpscale(vkex::CommandBuffer cmd, uint32_t frame_index);
  void UpscaleInternalToTarget(vkex::CommandBuffer cmd, uint32_t frame_index);
  void VisualizeInternalTargetDelta(vkex::CommandBuffer cmd,
                                    uint32_t frame_index);

  void DrawModel(vkex::CommandBuffer cmd);

  // AppSetup.cpp
  void SetupImagesAndRenderPasses(const VkExtent2D present_extent,
                                  const VkFormat color_format,
                                  const VkFormat depth_format);
  void SetupShaders(const std::vector<ShaderProgramInputs>& shader_inputs,
                    std::vector<GeneratedShaderState>& generated_shader_states);
  void BuildCheckerboardMaterialSampler();
  void CheckVulkanFeaturesForPipelines();
  void ConfigureCustomSampleLocationsState();

  // CAS.cpp
  void UpdateCASConstants(const VkExtent2D& srcExtent,
                          const VkExtent2D& dstExtent, const float sharpness,
                          CASUpscalingConstants& constants);

  // Checkerboard.cpp
  void UpdateCheckerboardConstants(const VkExtent2D& dstExtent,
                                   CBUpscalingConstants& constants);
  void UpdateCheckerboardRenderState(uint32_t cb_frame_index);

 private:
  // CPU side state
  bool m_animation_enabled = true;
  float m_animation_progress = 0.0f;
  std::vector<CPULightInfo> m_light_infos;

  std::vector<GeneratedShaderState> m_generated_shader_states;
  vkex::DescriptorPool m_shared_descriptor_pool = nullptr;

  PerFrameConstants m_per_frame_constants = {};
  PerObjectConstants m_per_object_constants = {};
  ScaledTexCopyDimsConstants m_internal_to_target_scaled_copy_constants = {};
  ImageDeltaOptionsConstants m_image_delta_options_constants = {};
  ScaledTexCopyDimsConstants m_target_to_present_scaled_copy_constants = {};
  CASUpscalingConstants m_cas_upscaling_constants = {};
  CBUpscalingConstants m_cb_upscaling_constants = {};

  SimpleRenderPass m_internal_draw_simple_render_pass = {};
  SimpleRenderPass m_internal_as_target_draw_simple_render_pass = {};
  SimpleRenderPass m_checkerboard_simple_render_pass[2];
  vkex::Texture m_target_texture = nullptr;
  vkex::Texture m_visualization_texture = nullptr;

  // TODO: Handle dynamic resolution?

  UpscalingTechniqueKey m_upscaling_technique_key = UpscalingTechniqueKey::None;
  PresentResolutionKey m_present_resolution_key = PresentResolutionKey::kpCount;
  ResolutionInfoKey m_internal_resolution_key = ResolutionInfoKey::krCount;
  CBResolutionInfoKey m_internal_cb_resolution_key =
      CBResolutionInfoKey::kcbCount;
  TargetResolutionKey m_target_resolution_key = TargetResolutionKey::ktCount;
  uint32_t m_selected_upscaling_technique_index = UINT32_MAX;
  uint32_t m_selected_internal_resolution_index = UINT32_MAX;
  uint32_t m_selected_cb_internal_resolution_index = UINT32_MAX;
  uint32_t m_selected_target_resolution_index = UINT32_MAX;

  VkRect2D m_internal_render_area = {};
  VkRect2D m_target_render_area = {};

  VkViewport m_cb_viewport = {};
  float m_texture_gradient_scaler = 0.5f;
  int32_t m_cb_upper_left_XOffset = 0;
  int32_t m_cb_upper_right_XOffset = 0;
  int32_t m_cb_lower_left_XOffset = 0;
  int32_t m_cb_lower_right_XOffset = 0;

  bool m_sample_locations_enabled = false;
  bool m_variable_sample_locations_available = false;
  VkSampleLocationsInfoEXT m_current_sample_locations_info;
  std::vector<VkSampleLocationEXT> m_current_sample_locations;
  VkRenderPassSampleLocationsBeginInfoEXT m_rp_sample_locations;
  std::vector<VkAttachmentSampleLocationsEXT> m_attachment_sample_locations;
  std::vector<VkSubpassSampleLocationsEXT> m_subpass_sample_locations;
  CheckerboardSampleMode m_checkerboard_samples_mode =
      CheckerboardSampleMode::kViewportJitter;

  vkex::Sampler m_cb_grad_adj_sampler = nullptr;

  DeltaVisualizerMode m_delta_visualizer_mode = kDisabled;
  float m_delta_amplifier = 1.0f;

  // TODO: Eventually this becomes a list of models (somewhere) and a pointer
  // for the active model
  GLTFModel m_helmet_model;

  ConstantBufferManager m_constant_buffer_manager;

  std::vector<PerFrameData> m_per_frame_datas;
};

#endif  // __APP_CORE_H__
