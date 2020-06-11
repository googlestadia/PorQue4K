/*
 Copyright 2018-2020 Google Inc.
 
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

/*

MIP File Format
--------------------------------------------------------------------------------
Section                 | Type     | Count | # Bytes | Details
--------------------------------------------------------------------------------
MIP File Signature      | char     | 4     | 4       |'FPIM' aka MIPF backwards 
Pixel Format            | uint32_t | 1     | 4       | See Pixel Format Table
MIP Level Count         | uint32_t | 1     | 4       | N = Mip Level Count, N <= 32
Reserved                | uint32_t | 16    | 64      | Reserved for future use
MIP Info Signature      | char     | 4     | 4       |'IPIM' aka MIPI backwards
MIP Infos for N Levels  | MIP Info | N     | N*36    | 
MIP Data Signature      | char     | 4     | 4       |'DPIM' aka MIPD backwards
MIP Data for Level 0    | char     | -     | -       | MIP level 0 data
...                     | char     | -     | -       | ...
MIP Data for Level N-1  | char     | -     | -       | MIP level 0 data
--------------------------------------------------------------------------------


MIP Info Structure
--------------------------------------------------------------------------------
Field           | Type     | Count | # Bytes | Details
--------------------------------------------------------------------------------
Level           | uint32_t | 1     | 4       | MIP level
Data Offset     | uint64_t | 1     | 8       | Data offset in bytes
Data Size       | uint64_t | 1     | 8       | Data size in bytes
Width           | uint32_t | 1     | 4       | Width
Height          | uint32_t | 1     | 4       | Height
Row Stride      | uint32_t | 1     | 4       | Row stride in bytes
--------------------------------------------------------------------------------


MIP Data Structure
--------------------------------------------------------------------------------
Field           | Type     | Count | # Bytes | Details
--------------------------------------------------------------------------------
Data            | uint8_t  | N     | N       | Row Stride * Height
--------------------------------------------------------------------------------


Pixel Format Table
--------------------------------------------------------------------------------
Format ID | Type      | # Channels | # Bytes | Format Name
--------------------------------------------------------------------------------
0         | n/a       | n/a        | n/a     | Undefined
--------------------------------------------------------------------------------
1         | uint8_t   | 1          | 1       | R8_UINT
2         | uint8_t   | 2          | 2       | R8G8_UINT
3         | uint8_t   | 3          | 3       | R8G8B8_UINT
4         | uint8_t   | 4          | 4       | R8G8B8A8_UINT
--------------------------------------------------------------------------------
5         | uint16_t  | 1          | 2       | R16_UINT
6         | uint16_t  | 2          | 4       | R16G16_UINT
7         | uint16_t  | 3          | 6       | R16G16B16_UINT
8         | uint16_t  | 4          | 8       | R16G16B16A16_UINT
--------------------------------------------------------------------------------
9         | float16_t | 1          | 2       | R16_FLOAT
10        | float16_t | 2          | 4       | R16G16_FLOAT
11        | float16_t | 3          | 6       | R16G16B16_FLOAT
12        | float16_t | 4          | 8       | R16G16B16A16_FLOAT
--------------------------------------------------------------------------------
13        | float32_t | 1          | 4       | R32_FLOAT
14        | float32_t | 2          | 8       | R32G32_FLOAT
15        | float32_t | 3          | 12      | R32G32B32_FLOAT
16        | float32_t | 4          | 32      | R32G32B32A32_FLOAT
--------------------------------------------------------------------------------

*/

#ifndef MIPFILE_H
#define MIPFILE_H

#include <cstdint>
#include <vector>

enum MIPPixelFormat {
  MIP_PIXEL_FORMAT_UNDEFINED          =  0,
  MIP_PIXEL_FORMAT_R8_UINT            =  1,
  MIP_PIXEL_FORMAT_R8G8_UINT          =  2,
  MIP_PIXEL_FORMAT_R8G8B8_UINT        =  3,
  MIP_PIXEL_FORMAT_R8G8B8A8_UINT      =  4,
  MIP_PIXEL_FORMAT_R16_UINT           =  5,
  MIP_PIXEL_FORMAT_R16G16_UINT        =  6,
  MIP_PIXEL_FORMAT_R16G16B16_UINT     =  7,
  MIP_PIXEL_FORMAT_R16G16B16A16_UINT  =  8,
  MIP_PIXEL_FORMAT_R16_FLOAT          =  9,
  MIP_PIXEL_FORMAT_R16G16_FLOAT       = 10,
  MIP_PIXEL_FORMAT_R16G16B16_FLOAT    = 11,
  MIP_PIXEL_FORMAT_R16G16B16A16_FLOAT = 12,
  MIP_PIXEL_FORMAT_R32_FLOAT          = 13,
  MIP_PIXEL_FORMAT_R32G32_FLOAT       = 14,
  MIP_PIXEL_FORMAT_R32G32B32_FLOAT    = 15,
  MIP_PIXEL_FORMAT_R32G32B32A32_FLOAT = 16,
};

enum {
  MIP_FILE_SIGNATURE = 0x4650494D,
  MIP_DATA_SIGNATURE = 0x4450494D,
  MIP_INFO_SIGNATURE = 0x4950494D,
  MAX_MIP_LEVELS     = 32
};

struct MIPInfo {
  uint32_t level;
  uint64_t data_offset;
  uint64_t data_size;
  uint32_t width;
  uint32_t height;
  uint32_t row_stride;
};

struct MIPFile {
  char                  file_signature[4];
  uint32_t              pixel_format;
  uint32_t              level_count;
  uint64_t              reserved[16];
  char                  info_signature[4];
  MIPInfo               infos[MAX_MIP_LEVELS];
  char                  data_signature[4];
  std::vector<uint8_t>  data;
};

uint32_t  MIPFormatComponentCount(MIPPixelFormat format);
bool      MIPWriteFile(const char* file_path, const MIPFile& mip_file);
bool      MIPLoadFile(const char* file_path, MIPFile* p_mip_file);

#endif // MIPFILE_H