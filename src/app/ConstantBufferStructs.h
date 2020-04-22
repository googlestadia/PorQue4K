#ifndef __CONSTANT_BUFFER_STRUCTS_H__
#define __CONSTANT_BUFFER_STRUCTS_H__

#include "SharedShaderConstants.h"

struct GPULightInfo
{
    float3        direction;
    int padding1;
    float3        color;
    int padding2;
    float         intensity;

    //int           type;

    // TODO: other fields once we have types
};

struct PerFrameConstantData {
    float4x4 viewProjectionMatrix;
    float3   cameraPos;
#if (CB_RESOLVE_DEBUG > 0)
    float    texGradScaler;
#else
    int      padding;
#endif

    GPULightInfo dirLight;
};

struct PerObjectConstantData {
    float4x4 worldMatrix;

    // material constants
    float4 baseColorFactor;
    float3 emissiveFactor;
    uint padding1;

    float metallicFactor;
    float roughnessFactor;
    uint2 padding2;
};

struct ScaledTexCopyDimensionsData {
    uint srcWidth, srcHeight;
    uint dstWidth, dstHeight;
};

struct ImageDeltaOptions {
    uint vizMode;
    float deltaAmplifier;
    uint2 padding1;
};

struct CASData {
  uint4 const0;
  uint4 const1;
};

struct CBResolveData {
    uint srcWidth, srcHeight;
    int ulXOffset;
    int urXOffset;
    int llXOffset;
    int lrXOffset;
    uint2 padding1;
};

#endif // __CONSTANT_BUFFER_STRUCTS_H__
