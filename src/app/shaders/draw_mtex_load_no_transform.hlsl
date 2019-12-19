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

struct TexLoadData {
  int2 tex_dims1;
  int2 tex_dims2;
  int2 tex_dims3;
  int2 tex_dims4;
};

Texture2D                   Tex0 : register(t0);
Texture2D                   Tex1 : register(t1);
Texture2D                   Tex2 : register(t2);
Texture2D                   Tex3 : register(t3);

PSOutput psmain(PSInput input)
{
  PSOutput o = (PSOutput)0;

 uint width      = 0;
 uint height     = 0;
 uint num_levels = 1;
 Tex0.GetDimensions(0, width, height, num_levels);

  int2 coord1 = input.TexCoord * int2(width, height);
  int2 coord2 = input.TexCoord * int2(width, height);
  int2 coord3 = input.TexCoord * int2(width, height);
  int2 coord4 = input.TexCoord * int2(width, height);

  o.rt_a += Tex0.Load(int3(coord1, 0));
  o.rt_a += Tex1.Load(int3(coord2, 0));
  o.rt_a += Tex2.Load(int3(coord3, 0));
  o.rt_a += Tex3.Load(int3(coord4, 0));

  return o;
}
