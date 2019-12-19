/*
 Copyright 2018-2019 Google Inc.
 
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

#include <vkex/CpuResource.h>

namespace vkex {

// =================================================================================================
// ElementDataStore
// =================================================================================================
ElementDataStore::ElementDataStore()
{
}

ElementDataStore::~ElementDataStore()
{
}

bool ElementDataStore::IsEmpty() const
{
  return m_data.empty();
}

size_t ElementDataStore::GetElementCount() const
{
  return m_element_count;
}

size_t ElementDataStore::GetDataSize() const
{
  size_t size = m_data.size();
  return size;
}

uint8_t* ElementDataStore::GetData()
{
  uint8_t* p_data = m_data.data();
  return p_data;
}

const uint8_t* ElementDataStore::GetData() const
{
  const uint8_t* p_data = m_data.data();
  return p_data;
}

// =================================================================================================
// VertexDataStore
// =================================================================================================
VertexBufferData::VertexBufferData()
{
}

VertexBufferData::~VertexBufferData()
{
}

const vkex::VertexBindingDescription& VertexBufferData::GetVertexBindingDescription() const
{
  return m_vertex_description;
}

void VertexBufferData::SetVertexBindingDescription(const vkex::VertexBindingDescription& vertex_description)
{
  m_vertex_description = vertex_description;
}

// =================================================================================================
// IndexDataStore
// =================================================================================================
IndexBufferData::IndexBufferData()
{
}

IndexBufferData::~IndexBufferData()
{
}

bool IndexBufferData::IsValid() const
{
  bool is_uint16 = (m_index_type == VK_INDEX_TYPE_UINT16);
  bool is_uint32 = (m_index_type == VK_INDEX_TYPE_UINT32);
  return is_uint16 || is_uint32;
}

VkIndexType IndexBufferData::GetIndexType() const 
{
  return m_index_type;
}

} // namespace vkex