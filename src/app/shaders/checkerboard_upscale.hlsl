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

[numthreads(8, 8, 1)]
void csmain(uint3 dispatch_id : SV_DispatchThreadID)
{
    // We will write out 4 pixels per thread here (re-assembled quad)
    // per source buffer pixel (2 real + 2 reconstructed)
    if ((dispatch_id.x >= CBResolveInfo.srcWidth) ||
        (dispatch_id.y >= CBResolveInfo.srcHeight)) {
        return;
    }
    
    // Image 0 is at X offset 0, Image 1 is X offset 0.5
    // We don't have have explicit sample location control, otherwise, we could flip-flop
    // the sample locations every other frame. With this method, we burn some pixels at
    // vertical edges :/
    // I did think of possibly doing a horizontally mirrored render every other frame,
    // which would provide perfect pixel interlocking between two frames, but who really
    // would do that (as in, integrating into an engine)?
    //
    //  cbL.1   (cbR+x1).1
    //  cbR.0   cbL.0
    //

    // In Vulkan, the sample index 0 is to the right (0.75, 0.75)
    // of sample index 1 (0.25, 0.25)
    const int kUpperLeftSampleIndex = 1;
    const int kLowerRightSampleIndex = 0;

    const uint2 quarterResPixelLocation = dispatch_id.xy;
    const uint2 upperLeftLocation = dispatch_id.xy * CB_RESOLVE_PIXELS_PER_THREAD_DIM;
    const uint2 upperRightLocation = upperLeftLocation + uint2(1, 0);
    const uint2 lowerLeftLocation = upperLeftLocation + uint2(0, 1);
    const uint2 lowerRightLocation = upperLeftLocation + uint2(1, 1);

    // TODO: Re-wire debug horizontal offsets

    float4 upperLeftColor = float4(0, 0, 0, 0);
    {
        int2 quarterResCoord = quarterResPixelLocation;
        quarterResCoord.x += CBResolveInfo.ulXOffset * CB_RESOLVE_DEBUG;
        upperLeftColor = cbLeft.Load(quarterResCoord, kUpperLeftSampleIndex);
    }

    float4 upperRightColor = float4(0, 0, 0, 0);
    {
        // The 'upper-left' pixel of the right CB image, one column to the right
        // Refer to docs/CHECKERBOARD.md for more info.
        int2 quarterResCoord = quarterResPixelLocation + int2(1,0);
        quarterResCoord.x += CBResolveInfo.urXOffset * CB_RESOLVE_DEBUG;
        upperRightColor = cbRight.Load(quarterResCoord, kUpperLeftSampleIndex);
    }

    float4 lowerLeftColor = float4(0, 0, 0, 0);
    {
        int2 quarterResCoord = quarterResPixelLocation;
        quarterResCoord.x += CBResolveInfo.llXOffset * CB_RESOLVE_DEBUG;
        lowerLeftColor = cbRight.Load(quarterResCoord, kLowerRightSampleIndex);
    }

    float4 lowerRightColor = float4(0, 0, 0, 0);
    {
        int2 quarterResCoord = quarterResPixelLocation;
        quarterResCoord.x += CBResolveInfo.lrXOffset * CB_RESOLVE_DEBUG;
        lowerRightColor = cbLeft.Load(quarterResCoord, kLowerRightSampleIndex);
    }

    out_texture[upperLeftLocation] = upperLeftColor;
    out_texture[upperRightLocation] = upperRightColor;
    out_texture[lowerLeftLocation] = lowerLeftColor;
    out_texture[lowerRightLocation] = lowerRightColor;
}
