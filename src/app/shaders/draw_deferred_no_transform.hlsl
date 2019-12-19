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
};

VSOutput vsmain(VSInput input)
{
  VSOutput output = (VSOutput)0;
  output.Position = float4(input.Position, 1);
  return output;
}

struct PSInput {
  float4 Position : SV_Position;
};

struct PSOutput {
  float4 rt_a : SV_Target0;
  float4 rt_b : SV_Target1;
  float4 rt_c : SV_Target2;
  float4 rt_d : SV_Target3;
};

PSOutput psmain(PSInput input)
{
  PSOutput o = (PSOutput)0;

  o.rt_a = float4(1, 0, 0, 1);
  o.rt_b = float4(0, 1, 0, 1);
  o.rt_c = float4(0, 0, 1, 1);
  o.rt_d = float4(1, 1, 0, 1);

  return o;
}
