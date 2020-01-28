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

#include "ConstantBufferStructs.h"

struct VSInput {
  float3  PositionOS  : POSITION;
  float3  Normal  : NORMAL;
  float2  UV0  : TEXCOORD0;
};

struct VSOutput {
  float4  PositionCS  : SV_Position;
  float3  Normal  : NORMAL;
  float2  UV0  : TEXCOORD0;
};

ConstantBuffer<ViewTransformData> ViewTransform : register(b0);

VSOutput vsmain(VSInput input)
{
  float4x4 ModelViewProjectionMatrix = ViewTransform.ModelViewProjectionMatrix;

  VSOutput output = (VSOutput)0;
  output.PositionCS = mul(ModelViewProjectionMatrix, float4(input.PositionOS, 1));
  
  output.Normal = input.Normal;
  output.UV0 = input.UV0;

  return output;
}

struct PSInput {
  float4  PositionCS  : SV_Position;
  float3  Normal  : NORMAL;
  float2  UV0  : TEXCOORD0;
};

// TODO: Can I set these to s0 and t1?
// TODO: Can I build SamplerState in shader?
SamplerState texSampler : register(s1);
Texture2D<float4> albedoTexture : register(t2);

float4 psmain(PSInput input) : SV_Target
{
  //return float4(1.0f, 0.f, 0.f, 1);
  //return float4(input.UV0.x, input.UV0.y, 0.f, 1);
  //return float4(input.Normal.x, input.Normal.y, input.Normal.z, 1);

    float4 albedoColor = albedoTexture.Sample(texSampler, input.UV0);

    return float4(albedoColor.rgb, 1.f);
}