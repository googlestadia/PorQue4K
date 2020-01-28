#ifndef __CONSTANT_BUFFER_STRUCTS_H__
#define __CONSTANT_BUFFER_STRUCTS_H__

struct ViewTransformData {
    float4x4 ModelViewProjectionMatrix;
};

struct ScaledTexCopyDimensionsData {
    uint srcWidth, srcHeight;
    uint dstWidth, dstHeight;
};

struct ImageDeltaOptions {
    uint vizMode;
};

#endif // __CONSTANT_BUFFER_STRUCTS_H__
