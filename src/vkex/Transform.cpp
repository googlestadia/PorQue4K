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

#include <vkex/Transform.h>

namespace vkex {

// =================================================================================================
// Transform
// =================================================================================================
Transform::Transform()
{
}

Transform::Transform(const float3& translation)
{
  SetTranslation(translation);
}

Transform::~Transform()
{
}

void Transform::SetTranslation(const float3& value)
{
  m_translation = value;
  m_dirty.translation = true;
  m_dirty.concatenated = true;
}

void Transform::SetRotation(const float3& value)
{
  m_rotation = value;
  m_dirty.rotation = true;
  m_dirty.concatenated = true;
}

void Transform::SetScale(const float3& value)
{
  m_scale = value;
  m_dirty.scale = true;
}

void Transform::SetRotationOrder(RotationOrder value)
{
  m_rotation_order = value;
  m_dirty.rotation = true;
  m_dirty.concatenated = true;
}

const float4x4& Transform::GetTranslationMatrix() const
{
  if (m_dirty.translation) {
    m_translation_matrix = glm::translate(m_translation);
    m_dirty.translation = false;
    m_dirty.concatenated = true;
  }
  return m_translation_matrix;
}

const float4x4& Transform::GetRotationMatrix() const
{
  if (m_dirty.rotation) {
    float4x4 xm = glm::rotate(m_rotation.x, float3(1, 0, 0));
    float4x4 ym = glm::rotate(m_rotation.y, float3(0, 1, 0));
    float4x4 zm = glm::rotate(m_rotation.z, float3(0, 0, 1));
    switch (m_rotation_order) {
      case RotationOrder::XYZ: m_rotation_matrix = xm * ym * zm; break;
      case RotationOrder::XZY: m_rotation_matrix = xm * zm * ym; break;
      case RotationOrder::YZX: m_rotation_matrix = ym * zm * xm; break;
      case RotationOrder::YXZ: m_rotation_matrix = ym * xm * zm; break;
      case RotationOrder::ZXY: m_rotation_matrix = zm * xm * ym; break;
      case RotationOrder::ZYX: m_rotation_matrix = zm * ym * xm; break;
    }
    m_dirty.rotation = false;
    m_dirty.concatenated = true;
  }
  return m_rotation_matrix;
}

const float4x4& Transform::GetScaleMatrix() const
{
  if (m_dirty.scale) {
    m_scale_matrix = glm::scale(m_scale);
    m_dirty.scale = false;
    m_dirty.concatenated = true;
  }
  return m_scale_matrix;
}

const float4x4& Transform::GetConcatenatedMatrix() const
{
  if (m_dirty.concatenated) {
    const float4x4& T = GetTranslationMatrix();
    const float4x4& R = GetRotationMatrix();
    const float4x4& S = GetScaleMatrix();
    m_concatenated_matrix = T*R*S;
    m_dirty.concatenated = false;
  }
  return m_concatenated_matrix;
}

} // namespace vkex