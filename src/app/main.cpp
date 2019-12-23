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

#include "vkex/Application.h"

#include "AssetUtil.h"
#include "DebugUi.h"
#include "SimpleRenderPass.h"

#define ARG_HEADLESS_SHORT_NAME "s"
#define ARG_HEADLESS_LONG_NAME  "headless"

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
    VkexInfoApp() : vkex::Application(1920, 1080, "0000_vts_info") {}
    virtual ~VkexInfoApp() {}

    void Configure(const vkex::ArgParser& args, vkex::Configuration& configuration);
    void Setup();
    void Update(double frame_elapsed_time) {}
    void Render(vkex::Application::RenderData* p_data) {}
    void Present(vkex::Application::PresentData* p_data);

protected:
    virtual void BuildSampleGPA(std::vector<vkex::CommandBuffer>& command_buffers) {}
    virtual void BuildSamplePipelineStatistics(vkex::CommandBuffer& command_buffer) {}

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
};

void VkexInfoApp::Configure(const vkex::ArgParser& args, vkex::Configuration& configuration)
{
    configuration.window.resizeable = false;
    configuration.swapchain.depth_stencil_format = VK_FORMAT_D32_SFLOAT;
    configuration.graphics_debug.enable = false;
    configuration.graphics_debug.message_severity.info = false;
    configuration.graphics_debug.message_severity.warning = false;
    configuration.graphics_debug.message_severity.error = false;
    configuration.graphics_debug.message_type.validation = false;
}

void VkexInfoApp::Setup()
{
    // Geometry data
    vkex::PlatonicSolid::Options cube_options = {};
    cube_options.vertex_colors = true;
    vkex::PlatonicSolid cube = vkex::PlatonicSolid::Cube(cube_options);
    const vkex::VertexBufferData* p_vertex_buffer_data = cube.GetVertexBufferByIndex(0);

    // Shader program
    {
        VKEX_CALL(asset_util::CreateShaderProgram(
            GetDevice(),
            GetAssetPath("shaders/draw_vertex.vs.spv"),
            GetAssetPath("shaders/draw_vertex.ps.spv"),
            &m_shader_program));
    }

    // Descriptor set layouts
    {
        const vkex::ShaderInterface& shader_interface = m_shader_program->GetInterface();
        vkex::DescriptorSetLayoutCreateInfo create_info = ToVkexCreateInfo(shader_interface.GetSet(0));
        VKEX_CALL(GetDevice()->CreateDescriptorSetLayout(create_info, &m_descriptor_set_layout));
    }

    // Descriptor pool
    {
        const vkex::ShaderInterface& shader_interface = m_shader_program->GetInterface();
        vkex::DescriptorPoolCreateInfo create_info = {};
        create_info.pool_sizes = shader_interface.GetDescriptorPoolSizes();
        VKEX_CALL(GetDevice()->CreateDescriptorPool(create_info, &m_descriptor_pool));
    }

    // Descriptor sets
    {
        vkex::DescriptorSetAllocateInfo allocate_info = {};
        allocate_info.layouts.push_back(m_descriptor_set_layout);
        VKEX_CALL(m_descriptor_pool->AllocateDescriptorSets(allocate_info, &m_descriptor_set));
    }

    // Pipeline layout
    {
        vkex::PipelineLayoutCreateInfo create_info = {};
        create_info.descriptor_set_layouts.push_back(vkex::ToVulkan(m_descriptor_set_layout));
        vkex::Result vkex_result = vkex::Result::Undefined;
        VKEX_CALL(GetDevice()->CreatePipelineLayout(create_info, &m_pipeline_layout));
    }

    // Pipeline
    {
        vkex::VertexBindingDescription vertex_binding_descriptions = p_vertex_buffer_data->GetVertexBindingDescription();

        vkex::GraphicsPipelineCreateInfo create_info = {};
        create_info.shader_program = m_shader_program;
        create_info.vertex_binding_descriptions = { vertex_binding_descriptions };
        create_info.samples = VK_SAMPLE_COUNT_1_BIT;
        create_info.depth_test_enable = true;
        create_info.depth_write_enable = true;
        create_info.pipeline_layout = m_pipeline_layout;
        create_info.rtv_formats = { GetConfiguration().swapchain.color_format };
        create_info.dsv_format = GetConfiguration().swapchain.depth_stencil_format;
        vkex::Result vkex_result = vkex::Result::Undefined;
        VKEX_CALL(GetDevice()->CreateGraphicsPipeline(create_info, &m_pipeline));
    }

    // Constant buffer
    {
        VKEX_CALL(asset_util::CreateConstantBuffer(
            m_view_transform_constants.size,
            nullptr,
            GetGraphicsQueue(),
            asset_util::MEMORY_USAGE_CPU_TO_GPU,
            &m_constant_buffer));
    }

    // Vertex buffer
    {
        VKEX_CALL(asset_util::CreateVertexBuffer(
            p_vertex_buffer_data->GetDataSize(),
            p_vertex_buffer_data->GetData(),
            GetGraphicsQueue(),
            asset_util::MEMORY_USAGE_GPU_ONLY,
            &m_vertex_buffer));
    }

    // Update descriptors
    {
        m_descriptor_set->UpdateDescriptor(0, m_constant_buffer);
    }
}

