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

struct TexDimData {
    uint srcWidth, srcHeight;
    uint dstWidth, dstHeight;
};

struct ImageDeltaOptions {
    uint vizMode;
};

ConstantBuffer<TexDimData> TexDims : register(b0);
ConstantBuffer<ImageDeltaOptions> Options : register(b1);

Texture2D<float4> internal_res_texture : register(t2);
Texture2D<float4> target_res_texture : register(t3);
RWTexture2D<float4> out_texture : register(u4);

static const float3 luma_consts = float3(0.2126, 0.7152, 0.0722);

static const uint kDisabledViz = 0;
static const uint kLumaDelta = 1;
static const uint kRGBDelta = 2;

[numthreads(16, 16, 1)]
void csmain(uint3 dispatch_id : SV_DispatchThreadID)
{
    if ((dispatch_id.x >= TexDims.dstWidth) ||
        (dispatch_id.y >= TexDims.dstHeight)) {
        return;
    }

    uint2 src_coords = dispatch_id.xy;

    float4 internalPixel = internal_res_texture[src_coords];
    float4 nativePixel = target_res_texture[dispatch_id.xy];

    float3 outColor;

    if (kLumaDelta == Options.vizMode) {
        float internalLuma = dot(internalPixel.rgb, luma_consts);
        float nativeLuma = dot(nativePixel.rgb, luma_consts);

        float lumaDelta = abs(internalLuma - nativeLuma);

        outColor = float3(lumaDelta, lumaDelta, lumaDelta);
    } else if (kRGBDelta == Options.vizMode) {
        outColor = abs(internalPixel.rgb - nativePixel.rgb);
    } else {
        outColor = internalPixel.rgb;
    }


    out_texture[dispatch_id.xy] = float4(outColor, 0.f);
}