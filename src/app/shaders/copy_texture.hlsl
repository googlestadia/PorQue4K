/*
 Copyright 2019-2020 Google Inc.
 
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
    uint srcWidth, srcHeight;
    uint dstWidth, dstHeight;
};

ConstantBuffer<TexDimData> TexDims : register(b0);

Texture2D<float4> in_texture : register(t1);
RWTexture2D<float4> out_texture : register(u2);

[numthreads(16, 16, 1)]
void csmain(uint3 dispatch_id : SV_DispatchThreadID)
{
    if ((dispatch_id.x >= TexDims.dstWidth) ||
        (dispatch_id.y >= TexDims.dstHeight)) {
        return;
    }

    uint2 src_coords;
    src_coords.x = uint((dispatch_id.x * TexDims.srcWidth) / TexDims.dstWidth);
    src_coords.y = uint((dispatch_id.y * TexDims.srcHeight) / TexDims.dstHeight);

    out_texture[dispatch_id.xy] = in_texture[src_coords];
}
