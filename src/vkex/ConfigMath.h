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

#ifndef __VKEX_CONFIG_MATH_H__
#define __VKEX_CONFIG_MATH_H__

#if defined(__cplusplus)
# define GLM_FORCE_RADIANS 
# define GLM_FORCE_DEPTH_ZERO_TO_ONE 
# define GLM_ENABLE_EXPERIMENTAL
# include <glm/glm.hpp>
# include <glm/gtc/matrix_inverse.hpp>
# include <glm/gtc/matrix_transform.hpp>
# include <glm/gtx/euler_angles.hpp>
# include <glm/gtx/matrix_decompose.hpp>
# include <glm/gtx/quaternion.hpp>
# include <glm/gtx/transform.hpp>
#endif

#if defined(__cplusplus)
namespace vkex {
#endif

#if defined(__cplusplus)
// Import GLM types as HLSL friendly names
// bool
using bool2     = glm::bool2;
using bool3     = glm::bool3;
using bool4     = glm::bool4;

// 32-bit signed integer
using int2      = glm::ivec2;
using int3      = glm::ivec3;
using int4      = glm::ivec4;
// 32-bit unsigned integer
using uint      = glm::uint;
using uint2     = glm::uvec2;
using uint3     = glm::uvec3;
using uint4     = glm::uvec4;

// 32-bit float
using float2    = glm::vec2;
using float3    = glm::vec3;
using float4    = glm::vec4;
// 32-bit float2 matrices
using float2x2  = glm::mat2x2;
using float2x3  = glm::mat2x3;
using float2x4  = glm::mat2x4;
// 32-bit float3 matrices
using float3x2  = glm::mat3x2;
using float3x3  = glm::mat3x3;
using float3x4  = glm::mat3x4;
// 32-bit float4 matrices
using float4x2  = glm::mat4x2;
using float4x3  = glm::mat4x3;
using float4x4  = glm::mat4x4;
// 32-bit float quaternion
using quat      = glm::quat;

// 64-bit float
using double2   = glm::dvec2;
using double3   = glm::dvec3;
using double4   = glm::dvec4;
// 64-bit float2 matrices
using double2x2 = glm::dmat2x2;
using double2x3 = glm::dmat2x3;
using double2x4 = glm::dmat2x4;
// 64-bit float3 matrices
using double3x2 = glm::dmat3x2;
using double3x3 = glm::dmat3x3;
using double3x4 = glm::dmat3x4;
// 64-bit float4 matrices
using double4x2 = glm::dmat4x2;
using double4x3 = glm::dmat4x3;
using double4x4 = glm::dmat4x4;
#endif

#if defined(__cplusplus)
} // namespace vkex
#endif

#endif // __VKEX_CONFIG_MATH_H__
