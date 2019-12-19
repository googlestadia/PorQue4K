/*
 Copyright 2018 Google Inc.

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

struct VSInput {
  float3 Position : Position;
};

struct VSOutput {
  float4 Position : SV_Position;
  float2 TexCoord : TexCoord;
};

VSOutput vsmain(VSInput input)
{
  VSOutput output = (VSOutput)0;
  output.Position = float4(input.Position, 1);
  output.TexCoord = float2((input.Position.xy + 1.0f) * 0.5f);
  return output;
}

struct PSInput {
  float4 Position : SV_Position;
  float2 TexCoord : TexCoord;
};

struct PSOutput {
  [[vk::location(0)]] float4 rt_a : SV_Target;
};

struct ConstantSurfaceData {
  float3 albedo;
  float3 pos;
  float3 normal;
  float3 view;
  float  roughness;
  float  metalness;
};

static const int LIGHT_COUNT = 64;

struct LightData {
  float3 pos[LIGHT_COUNT];
};

ConstantBuffer<ConstantSurfaceData> SurfaceData : register(b0);
ConstantBuffer<LightData>           Lights : register(b1);

static const float kPi = 3.14159265359;

float chiGGX(float v)
{
  return v > 0 ? 1 : 0;
}

float GGX_Distribution(float3 N, float3 H, float roughness)
{
  float NdotH  = max(0.0, dot(N, H));
  float NdotH2 = NdotH * NdotH;
  float alpha  = saturate(roughness * roughness);
  float alpha2 = alpha * alpha;
  float numer  = alpha2;
  float denom  = (NdotH2 * (alpha2 - 1.0) + 1.0);
  float result = numer / (kPi * denom * denom);
  return result;
}

float GGX_GeometrySchlick(float3 V, float3 N, float roughness)
{
  float alpha  = (roughness + 1.0);
  float alpha2 = alpha * alpha;
  float k      = alpha2 / 8.0;
  float NdotV  = saturate(dot(N, V));
  float numer  = NdotV;
  float denom  = NdotV * (1.0 - k) + k;
  float result = numer / denom;
  return result;
}

float GGX_GeometrySmith(float3 L, float3 V, float3 N, float roughness)
{
  float ggx2 = GGX_GeometrySchlick(V, N, roughness);
  float ggx1 = GGX_GeometrySchlick(L, N, roughness);
  return ggx1 * ggx2;
}

float FresnelSchlick(float F0, float cos_theta)
{
  return F0 + (1.0 - F0) * pow(1.0 - cos_theta, 5.0);
}

float CookTorrance(float3 L, float3 V, float3 N, float roughness, float metalness, out float kS)
{
  float3 H = normalize(V + L);

  float VdotN = saturate(dot(V, N));
  float LdotN = saturate(dot(L, N));
  float VdotH = saturate(dot(V, H));

  float NDF    = GGX_Distribution(N, H, roughness);
  float G      = GGX_GeometrySmith(L, V, N, roughness);
  float F      = FresnelSchlick(metalness, VdotH);
  float numer  = NDF * G * F;
  float denom  = saturate(4.0 * VdotN * LdotN) + 0.001;
  float result = numer / denom;

  kS += F;

  return result;
}

float3 PBR(float3 Albedo, float3 L, float3 V, float3 N, float roughness, float metalness)
{
  float3 radiance = (float3)300.0;
  float  LdotN    = saturate(dot(L, N) + 0.0);
  float  kS       = 0;
  float  specular = CookTorrance(L, V, N, roughness, metalness, kS);
  float  kD       = (1.0 - kS) * (1.0 - metalness);
  float3 result   = (kD * Albedo + (float3)specular) * radiance * LdotN;
  return result;
}

PSOutput psmain(PSInput input)
{
  PSOutput o;

  float3 Albedo    = SurfaceData.albedo;
  float3 P         = SurfaceData.pos;
  float3 N         = SurfaceData.normal;
  float3 V         = SurfaceData.view;
  float  roughness = SurfaceData.roughness;
  float  metalness = SurfaceData.metalness;

  float3 Co = (float3)0.0;
  for (uint i = 0; i < LIGHT_COUNT; ++i) {
    float3 LP   = Lights.pos[i];
    float3 LD   = LP - P;
    float  dist = length(LD);
    float  s    = 1.0 / pow(dist, 1.005);
    float3 L    = normalize(LD);
    float3 pbr  = PBR(Albedo, L, V, N, roughness, metalness) * s;
    Co += pbr;
  }

  // Co = pow(Co, 1.0 / 1.25)*0 + ((dot(N, float3(0, 0, -1)) < 0.0f) ? float3(1, 0, 0) : float3(0,
  // 1, 0)); Co = pow(Co, 1.0 / 1.25)*0 + N;
  Co = pow(Co, 1.0 / 1.25);

  o.rt_a = float4(Co, 1);

  return o;
}
