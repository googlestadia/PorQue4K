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

#define TINYGLTF_IMPLEMENTATION

// TODO: STB already in use?
//#define STB_IMAGE_IMPLEMENTATION
//#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "GLTFModel.h"

#include "AssetUtil.h"

// TODO: There's going to be a lot of work to populate a more fully-features GLTF loader
//       But we'll build it out as needed :p

VkIndexType GetIndexTypeFromAccessor(const tinygltf::Accessor& accessor)
{
    VkIndexType indexType = VkIndexType::VK_INDEX_TYPE_MAX_ENUM;

    auto index_byte_count = tinygltf::GetComponentSizeInBytes(accessor.componentType);
    if (index_byte_count == 2) {
        indexType = VK_INDEX_TYPE_UINT16;
    }
    else if (index_byte_count == 4) {
        indexType = VK_INDEX_TYPE_UINT32;
    } else {
        VKEX_ASSERT(!"Unknown index type");
    }

    return indexType;
}

VkFormat GetBufferFormatFromAccessor(const tinygltf::Accessor& accessor)
{
    VkFormat bufferFormat = VK_FORMAT_UNDEFINED;

    auto numComponents = tinygltf::GetNumComponentsInType(accessor.type);
    if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
        switch (numComponents) {
        case 1:
            bufferFormat = VK_FORMAT_R32_SFLOAT;
            break;
        case 2:
            bufferFormat = VK_FORMAT_R32G32_SFLOAT;
            break;
        case 3:
            bufferFormat = VK_FORMAT_R32G32B32_SFLOAT;
            break;
        case 4:
            bufferFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
            break;
        default:
            break;
        }
    } else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
        switch (numComponents) {
        case 1:
            bufferFormat = VK_FORMAT_R16_UINT;
            break;
        case 2:
            bufferFormat = VK_FORMAT_R16G16_UINT;
            break;
        case 3:
            bufferFormat = VK_FORMAT_R16G16B16_UINT;
            break;
        case 4:
            bufferFormat = VK_FORMAT_R16G16B16A16_UINT;
            break;
        default:
            break;
        }
    } else {
        VKEX_ASSERT(!"Unknown buffer format");
    }

    return bufferFormat;
}

VkFormat GetTextureFormatFromImage(const tinygltf::Image& image, const bool use_sRGB)
{
    VkFormat textureFormat = VK_FORMAT_UNDEFINED;

    // TODO: OBVIOUSLY MORE FORMATS

    if (image.pixel_type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
        switch (image.component) {
        case 1:
            textureFormat = use_sRGB ? VK_FORMAT_R8_SRGB : VK_FORMAT_R8_UNORM;
            break;
        case 2:
            textureFormat = use_sRGB ? VK_FORMAT_R8G8_SRGB : VK_FORMAT_R8G8_UNORM;
            break;
        case 3:
            textureFormat = use_sRGB ? VK_FORMAT_R8G8B8_SRGB : VK_FORMAT_R8G8B8_UNORM;
            break;
        case 4:
            textureFormat = use_sRGB ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM;
            break;
        default:
            break;
        }
    }

    return textureFormat;
}

