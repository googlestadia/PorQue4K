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

struct ViewTransformData {
    float4x4 ModelViewProjectionMatrix;
};

using ViewTransformConstants = vkex::ConstantBufferData<ViewTransformData>;

struct ScaledTexCopyDimensionsData {
    uint srcWidth, srcHeight;
    uint dstWidth, dstHeight;
};

using ScaledTexCopyDimsConstants = vkex::ConstantBufferData<ScaledTexCopyDimensionsData>;

// TODO: Move constant struct defs to another file to share with shaders

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
    void DrawAppInfoGUI();

private:
    // TODO: Once these start getting grouped, I'll have to think about
    // what things belong to a pass versus a shader. The descriptor sets
    // that get allocated will be in different quantities depending on usage
    vkex::DescriptorPool        m_descriptor_pool = nullptr;

    vkex::ShaderProgram         m_simple_draw_shader_program = nullptr;
    vkex::DescriptorSetLayout   m_simple_draw_descriptor_set_layout = nullptr;
    vkex::DescriptorSet         m_simple_draw_descriptor_set = nullptr;
    vkex::PipelineLayout        m_simple_draw_pipeline_layout = nullptr;
    vkex::GraphicsPipeline      m_simple_draw_pipeline = nullptr;
    ViewTransformConstants      m_simple_draw_view_transform_constants = {};
    vkex::Buffer                m_simple_draw_constant_buffer = nullptr;
    vkex::Buffer                m_simple_draw_vertex_buffer = nullptr;

    vkex::ShaderProgram         m_scaled_tex_copy_shader_program = nullptr;
    vkex::DescriptorSetLayout   m_scaled_tex_copy_descriptor_set_layout = nullptr;
    vkex::PipelineLayout        m_scaled_tex_copy_pipeline_layout = nullptr;
    vkex::ComputePipeline       m_scaled_tex_copy_pipeline = nullptr;

    struct InternalDrawState {
        ScaledTexCopyDimsConstants  scaled_tex_copy_dims_constants = {};
        vkex::Buffer                scaled_tex_copy_constant_buffer = nullptr;
        
        vkex::DescriptorSet         scaled_tex_copy_descriptor_set = nullptr;

        SimpleRenderPass            simple_render_pass = {};
    };

    InternalDrawState                m_target_res_draw;
    InternalDrawState                m_half_res_draw;
    InternalDrawState*               m_current_internal_draw = nullptr;

    // TODO: Map out descriptor sets + descriptor pool layout

    enum InternalResolution {
        Full = 0,
        Half = 1,
        // TODO: Dynamic?
        // TODO: If our target resolutions are 1080p and 4K, we could
        //       just have three internal resolutions per target resolution
        //       1080p: 540p, 720p, 1080p
        //       4K: 1080p, 1440p, 2160p
        //       This also would extend to other target resolutions.
        //       Our chain would be Internal -> Target -> Swapchain
        //       Render to Internal resolution and Target resolution
        //       Upscale/visualize to Target resolution
        //       Copy to swapchain
    };
    InternalResolution               m_internal_res_selector = InternalResolution::Full;
};

#endif // __APP_CORE_H__
