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

#define A_GPU 1
#define A_HLSL 1

ConstantBuffer<CASData> CASValues : register(b0);

Texture2D<float4> in_texture : register(t1);
RWTexture2D<float4> out_texture : register(u2);

#include "ffx_a.h"

AF3 CasLoad(in int2 ip)
{
    return in_texture[ip].rgb;
}

void CasInput(inout float r, inout float g, inout float b) {}

#include "ffx_cas.h"

// clang-format off
[numthreads(64, 1, 1)]
void csmain(uint3 invocation_id : SV_GroupThreadID, uint3 workgroup_id : SV_GroupID) // clang-format on
{
    AU4 const0 = CASValues.const0;
    AU4 const1 = CASValues.const1;

    // Do remapping of local xy in workgroup for a more PS-like swizzle pattern
    AU2 gxy = ARmp8x8(invocation_id.x) + AU2(workgroup_id.x << 4u, workgroup_id.y << 4u);
    // Filter.
    AF4 c;
    CasFilter(c.r, c.g, c.b, gxy, const0, const1, false);
    out_texture[ASU2(gxy)] = c;
    gxy.x += 8u;

    CasFilter(c.r, c.g, c.b, gxy, const0, const1, false);
    out_texture[ASU2(gxy)] = c;
    gxy.y += 8u;

    CasFilter(c.r, c.g, c.b, gxy, const0, const1, false);
    out_texture[ASU2(gxy)] = c;
    gxy.x -= 8u;

    CasFilter(c.r, c.g, c.b, gxy, const0, const1, false);
    out_texture[ASU2(gxy)] = c;
}