void GLTFModel::PopulateFromModel(vkex::fs::path model_path, vkex::Queue queue)
{
    tinygltf::Model model;
    {
        tinygltf::TinyGLTF loader;
        std::string err;
        std::string warn;

        bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, model_path.str());
    }

    // TODO: accessors could be sparse, gotta check for it

    m_buffers.resize(model.bufferViews.size());

    // Populate VkBuffers based on bufferViews
    for (size_t bufferViewIndex = 0; bufferViewIndex < model.bufferViews.size(); bufferViewIndex++) {
        const auto& bufferView = model.bufferViews[bufferViewIndex];
        const auto& buffer = model.buffers[bufferView.buffer];

        VKEX_LOG_INFO("Buffer View " << bufferViewIndex);
        VKEX_LOG_INFO("  Size in bytes: " << bufferView.byteLength);
        VKEX_LOG_INFO("  Offset in bytes: " << bufferView.byteOffset);

        VKEX_CALL(asset_util::CreateGeometryBuffer(
            bufferView.byteLength,
            (&buffer.data.at(0) + bufferView.byteOffset),
            queue,
            asset_util::MEMORY_USAGE_GPU_ONLY,
            &m_buffers[bufferViewIndex]));
    }

    // Mirror glTF file data into local structs
    {
        {
            size_t scene_count = model.scenes.size();
            m_scenes.resize(scene_count);
            for (size_t sceneIndex = 0; sceneIndex < scene_count; sceneIndex++) {
                auto nodeCount = model.scenes[sceneIndex].nodes.size();
                auto& target_scene = m_scenes[sceneIndex];
                target_scene.nodeIndices.resize(nodeCount);

                for (size_t nodeListIndex = 0; nodeListIndex < nodeCount; nodeListIndex++) {
                    target_scene.nodeIndices[nodeListIndex] = model.scenes[sceneIndex].nodes[nodeListIndex];
                }
            }
        }
        {
            size_t nodeCount = model.nodes.size();
            m_nodes.resize(nodeCount);

            for (size_t nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++) {
                m_nodes[nodeIndex].meshIndex = model.nodes[nodeIndex].mesh;
            }
        }
        {
            size_t meshCount = model.meshes.size();
            m_meshes.resize(meshCount);

            for (size_t meshIndex = 0; meshIndex < meshCount; meshIndex++) {
                const auto& sourceMesh = model.meshes[meshIndex];
                auto& destMesh = m_meshes[meshIndex];

                size_t primitiveCount = sourceMesh.primitives.size();
                destMesh.primitives.resize(primitiveCount);

                for (size_t primitiveIndex = 0; primitiveIndex < primitiveCount; primitiveIndex++) {
                    const auto& sourcePrimitive = sourceMesh.primitives[primitiveIndex];
                    auto& destPrimitive = destMesh.primitives[primitiveIndex];

                    destPrimitive.materialIndex = sourcePrimitive.material;
                    destPrimitive.indexBufferAccessorIndex = sourcePrimitive.indices;

                    std::map<std::string, int>::const_iterator itAttrib(sourcePrimitive.attributes.begin());
                    std::map<std::string, int>::const_iterator itEndAttrib(sourcePrimitive.attributes.end());

                    destPrimitive.attributes.resize(sourcePrimitive.attributes.size());
                    size_t attribIndex = 0;
                    for (; itAttrib != itEndAttrib; itAttrib++) {
                        VKEX_ASSERT(itAttrib->second >= 0);

                        destPrimitive.attributes[attribIndex].name = itAttrib->first;
                        destPrimitive.attributes[attribIndex].accessorIndex = itAttrib->second;

                        attribIndex++;
                    } // for (; itAttrib != itEndAttrib; itAttrib++) {
                } // for (size_t primitiveIndex = 0; primitiveIndex

                // derived per-primitive state
                for (size_t primitiveIndex = 0; primitiveIndex < primitiveCount; primitiveIndex++) {
                    auto& destPrimitive = destMesh.primitives[primitiveIndex];

                    {
                        const auto& index_accessor = model.accessors[destPrimitive.indexBufferAccessorIndex];
                        VKEX_ASSERT(index_accessor.count > 0);

                        destPrimitive.index_buffer = m_buffers[index_accessor.bufferView];
                        destPrimitive.index_count = uint32_t(index_accessor.count);

                        destPrimitive.index_type = GetIndexTypeFromAccessor(index_accessor);
                    }

                    {
                        destPrimitive.vertex_buffers.resize(BufferType::BufferTypeCount);
                        destPrimitive.vertex_binding_descriptions.resize(BufferType::BufferTypeCount);
                        destPrimitive.vertex_buffer_formats.resize(BufferType::BufferTypeCount);

                        for (auto& attribute : destPrimitive.attributes) {
                            const auto &buffer_accessor = model.accessors[attribute.accessorIndex];

                            VkFormat buffer_format = GetBufferFormatFromAccessor(buffer_accessor);

                            BufferType bufferTypeIndex = BufferType::BufferTypeCount;
                            if (attribute.name.compare("POSITION") == 0) {
                                bufferTypeIndex = BufferType::Position;
                            } else if (attribute.name.compare("NORMAL") == 0) {
                                bufferTypeIndex = BufferType::Normal;
                            } else if (attribute.name.compare("TEXCOORD_0") == 0) {
                                bufferTypeIndex = BufferType::TexCoord0;
                            }

                            destPrimitive.vertex_buffers[bufferTypeIndex] =
                                m_buffers[buffer_accessor.bufferView];

                            vkex::VertexBindingDescription desc(bufferTypeIndex, VK_VERTEX_INPUT_RATE_VERTEX);

                            destPrimitive.vertex_binding_descriptions[bufferTypeIndex] = desc;
                            destPrimitive.vertex_buffer_formats[bufferTypeIndex] = buffer_format;

                        } // for (auto& attribute 
                    }
                } // for (size_t primitiveIndex = 0;
            }
        }
    }



    {
        size_t samplerCount = model.samplers.size();
        m_samplers.resize(samplerCount);

        for (size_t samplerIndex = 0; samplerIndex < samplerCount; samplerIndex++) {
            // TODO: Drive field population from the actual sampler data

            vkex::SamplerCreateInfo create_info = {};
            create_info.min_filter = VK_FILTER_LINEAR;
            create_info.mag_filter = VK_FILTER_LINEAR;
            create_info.mipmap_mode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            create_info.min_lod = 0.0f;
            create_info.max_lod = 15.0f;
            create_info.address_mode_u = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            create_info.address_mode_v = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            create_info.address_mode_w = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            VKEX_CALL(queue->GetDevice()->CreateSampler(create_info, &(m_samplers[samplerIndex])));
        }
    }

    {
        size_t matCount = model.materials.size();
        m_materials.resize(matCount);

        for (size_t matIndex = 0; matIndex < matCount; matIndex++) {
            auto& source_material = model.materials[matIndex];

            VKEX_ASSERT(source_material.pbrMetallicRoughness.baseColorTexture.index >= 0);
            m_materials[matIndex].baseColorTextureIndex = uint32_t(source_material.pbrMetallicRoughness.baseColorTexture.index);

            VKEX_ASSERT(source_material.pbrMetallicRoughness.metallicRoughnessTexture.index >= 0);
            m_materials[matIndex].metallicRoughnessTextureIndex = uint32_t(source_material.pbrMetallicRoughness.metallicRoughnessTexture.index);

            VKEX_ASSERT(source_material.emissiveTexture.index >= 0);
            m_materials[matIndex].emissiveTextureIndex = uint32_t(source_material.emissiveTexture.index);

            VKEX_ASSERT(source_material.occlusionTexture.index >= 0);
            m_materials[matIndex].occlusionTextureIndex = uint32_t(source_material.occlusionTexture.index);

            VKEX_ASSERT(source_material.normalTexture.index >= 0);
            m_materials[matIndex].normalTextureIndex = uint32_t(source_material.normalTexture.index);

            VKEX_ASSERT(source_material.pbrMetallicRoughness.baseColorFactor.size() == 4);
            for (size_t factor_index = 0; factor_index < source_material.pbrMetallicRoughness.baseColorFactor.size(); factor_index++) {
                m_materials[matIndex].baseColorFactor[factor_index] = float(source_material.pbrMetallicRoughness.baseColorFactor[factor_index]);

            }

            VKEX_ASSERT(source_material.emissiveFactor.size() == 3);
            for (size_t factor_index = 0; factor_index < source_material.emissiveFactor.size(); factor_index++) {
                m_materials[matIndex].emissiveFactor[factor_index] = float(source_material.emissiveFactor[factor_index]);

            }

            m_materials[matIndex].metallicFactor = float(source_material.pbrMetallicRoughness.metallicFactor);
            m_materials[matIndex].roughnessFactor = float(source_material.pbrMetallicRoughness.roughnessFactor);
        }
    }

    {
        size_t texCount = model.textures.size();
        m_textures.resize(texCount);

        for (size_t texIndex = 0; texIndex < texCount; texIndex++) {
            m_textures[texIndex].samplerIndex = model.textures[texIndex].sampler;
            m_textures[texIndex].imageIndex= model.textures[texIndex].source;

            // TODO: Validate sampler/source?
        }
    }

    {
        size_t imageCount = model.images.size();
        m_images.resize(imageCount);

        for (size_t imageIndex = 0; imageIndex < imageCount; imageIndex++) {
            const auto& sourceImage = model.images[imageIndex];
            m_images[imageIndex].name = sourceImage.uri;

            bool use_sRGB = IsImageSRGB(uint32_t(imageIndex));
            VkFormat texture_format = GetTextureFormatFromImage(sourceImage, use_sRGB);

            asset_util::CreateTexture(
                sourceImage.image.size(),
                &(sourceImage.image.at(0)),
                sourceImage.width,
                sourceImage.height,
                texture_format,
                queue,
                asset_util::MEMORY_USAGE_GPU_ONLY,
                &(m_images[imageIndex].gpuTexture));
        }
    }

    {
        for (auto& material : m_materials) {
            material.textures.resize(MaterialTextureType::MaterialComponentTypeCount);
            material.samplers.resize(MaterialTextureType::MaterialComponentTypeCount);

            const auto& baseColorTexture = m_textures[material.baseColorTextureIndex];
            material.textures[MaterialTextureType::BaseColor] = m_images[baseColorTexture.imageIndex].gpuTexture;
            material.samplers[MaterialTextureType::BaseColor] = m_samplers[baseColorTexture.samplerIndex];

            const auto& metallicRoughnessTexture = m_textures[material.metallicRoughnessTextureIndex];
            material.textures[MaterialTextureType::MetallicRoughness] = m_images[metallicRoughnessTexture.imageIndex].gpuTexture;
            material.samplers[MaterialTextureType::MetallicRoughness] = m_samplers[metallicRoughnessTexture.samplerIndex];

            const auto& emissiveTexture = m_textures[material.emissiveTextureIndex];
            material.textures[MaterialTextureType::Emissive] = m_images[emissiveTexture.imageIndex].gpuTexture;
            material.samplers[MaterialTextureType::Emissive] = m_samplers[emissiveTexture.samplerIndex];

            const auto& occlusionTexture = m_textures[material.occlusionTextureIndex];
            material.textures[MaterialTextureType::Occlusion] = m_images[occlusionTexture.imageIndex].gpuTexture;
            material.samplers[MaterialTextureType::Occlusion] = m_samplers[occlusionTexture.samplerIndex];

            const auto& normalTexture = m_textures[material.normalTextureIndex];
            material.textures[MaterialTextureType::NormalTex] = m_images[normalTexture.imageIndex].gpuTexture;
            material.samplers[MaterialTextureType::NormalTex] = m_samplers[normalTexture.samplerIndex];
        }
    }
}

