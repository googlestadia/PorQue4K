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

#include "AppCore.h"

#include "AssetUtil.h"

void VkexInfoApp::SetupShaders(const std::vector<ShaderProgramInputs>& shader_inputs, 
                                     std::vector<GeneratedShaderState>& generated_shader_states)
{
    generated_shader_states.reserve(shader_inputs.size());

    vkex::DescriptorPoolCreateInfo descriptor_pool_create_info = {};

    for (auto& shader_input : shader_inputs) {
        GeneratedShaderState gen_shader_state = {};

        {
            gen_shader_state.pipeline_type = shader_input.pipeline_type;
            
            // TODO: Check shader path length?
            if (gen_shader_state.pipeline_type == ShaderPipelineType::Compute) {
                VKEX_CALL(asset_util::CreateShaderProgramCompute(
                    GetDevice(),
                    shader_input.shader_paths[0],
                    &gen_shader_state.program));
            } else {
                VKEX_CALL(asset_util::CreateShaderProgram(
                    GetDevice(),
                    shader_input.shader_paths[0],
                    shader_input.shader_paths[1],
                    &gen_shader_state.program));
            }
        }

        {
            const vkex::ShaderInterface& shader_interface = gen_shader_state.program->GetInterface();
            vkex::DescriptorSetLayoutCreateInfo create_info = ToVkexCreateInfo(shader_interface.GetSet(0));
            VKEX_CALL(GetDevice()->CreateDescriptorSetLayout(create_info, &gen_shader_state.descriptor_set_layout));

            descriptor_pool_create_info.pool_sizes += shader_interface.GetDescriptorPoolSizes();
        }

        {
            vkex::PipelineLayoutCreateInfo create_info = {};
            create_info.descriptor_set_layouts.push_back(vkex::ToVulkan(gen_shader_state.descriptor_set_layout));
            vkex::Result vkex_result = vkex::Result::Undefined;
            VKEX_CALL(GetDevice()->CreatePipelineLayout(create_info, &gen_shader_state.pipeline_layout));
        }

        {
            if (gen_shader_state.pipeline_type == ShaderPipelineType::Compute) {
                vkex::ComputePipelineCreateInfo create_info = {};
                create_info.shader_program = gen_shader_state.program;
                create_info.pipeline_layout = gen_shader_state.pipeline_layout;

                vkex::Result vkex_result = vkex::Result::Undefined;
                VKEX_CALL(GetDevice()->CreateComputePipeline(create_info, &gen_shader_state.compute_pipeline));
            } else {
                vkex::GraphicsPipelineCreateInfo gfx_create_info = shader_input.graphics_pipeline_create_info;
                gfx_create_info.shader_program = gen_shader_state.program;
                gfx_create_info.pipeline_layout = gen_shader_state.pipeline_layout;

                vkex::Result vkex_result = vkex::Result::Undefined;
                VKEX_CALL(GetDevice()->CreateGraphicsPipeline(gfx_create_info, &gen_shader_state.graphics_pipeline));
            }
        }

        generated_shader_states.push_back(gen_shader_state);
    }

    const uint32_t frame_count = GetConfiguration().frame_count;

    {
        // TODO: DescriptorPoolCreateInfo does have a size_multiplier, but it isn't used yet...
        descriptor_pool_create_info.pool_sizes *= frame_count;
        VKEX_CALL(GetDevice()->CreateDescriptorPool(descriptor_pool_create_info, &m_shared_descriptor_pool));
    }

    for (auto& gen_shader_state : generated_shader_states) {
        vkex::DescriptorSetAllocateInfo allocate_info = {};
        allocate_info.layouts.push_back(gen_shader_state.descriptor_set_layout);

        gen_shader_state.descriptor_sets.resize(frame_count);
        for (uint32_t frame_index = 0; frame_index < frame_count; frame_index++) {
            VKEX_CALL(m_shared_descriptor_pool->AllocateDescriptorSets(allocate_info,
                &gen_shader_state.descriptor_sets[frame_index]));
        }
    }
}

