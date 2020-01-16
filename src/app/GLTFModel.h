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

class GLTFModel 
{
public:
    GLTFModel() {}
    virtual ~GLTFModel() {}

    void PopulateFromModel(tinygltf::Model& model, vkex::Queue queue);

    // For building pipeline binding descriptions/attributes
    std::vector<vkex::VertexBindingDescription> GetVertexBindingDescriptions(uint32_t node_index, uint32_t primitive_index);
    std::vector<VkFormat> GetVertexBufferFormats(uint32_t node_index, uint32_t primitive_index);

    // For descriptor sets
    const std::vector<vkex::Texture>& GetMaterialTextures(uint32_t node_index, uint32_t primitive_index);
    const std::vector<vkex::Sampler>& GetMaterialSamplers(uint32_t node_index, uint32_t primitive_index);

    // For draws
    vkex::Buffer GetIndexBuffer(uint32_t node_index, uint32_t primitive_index);
    VkIndexType GetIndexType(uint32_t node_index, uint32_t primitive_index);
    void GetVertexBuffers(uint32_t node_index, uint32_t primitive_index, std::vector<VkBuffer>& vertex_buffers);
    uint32_t GetIndexCount(uint32_t node_index, uint32_t primitive_index);

    // For Aiur!

    enum BufferType {
        Position = 0,
        Normal = 1,
        TexCoord0 = 2,
        BufferTypeCount,
    };

    enum MaterialComponentType {
        BaseColor = 0,
        MaterialComponentTypeCount,
    };

protected:
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

    struct Material {
        uint32_t baseColorTextureIndex = UINT32_MAX;

        // Derived state
        std::vector<vkex::Texture> textures;
        std::vector<vkex::Sampler> samplers;
    };

    // TODO: Do we need to stash off some sampler state?

    struct Texture {
        uint32_t samplerIndex = UINT32_MAX;
        uint32_t imageIndex = UINT32_MAX;
    };

    struct Image {
        std::string name;
        vkex::Texture gpuTexture;
    };

    bool IsImageSRGB(const uint32_t image_index);
    const Primitive& GetPrimitive(uint32_t node_index, uint32_t primitive_index);

private:
    std::vector<vkex::Buffer> m_buffers; // derived from bufferViews

    std::vector<Scene> m_scenes;
    std::vector<Node> m_nodes;
    std::vector<Mesh> m_meshes;

    std::vector<Material> m_materials;
    std::vector<vkex::Sampler> m_samplers;
    std::vector<Texture> m_textures;
    std::vector<Image> m_images;
};

#endif // __GLTF_MODEL_H__