bool GLTFModel::IsImageSRGB(const uint32_t image_index)
{
    // albedo and emissive are SRGB, remainder should be linear (Normal, AO, Roughness?)
    bool use_sRGB = false;

    for (const auto& mat : m_materials) {
        const auto& baseColorTexture = m_textures[mat.baseColorTextureIndex];
        if (baseColorTexture.imageIndex == image_index) {
            use_sRGB = true;
            break;
        }

        const auto& emissiveTexture = m_textures[mat.emissiveTextureIndex];
        if (emissiveTexture.imageIndex == image_index) {
            use_sRGB = true;
            break;
        }
    }

    return use_sRGB;
}

const GLTFModel::Primitive& GLTFModel::GetPrimitive(uint32_t node_index, uint32_t primitive_index)
{
    auto meshIndex = m_nodes[node_index].meshIndex;
    return m_meshes[meshIndex].primitives[primitive_index];
}

std::vector<vkex::VertexBindingDescription> 
GLTFModel::GetVertexBindingDescriptions(uint32_t node_index, uint32_t primitive_index)
{
    const auto& prim = GetPrimitive(node_index, primitive_index);

    return prim.vertex_binding_descriptions;
}

std::vector<VkFormat> 
GLTFModel::GetVertexBufferFormats(uint32_t node_index, uint32_t primitive_index)
{
    const auto& prim = GetPrimitive(node_index, primitive_index);

    return prim.vertex_buffer_formats;
}

