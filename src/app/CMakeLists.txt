#
# Copyright 2019 Google Inc.
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

list(APPEND HDR_FILES
    ${SRC_DIR}/AssetUtil.h
    ${SRC_DIR}/DebugUI.h
    ${SRC_DIR}/SimpleRenderPass.h
)

list(APPEND SRC_FILES
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/AssetUtil.cpp
    ${SRC_DIR}/DebugUI.cpp
    ${SRC_DIR}/SimpleRenderPass.cpp
)

list(APPEND SHADER_FILES
  ${SHADERS_DIR}/draw_vertex.hlsl
)

source_group ("Shaders" FILES
  ${SHADER_FILES}
)

if (BUILD_SHADERS)
  compile_hlsl_vs_ps(${SHADERS_DIR}/draw_vertex.hlsl ${ASSETS_DIR}/shaders ${CMAKE_CURRENT_SOURCE_DIR})
else()
  set_source_files_properties(${SHADER_FILES} PROPERTIES VS_TOOL_OVERRIDE "None")
endif()

add_executable(${PROJECT_NAME} ${HDR_FILES} ${SRC_FILES} ${SHADER_FILES})

target_include_directories(${PROJECT_NAME} 
  PRIVATE   ${SRC_DIR}
  PUBLIC    ${VULKAN_INCLUDE_DIR}
            ${VKEX_TOP_INC_DIR}
)

target_link_libraries(${PROJECT_NAME} 
  PRIVATE vkex
)

if(GGP)
  target_link_libraries(${PROJECT_NAME} 
    PRIVATE ${GGP_LIBRARY_PATH}/lib/libggp.so	
            dl
            pthread)
endif()