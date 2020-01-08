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

#include "AppCore.h"

#include "AssetUtil.h"

// TODO: Maybe move resolution tables to another file? To be included directly?

struct ResolutionInfo {
    ResolutionInfoKey id;
    VkExtent2D resolution_extent;
    std::string text;
};

struct TargetResolutionChain {
    TargetResolutionKey id;
    ResolutionInfoKey resolution_info_key;
    std::vector<ResolutionInfoKey> internal_resolution_info_keys;
};

struct PresentResolutionChain {
    PresentResolutionKey id;
    ResolutionInfoKey resolution_info_key;
    std::vector<TargetResolutionKey> target_resolution_keys;
};

static ResolutionInfo s_resolution_infos[ResolutionInfoKey::krCount] = {
    {
        ResolutionInfoKey::kr540p, {960, 540}, "960 x 540"
    },
    {
        ResolutionInfoKey::kr720p, {1280, 720}, "1280 x 720"
    },
    {
        ResolutionInfoKey::kr1080p, {1920, 1080}, "1920 x 1080"
    },
    {
        ResolutionInfoKey::kr1440p, {2560, 1440}, "2560 x 1440"
    },
    {
        ResolutionInfoKey::kr2160p, {3840, 2160}, "3840 x 2160"
    },
};

static TargetResolutionChain s_target_resolutions[TargetResolutionKey::ktCount] = {
    {
        TargetResolutionKey::kt1080p, ResolutionInfoKey::kr1080p, {ResolutionInfoKey::kr1080p, ResolutionInfoKey::kr720p, ResolutionInfoKey::kr540p}
    },
    {
        TargetResolutionKey::kt2160p, ResolutionInfoKey::kr2160p, {ResolutionInfoKey::kr2160p, ResolutionInfoKey::kr1440p, ResolutionInfoKey::kr1080p}
    },
};

static PresentResolutionChain s_present_resolutions[PresentResolutionKey::kpCount] = {
    {
        PresentResolutionKey::kp1080p, ResolutionInfoKey::kr1080p, {TargetResolutionKey::kt1080p},
    },
    {
        PresentResolutionKey::kp2160p, ResolutionInfoKey::kr2160p, {TargetResolutionKey::kt2160p, TargetResolutionKey::kt1080p},
    },
};

PresentResolutionKey VkexInfoApp::FindPresentResolutionKey(const uint32_t width)
{
    PresentResolutionKey detected_present_key = PresentResolutionKey::kpCount;

    for (auto& present_chain : s_present_resolutions) {
        auto res_info_key = present_chain.resolution_info_key;
        if (width == s_resolution_infos[res_info_key].resolution_extent.width) {
            detected_present_key = present_chain.id;
            break;
        }
    }

    VKEX_ASSERT(detected_present_key != PresentResolutionKey::kpCount);

    return detected_present_key;
}

void VkexInfoApp::SetPresentResolution(PresentResolutionKey new_present_resolution)
{
    m_present_resolution_key = new_present_resolution;

    m_selected_target_resolution_index = 0;
    m_target_resolution_key = s_present_resolutions[m_present_resolution_key].target_resolution_keys[m_selected_target_resolution_index];

    m_selected_internal_resolution_index = 0;
    m_internal_resolution_key = s_target_resolutions[m_target_resolution_key].internal_resolution_info_keys[m_selected_internal_resolution_index];
}

VkExtent2D VkexInfoApp::GetTargetResolutionExtent()
{
    auto res_info_key = s_target_resolutions[m_target_resolution_key].resolution_info_key;
    return s_resolution_infos[res_info_key].resolution_extent;
}

VkExtent2D VkexInfoApp::GetInternalResolutionExtent()
{
    return s_resolution_infos[m_internal_resolution_key].resolution_extent;
}

const char * VkexInfoApp::GetTargetResolutionText()
{
    auto res_info_key = s_target_resolutions[m_target_resolution_key].resolution_info_key;
    return s_resolution_infos[res_info_key].text.c_str();
}

const char * VkexInfoApp::GetPresentResolutionText()
{
    auto res_info_key = s_present_resolutions[m_present_resolution_key].resolution_info_key;
    return s_resolution_infos[res_info_key].text.c_str();
}

void VkexInfoApp::BuildInternalResolutionTextList(std::vector<const char*>& internal_text_list)
{
    for (auto internal_resolution : s_target_resolutions[m_target_resolution_key].internal_resolution_info_keys) {
        auto& res_info = s_resolution_infos[internal_resolution];
        internal_text_list.push_back(res_info.text.c_str());
    }
}

void VkexInfoApp::BuildTargetResolutionTextList(std::vector<const char*>& target_text_list)
{
    for (auto target_resolution : s_present_resolutions[m_present_resolution_key].target_resolution_keys) {
        auto res_info_key = s_target_resolutions[target_resolution].resolution_info_key;
        auto& res_info = s_resolution_infos[res_info_key];
        target_text_list.push_back(res_info.text.c_str());
    }
}

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
                std::vector<const char*> resolution_items;
                BuildInternalResolutionTextList(resolution_items);

                ImGui::Text("Internal resolution");
                ImGui::NextColumn();
                ImGui::Combo("", (int*)(&m_selected_internal_resolution_index), resolution_items.data(), int(resolution_items.size()));
                ImGui::NextColumn();
            }
            {
                // TODO: Make target resolution selectable
                ImGui::Text("Target Resolution");
                ImGui::NextColumn();
                ImGui::Text(GetTargetResolutionText());
                ImGui::NextColumn();
            }
            {
                ImGui::Text("Present Resolution");
                ImGui::NextColumn();
                ImGui::Text(GetPresentResolutionText());
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
