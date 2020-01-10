/*
 Copyright 2018-2019 Google Inc.
 
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

#include "vkex/Shader.h"
#include "vkex/Device.h"
#include "vkex/ToString.h"

// SPIRV-Reflect
#include "spirv_reflect.h"

namespace vkex {

// =================================================================================================
// ShaderModule
// =================================================================================================
CShaderModule::CShaderModule()
{
}

CShaderModule::~CShaderModule()
{
}

vkex::Result CShaderModule::InternalCreate(
  const vkex::ShaderModuleCreateInfo& create_info,
  const VkAllocationCallbacks*        p_allocator
)
{
  // Copy create info
  m_create_info = create_info;

  // Vulkan create info
  m_vk_create_info = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
  m_vk_create_info.flags    = 0;
  m_vk_create_info.codeSize = m_create_info.code_size;
  m_vk_create_info.pCode    = reinterpret_cast<const uint32_t*>(m_create_info.code);
  VkResult vk_result = InvalidValue<VkResult>::Value;
  VKEX_VULKAN_RESULT_CALL(
    vk_result,
    vkex::CreateShaderModule(
      *m_device,
      &m_vk_create_info,
      p_allocator,
      &m_vk_object)
  );
  if (vk_result != VK_SUCCESS) {
    return vkex::Result(vk_result);
  }

  // Reflect information
  {
    spv_reflect::ShaderModule reflection(m_create_info.code_size, m_create_info.code);
    SpvReflectResult spv_reflect_result = reflection.GetResult();
    if (spv_reflect_result != SPV_REFLECT_RESULT_SUCCESS) {
      return vkex::Result(spv_reflect_result);
    }

    // Grab shader stage if not specified    
    if (m_create_info.stage == 0) {
      m_create_info.stage = static_cast<VkShaderStageFlagBits>(reflection.GetShaderStage());
    }

    // Grab entry point if not specified
    if (m_create_info.entry_point.empty()) {
      m_create_info.entry_point = reflection.GetEntryPointName();
    }

    // Grab file source if not specified
    if (m_create_info.source_file.empty() && (reflection.GetSourceFile() != nullptr)) {
      m_create_info.source_file = reflection.GetSourceFile();
    }

    if ((m_create_info.stage & VK_SHADER_STAGE_COMPUTE_BIT) != 0)
    {
      // HACK: No formal getter yet?
      auto dispatch_local_size = reflection.GetShaderModule().entry_points[0].local_size;
      m_interface.AddThreadgroupDimensions(dispatch_local_size.x, dispatch_local_size.y, dispatch_local_size.z);
    }
  
    // Descriptor set create infos
    {
      uint32_t count = 0;
      spv_reflect_result = reflection.EnumerateDescriptorBindings(&count, nullptr);
      if (spv_reflect_result != SPV_REFLECT_RESULT_SUCCESS) {
        return vkex::Result(spv_reflect_result);
      }
      
      std::vector<SpvReflectDescriptorBinding*> bindings(count);
      spv_reflect_result = reflection.EnumerateDescriptorBindings(&count, bindings.data());
      if (spv_reflect_result != SPV_REFLECT_RESULT_SUCCESS) {
        return vkex::Result(spv_reflect_result);
      }

      // Build out descriptor set and binding information
      for (auto& binding : bindings) {
        vkex::ShaderInterface::Binding desc = {};
        desc.name               = (binding->name != nullptr ? binding->name : "");
        desc.set_number         = binding->set;
        desc.binding_number     = binding->binding;
        desc.descriptor_type    = static_cast<VkDescriptorType>(binding->descriptor_type);
        desc.descriptor_count   = binding->count;
        desc.stage_flags        = m_create_info.stage;
        vkex::Result vkex_result = vkex::Result::Undefined;
        VKEX_RESULT_CALL(
          vkex_result,
          m_interface.AddBinding(desc)
        );
        if (!vkex_result) {
          return vkex_result;
        }
      }
    }
  }

  return vkex::Result::Success;
}

vkex::Result CShaderModule::InternalDestroy(const VkAllocationCallbacks* p_allocator)
{
  if (m_vk_object) {
    vkex::DestroyShaderModule(
      *m_device,
      m_vk_object,
      p_allocator);
    
    m_vk_object = VK_NULL_HANDLE;
  }

  return vkex::Result::Success;
}

// =================================================================================================
// ShaderProgram
// =================================================================================================
CShaderProgram::CShaderProgram()
{
}

CShaderProgram::~CShaderProgram()
{
}

vkex::Result CShaderProgram::InternalCreate(
  const vkex::ShaderProgramCreateInfo&  create_info,
  const VkAllocationCallbacks*          p_allocator
)
{
  // Copy create info
  m_create_info = create_info;

  // Shader modules
  std::vector<vkex::ShaderModule> modules = {
    m_create_info.vs,
    m_create_info.hs,
    m_create_info.ds,
    m_create_info.gs,
    m_create_info.ps,
    m_create_info.cs,
  };

  // Build set descriptions
  for (auto& module : modules) {
    if (module == nullptr) {
      continue;
    }

    auto& module_shader_interface = module->GetInterface();
    m_interface.AddBindings(module_shader_interface);

    if ((module->GetStage() & VK_SHADER_STAGE_COMPUTE_BIT) != 0) {
        auto tg_dims = module_shader_interface.GetThreadgroupDimensions();
        m_interface.AddThreadgroupDimensions(tg_dims.x, tg_dims.y, tg_dims.z);
    }
  }

  return vkex::Result::Success;
}

vkex::Result CShaderProgram::InternalDestroy(const VkAllocationCallbacks* p_allocator)
{
  return vkex::Result::Success;
}

// =================================================================================================
// Support functions
// =================================================================================================
vkex::Result CreateShaderProgram(
  vkex::Device                device,
  const std::vector<uint8_t>& spv_cs,
  vkex::ShaderProgram*        p_shader_program
)
{
  // CS
  vkex::ShaderModule cs = nullptr;
  if (!spv_cs.empty()) {
    vkex::ShaderModuleCreateInfo create_info = {};
    create_info.code_size = CountU32(spv_cs);
    create_info.code      = DataPtr(spv_cs);
    vkex::Result vkex_result = vkex::Result::Undefined; 
    VKEX_RESULT_CALL(
      vkex_result,
      device->CreateShaderModule(create_info, &cs);
    );
    if (!vkex_result) {
      return vkex_result;
    }
  }
 
  // Shader program
  {
    vkex::ShaderProgramCreateInfo create_info = {};
    create_info.cs = cs;
    vkex::Result vkex_result = vkex::Result::Undefined; 
    VKEX_RESULT_CALL(
      vkex_result,
      device->CreateShaderProgram(create_info, p_shader_program);
    );
    if (!vkex_result) {
      return vkex_result;
    }
  }

  return vkex::Result::Success;
}

vkex::Result CreateShaderProgram(
  vkex::Device                device,
  const std::vector<uint8_t>& spv_vs,
  const std::vector<uint8_t>& spv_ps,
  vkex::ShaderProgram*        p_shader_program
)
{
  // VS
  vkex::ShaderModule vs = nullptr;
  if (!spv_vs.empty()) {
    vkex::ShaderModuleCreateInfo create_info = {};
    create_info.code_size = CountU32(spv_vs);
    create_info.code      = DataPtr(spv_vs);
    vkex::Result vkex_result = vkex::Result::Undefined; 
    VKEX_RESULT_CALL(
      vkex_result,
      device->CreateShaderModule(create_info, &vs);
    );
    if (!vkex_result) {
      return vkex_result;
    }
  }

  // PS
  vkex::ShaderModule ps = nullptr;
  if (!spv_ps.empty()) {
    vkex::ShaderModuleCreateInfo create_info = {};
    create_info.code_size = CountU32(spv_ps);
    create_info.code      = DataPtr(spv_ps);
    vkex::Result vkex_result = vkex::Result::Undefined; 
    VKEX_RESULT_CALL(
      vkex_result,
      device->CreateShaderModule(create_info, &ps);
    );
    if (!vkex_result) {
      return vkex_result;
    }
  }
  
  // Shader program
  {
    vkex::ShaderProgramCreateInfo create_info = {};
    create_info.vs = vs;
    create_info.ps = ps;
    vkex::Result vkex_result = vkex::Result::Undefined; 
    VKEX_RESULT_CALL(
      vkex_result,
      device->CreateShaderProgram(create_info, p_shader_program);
    );
    if (!vkex_result) {
      return vkex_result;
    }
  }

  return vkex::Result::Success;
}


} // namespace vkex