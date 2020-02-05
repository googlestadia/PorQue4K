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

struct CASData {
  uint4 const0;
  uint4 const1;
};

#endif // __CONSTANT_BUFFER_STRUCTS_H__
