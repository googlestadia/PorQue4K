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

RWStructuredBuffer<uint> buffer_out : register(u0);

[numthreads(64, 1, 1)]
void csmain(uint3 group_id : SV_GroupID, uint group_index :SV_GroupIndex)
{
    buffer_out[group_id.x * 64 + group_index] = 1;
}
