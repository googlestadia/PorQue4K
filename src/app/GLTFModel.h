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

#ifndef __GLTF_MODEL_H__
#define __GLTF_MODEL_H__

#include "tiny_gltf.h"

#include "vkex/Application.h"

class GLTFModel {
 public:
  GLTFModel() {}
  virtual ~GLTFModel() {}

  enum BufferType {
    Position = 0,
    Normal = 1,
    TexCoord0 = 2,
    BufferTypeCount,
  };

  enum MaterialTextureType {
    BaseColor = 0,
    MetallicRoughness = 1,
    Emissive = 2,
    Occlusion = 3,
    NormalTex = 4,
    MaterialComponentTypeCount,
  };

  struct Scene {
    std::vector<uint32_t> nodeIndices;
  };

  struct Node {
    uint32_t meshIndex;
  };

  struct Attribute {
    std::string name;
    uint32_t accessorIndex;
  };

  struct Primitive {
    uint32_t materialIndex;

    uint32_t indexBufferAccessorIndex;
    std::vector<Attribute> attributes;

    // Derived state
    vkex::Buffer index_buffer;
    VkIndexType index_type = VK_INDEX_TYPE_MAX_ENUM;
    uint32_t index_count = UINT32_MAX;

    std::vector<vkex::Buffer> vertex_buffers;
    std::vector<vkex::VertexBindingDescription> vertex_binding_descriptions;
    std::vector<VkFormat> vertex_buffer_formats;
  };

  struct Mesh {
    std::vector<Primitive> primitives;
  };

  struct Sampler {
    VkFilter magFilter = VK_FILTER_NEAREST;
    VkFilter minFilter = VK_FILTER_NEAREST;

    VkSamplerMipmapMode mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    VkSamplerAddressMode wrapS = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    VkSamplerAddressMode wrapT = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    VkSamplerAddressMode wrapR = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  };

  struct Material {
    float baseColorFactor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    float emissiveFactor[3] = {1.0f, 1.0f, 1.0f};
    float metallicFactor = 1.0f;
    float roughnessFactor = 1.0f;

    uint32_t textureIndices[MaterialTextureType::MaterialComponentTypeCount];
  };

  struct Texture {
    uint32_t samplerIndex = UINT32_MAX;
    uint32_t imageIndex = UINT32_MAX;
  };

  struct Image {
    std::string name;
    vkex::Texture gpuTexture;
  };

  void PopulateFromModel(vkex::fs::path model_path, vkex::Queue queue);

  // For building pipeline binding descriptions/attributes
  std::vector<vkex::VertexBindingDescription> GetVertexBindingDescriptions(
      uint32_t node_index, uint32_t primitive_index);
  std::vector<VkFormat> GetVertexBufferFormats(uint32_t node_index,
                                               uint32_t primitive_index);

  // For descriptor sets
  const Material& GetMaterialInfo(uint32_t node_index,
                                  uint32_t primitive_index);
  const Texture& GetTextureInfo(uint32_t texture_index);
  const Image& GetImageInfo(uint32_t image_index);
  const Sampler& GetSamplerInfo(uint32_t sampler_index);
  vkex::Sampler GetVkSampler(uint32_t sampler_index);

  vkex::Sampler GetVkSamplerFromMaterialComponent(uint32_t node_index,
                                                  uint32_t primitive_index,
                                                  MaterialTextureType type);
  vkex::Texture GetVkTextureFromMaterialComponent(uint32_t node_index,
                                                  uint32_t primitive_index,
                                                  MaterialTextureType type);

  // For constant buffers
  const float* GetBaseColorFactor(uint32_t node_index,
                                  uint32_t primitive_index);
  const float* GetEmissiveFactor(uint32_t node_index, uint32_t primitive_index);
  float GetMetallicFactor(uint32_t node_index, uint32_t primitive_index);
  float GetRoughnessFactor(uint32_t node_index, uint32_t primitive_index);

  // For draws
  vkex::Buffer GetIndexBuffer(uint32_t node_index, uint32_t primitive_index);
  VkIndexType GetIndexType(uint32_t node_index, uint32_t primitive_index);
  void GetVertexBuffers(uint32_t node_index, uint32_t primitive_index,
                        std::vector<VkBuffer>& vertex_buffers);
  uint32_t GetIndexCount(uint32_t node_index, uint32_t primitive_index);

  // Debug UI functionality
  // These APIs are intentionally obtuse to prevent accidental easy use!
  void GetDebugBaseColorFactor(uint32_t node_index, uint32_t primitive_index,
                               float** out_base_color_factor);
  void GetDebugEmissiveFactor(uint32_t node_index, uint32_t primitive_index,
                              float** out_emissive_factor);
  void GetDebugMetallicFactor(uint32_t node_index, uint32_t primitive_index,
                              float** out_metallic);
  void GetDebugRoughnessFactor(uint32_t node_index, uint32_t primitive_index,
                               float** out_roughness);

 protected:
  bool IsImageSRGB(const uint32_t image_index);
  const Primitive& GetPrimitive(uint32_t node_index, uint32_t primitive_index);

 private:
  std::vector<vkex::Buffer> m_buffers;  // derived from bufferViews

  std::vector<Scene> m_scenes;
  std::vector<Node> m_nodes;
  std::vector<Mesh> m_meshes;

  std::vector<Material> m_materials;
  std::vector<Sampler> m_samplers;
  std::vector<vkex::Sampler> m_vk_samplers;
  std::vector<Texture> m_textures;
  std::vector<Image> m_images;
};

#endif  // __GLTF_MODEL_H__