const std::vector<vkex::Texture>& 
GLTFModel::GetMaterialTextures(uint32_t node_index, uint32_t primitive_index)
{
    const auto& prim = GetPrimitive(node_index, primitive_index);

    const auto& material = m_materials[prim.materialIndex];

    return material.textures;
}

const std::vector<vkex::Sampler>& 
GLTFModel::GetMaterialSamplers(uint32_t node_index, uint32_t primitive_index)
{
    const auto& prim = GetPrimitive(node_index, primitive_index);
    const auto& material = m_materials[prim.materialIndex];

    return material.samplers;
}

const float* GLTFModel::GetBaseColorFactor(uint32_t node_index, uint32_t primitive_index)
{
    const auto& prim = GetPrimitive(node_index, primitive_index);
    const auto& material = m_materials[prim.materialIndex];

    return &material.baseColorFactor[0];
}

const float* GLTFModel::GetEmissiveFactor(uint32_t node_index, uint32_t primitive_index)
{
    const auto& prim = GetPrimitive(node_index, primitive_index);
    const auto& material = m_materials[prim.materialIndex];

    return &material.emissiveFactor[0];
}

float GLTFModel::GetMetallicFactor(uint32_t node_index, uint32_t primitive_index)
{
    const auto& prim = GetPrimitive(node_index, primitive_index);
    const auto& material = m_materials[prim.materialIndex];

    return material.metallicFactor;
}