void VkexInfoApp::Present(vkex::Application::PresentData* p_data)
{
    auto cmd = p_data->GetCommandBuffer();
    auto render_pass = p_data->GetRenderPass();

    float3 eye = float3(0, 0, 2);
    float3 center = float3(0, 0, 0);
    float3 up = float3(0, 1, 0);
    float aspect = GetWindowAspect();
    vkex::PerspCamera camera(eye, center, up, 60.0f, aspect);

    float t = GetFrameStartTime();
    float4x4 M = glm::translate(float3(0, 0, 0)) * glm::rotate(t / 2.0f, float3(0, 1, 0)) * glm::rotate(t / 4.0f, float3(1, 0, 0));
    float4x4 V = camera.GetViewMatrix();
    float4x4 P = camera.GetProjectionMatrix();

    m_view_transform_constants.data.ModelViewProjectionMatrix = P * V*M;

    VKEX_CALL(m_constant_buffer->Copy(m_view_transform_constants.size, &m_view_transform_constants.data));

    VkClearValue rtv_clear = {};
    VkClearValue dsv_clear = {};
    dsv_clear.depthStencil.depth = 1.0f;
    dsv_clear.depthStencil.stencil = 0xFF;
    std::vector<VkClearValue> clear_values = { rtv_clear, dsv_clear };
    cmd->Begin();
    cmd->CmdBeginRenderPass(render_pass, &clear_values);
    cmd->CmdSetViewport(render_pass->GetFullRenderArea());
    cmd->CmdSetScissor(render_pass->GetFullRenderArea());
    cmd->CmdBindPipeline(m_pipeline);
    cmd->CmdBindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, *m_pipeline_layout, 0, { *m_descriptor_set });
    cmd->CmdBindVertexBuffers(m_vertex_buffer);
    cmd->CmdDraw(36, 1, 0, 0);

    std::string window = "Physical Device : " + std::string(GetDevice()->GetDescriptiveName());
    if (ImGui::Begin(window.c_str())) {
        DrawDebugUiPhyiscalDevice(GetDevice()->GetPhysicalDevice());
    }
    ImGui::End();

    this->DrawDebugApplicationInfo();
    this->DrawImGui(cmd);

    cmd->CmdEndRenderPass();
    cmd->End();

    SubmitPresent(p_data);
}

int main(int argc, char** argv)
{
    VkexInfoApp app;
    vkex::Result vkex_result = app.Run(argc, argv);
    if (!vkex_result) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}