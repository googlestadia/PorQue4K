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

struct TexDimData {
  int width;
  int height;
};

ConstantBuffer<TexDimData> TexDims : register(b0);
RWStructuredBuffer<float4> BufferOut : register(u1);

// clang-format off
[numthreads(16, 16, 1)]
void csmain(uint3 thread_id : SV_DispatchThreadID) 
{
  float width = (float)TexDims.width;
  float height = (float)TexDims.height;
  BufferOut[thread_id.x + thread_id.y * 1920] = float4((thread_id.x / width + thread_id.y / height) / 2, 0, 0, 1);
}
// clang-format on
