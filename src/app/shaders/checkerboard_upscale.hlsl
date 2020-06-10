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
#include "SharedShaderConstants.h"

ConstantBuffer<CBResolveData> CBResolveInfo : register(b0);

// TODO: Can I use depth in this resolve? Do I want to use depth?
// TODO: Can I add object/primitive IDs to this resolve? No VK-side support yet
Texture2DMS<float4> currentColor : register(t1);
Texture2DMS<float2> currentVelocity : register(t2);
Texture2D<float4> previousResolvedColor : register(t3);
RWTexture2D<float4> currentResolvedColor : register(u4);

static const uint kSampleModeViewportJitter = 0;
static const uint kSampleModeCustomSampleLocs = 1;

// In Vulkan, the sample index 0 (0.75, 0.75) is below sample
// index 1 (0.25, 0.25), assuming default MSAA 2x sample positions
static const int kTopSampleIndex = 1;
static const int kBottomSampleIndex = 0;

// Viewport Jittered samples
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

// Top, Bottom = Vertical alignment of samples in pixel quad
// upper, lower, left, right = pixel quad neighbors used for reconstruction

// TODO: The previous implementation ran one thread per output pixel
// This is now one thread per re-assembled quad of pixels of the interlocked
// checkerboard. Sped up by 50%, and that is just doing the simple reads.
// RGP reports this as ~60 us, which is closer (but not quite at) bandwidth
// which would be around ~40 us. This should be bandwidth limited...
// I'd kinda like to keep a 1-pixel-per-thread implementation hanging around
// for experimentation

// TODO: Can I use 16-bit floats for some parts of the code here?

// TODO: How could I use LDS here? Could I pre-load current color and velocity
// to enable quicker lookups from neighboring threads in the same threadgroup?

void GetHorizontalNeighborOffsets(out int topHorizNeighborOffset, out int bottomHorizNeighborOffset)
{
    // This code avoids branches at the cost of readability. We have a 4-sample
    // neighborhood around every reconstructed sample. Since we're working at a
    // quad level, the two reconstructed samples share the real neighbors as
    // part of the neighborhood.
    //
    // CB Index = 0                       |   CB Index = 1
    //                 x,y-1,0            |           x,y-1,0
    //         |x,y,1      Top| x+1,y,1   |   x-1,y,1|Top       x,y,1|
    // x-1,y,0 |Bottom   x,y,0|           |          |x,y,0    Bottom| x+1,y,1
    //          x,y+1,1                   |                   x,y+1,1
    //
    // The diagram shows the upper and lower quad neighbors always have the same
    // y-offset and sample indices. However, the horizontal (left + right) neighbors
    // toggle their association between the reconstructed upper and lower samples.
    // Since they keep the same sample indices, we can just toggle the x-offset
    // based on checkerboard frame index. The offset is either -1 or 1, and flipped
    // between the horizontal neighbors.

    topHorizNeighborOffset = 1 - (2 * CBResolveInfo.cbIndex);
    bottomHorizNeighborOffset = -topHorizNeighborOffset;
}

