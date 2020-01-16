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

#include "GLTFModel.h"
#include "SimpleRenderPass.h"

using float2 = vkex::float2;
using float3 = vkex::float3;
using float4 = vkex::float4;
using float2x2 = vkex::float2x2;
using float3x3 = vkex::float3x3;
using float4x4 = vkex::float4x4;

using uint = vkex::uint;

// TODO: Move constant struct defs to another file to share with shaders

struct ViewTransformData {
    float4x4 ModelViewProjectionMatrix;
};

using ViewTransformConstants = vkex::ConstantBufferData<ViewTransformData>;

struct ScaledTexCopyDimensionsData {
    uint srcWidth, srcHeight;
    uint dstWidth, dstHeight;
};

using ScaledTexCopyDimsConstants = vkex::ConstantBufferData<ScaledTexCopyDimensionsData>;

struct ImageDeltaOptions {
    uint vizMode;
};
using ImageDeltaOptionsConstants = vkex::ConstantBufferData<ImageDeltaOptions>;

enum ShaderPipelineType {
    Graphics = 0,
    Compute = 1,
};

enum AppShaderList {
    Geometry = 0,
    InternalToTargetScaledCopy = 1,
    TargetToPresentScaledCopy = 2,
    InternalTargetImageDelta = 3,
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
    ShaderPipelineType          pipeline_type;
    vkex::ShaderProgram         program = nullptr;
    vkex::DescriptorSetLayout   descriptor_set_layout = nullptr;
    vkex::PipelineLayout        pipeline_layout = nullptr;
    union {
        vkex::GraphicsPipeline  graphics_pipeline = nullptr;
        vkex::ComputePipeline   compute_pipeline;
    };
    std::vector<vkex::DescriptorSet> descriptor_sets;
};

// TODO: Rename these enums into something more sane...

enum ResolutionInfoKey {
    kr540p = 0,
    kr720p = 1,
    kr1080p = 2,
    kr1440p = 3,
    kr2160p = 4,
    krCount,
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

class VkexInfoApp
    : public vkex::Application
{
public:
    VkexInfoApp() : vkex::Application(1920, 1080, "PorQue4K") {}
    virtual ~VkexInfoApp() {}

    // main.cpp
    void Configure(const vkex::ArgParser& args, vkex::Configuration& configuration);
    void Setup();
    void Update(double frame_elapsed_time);
    void Render(vkex::Application::RenderData* p_data);
    void Present(vkex::Application::PresentData* p_data);

protected:
    // AppCore.cpp
    PresentResolutionKey FindPresentResolutionKey(const uint32_t width);
    void UpdateInternalResolutionState();
    void UpdateTargetResolutionState();
    void SetPresentResolution(PresentResolutionKey new_present_resolution);
    VkExtent2D GetInternalResolutionExtent();
    VkExtent2D GetTargetResolutionExtent();
    VkExtent2D GetPresentResolutionExtent();

    const char * GetTargetResolutionText();
    const char * GetPresentResolutionText();

    void BuildInternalResolutionTextList(std::vector<const char*>& internal_text_list);
    void BuildTargetResolutionTextList(std::vector<const char*>& target_text_list);

    // TODO: This is very informal because the TG size doesn't necessarily 
    // mean it represents 1 output pixel per thread
    vkex::uint3 CalculateSimpleDispatchDimensions(GeneratedShaderState& gen_shader_state, VkExtent2D dest_image_extent);

    void DrawAppInfoGUI();

    // AppRender.cpp
    void ProcessInternalToTarget(vkex::CommandBuffer cmd, uint32_t frame_index);
    void UpscaleInternalToTarget(vkex::CommandBuffer cmd, uint32_t frame_index);
    void VisualizeInternalTargetDelta(vkex::CommandBuffer cmd, uint32_t frame_index);

    void DrawModel(vkex::CommandBuffer cmd);

    // AppSetup.cpp
    void SetupImagesAndRenderPasses(const VkExtent2D present_extent, 
                                    const VkFormat color_format, 
                                    const VkFormat depth_format);
    void SetupShaders(const std::vector<ShaderProgramInputs>& shader_inputs,
                            std::vector<GeneratedShaderState>& generated_shader_states);

private:
    std::vector<GeneratedShaderState> m_generated_shader_states;
    vkex::DescriptorPool              m_shared_descriptor_pool = nullptr;

    // TODO: Is there a struct we can use to manage the CPU/GPU copies of constant data?
    // TODO: What we should do is to create one big Buffer for all constants (per-frame)
    // and then allocate chunks out of it, instead of creating all of these stupid
    // little buffers. VkDescriptorBufferInfo handles the offset into the buffer just fine.

    ViewTransformConstants      m_simple_draw_view_transform_constants = {};
    std::vector<vkex::Buffer>   m_simple_draw_constant_buffers;

    ScaledTexCopyDimsConstants  m_internal_to_target_scaled_copy_constants = {};
    std::vector<vkex::Buffer>   m_internal_to_target_scaled_copy_constant_buffers;

    ScaledTexCopyDimsConstants  m_target_to_present_scaled_copy_constants = {};
    std::vector<vkex::Buffer>   m_target_to_present_scaled_copy_constant_buffers;

    ImageDeltaOptionsConstants  m_image_delta_options_constants = {};
    std::vector<vkex::Buffer>   m_image_delta_options_constant_buffers;

    SimpleRenderPass            m_internal_draw_simple_render_pass = {};
    SimpleRenderPass            m_internal_as_target_draw_simple_render_pass = {};
    vkex::Texture               m_target_texture = nullptr;

    // TODO: Handle dynamic resolution?

    PresentResolutionKey             m_present_resolution_key = PresentResolutionKey::kpCount;
    TargetResolutionKey              m_target_resolution_key = TargetResolutionKey::ktCount;
    ResolutionInfoKey                m_internal_resolution_key = ResolutionInfoKey::krCount;
    uint32_t                         m_selected_internal_resolution_index = UINT32_MAX;
    uint32_t                         m_selected_target_resolution_index = UINT32_MAX;

    VkRect2D                         m_internal_render_area = {};
    VkRect2D                         m_target_render_area = {};

    DeltaVisualizerMode              m_delta_visualizer_mode = kDisabled;

    // TODO: Eventually this becomes a list of models (somewhere) and a pointer for the active model
    GLTFModel                        m_helmet_model;
};

#endif // __APP_CORE_H__
