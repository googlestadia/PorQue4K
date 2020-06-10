/*
 Copyright 2020 Google Inc.

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

#ifndef __CONSTANT_BUFFER_STRUCTS_H__
#define __CONSTANT_BUFFER_STRUCTS_H__

#include "SharedShaderConstants.h"

struct GPULightInfo {
  float3 direction;
  int padding1;
  float3 color;
  int padding2;
  float intensity;

  // int           type;

  // TODO: other fields once we have types
};

struct PerFrameConstantData {
  float4x4 viewProjectionMatrix;
  float4x4 prevViewProjectionMatrix;
  float3 cameraPos;
#if (CB_RESOLVE_DEBUG > 0)
  float texGradScaler;
#else
  int padding;
#endif

  GPULightInfo dirLight;
};

struct PerObjectConstantData {
  float4x4 worldMatrix;
  float4x4 prevWorldMatrix;

  // material constants
  float4 baseColorFactor;
  float3 emissiveFactor;
  uint padding1;

  float metallicFactor;
  float roughnessFactor;
  uint2 padding2;
};

struct ScaledTexCopyDimensionsData {
  uint srcWidth, srcHeight;
  uint dstWidth, dstHeight;
};

struct ImageDeltaOptions {
  uint vizMode;
  float deltaAmplifier;
  uint2 padding1;
};

struct CASData {
  uint4 const0;
  uint4 const1;
};

struct CBResolveData {
  uint srcWidth;
  uint srcHeight;
  uint cbIndex;
  uint padding1;
  int ulXOffset;
  int urXOffset;
  int llXOffset;
  int lrXOffset;
};

#endif  // __CONSTANT_BUFFER_STRUCTS_H__
