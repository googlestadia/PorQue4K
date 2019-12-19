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

#ifndef __VKEX_TRANSFORM_H__
#define __VKEX_TRANSFORM_H__

#include <vkex/Config.h>

namespace vkex {

// =================================================================================================
// Transform
// =================================================================================================
class Transform {
public:
  enum RotationOrder {
    XYZ,
    XZY,
    YZX,
    YXZ,
    ZXY,
    ZYX,
  };

  Transform();
  Transform(const float3& translation);
  virtual ~Transform();

  const float3&   GetTranslation() const { return m_translation; }
  const float3&   GetRotation() const { return m_rotation; }
  const float3&   GetScale() const { return m_scale; }
  RotationOrder   GetRotationOrder() const { return m_rotation_order; }

  void            SetTranslation(const float3& value);
  void            SetTranslation(float x, float y, float z) { SetTranslation(float3(x, y, z)); }
  void            SetRotation(const float3& value);
  void            SetRotation(float x, float y, float z) { SetRotation(float3(x, y, z)); }
  void            SetScale(const float3& value);
  void            SetScale(float x, float y, float z) { SetScale(float3(x, y, z)); }
  void            SetRotationOrder(RotationOrder value);

  const float4x4& GetTranslationMatrix() const;
  const float4x4& GetRotationMatrix() const;
  const float4x4& GetScaleMatrix() const;
  const float4x4& GetConcatenatedMatrix() const;

protected:
  mutable struct {
    union {
      struct {
        bool    translation  : 1;
        bool    rotation     : 1;
        bool    scale        : 1;
        bool    concatenated : 1;
      };
      uint32_t  mask = 0xF;
    };
  } m_dirty;

  float3            m_translation     = float3(0, 0, 0);
  float3            m_rotation        = float3(0, 0, 0);
  float3            m_scale           = float3(1, 1, 1);
  RotationOrder     m_rotation_order  = RotationOrder::XYZ;
  mutable float4x4  m_translation_matrix;
  mutable float4x4  m_rotation_matrix;
  mutable float4x4  m_scale_matrix;
  mutable float4x4  m_concatenated_matrix;
};

} // namespace vkex

#endif // __VKEX_TRANSFORM_H__