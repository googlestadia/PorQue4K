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
                //auto target_res_extent = m_target_res_internal_draw.simple_render_pass.rtv_texture->GetImage()->GetExtent();
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
