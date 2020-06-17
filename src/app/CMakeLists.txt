#
# Copyright 2019-2020 Google Inc.
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
# http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(4KApp)

set(SRC_DIR      ${CMAKE_CURRENT_SOURCE_DIR})
set(ASSETS_DIR   ${CMAKE_SOURCE_DIR}/assets)
set(SHADERS_DIR  ${SRC_DIR}/shaders)
set(TINYGLTF_INC_DIR	      ${CMAKE_SOURCE_DIR}/third_party/tinygltf)
set(FIDELITYFX_INC_DIR ${CMAKE_SOURCE_DIR}/third_party/FidelityFX/FFX_CAS/ffx-cas-headers)

list(APPEND HDR_FILES
    ${SRC_DIR}/AppCore.h
    ${SRC_DIR}/AssetUtil.h
    ${SRC_DIR}/ConstantBufferManager.h
    ${SRC_DIR}/ConstantBufferStructs.h
    ${SRC_DIR}/GLTFModel.h
    ${SRC_DIR}/SharedShaderConstants.h
    ${SRC_DIR}/SimpleRenderPass.h
    ${SHADERS_DIR}/draw_shader_core.h
    ${FIDELITYFX_INC_DIR}/ffx_a.h
    ${FIDELITYFX_INC_DIR}/ffx_cas.h
)

list(APPEND SRC_FILES
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/AppUtil.cpp
    ${SRC_DIR}/AppRender.cpp
    ${SRC_DIR}/AppSetup.cpp
    ${SRC_DIR}/AssetUtil.cpp
    ${SRC_DIR}/CAS.cpp
    ${SRC_DIR}/Checkerboard.cpp
    ${SRC_DIR}/ConstantBufferManager.cpp
    ${SRC_DIR}/GLTFModel.cpp
    ${SRC_DIR}/SimpleRenderPass.cpp
)

list(APPEND VSPS_SHADER_FILES
  ${SHADERS_DIR}/draw_cb.hlsl
  ${SHADERS_DIR}/draw_standard.hlsl
)

list(APPEND CS_SHADER_FILES
  ${SHADERS_DIR}/cas.hlsl
  ${SHADERS_DIR}/checkerboard_upscale.hlsl
  ${SHADERS_DIR}/copy_texture.hlsl
  ${SHADERS_DIR}/image_delta.hlsl
)

source_group ("Shaders\\VsPs" FILES
  ${VSPS_SHADER_FILES}
  ${SHADERS_DIR}/draw_shader_core.h
)

source_group ("Shaders\\Cs" FILES
  ${CS_SHADER_FILES}
)

if (BUILD_SHADERS)
  file (TO_NATIVE_PATH ${FIDELITYFX_INC_DIR} CAS_INC_DIR)
  
  foreach(VSPS_SHADER_PATH ${VSPS_SHADER_FILES})
    compile_hlsl_vs_ps(${VSPS_SHADER_PATH} ${ASSETS_DIR}/shaders ${CMAKE_CURRENT_SOURCE_DIR})
  endforeach()
  foreach(CS_SHADER_PATH ${CS_SHADER_FILES})
    compile_hlsl_cs(${CS_SHADER_PATH} ${ASSETS_DIR}/shaders ${CMAKE_CURRENT_SOURCE_DIR} ${CAS_INC_DIR})
  endforeach()
else()
  set_source_files_properties(${VSPS_SHADER_FILES} PROPERTIES VS_TOOL_OVERRIDE "None")
  set_source_files_properties(${CS_SHADER_FILES} PROPERTIES VS_TOOL_OVERRIDE "None")
endif()

add_executable(${PROJECT_NAME} ${HDR_FILES} ${SRC_FILES} ${VSPS_SHADER_FILES} ${CS_SHADER_FILES})

target_include_directories(${PROJECT_NAME} 
  PRIVATE   ${SRC_DIR}
  PUBLIC    ${TINYGLTF_INC_DIR}
            ${FIDELITYFX_INC_DIR}
            ${VULKAN_INCLUDE_DIR}
            ${VKEX_TOP_INC_DIR}
)

target_link_libraries(${PROJECT_NAME} 
  PRIVATE vkex
)

if(GGP)
  target_link_libraries(${PROJECT_NAME}
    PRIVATE ggp
            dl
            pthread)
endif()