float GLTFModel::GetRoughnessFactor(uint32_t node_index, uint32_t primitive_index)
{
    const auto& prim = GetPrimitive(node_index, primitive_index);
    const auto& material = m_materials[prim.materialIndex];

    return material.roughnessFactor;
}

void GLTFModel::GetDebugBaseColorFactor(uint32_t node_index, uint32_t primitive_index, float** out_base_color_factor)
{
    const auto& prim = GetPrimitive(node_index, primitive_index);
    auto& material = m_materials[prim.materialIndex];

    *out_base_color_factor = &material.baseColorFactor[0];
}

void GLTFModel::GetDebugEmissiveFactor(uint32_t node_index, uint32_t primitive_index, float** out_emissive_factor)
{
    const auto& prim = GetPrimitive(node_index, primitive_index);
    auto& material = m_materials[prim.materialIndex];

    *out_emissive_factor = &material.emissiveFactor[0];
}

void GLTFModel::GetDebugMetallicFactor(uint32_t node_index, uint32_t primitive_index, float** out_metallic)
{
    const auto& prim = GetPrimitive(node_index, primitive_index);
    auto& material = m_materials[prim.materialIndex];

    *out_metallic = &material.metallicFactor;
}

void GLTFModel::GetDebugRoughnessFactor(uint32_t node_index, uint32_t primitive_index, float** out_roughness)
{
    const auto& prim = GetPrimitive(node_index, primitive_index);
    auto& material = m_materials[prim.materialIndex];

    *out_roughness = &material.roughnessFactor;
}

vkex::Buffer GLTFModel::GetIndexBuffer(uint32_t node_index, uint32_t primitive_index)
{
    const auto& prim = GetPrimitive(node_index, primitive_index);

    return prim.index_buffer;
}

VkIndexType GLTFModel::GetIndexType(uint32_t node_index, uint32_t primitive_index)
{
    const auto& prim = GetPrimitive(node_index, primitive_index);

    return prim.index_type;
}

void GLTFModel::GetVertexBuffers(uint32_t node_index, uint32_t primitive_index, std::vector<VkBuffer>& vertex_buffers)
{
    const auto& prim = GetPrimitive(node_index, primitive_index);

    vertex_buffers.resize(BufferType::BufferTypeCount);

    vertex_buffers[BufferType::Position] = *(prim.vertex_buffers[BufferType::Position]);
    vertex_buffers[BufferType::Normal] = *(prim.vertex_buffers[BufferType::Normal]);
    vertex_buffers[BufferType::TexCoord0] = *(prim.vertex_buffers[BufferType::TexCoord0]);
}

uint32_t GLTFModel::GetIndexCount(uint32_t node_index, uint32_t primitive_index)
{
    const auto& prim = GetPrimitive(node_index, primitive_index);

    return prim.index_count;
}