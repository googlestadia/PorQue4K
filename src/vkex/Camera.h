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

#ifndef __VKEX_CAMERA_H__
#define __VKEX_CAMERA_H__

#include <vkex/Config.h>

namespace vkex {

// =================================================================================================
// Camera
// =================================================================================================
class Camera {
public:
  Camera() {}
  Camera(float near_clip, float far_clip);
  ~Camera() {}

  void              LookAt(const float3& eye, const float3& center, const float3& up);

  const float3&     GetEyePosition() const { return m_eye_position; }

  const float4x4&   GetViewMatrix() const { return m_view_matrix ;}
  const float4x4&   GetProjectionMatrix() const { return m_projection_matrix ;}
  const float4x4&   GetViewProjectionMatrix() const { return m_view_projection_matrix; }

  float3            WorldToViewPoint(const float3& world_point) const;
  float3            WorldToViewVector(const float3& world_vector) const;

protected:
  float             m_aspect          = 0;
  float             m_near_clip       = 0.1f;
  float             m_far_clip        = 10000.0f;
  float3            m_eye_position    = float3(0, 0,  0);
  float3            m_look_at         = float3(0, 0, -1);
  float3            m_view_direction  = float3(0, 0, -1);
  float3            m_world_up        = float3(0, 1,  0);
  mutable float4x4  m_view_matrix;
  mutable float4x4  m_projection_matrix;
  mutable float4x4  m_view_projection_matrix;
};

// =================================================================================================
// PerspCamera
// =================================================================================================
class PerspCamera : public Camera {
public:
  PerspCamera();
  PerspCamera(
    float fov_degrees,
    float aspect,
    float near_clip = 0.1f, 
    float far_clip = 10000.0f);
  PerspCamera(
    const float3& eye,
    const float3& center,
    const float3& up,
    float         fov_degrees, 
    float         aspect,
    float         near_clip = 0.1f, 
    float         far_clip = 10000.0f);
  ~PerspCamera();

  void SetPerspective(float fov_degrees, float aspect, float near_clip, float far_clip);

private:
  bool  m_pixel_aligned   = false;
  float m_fov_degrees     = 60.0f;
  float m_aspect          = 1.0f;
};

// =================================================================================================
// OrthoCamera
// =================================================================================================
class OrthoCamera : public Camera {
public:
  OrthoCamera();
  OrthoCamera(
    float left, 
    float right, 
    float bottom, 
    float top, 
    float near_clip, 
    float far_clip);
  ~OrthoCamera();

  void SetOrthographic(
    float left, 
    float right, 
    float bottom, 
    float top, 
    float near_clip, 
    float far_clip);

private:
    float m_left    = -1.0f;
    float m_right   =  1.0f;
    float m_bottom  = -1.0f;
    float m_top     =  1.0f;
};

} // namespace vkex

#endif // __VKEX_CAMERA_H__