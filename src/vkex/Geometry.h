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

#ifndef __VKEX_GEOMETRY_H__
#define __VKEX_GEOMETRY_H__

#include <vkex/Config.h>
#include <vkex/CpuResource.h>

namespace vkex {

// =================================================================================================
// Geometry
// =================================================================================================

/** @class Geometry
 *
 */ 
class Geometry {
public:
  Geometry();
  Geometry(const vkex::VertexBufferData& vertex_data);
  Geometry(const vkex::VertexBufferData& vertex_data, const vkex::IndexBufferData& index_data);
  ~Geometry();

  VkPrimitiveTopology GetPrimitiveTopology() const;

  bool  HasIndices() const;

  uint32_t                      GetVertexBufferCount() const;
  const vkex::VertexBufferData* GetVertexBufferByIndex(uint32_t index = 0) const;
  const vkex::VertexBufferData* GetVertexBufferByBinding(uint32_t binding = 0) const;
  const vkex::IndexBufferData*  GetIndexBuffer() const;

protected:
  void AddVertexBuffer(const vkex::VertexBufferData& data);
  void SetIndexBuffer(const vkex::IndexBufferData& data);


protected:
  VkPrimitiveTopology                 m_primitive_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  std::vector<vkex::VertexBufferData> m_vertex_data;
  vkex::IndexBufferData               m_index_data;
};

// =================================================================================================
// PlatonicSolid
// =================================================================================================

/** @class PlatonicSolid
 *
 */
class PlatonicSolid : public Geometry {
public:
  struct Options {
    bool indices;
    bool vertex_colors;
    bool tex_coords;
    bool normals;
  };

  PlatonicSolid();
  PlatonicSolid(const vkex::VertexBufferData& vertex_data);
  PlatonicSolid(const vkex::VertexBufferData& vertex_data, const vkex::IndexBufferData& index_data);
  ~PlatonicSolid();

  static PlatonicSolid Cube(const vkex::PlatonicSolid::Options& options = vkex::PlatonicSolid::Options());

private:
  static void AppendVertexData(const float2& data, std::vector<float>& vertex_data);
  static void AppendVertexData(const float3& data, std::vector<float>& vertex_data);
  static void AppendVertexData(const float4& data, std::vector<float>& vertex_data);

  static void AppendVertexData(
    const float3&                       position, 
    const float3&                       vertex_color, 
    const float2&                       tex_coord, 
    const float3&                       normal,
    const vkex::PlatonicSolid::Options& options,
    std::vector<float>&                 vertex_data);
};

// =================================================================================================
// TriangleList
// =================================================================================================

/** @class PlatonicSolid
 *
 */
class TriangleList : public Geometry {
public:
  struct Vertex {
    float3  position;
    float3  vertex_color;
    float2  tex_coord;
    float3  normal;
  };

  struct Triangle {
    Vertex  v1;
    Vertex  v2;
    Vertex  v3;
  };

  struct Options {
    Options() {}
    bool indices        = false;
    bool vertex_colors  = false;
    bool tex_coords     = false;
    bool normals        = false;
  };

  TriangleList(const Options& options = Options{});
  ~TriangleList();

  uint32_t  GetIndexCount() const;

  uint32_t  GetVertexCount() const;

  uint32_t  GetTriangleCount() const;

  /** @fn AppendVertex
   * 
   * Options::indices must be 'true', otherwise will fail andr return false.
   * 
   */
  bool AppendVertex(const Vertex& vertex);

    /** @fn AppendTriangle
   * 
   * Options::indices must be 'true', otherwise will fail andr return false.
   * 
   */
  bool AppendTriangle(uint32_t v1, uint32_t v2, uint32_t v3);
 
    /** @fn AppendTriangle
   *
   * Options::indices must be 'false', otherwise will fail andr return false.
   *
   */
  bool AppendTriangle(const Triangle& tri);

  vkex::VertexBufferData* GetVertexBuffer() const;

private:
  void InternalAppendVertex(const Vertex& vertex);

private:
  Options m_options                       = {};
  uint32_t              m_index_count     = 0;
  uint32_t              m_vertex_count    = 0;
  uint32_t              m_triangle_count  = 0;
};

} // namespace vkex

#endif // __VKEX_GEOMETRY_H__