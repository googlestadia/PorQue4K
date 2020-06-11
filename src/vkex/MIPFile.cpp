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

#include "MIPFile.h"

#include <fstream>

const uint32_t kFileSignature = MIP_FILE_SIGNATURE;
const uint32_t kDataSignature = MIP_DATA_SIGNATURE;
const uint32_t kInfoSignature = MIP_INFO_SIGNATURE;

static void StreamWrite(std::ostream& os, const char* data, size_t size)
{
  os.write(data, size);
}

static void StreamRead(std::istream& is, char* data, size_t size)
{
  is.read(data, size);
}

uint32_t MIPFormatComponentCount(MIPPixelFormat format)
{
  uint32_t count = 0;
  switch (format) {
    default: break;
    case MIP_PIXEL_FORMAT_R8_UINT            : count = 1; break;
    case MIP_PIXEL_FORMAT_R8G8_UINT          : count = 2; break;
    case MIP_PIXEL_FORMAT_R8G8B8_UINT        : count = 3; break;
    case MIP_PIXEL_FORMAT_R8G8B8A8_UINT      : count = 4; break;
    case MIP_PIXEL_FORMAT_R16_UINT           : count = 1; break;
    case MIP_PIXEL_FORMAT_R16G16_UINT        : count = 2; break;
    case MIP_PIXEL_FORMAT_R16G16B16_UINT     : count = 3; break;
    case MIP_PIXEL_FORMAT_R16G16B16A16_UINT  : count = 4; break;
    case MIP_PIXEL_FORMAT_R16_FLOAT          : count = 1; break;
    case MIP_PIXEL_FORMAT_R16G16_FLOAT       : count = 2; break;
    case MIP_PIXEL_FORMAT_R16G16B16_FLOAT    : count = 3; break;
    case MIP_PIXEL_FORMAT_R16G16B16A16_FLOAT : count = 4; break;
    case MIP_PIXEL_FORMAT_R32_FLOAT          : count = 1; break;
    case MIP_PIXEL_FORMAT_R32G32_FLOAT       : count = 2; break;
    case MIP_PIXEL_FORMAT_R32G32B32_FLOAT    : count = 3; break;
    case MIP_PIXEL_FORMAT_R32G32B32A32_FLOAT : count = 4; break;
  }
  return count;
}

bool MIPWriteFile(const char* file_path, const MIPFile& mip_file)
{
  std::ofstream os(file_path, std::ios::binary);
  if (!os.is_open()) {
    return false;
  }

  // File signature
  StreamWrite(os, reinterpret_cast<const char*>(&kFileSignature), sizeof(kFileSignature));
  // Pixel format
  StreamWrite(os, reinterpret_cast<const char*>(&mip_file.pixel_format), sizeof(mip_file.pixel_format));
  // Level count
  StreamWrite(os, reinterpret_cast<const char*>(&mip_file.level_count), sizeof(mip_file.level_count));
  // Reserved
  StreamWrite(os, reinterpret_cast<const char*>(&mip_file.reserved), sizeof(mip_file.reserved));

  // Write MIP infos
  {
    // Write MIP info signature
    StreamWrite(os, reinterpret_cast<const char*>(&kInfoSignature), sizeof(kInfoSignature));

    // Write MIP info for each level
    for (uint32_t level = 0; level < mip_file.level_count; ++level) {
      const MIPInfo* p_info = &mip_file.infos[level];
      StreamWrite(os, reinterpret_cast<const char*>(&p_info->level), sizeof(p_info->level));
      StreamWrite(os, reinterpret_cast<const char*>(&p_info->data_offset), sizeof(p_info->data_offset));
      StreamWrite(os, reinterpret_cast<const char*>(&p_info->data_size), sizeof(p_info->data_size));
      StreamWrite(os, reinterpret_cast<const char*>(&p_info->width), sizeof(p_info->width));
      StreamWrite(os, reinterpret_cast<const char*>(&p_info->height), sizeof(p_info->height));
      StreamWrite(os, reinterpret_cast<const char*>(&p_info->row_stride), sizeof(p_info->row_stride));      
    }
  }

  // Data
  {
    // Data Signature
    size_t file_offset = os.tellp();
    StreamWrite(os, reinterpret_cast<const char*>(&kDataSignature), sizeof(kDataSignature));
    // Data Size
    const uint64_t data_size = static_cast<uint64_t>(mip_file.data.size());
    file_offset = os.tellp();
    // Data
    if (data_size > 0) {
      const char* data = reinterpret_cast<const char*>(mip_file.data.data());
      StreamWrite(os, data, data_size);
    }
  }

  os.close();

  return true;
}

bool MIPLoadFile(const char* file_path, MIPFile* p_mip_file)
{
  std::ifstream is(file_path, std::ios::binary);
  if (!is.is_open()) {
    return false;
  }

  // File Signature
  StreamRead(is, reinterpret_cast<char*>(&p_mip_file->file_signature), sizeof(kFileSignature));
  // Pixel format
  StreamRead(is, reinterpret_cast<char*>(&p_mip_file->pixel_format), sizeof(p_mip_file->pixel_format));
  // Level count
  StreamRead(is, reinterpret_cast<char*>(&p_mip_file->level_count), sizeof(p_mip_file->level_count));
  // Reserved
  StreamRead(is, reinterpret_cast<char*>(&p_mip_file->reserved), sizeof(p_mip_file->reserved));

  // Load MIP infos
  uint64_t total_data_size = 0;
  {
    // Load MIP info signature
    StreamRead(is, reinterpret_cast<char*>(&p_mip_file->info_signature), sizeof(kInfoSignature));

    // Load MIP info for each level
    for (uint32_t level = 0; level < p_mip_file->level_count; ++level) {
      MIPInfo* p_info = &p_mip_file->infos[level];
      StreamRead(is, reinterpret_cast<char*>(&p_info->level), sizeof(p_info->level));
      StreamRead(is, reinterpret_cast<char*>(&p_info->data_offset), sizeof(p_info->data_offset));
      StreamRead(is, reinterpret_cast<char*>(&p_info->data_size), sizeof(p_info->data_size));
      StreamRead(is, reinterpret_cast<char*>(&p_info->width), sizeof(p_info->width));
      StreamRead(is, reinterpret_cast<char*>(&p_info->height), sizeof(p_info->height));
      StreamRead(is, reinterpret_cast<char*>(&p_info->row_stride), sizeof(p_info->row_stride));
      // Accumulate data size
      total_data_size += p_info->data_size;
    }
  }

  // Load data
  {
    p_mip_file->data.resize(total_data_size);
    
    // Data Signature
    size_t file_offset = is.tellg();
    StreamRead(is, reinterpret_cast<char*>(&p_mip_file->data_signature), sizeof(kDataSignature));
    // Data
    file_offset = is.tellg();
    if (total_data_size > 0) {
      char* data = reinterpret_cast<char*>(p_mip_file->data.data());
      StreamRead(is, data, total_data_size);
    }
  }

  return true;
}

//bool LoadMIPFile(const char* file_path, MIPFile* p_mip_file)
//{
//}
