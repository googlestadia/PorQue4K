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

#include <vkex/Geometry.h>

namespace vkex {

// =================================================================================================
// Geometry
// =================================================================================================
Geometry::Geometry()
{
}

Geometry::Geometry(const vkex::VertexBufferData& vertex_data)
{
  m_vertex_data.push_back(vertex_data);
}

Geometry::Geometry(const vkex::VertexBufferData& vertex_data, const vkex::IndexBufferData& index_data)
  : m_index_data(index_data)
{
  m_vertex_data.push_back(vertex_data);
}

Geometry::~Geometry()
{
}

VkPrimitiveTopology Geometry::GetPrimitiveTopology() const
{
  return m_primitive_topology;
}

bool Geometry::HasIndices() const
{
  bool has_indices = !m_index_data.IsEmpty();
  return has_indices;
}

uint32_t Geometry::GetVertexBufferCount() const
{
  uint32_t count = CountU32(m_vertex_data);
  return count;
}

const vkex::VertexBufferData* Geometry::GetVertexBufferByIndex(uint32_t index) const
{
  const vkex::VertexBufferData* p_data = nullptr;
  uint32_t count = CountU32(m_vertex_data);
  if (index < count) {
    p_data = &m_vertex_data[index];
  }
  return p_data;
}

const vkex::VertexBufferData* Geometry::GetVertexBufferByBinding(uint32_t binding) const
{
  const vkex::VertexBufferData* p_data = nullptr;
  auto it = FindIf(
    m_vertex_data,
    [binding](const vkex::VertexBufferData& elem) -> bool {
      return elem.GetVertexBindingDescription().GetBinding() == binding; });
  if (it != std::end(m_vertex_data)) {
    p_data = &(*it);
  }
  return p_data;
}

void Geometry::AddVertexBuffer(const vkex::VertexBufferData& data)
{
  m_vertex_data.push_back(data);
}

const vkex::IndexBufferData* Geometry::GetIndexBuffer() const
{
  return &m_index_data;
}

void Geometry::SetIndexBuffer(const vkex::IndexBufferData& data)
{
  m_index_data = data;
}

// =================================================================================================
// PlatonicSolid
// =================================================================================================
PlatonicSolid::PlatonicSolid()
{
}

PlatonicSolid::PlatonicSolid(const vkex::VertexBufferData& vertex_data)
  : Geometry(vertex_data)
{
}

PlatonicSolid::PlatonicSolid(const vkex::VertexBufferData& vertex_data, const vkex::IndexBufferData& index_data)
  : Geometry(vertex_data, index_data)
{
}

PlatonicSolid::~PlatonicSolid()
{
}

void PlatonicSolid::AppendVertexData(const float2& data, std::vector<float>& vertex_data)
{
  vertex_data.push_back(data.x);
  vertex_data.push_back(data.y);
}

void PlatonicSolid::AppendVertexData(const float3& data, std::vector<float>& vertex_data)
{
  vertex_data.push_back(data.x);
  vertex_data.push_back(data.y);
  vertex_data.push_back(data.z);
}

void PlatonicSolid::AppendVertexData(const float4& data, std::vector<float>& vertex_data)
{
  vertex_data.push_back(data.x);
  vertex_data.push_back(data.y);
  vertex_data.push_back(data.z);
  vertex_data.push_back(data.w);
}

void PlatonicSolid::AppendVertexData(
  const float3&                       position, 
  const float3&                       vertex_color, 
  const float2&                       tex_coord, 
  const float3&                       normal,
  const vkex::PlatonicSolid::Options& options,
  std::vector<float>&                 vertex_data
)
{
  // Position
  AppendVertexData(position, vertex_data);
  // Vertex color
  if (options.vertex_colors) {
    AppendVertexData(vertex_color, vertex_data);
  }
  // Tex coord
  if (options.tex_coords) {
    AppendVertexData(tex_coord, vertex_data);
  }
  // Normal
  if (options.normals) {
    AppendVertexData(normal, vertex_data);
  }
}

PlatonicSolid PlatonicSolid::Cube(const vkex::PlatonicSolid::Options& options)
{
  float3 positions[8] = {
    float3(-0.5f,  0.5f,  0.5f),  // 0: -X,  Y, +Z
    float3(-0.5f, -0.5f,  0.5f),  // 1: -X, -Y, +Z
    float3( 0.5f, -0.5f,  0.5f),  // 2:  X, -Y, +Z
    float3( 0.5f,  0.5f,  0.5f),  // 3:  X,  Y, +Z
    float3( 0.5f,  0.5f, -0.5f),  // 4:  X,  Y,  Z
    float3( 0.5f, -0.5f, -0.5f),  // 5:  X, -Y,  Z
    float3(-0.5f, -0.5f, -0.5f),  // 6: -X, -Y,  Z
    float3(-0.5f,  0.5f, -0.5f),  // 7: -X,  Y,  Z
  };

  float3 colors[6] = {
    float3(1, 0, 0),
    float3(0, 1, 0),
    float3(0, 0, 1),
    float3(1, 1, 0),
    float3(1, 0, 1),
    float3(0, 1, 1),
  };

  float2 uvs[4] = {
    float2(0, 0),
    float2(0, 1),
    float2(1, 1),
    float2(1, 0),
  };

  float3 normals[6] = {
    float3( 0,  0,  1),  // +Z
    float3( 0,  0, -1),  // -Z
    float3( 1,  0,  0),  // +X
    float3(-1,  0,  0),  // -X
    float3( 0,  1,  0),  // +Y
    float3( 0, -1,  0),  // -Y
  };

  vkex::VertexBindingDescription vertex_description = vkex::VertexBindingDescription(0, VK_VERTEX_INPUT_RATE_VERTEX);
  {
    // Location
    uint32_t location = 0;
    // Position
    vertex_description.AddAttribute(location, VK_FORMAT_R32G32B32_SFLOAT);
    ++location;
    // Vertex colors
    if (options.vertex_colors) {
      vertex_description.AddAttribute(location, VK_FORMAT_R32G32B32_SFLOAT);
      ++location;
    }
    // Tex coords
    if (options.tex_coords) {
      vertex_description.AddAttribute(location, VK_FORMAT_R32G32_SFLOAT);
      ++location;
    }
    // Normals
    if (options.normals) {
      vertex_description.AddAttribute(location, VK_FORMAT_R32G32B32_SFLOAT);
      ++location;
    }
  }

  // Vertex data
  std::vector<float> vertex_data;
  {
    // Front +Z (near to camera)
    AppendVertexData(positions[0], colors[0], uvs[0], normals[0], options, vertex_data);
    AppendVertexData(positions[1], colors[0], uvs[1], normals[0], options, vertex_data);
    AppendVertexData(positions[2], colors[0], uvs[2], normals[0], options, vertex_data);
    AppendVertexData(positions[0], colors[0], uvs[0], normals[0], options, vertex_data);
    AppendVertexData(positions[2], colors[0], uvs[2], normals[0], options, vertex_data);
    AppendVertexData(positions[3], colors[0], uvs[3], normals[0], options, vertex_data);
    // Back -Z (far from camera)
    AppendVertexData(positions[4], colors[1], uvs[0], normals[1], options, vertex_data);
    AppendVertexData(positions[5], colors[1], uvs[1], normals[1], options, vertex_data);
    AppendVertexData(positions[6], colors[1], uvs[2], normals[1], options, vertex_data);
    AppendVertexData(positions[4], colors[1], uvs[0], normals[1], options, vertex_data);
    AppendVertexData(positions[6], colors[1], uvs[2], normals[1], options, vertex_data);
    AppendVertexData(positions[7], colors[1], uvs[3], normals[1], options, vertex_data);
    // Right +X
    AppendVertexData(positions[3], colors[2], uvs[0], normals[2], options, vertex_data);
    AppendVertexData(positions[2], colors[2], uvs[1], normals[2], options, vertex_data);
    AppendVertexData(positions[5], colors[2], uvs[2], normals[2], options, vertex_data);
    AppendVertexData(positions[3], colors[2], uvs[0], normals[2], options, vertex_data);
    AppendVertexData(positions[5], colors[2], uvs[2], normals[2], options, vertex_data);
    AppendVertexData(positions[4], colors[2], uvs[3], normals[2], options, vertex_data);
    // Left -X
    AppendVertexData(positions[7], colors[3], uvs[0], normals[3], options, vertex_data);
    AppendVertexData(positions[6], colors[3], uvs[1], normals[3], options, vertex_data);
    AppendVertexData(positions[1], colors[3], uvs[2], normals[3], options, vertex_data);
    AppendVertexData(positions[7], colors[3], uvs[0], normals[3], options, vertex_data);
    AppendVertexData(positions[1], colors[3], uvs[2], normals[3], options, vertex_data);
    AppendVertexData(positions[0], colors[3], uvs[3], normals[3], options, vertex_data);
    // Top +Y
    AppendVertexData(positions[7], colors[4], uvs[0], normals[4], options, vertex_data);
    AppendVertexData(positions[0], colors[4], uvs[1], normals[4], options, vertex_data);
    AppendVertexData(positions[3], colors[4], uvs[2], normals[4], options, vertex_data);
    AppendVertexData(positions[7], colors[4], uvs[0], normals[4], options, vertex_data);
    AppendVertexData(positions[3], colors[4], uvs[2], normals[4], options, vertex_data);
    AppendVertexData(positions[4], colors[4], uvs[3], normals[4], options, vertex_data);
    // Top -Y
    AppendVertexData(positions[1], colors[5], uvs[0], normals[5], options, vertex_data);
    AppendVertexData(positions[6], colors[5], uvs[1], normals[5], options, vertex_data);
    AppendVertexData(positions[5], colors[5], uvs[2], normals[5], options, vertex_data);
    AppendVertexData(positions[1], colors[5], uvs[0], normals[5], options, vertex_data);
    AppendVertexData(positions[5], colors[5], uvs[2], normals[5], options, vertex_data);
    AppendVertexData(positions[2], colors[5], uvs[3], normals[5], options, vertex_data);
  }
  
  vkex::VertexBufferData vertex_buffer = vkex::VertexBufferData(vertex_description, vertex_data);
  PlatonicSolid platonic(vertex_buffer);
  return platonic;
}

// =================================================================================================
// TriangleList
// =================================================================================================
TriangleList::TriangleList(const TriangleList::Options& options)
  : m_options(options)    
{
  vkex::VertexBindingDescription vertex_description = vkex::VertexBindingDescription(0, VK_VERTEX_INPUT_RATE_VERTEX);
  {
    // Location
    uint32_t location = 0;
    // Position
    vertex_description.AddAttribute(location, VK_FORMAT_R32G32B32_SFLOAT);
    ++location;
    // Vertex colors
    if (m_options.vertex_colors) {
      vertex_description.AddAttribute(location, VK_FORMAT_R32G32B32_SFLOAT);
      ++location;
    }
    // Tex coords
    if (m_options.tex_coords) {
      vertex_description.AddAttribute(location, VK_FORMAT_R32G32_SFLOAT);
      ++location;
    }
    // Normals
    if (m_options.normals) {
      vertex_description.AddAttribute(location, VK_FORMAT_R32G32B32_SFLOAT);
      ++location;
    }
  }
  
  vkex::VertexBufferData vertex_buffer = vkex::VertexBufferData(vertex_description);
  m_vertex_data.push_back(vertex_buffer);
}

TriangleList::~TriangleList()
{
}

void TriangleList::InternalAppendVertex(const TriangleList::Vertex& vertex)
{
  vkex::VertexBufferData* p_vertex_buffer = &m_vertex_data[0];

  p_vertex_buffer->AppendData(vertex.position);
  // Vertex colors
  if (m_options.vertex_colors) {
    p_vertex_buffer->AppendData(vertex.vertex_color);
  }
  // Tex coords
  if (m_options.tex_coords) {
    p_vertex_buffer->AppendData(vertex.tex_coord);
  }
  // Normals
  if (m_options.normals) {
    p_vertex_buffer->AppendData(vertex.normal);
  }

  m_vertex_count += 1;
}

uint32_t TriangleList::GetIndexCount() const
{
  return m_index_count;
}

uint32_t TriangleList::GetVertexCount() const
{
  return m_vertex_count;
}

uint32_t TriangleList::GetTriangleCount() const
{
  return m_triangle_count;
}

bool TriangleList::AppendVertex(const TriangleList::Vertex& vertex)
{
  if (!m_options.indices) {
    return false;
  }

  InternalAppendVertex(vertex);

  return true;
}

bool TriangleList::AppendTriangle(uint32_t v1, uint32_t v2, uint32_t v3)
{
  if (!m_options.indices) {
    return false;
  }

  m_index_data.AppendData(v1);
  m_index_data.AppendData(v2);
  m_index_data.AppendData(v3);

  m_triangle_count += 1;

  return true;
}

bool TriangleList::AppendTriangle(const TriangleList::Triangle& tri)
{ 
  if (m_options.indices) {
    return false;
  }

  InternalAppendVertex(tri.v1);
  InternalAppendVertex(tri.v2);
  InternalAppendVertex(tri.v3);

  m_vertex_count  += 1;
  m_triangle_count +=1;

  return true;
}

vkex::VertexBufferData* TriangleList::GetVertexBuffer() const
{
  vkex::VertexBufferData* ptr = const_cast<vkex::VertexBufferData*>(&m_vertex_data[0]);
  return ptr;
}

} // namespace vkex