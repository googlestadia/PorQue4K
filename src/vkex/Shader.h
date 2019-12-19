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

#ifndef __VKEX_SHADER_H__
#define __VKEX_SHADER_H__

#include <vkex/Config.h>
#include <vkex/Traits.h>
#include <vkex/VulkanUtil.h>

namespace vkex {

// =================================================================================================
// ShaderModule
// =================================================================================================

/** @struct ShaderModuleCreateInfo 
 *
 */
struct ShaderModuleCreateInfo {
  VkShaderStageFlagBits stage;
  uint32_t              code_size;
  const uint8_t*        code;
  std::string           entry_point;
  std::string           source_file;
};

/** @class IShaderModule
 *
 */ 
class CShaderModule : public IDeviceObject {
public:
  CShaderModule();
  ~CShaderModule();

  /** @fn operator VkShaderModule()
   *
   */
  operator VkShaderModule() const { 
    return m_vk_object; 
  }

  /** @fn GetVkObject
   *
   */
  VkShaderModule GetVkObject() const { 
    return m_vk_object; 
  }

  /** @fn GetStage
   *
   */
  VkShaderStageFlagBits GetStage() const {
    return m_create_info.stage;
  }

  /** @fn GetEntryPoint
   *
   */
  const std::string& GetEntryPoint() const {
    return m_create_info.entry_point;
  }

  //! @fn GetInterface
  const vkex::ShaderInterface& GetInterface() const {
    return m_interface;
  }

private:  
  friend class CDevice;
  friend class CShaderProgram;
  friend class IObjectStorageFunctions;

  /** @fn InternalCreate
   *
   */
  vkex::Result InternalCreate(
    const vkex::ShaderModuleCreateInfo& create_info,
    const VkAllocationCallbacks*        p_allocator
  );

  /** @fn InternalDestroy
   *
   */
  vkex::Result InternalDestroy(const VkAllocationCallbacks* p_allocator);

private:
  vkex::ShaderModuleCreateInfo  m_create_info = {};
  VkShaderModuleCreateInfo      m_vk_create_info = {};
  VkShaderModule                m_vk_object = VK_NULL_HANDLE;
  vkex::ShaderInterface         m_interface;
};

// =================================================================================================
// ShaderProgram
// =================================================================================================

/** @struct ShaderProgramCreateInfo 
 *
 */
struct ShaderProgramCreateInfo {
  vkex::ShaderModule  vs;
  vkex::ShaderModule  hs;
  vkex::ShaderModule  ds;
  vkex::ShaderModule  gs;
  vkex::ShaderModule  ps;
  vkex::ShaderModule  cs;
};

/** @class IShaderProgram
 *
 */ 
class CShaderProgram : public IDeviceObject {
public:
  CShaderProgram();
  ~CShaderProgram();

  //! @fn GetVS
  const ShaderModule& GetVS() const {
    return m_create_info.vs;
  }

  //! @fn GetHS
  const ShaderModule& GetHS() const {
    return m_create_info.hs;
  }

  //! @fn GetDS
  const ShaderModule& GetDS() const {
    return m_create_info.ds;
  }

  //! @fn GetGS
  const ShaderModule& GetGS() const {
    return m_create_info.gs;
  }

  //! @fn GetPS
  const ShaderModule& GetPS() const {
    return m_create_info.ps;
  }

  //! @fn GetCS
  const ShaderModule& GetCS() const {
    return m_create_info.cs;
  }

  //! @fn GetInterface
  const vkex::ShaderInterface& GetInterface() const {
    return m_interface;
  }

private:  
  friend class CDevice;
  friend class IObjectStorageFunctions;

  /** @fn InternalCreate
   *
   */
  vkex::Result InternalCreate(
    const vkex::ShaderProgramCreateInfo&  create_info,
    const VkAllocationCallbacks*          p_allocator
  );

  /** @fn InternalDestroy
   *
   */
  vkex::Result InternalDestroy(const VkAllocationCallbacks* p_allocator);

private:
  vkex::ShaderProgramCreateInfo m_create_info = {};
  vkex::ShaderInterface         m_interface;
};

// =================================================================================================
// Support functions
// =================================================================================================
vkex::Result CreateShaderProgram(
  vkex::Device                device,
  const std::vector<uint8_t>& spv_cs,
  vkex::ShaderProgram*        p_shader_program
);

vkex::Result CreateShaderProgram(
  vkex::Device                device,
  const std::vector<uint8_t>& spv_vs,
  const std::vector<uint8_t>& spv_ps,
  vkex::ShaderProgram*        p_shader_program
);

} // namespace vkex

#endif // __VKEX_SHADER_H__