// TODO: Do I want to reproject the 'real' samples as well? Do some sort of blend
// between the real and reprojected samples (heavily weighted toward real).
// And what velocity to use? Some papers I've seen show the 'real' color has an
// extended color + velocity neighborhood to filter from (4 corners + real)
// TODO: Do I want to 'filter' the velocities? I don't have object/primitive information
// so I don't really have any smart way to figure out which velocities are 'better' besides
// the color data I have
void GenVelocityForReconstructedSamples(const int2 quarterResPixelLocation, out float2 topVelocitySS, out float2 bottomVelocitySS)
{
    float2 centerTopVelocity = currentVelocity.Load(quarterResPixelLocation, kTopSampleIndex);
    float2 centerBottomVelocity = currentVelocity.Load(quarterResPixelLocation, kBottomSampleIndex);
    float2 summedCenterVelocity = centerTopVelocity + centerBottomVelocity;

    float2 upperNeighborVelocity = currentVelocity.Load(quarterResPixelLocation + int2(0, -1), kBottomSampleIndex);
    float2 lowerNeighborVelocity = currentVelocity.Load(quarterResPixelLocation + int2(0, 1), kTopSampleIndex);

    int topHorizNeighborOffset, bottomHorizNeighborOffset;
    GetHorizontalNeighborOffsets(topHorizNeighborOffset, bottomHorizNeighborOffset);
    float2 topHorizNeighborVelocity = currentVelocity.Load(quarterResPixelLocation + int2(topHorizNeighborOffset, 0), kTopSampleIndex);
    float2 bottomHorizNeighborVelocity = currentVelocity.Load(quarterResPixelLocation + int2(bottomHorizNeighborOffset, 0), kBottomSampleIndex);

    float2 topSummedVelocity = summedCenterVelocity + upperNeighborVelocity + topHorizNeighborVelocity;
    float2 bottomSummedVelocity = summedCenterVelocity + lowerNeighborVelocity + bottomHorizNeighborVelocity;

    float2 topAvgVelocityNDC = topSummedVelocity * 0.25f;
    float2 bottomAvgVelocityNDC = bottomSummedVelocity * 0.25f;

    // Motion vectors are in NDC space (with Y-up). X and Y go from
    // -1 to 1, and we need to scale it to destination screenspace
    // to do the relative translation of texels, and flip Y.
    const float2 destScreenScaler = float2(CBResolveInfo.srcWidth * CB_RESOLVE_PIXELS_PER_THREAD_DIM * 0.5f,
                                           CBResolveInfo.srcHeight * CB_RESOLVE_PIXELS_PER_THREAD_DIM * -0.5f);
    topVelocitySS = topAvgVelocityNDC * destScreenScaler;
    bottomVelocitySS = bottomAvgVelocityNDC * destScreenScaler;
}

