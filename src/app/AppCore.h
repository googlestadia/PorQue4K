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


enum ShaderPipelineType {
    Graphics = 0,
    Compute = 1,
};

enum AppShaderList {
    Geometry = 0,
    ScaledTexCopy = 1,
    NumTypes,
};

struct ShaderProgramInputs {
    ShaderPipelineType pipeline_type;
    std::vector<vkex::fs::path> shader_paths;
    vkex::GraphicsPipelineCreateInfo graphics_pipeline_create_info;
};

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

class VkexInfoApp
    : public vkex::Application
{
public:
    VkexInfoApp() : vkex::Application(1920, 1080, "PorQue4K") {}
    virtual ~VkexInfoApp() {}

    void Configure(const vkex::ArgParser& args, vkex::Configuration& configuration);
    void Setup();
    void Update(double frame_elapsed_time);
    void Render(vkex::Application::RenderData* p_data);
    void Present(vkex::Application::PresentData* p_data);

protected:
    PresentResolutionKey FindPresentResolutionKey(const uint32_t width);
    void SetPresentResolution(PresentResolutionKey new_present_resolution);
    VkExtent2D GetTargetResolutionExtent();
    VkExtent2D GetInternalResolutionExtent();

    const char * GetTargetResolutionText();
    const char * GetPresentResolutionText();

    void BuildInternalResolutionTextList(std::vector<const char*>& internal_text_list);
    void BuildTargetResolutionTextList(std::vector<const char*>& target_text_list);

    void DrawAppInfoGUI();

    void SetupShaders(const std::vector<ShaderProgramInputs>& shader_inputs,
                            std::vector<GeneratedShaderState>& generated_shader_states);

private:
    std::vector<GeneratedShaderState> m_generated_shader_states;
    vkex::DescriptorPool              m_shared_descriptor_pool = nullptr;

    vkex::Buffer                m_simple_draw_vertex_buffer = nullptr;

    // TODO: Is there a struct we can use to manage the CPU/GPU copies of constant data?
    ViewTransformConstants      m_simple_draw_view_transform_constants = {};
    std::vector<vkex::Buffer>   m_simple_draw_constant_buffers;

    ScaledTexCopyDimsConstants  m_scaled_tex_copy_dims_constants = {};
    std::vector<vkex::Buffer>   m_scaled_tex_copy_constant_buffers;

    SimpleRenderPass            m_draw_simple_render_pass = {};

    // TODO: Handle dynamic resolution?
    // TODO: Build out Internal -> Target -> Swapchain render paths
    //       Render to Internal resolution and Target resolution
    //       Upscale/visualize to Target resolution
    //       Copy to swapchain

    PresentResolutionKey             m_present_resolution_key;
    TargetResolutionKey              m_target_resolution_key;
    ResolutionInfoKey                m_internal_resolution_key;
    uint32_t                         m_selected_internal_resolution_index;
    uint32_t                         m_selected_target_resolution_index;

    uint32_t                         m_internal_width = UINT32_MAX;
    uint32_t                         m_internal_height = UINT32_MAX;
    VkRect2D                         m_internal_render_area = {};
};

#endif // __APP_CORE_H__
