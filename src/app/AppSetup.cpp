/*
 Copyright 2020 Google Inc.

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

void VkexInfoApp::SetupImagesAndRenderPasses(const VkExtent2D present_extent,
                                             const VkFormat color_format,
                                             const VkFormat depth_format)
{
    {
        vkex::Result vkex_result = vkex::Result::Undefined;
        VKEX_CALL(CreateSimpleRenderPass(
            GetDevice(),
            GetGraphicsQueue(),
            present_extent.width, present_extent.height,
            color_format, depth_format,
            &m_internal_draw_simple_render_pass));
    }

    {
        vkex::Result vkex_result = vkex::Result::Undefined;
        VKEX_CALL(CreateSimpleRenderPass(
            GetDevice(),
            GetGraphicsQueue(),
            present_extent.width, present_extent.height,
            color_format, depth_format,
            &m_internal_as_target_draw_simple_render_pass));
    }

    {
        vkex::TextureCreateInfo create_info = {};
        create_info.image.image_type = VK_IMAGE_TYPE_2D;
        create_info.image.format = color_format;
        create_info.image.extent = { present_extent.width, present_extent.height, 1 };
        create_info.image.mip_levels = 1;
        create_info.image.array_layers = 1;
        create_info.image.samples = VK_SAMPLE_COUNT_1_BIT;
        create_info.image.tiling = VK_IMAGE_TILING_OPTIMAL;
        create_info.image.usage_flags.bits.color_attachment = false;
        create_info.image.usage_flags.bits.transfer_src = true; // Maybe if we use vkCmdBlitImage?
        create_info.image.usage_flags.bits.sampled = true;
        create_info.image.usage_flags.bits.storage = true;
        create_info.image.sharing_mode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.image.initial_layout = VK_IMAGE_LAYOUT_UNDEFINED;
        create_info.image.committed = true;
        create_info.image.host_visible = false;
        create_info.image.device_local = true;
        create_info.view.derive_from_image = true;

        vkex::Result vkex_result = vkex::Result::Undefined;
        VKEX_CALL(GetDevice()->CreateTexture(create_info, &m_target_texture));
    }

    {
        vkex::Result vkex_result = vkex::Result::Undefined;
        VKEX_CALL(vkex::TransitionImageLayout(GetGraphicsQueue(),
            m_target_texture,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_GENERAL,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT));
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
            }
            else {
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
            }
            else {
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