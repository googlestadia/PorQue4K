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

#include <vkex/Camera.h>
#include <vkex/VulkanUtil.h>

namespace vkex {

// =================================================================================================
// Camera
// =================================================================================================
Camera::Camera(float near_clip, float far_clip)
  : m_near_clip(near_clip),
    m_far_clip(far_clip)
{
}

void Camera::LookAt(const float3& eye, const float3& center, const float3& up)
{
  m_eye_position = eye;
  m_view_matrix = glm::lookAt(m_eye_position, center, up);
}

float3 Camera::WorldToViewPoint(const float3& world_point) const
{
  float3 view_point = float3(m_view_matrix * float4(world_point, 1.0f));
  return view_point;
}

float3 Camera::WorldToViewVector(const float3& world_vector) const
{
  float3 view_point = float3(m_view_matrix * float4(world_vector, 0.0f));
  return view_point;
}

// =================================================================================================
// PerspCamera
// =================================================================================================
PerspCamera::PerspCamera()
{
}

PerspCamera::PerspCamera(
  float fov_degrees, 
  float aspect,
  float near_clip, 
  float far_clip
)
  : Camera(near_clip, far_clip)
{
  SetPerspective(
    fov_degrees,
    aspect,
    near_clip,
    far_clip);
}

PerspCamera::PerspCamera(
  const float3& eye,
  const float3& center,
  const float3& up,
  float         fov_degrees, 
  float         aspect,
  float         near_clip, 
  float         far_clip
)
  : Camera(near_clip, far_clip)
{
  LookAt(eye, center, up);
  SetPerspective(
    fov_degrees,
    aspect,
    near_clip,
    far_clip);
}

PerspCamera::~PerspCamera()
{
}

void PerspCamera::SetPerspective(float fov_degrees, float aspect, float near_clip, float far_clip)
{
  m_fov_degrees = fov_degrees;
  m_aspect = aspect;
  m_near_clip = near_clip;
  m_far_clip = far_clip;

  m_projection_matrix = glm::perspective(
    glm::radians(m_fov_degrees),
    m_aspect,
    m_near_clip,
    m_far_clip);
}

// =================================================================================================
// OrthoCamera
// =================================================================================================
OrthoCamera::OrthoCamera()
{
}

OrthoCamera::OrthoCamera(
  float left, 
  float right, 
  float bottom, 
  float top, 
  float near_clip, 
  float far_clip
)
{
  SetOrthographic(
    left,
    right,
    bottom,
    top,
    near_clip,
    far_clip);
}

OrthoCamera::~OrthoCamera()
{
}

void OrthoCamera::SetOrthographic(
  float left, 
  float right, 
  float bottom, 
  float top, 
  float near_clip, 
  float far_clip
)
{
  m_left      = left;
  m_right     = right;
  m_bottom    = bottom;
  m_top       = top;
  m_near_clip = near_clip;
  m_far_clip  = far_clip;

  m_projection_matrix = glm::ortho(
    m_left, 
    m_right, 
    m_bottom, 
    m_top, 
    m_near_clip, 
    m_far_clip);
}

} // namespace vkex