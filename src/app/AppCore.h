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

struct ViewTransformData {
    float4x4 ModelViewProjectionMatrix;
};

using ViewTransformConstants = vkex::ConstantBufferData<ViewTransformData>;

class VkexInfoApp
    : public vkex::Application
{
public:
    VkexInfoApp() : vkex::Application(1920, 1080, "PorQue4K") {}
    virtual ~VkexInfoApp() {}

    void Configure(const vkex::ArgParser& args, vkex::Configuration& configuration);
    void Setup();
    void Update(double frame_elapsed_time) {}
    void Render(vkex::Application::RenderData* p_data);
    void Present(vkex::Application::PresentData* p_data);

protected:
    void DrawAppInfoGUI();

private:
    vkex::ShaderProgram         m_shader_program = nullptr;
    vkex::DescriptorSetLayout   m_descriptor_set_layout = nullptr;
    vkex::DescriptorPool        m_descriptor_pool = nullptr;
    vkex::DescriptorSet         m_descriptor_set = nullptr;
    vkex::PipelineLayout        m_pipeline_layout = nullptr;
    vkex::GraphicsPipeline      m_pipeline = nullptr;
    ViewTransformConstants      m_view_transform_constants = {};
    vkex::Buffer                m_constant_buffer = nullptr;
    vkex::Buffer                m_vertex_buffer = nullptr;

    SimpleRenderPass            m_target_res_simple_render_pass = {};
};

#endif // __APP_CORE_H__
