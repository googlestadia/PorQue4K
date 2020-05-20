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

// TODO: The previous implementation ran one thread per output pixel
// This is now one thread per re-assembled quad of pixels of the interlocked
// checkerboard. Sped up by 50%, and that is just doing the simple reads.
// RGP reports this as ~60 us, which is closer (but not quite at) bandwidth
// which would be around ~40 us. This should be bandwidth limited...
// I'd kinda like to keep a 1-pixel-per-thread implementation hanging around
// for experimentation

#include "ConstantBufferStructs.h"
#include "SharedShaderConstants.h"

ConstantBuffer<CBResolveData> CBResolveInfo : register(b0);

Texture2DMS<float4> cbLeft : register(t1);
Texture2DMS<float4> cbRight : register(t2);
RWTexture2D<float4> out_texture : register(u3);

static const uint kSampleModeViewportJitter = 0;
static const uint kSampleModeCustomSampleLocs = 1;

static const uint kUpperLeftQuadrantIndex = 0;
static const uint kUpperRightQuadrantIndex = 1;
static const uint kLowerLeftQuadrantIndex = 2;
static const uint kLowerRightQuadrantIndex = 3;

static const int kUpperSampleIndex = 1;
static const int kLowerSampleIndex = 0;

// In Vulkan, the sample index 0 is to the right (0.75, 0.75)
// of sample index 1 (0.25, 0.25), assuming default MSAA 2x
// sample positions

// Jittered samples
// We offset the 'right' checkerboard image 0.5 pixels to the right.
// In order to reconstruct our quad, we have to 'reach over' to the next
// column of cbRight pixels in-order to reconstruct the quad.
// Reconstructed quad:
//  cbL.1   (cbR+x1).1
//  cbR.0   cbL.0
// I had the stupid idea of generating a horizontal 'mirrored' image for the
// cbRight frame, which would give me 'perfect' pixel coverage, but who the
// hell would ever want to integrate that shit into their engine?

// Custom Sample Locations
// We toggle the horizontal aspect of the two samples between frames,
// but retain the vertical alignment. This is done to be able to use
// the same sample index between the jittered or custom sample path
// Reconstructed quad:
// cbL.1  cbR.1
// cbR.0  cbL.0

// clang-format off
[numthreads(8, 8, 1)]
void csmain(uint3 dispatch_id : SV_DispatchThreadID) // clang-format on
{
    // We will write out 4 pixels per thread here (re-assembled quad)
    // per source buffer pixel (2 real + 2 reconstructed)
    if ((dispatch_id.x >= CBResolveInfo.srcWidth) ||
        (dispatch_id.y >= CBResolveInfo.srcHeight))
    {
        return;
    }

    const uint2 quarterResPixelLocation = dispatch_id.xy;

    float4 upperLeftColor;
    {
        int2 quarterResCoord = quarterResPixelLocation;
        quarterResCoord.x += CBResolveInfo.ulXOffset * CB_RESOLVE_DEBUG;
        upperLeftColor = cbLeft.Load(quarterResCoord, kUpperSampleIndex);
    }
    float4 upperRightColor;
    {
        int2 quarterResCoord = quarterResPixelLocation;
        quarterResCoord.x += uint(CBResolveInfo.sampleGenMode == kSampleModeViewportJitter);
        quarterResCoord.x += CBResolveInfo.urXOffset * CB_RESOLVE_DEBUG;
        upperRightColor = cbRight.Load(quarterResCoord, kUpperSampleIndex);
    }
    float4 lowerLeftColor;
    {
        int2 quarterResCoord = quarterResPixelLocation;
        quarterResCoord.x += CBResolveInfo.llXOffset * CB_RESOLVE_DEBUG;
        lowerLeftColor = cbRight.Load(quarterResCoord, kLowerSampleIndex);
    }
    float4 lowerRightColor;
    {
        int2 quarterResCoord = quarterResPixelLocation;
        quarterResCoord.x += CBResolveInfo.lrXOffset * CB_RESOLVE_DEBUG;
        lowerRightColor = cbLeft.Load(quarterResCoord, kLowerSampleIndex);
    }

    const uint2 outQuadUpperLeft = dispatch_id.xy * CB_RESOLVE_PIXELS_PER_THREAD_DIM;
    const uint2 outQuadUpperRight = outQuadUpperLeft + uint2(1, 0);
    const uint2 outQuadLowerLeft = outQuadUpperLeft + uint2(0, 1);
    const uint2 outQuadLowerRight = outQuadUpperLeft + uint2(1, 1);

    out_texture[outQuadUpperLeft] = upperLeftColor;
    out_texture[outQuadUpperRight] = upperRightColor;
    out_texture[outQuadLowerLeft] = lowerLeftColor;
    out_texture[outQuadLowerRight] = lowerRightColor;
}