void VkexInfoApp::DrawAppInfoGUI()
{
    if (!m_configuration.enable_imgui) {
        return;
    }

    if (ImGui::Begin("Application Info")) {
        {
            ImGui::Columns(2);
            {
                ImGui::Text("Application PID");
                ImGui::NextColumn();
                ImGui::Text("%d", GetProcessId());
                ImGui::NextColumn();
            }
            {
                ImGui::Text("Application Name");
                ImGui::NextColumn();
                ImGui::Text("%s", m_configuration.name.c_str());
                ImGui::NextColumn();
            }
            ImGui::Columns(1);
        }

        ImGui::Separator();

        {
            ImGui::Columns(2);
            {
                ImGui::Text("CPU Stats");
                ImGui::NextColumn();
                ImGui::NextColumn();
            }
            {
                ImGui::Text("Average Frame Time");
                ImGui::NextColumn();
                ImGui::Text("%f ms", (GetAverageFrameTime() * 1000.0));
                ImGui::NextColumn();
            }
            {
                ImGui::Text("Current Frame Time");
                ImGui::NextColumn();
                ImGui::Text("%f ms", GetFrameElapsedTime() * 1000.0f);
                ImGui::NextColumn();
            }
            {
                ImGui::Text("Max Past %d Frames Time", kWindowFrames);
                ImGui::NextColumn();
                ImGui::Text("%f ms", GetMaxWindowFrameTime() * 1000.0f);
                ImGui::NextColumn();
            }
            {
                ImGui::Text("Min Past %d Frames Time", kWindowFrames);
                ImGui::NextColumn();
                ImGui::Text("%f ms", GetMinWindowFrameTime() * 1000.0f);
                ImGui::NextColumn();
            }
            {
                ImGui::Text("Frames Per Second");
                ImGui::NextColumn();
                ImGui::Text("%f fps", GetFramesPerSecond());
                ImGui::NextColumn();
            }
            {
                ImGui::Text("Total Frames");
                ImGui::NextColumn();
                ImGui::Text("%llu frames", static_cast<unsigned long long>(GetElapsedFrames()));
                ImGui::NextColumn();
            }
            {
                ImGui::Text("Elapsed Time (s)");
                ImGui::NextColumn();
                ImGui::Text("%f seconds", GetElapsedTime());
                ImGui::NextColumn();
            }
            ImGui::Columns(1);
        }

        ImGui::Separator();

        // Function call times
        {
            ImGui::Columns(2);
            {
                ImGui::Text("Update Call Time");
                ImGui::NextColumn();
                ImGui::Text("%f ms", m_update_fn_time * 1000.0);
                ImGui::NextColumn();
            }
            {
                ImGui::Text("Render Call Time");
                ImGui::NextColumn();
                ImGui::Text("%f ms", m_render_fn_time * 1000.0f);
                ImGui::NextColumn();
            }
            {
                ImGui::Text("Present Call Time");
                ImGui::NextColumn();
                ImGui::Text("%f ms", m_present_fn_time * 1000.0f);
                ImGui::NextColumn();
            }
            ImGui::Columns(1);
        }
        
        ImGui::Separator();

        // Upscale info
        {
            // TODO: Upscale selector
            // TODO: Visualizer selector
            //          Target res
            //          Upscaled
            //          Delta visualizers...
            ImGui::Columns(2);
            {
                ImGui::Text("Internal resolution");
                ImGui::NextColumn();
                const char* items[] = { "Native", "Half", };
                ImGui::Combo("", (int*)(&m_internal_res_selector), items, IM_ARRAYSIZE(items));
                ImGui::NextColumn();
            }
            {
                // TODO: This info will come from another data struct, that will also be used to update constant buffer values
                // and scissor/viewport
                auto target_res_extent = m_current_internal_draw->simple_render_pass.rtv_texture->GetImage()->GetExtent();

                ImGui::Text("Target Resolution");
                ImGui::NextColumn();
                ImGui::Text("%d x %d", target_res_extent.width, target_res_extent.height);
                ImGui::NextColumn();
            }
            ImGui::Columns(1);
        }

        ImGui::Separator();

        // TODO: GPU stats
        {
            // TODO: GPU Render time
            // TODO: Total full res pass time
            // TODO: Total upscaled pass time
            // TODO: Upscale pass time
        }
    }
    ImGui::End();
}