// clang-format off
[numthreads(8, 8, 1)]
void csmain(uint3 dispatch_id : SV_DispatchThreadID) // clang-format on
{
    if ((dispatch_id.x >= CBResolveInfo.srcWidth) ||
        (dispatch_id.y >= CBResolveInfo.srcHeight))
    {
        return;
    }

    // Write 4 pixels per-thread/per-source-pixel (2 real + 2 reconstructed)

    const int2 quarterResPixelLocation = dispatch_id.xy;

    int topRealXOffset = CBResolveInfo.cbIndex * 1;
    int topReconXOffset = 1 - topRealXOffset;

    const int2 fullResUpperLeft = quarterResPixelLocation * CB_RESOLVE_PIXELS_PER_THREAD_DIM;
    int2 realTopPos = fullResUpperLeft + int2(topRealXOffset, 0);
    int2 reconTopPos = fullResUpperLeft + int2(topReconXOffset, 0);
    int2 realBottomPos = reconTopPos + int2(0, 1);
    int2 reconBottomPos = realTopPos + int2(0, 1);

    float4 realTopColor = currentColor.Load(quarterResPixelLocation, kTopSampleIndex);
    float4 realBottomColor = currentColor.Load(quarterResPixelLocation, kBottomSampleIndex);

    // color bounding box + filtered color
    // TODO: Color operations in YCoCg space?
    float3 centerTopColor = realTopColor.rgb;
    float3 centerBottomColor = realBottomColor.rgb;

    float3 upperNeighborColor = (currentColor.Load(quarterResPixelLocation + int2(0, -1), kBottomSampleIndex)).rgb;
    float3 lowerNeighborColor = (currentColor.Load(quarterResPixelLocation + int2(0, 1), kTopSampleIndex)).rgb;

    int topHorizNeighborOffset, bottomHorizNeighborOffset;
    GetHorizontalNeighborOffsets(topHorizNeighborOffset, bottomHorizNeighborOffset);
    float3 topHorizNeighborColor = (currentColor.Load(quarterResPixelLocation + int2(topHorizNeighborOffset, 0), kTopSampleIndex)).rgb;
    float3 bottomHorizNeighborColor = (currentColor.Load(quarterResPixelLocation + int2(bottomHorizNeighborOffset, 0), kBottomSampleIndex)).rgb;

    float3 centerColorMin = min(centerTopColor, centerBottomColor);
    float3 topColorMin = min(centerColorMin, min(upperNeighborColor, topHorizNeighborColor));
    float3 bottomColorMin = min(centerColorMin, min(lowerNeighborColor, bottomHorizNeighborColor));
    float3 centerColorMax = max(centerTopColor, centerBottomColor);
    float3 topColorMax = max(centerColorMax, max(upperNeighborColor, topHorizNeighborColor));
    float3 bottomColorMax = max(centerColorMax, max(lowerNeighborColor, bottomHorizNeighborColor));

    // TODO: Improve filtering algorithm aka use any filtering algorithm :p
    // Could try to do some voting based on UP/DOWN vs LEFT/RIGHT
    float3 summedCenterColors = centerTopColor + centerBottomColor;
    float3 filteredTopColor = (summedCenterColors + upperNeighborColor + topHorizNeighborColor) * 0.25f;
    float3 filteredBottomColor = (summedCenterColors + lowerNeighborColor + bottomHorizNeighborColor) * 0.25f;

    // Reprojected previous color via motion vectors
    float2 reconTopVelocitySS, reconBottomVelocitySS;
    GenVelocityForReconstructedSamples(quarterResPixelLocation, reconTopVelocitySS, reconBottomVelocitySS);

    float3 prevReconTopColor, prevReconBottomColor;
    {
        float2 reconTopPosCenter = float2(reconTopPos) + float2(0.5f, 0.5f);
        int2 previousReconTopPos = floor(reconTopPosCenter - reconTopVelocitySS);
        prevReconTopColor = (previousResolvedColor[previousReconTopPos]).rgb;
    }
    {
        float2 reconBottomPosCenter = float2(reconBottomPos) + float2(0.5f, 0.5f);
        int2 previousReconBottomPos = floor(reconBottomPosCenter - reconBottomVelocitySS);
        prevReconBottomColor = (previousResolvedColor[previousReconBottomPos]).rgb;
    }

    float3 clampedPrevReconTopColor = clamp(prevReconTopColor, topColorMin, topColorMax);
    float3 clampedPrevReconBottomColor = clamp(prevReconBottomColor, bottomColorMin, bottomColorMax);


    // TODO: What's the right value to blend between the filtered color and the reconstructured color?
    // TODO: What is the 'right' velocity length to use to decide if something is moving or not?
    // Perhaps add a debug control to mess with the lerp?
    float4 reconTopColor = float4(0, 0, 0, 1);
    float reconTopVelocityLen = length(reconTopVelocitySS);
    if (reconTopVelocityLen > 0.25)
    {
        reconTopColor.rgb = lerp(filteredTopColor, clampedPrevReconTopColor, 0.5f);
    }
    else
    {
        reconTopColor.rgb = prevReconTopColor;
    }

    float4 reconBottomColor = float4(0, 0, 0, 1);
    float reconBottomVelocityLen = length(reconBottomVelocitySS);
    if (reconBottomVelocityLen > 0.25)
    {
        reconBottomColor.rgb = lerp(filteredBottomColor, clampedPrevReconBottomColor, 0.5f);
    }
    else
    {
        reconBottomColor.rgb = prevReconBottomColor;
    }

    currentResolvedColor[realTopPos] = realTopColor;
    currentResolvedColor[realBottomPos] = realBottomColor;
    currentResolvedColor[reconTopPos] = reconTopColor;
    currentResolvedColor[reconBottomPos] = reconBottomColor;
}
