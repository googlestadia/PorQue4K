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
  float3  Position  : Position;
  float3  VertexColor : VertexColor;
};

struct VSOutput {
  float4  Position  : SV_Position;
  float3  VertexColor : VertexColor;
};

VSOutput vsmain(VSInput input)
{
  VSOutput output = (VSOutput)0;
  output.Position = float4(input.Position, 1);
  output.VertexColor = input.VertexColor;
  return output;
}

struct PSInput {
  float4  Position  : SV_Position;
  float3  VertexColor : VertexColor; 
};

float4 psmain(PSInput input) : SV_Target
{
  return float4(input.VertexColor, 1);
}