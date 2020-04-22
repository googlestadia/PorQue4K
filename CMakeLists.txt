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

project(VulkanPorQue4K)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Don't generate ZERO_CHECK
set(CMAKE_SUPPRESS_REGENERATION true)

# We don't need all configs...
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

# We can't use CXX_STANDARD here because in the GGP + VS usage,
# an invalid CppLanguageStandard token is generated. Using CXX_FLAGS
# seems to work, and relatively well-known workaround
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14") 

if(DEFINED GGP_TOOLCHAIN_PATH)
  set(GGP TRUE)
endif()

if ((${CMAKE_GENERATOR} STREQUAL "Visual Studio 15 2017") AND GGP)
  set(GGP_VS TRUE)
  message(STATUS "Generating VS files with GGP support")
endif()

if(MSVC)
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} /MP")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()

# TODO: Can we ever use find_package(vulkan) to handle this?
# Couldn't get this working on Windows, so gave up...
set(VULKAN_DIR_FOUND FALSE)
if (NOT VULKAN_DIR)
    set(VULKAN_DIR $ENV{VULKAN_SDK})
endif()
if (NOT "${VULKAN_DIR}" STREQUAL "")
    message(STATUS "Using Vulkan found at ${VULKAN_DIR}")
    set(VULKAN_DIR_FOUND TRUE)
endif()

# Compiling with GGP doesn't require the VULKAN_SDK environment variable,
# because the GGP SDK includes the headers and libraries that we need.
if (NOT GGP)
    if (NOT VULKAN_DIR_FOUND)
        message( FATAL_ERROR "Vulkan SDK could not be located" )
    endif()
endif()

if(WIN32)
    set(VULKAN_INCLUDE_DIR  ${VULKAN_DIR}/include)
    set(VULKAN_LIBRARY  ${VULKAN_DIR}/lib/vulkan-1.lib)
elseif(GGP)
    set(VULKAN_INCLUDE_DIR  ${GGP_SYSROOT_PATH}/usr/include)
    set(VULKAN_LIBRARY  ${GGP_SYSROOT_PATH}/lib/libvulkan.so)
elseif(UNIX AND NOT APPLE)
    set(VULKAN_INCLUDE_DIR  ${VULKAN_DIR}/include)
    set(VULKAN_LIBRARY  ${VULKAN_DIR}/lib/libvulkan.so)
endif()

message(STATUS "Using Vulkan include dir: ${VULKAN_INCLUDE_DIR}")
message(STATUS "Using Vulkan lib: ${VULKAN_LIBRARY}")

set(VKEX_TOP_INC_DIR ${CMAKE_SOURCE_DIR}/src)

if (NOT DXC_PATH)
  if (WIN32)
    message("Path to DXC not specified! Point to dxc.exe via DXC_PATH CMake cache entry")
    find_program(DXC_PATH dxc.exe)
  else()
    set(DXC_PATH dxc)
  endif()
  message("Found DXC: ${DXC_PATH}")
endif()

get_filename_component(ABSOLUTE ${DXC_PATH} PROGRAM)
file(TO_NATIVE_PATH ${DXC_PATH} DXC_PATH)

message("Using DXC: ${DXC_PATH}")

# TODO: Do I want to convert add_custom_command to
# add_custom_target in order to properly manage
# dependencies?

function(compile_hlsl_vs_ps hlsl_path output_dir working_dir)
  #message("${hlsl_path}")
  get_filename_component(SHADER_DIR ${hlsl_path} DIRECTORY)
  file(TO_NATIVE_PATH ${SHADER_DIR} SHADER_DIR)
  string(REPLACE "hlsl" "vs.spv" vs_file ${hlsl_path})
  string(REPLACE "hlsl" "ps.spv" ps_file ${hlsl_path})
  get_filename_component(vs_file ${vs_file} NAME)
  get_filename_component(ps_file ${ps_file} NAME)
  set(vs_file ${output_dir}/${vs_file})
  set(ps_file ${output_dir}/${ps_file})
  file(TO_NATIVE_PATH ${working_dir} INCLUDE_PATH)
  #message("${vs_file} ${ps_file}")
  add_custom_command(
    COMMAND ${DXC_PATH} -I ${INCLUDE_PATH} -I ${SHADER_DIR} -T vs_6_0 -spirv -E vsmain -Fo ${vs_file} ${hlsl_path}
    COMMAND ${CMAKE_COMMAND} -E echo "Compiling VS ${hlsl_path} to ${vs_file}"
    COMMAND ${DXC_PATH} -I ${INCLUDE_PATH} -I ${SHADER_DIR} -T ps_6_0 -spirv -E psmain -Fo ${ps_file} ${hlsl_path}
    COMMAND ${CMAKE_COMMAND} -E echo "Compiling PS ${hlsl_path} to ${ps_file}"
    IMPLICIT_DEPENDS CXX ${hlsl_path}
    MAIN_DEPENDENCY ${hlsl_path}
    OUTPUT ${vs_file} ${ps_file}
    WORKING_DIRECTORY ${working_dir}
  )
endfunction()

function(compile_hlsl_cs hlsl_path output_dir working_dir addl_incl_dir)
  #message("${hlsl_path}")
  string(REPLACE "hlsl" "cs.spv" cs_file ${hlsl_path})
  get_filename_component(cs_file ${cs_file} NAME)
  set(cs_file ${output_dir}/${cs_file})
  file(TO_NATIVE_PATH ${working_dir} INCLUDE_PATH)
  add_custom_command(
    COMMAND ${DXC_PATH} -I ${INCLUDE_PATH} -I ${addl_incl_dir} -T cs_6_0 -spirv -E csmain -Fo ${cs_file} ${hlsl_path}
    COMMAND ${CMAKE_COMMAND} -E echo "Compiling VS ${hlsl_path} to ${cs_file}"
    IMPLICIT_DEPENDS CXX ${hlsl_path}
    MAIN_DEPENDENCY ${hlsl_path}
    OUTPUT ${cs_file}
    WORKING_DIRECTORY ${working_dir}
  )
endfunction()

if(GGP)
    # Needed?
    add_compile_definitions(VKEX_LINUX_GGP=1)
    add_compile_definitions(__yeti__=1)
endif()

add_subdirectory(third_party/glfw EXCLUDE_FROM_ALL)

# TODO: There's really only one app here, so do we really need to
# build vkex in it's own lib?
add_subdirectory(src/vkex)

add_subdirectory(src/app)