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

// TODO: There's going to be a lot of work to populate a more fully-features
// GLTF loader
//       But we'll build it out as needed :p

VkIndexType GetIndexTypeFromAccessor(const tinygltf::Accessor& accessor) {
  VkIndexType indexType = VkIndexType::VK_INDEX_TYPE_MAX_ENUM;

  auto index_byte_count =
      tinygltf::GetComponentSizeInBytes(accessor.componentType);
  if (index_byte_count == 2) {
    indexType = VK_INDEX_TYPE_UINT16;
  } else if (index_byte_count == 4) {
    indexType = VK_INDEX_TYPE_UINT32;
  } else {
    VKEX_ASSERT(!"Unknown index type");
  }

  return indexType;
}

VkFormat GetBufferFormatFromAccessor(const tinygltf::Accessor& accessor) {
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

VkFormat GetTextureFormatFromImage(const tinygltf::Image& image,
                                   const bool use_sRGB) {
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
        textureFormat =
            use_sRGB ? VK_FORMAT_R8G8B8_SRGB : VK_FORMAT_R8G8B8_UNORM;
        break;
      case 4:
        textureFormat =
            use_sRGB ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM;
        break;
      default:
        break;
    }
  }

  return textureFormat;
}

VkFilter ConvertGLTFFilterToVkFilter(int gltfFilter) {
  VkFilter convertedFilter = VK_FILTER_MAX_ENUM;
  switch (gltfFilter) {
    case -1: {
      convertedFilter = VK_FILTER_LINEAR;
      break;
    }
    case TINYGLTF_TEXTURE_FILTER_NEAREST:
    case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
    case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR: {
      convertedFilter = VK_FILTER_NEAREST;
      break;
    }
    case TINYGLTF_TEXTURE_FILTER_LINEAR:
    case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
    case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR: {
      convertedFilter = VK_FILTER_LINEAR;
      break;
    }
    default: {
      assert(!"Unknown GLTF texture filter");
      break;
    }
  }
  return convertedFilter;
}

VkSamplerMipmapMode ConvertGLTFFilterToVkMipmapMode(int gltfFilter) {
  VkSamplerMipmapMode convertedMode = VK_SAMPLER_MIPMAP_MODE_MAX_ENUM;
  switch (gltfFilter) {
    case -1:
    case TINYGLTF_TEXTURE_FILTER_NEAREST:
    case TINYGLTF_TEXTURE_FILTER_LINEAR: {
      convertedMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
      break;
    }
    case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
    case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST: {
      convertedMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
      break;
    }
    case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
    case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR: {
      convertedMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
      break;
    }
    default: {
      assert(!"Unknown GLTF mipmap mode");
      break;
    }
  }
  return convertedMode;
}

VkSamplerAddressMode ConvertGLTFTextureWrapToVkSamplerAddressMode(
    int gltfTextureWrap) {
  VkSamplerAddressMode convertedAddressMode = VK_SAMPLER_ADDRESS_MODE_MAX_ENUM;
  switch (gltfTextureWrap) {
    case TINYGLTF_TEXTURE_WRAP_REPEAT: {
      convertedAddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
      break;
    }
    case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE: {
      convertedAddressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
      break;
    }
    case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT: {
      convertedAddressMode = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
      break;
    }
    default: {
      assert(!"Unknown GLTF texture wrap");
      break;
    }
  }
  return convertedAddressMode;
}

void BuildSamplerFromModelData(const tinygltf::Sampler& gltfSampler,
                               GLTFModel::Sampler& sampler) {
  sampler.magFilter = ConvertGLTFFilterToVkFilter(gltfSampler.magFilter);
  sampler.minFilter = ConvertGLTFFilterToVkFilter(gltfSampler.minFilter);

  // We'll just use the minFilter. Vulkan doesn't support separate mipmap
  // modes for the minFilter and magFilter
  sampler.mipmapMode = ConvertGLTFFilterToVkMipmapMode(gltfSampler.minFilter);

  sampler.wrapS =
      ConvertGLTFTextureWrapToVkSamplerAddressMode(gltfSampler.wrapS);
  sampler.wrapT =
      ConvertGLTFTextureWrapToVkSamplerAddressMode(gltfSampler.wrapT);
  sampler.wrapR =
      ConvertGLTFTextureWrapToVkSamplerAddressMode(gltfSampler.wrapR);
}

vkex::Sampler BuildVkexSamplerFromGLTFSampler(
    vkex::Queue queue, const GLTFModel::Sampler& sampler_info) {
  vkex::Sampler sampler = nullptr;
  vkex::SamplerCreateInfo create_info = {};
  create_info.min_filter = sampler_info.minFilter;
  create_info.mag_filter = sampler_info.magFilter;
  create_info.mipmap_mode = sampler_info.mipmapMode;
  create_info.min_lod = 0.0f;
  create_info.max_lod = 15.0f;
  create_info.address_mode_u = sampler_info.wrapS;
  create_info.address_mode_v = sampler_info.wrapT;
  create_info.address_mode_w = sampler_info.wrapR;
  VKEX_CALL(queue->GetDevice()->CreateSampler(create_info, &sampler));
  return sampler;
}

void GLTFModel::PopulateFromModel(vkex::fs::path model_path,
                                  vkex::Queue queue) {
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
  for (size_t bufferViewIndex = 0; bufferViewIndex < model.bufferViews.size();
       bufferViewIndex++) {
    const auto& bufferView = model.bufferViews[bufferViewIndex];
    const auto& buffer = model.buffers[bufferView.buffer];

    VKEX_LOG_INFO("Buffer View " << bufferViewIndex);
    VKEX_LOG_INFO("  Size in bytes: " << bufferView.byteLength);
    VKEX_LOG_INFO("  Offset in bytes: " << bufferView.byteOffset);

    VKEX_CALL(asset_util::CreateGeometryBuffer(
        bufferView.byteLength, (&buffer.data.at(0) + bufferView.byteOffset),
        queue, asset_util::MEMORY_USAGE_GPU_ONLY, &m_buffers[bufferViewIndex]));
  }

  // Mirror glTF file data into local structs
  {
    size_t scene_count = model.scenes.size();
    m_scenes.resize(scene_count);
    for (size_t sceneIndex = 0; sceneIndex < scene_count; sceneIndex++) {
      auto nodeCount = model.scenes[sceneIndex].nodes.size();
      auto& target_scene = m_scenes[sceneIndex];
      target_scene.nodeIndices.resize(nodeCount);

      for (size_t nodeListIndex = 0; nodeListIndex < nodeCount;
           nodeListIndex++) {
        target_scene.nodeIndices[nodeListIndex] =
            model.scenes[sceneIndex].nodes[nodeListIndex];
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

      for (size_t primitiveIndex = 0; primitiveIndex < primitiveCount;
           primitiveIndex++) {
        const auto& sourcePrimitive = sourceMesh.primitives[primitiveIndex];
        auto& destPrimitive = destMesh.primitives[primitiveIndex];

        destPrimitive.materialIndex = sourcePrimitive.material;
        destPrimitive.indexBufferAccessorIndex = sourcePrimitive.indices;

        std::map<std::string, int>::const_iterator itAttrib(
            sourcePrimitive.attributes.begin());
        std::map<std::string, int>::const_iterator itEndAttrib(
            sourcePrimitive.attributes.end());

        destPrimitive.attributes.resize(sourcePrimitive.attributes.size());
        size_t attribIndex = 0;
        for (; itAttrib != itEndAttrib; itAttrib++) {
          VKEX_ASSERT(itAttrib->second >= 0);

          destPrimitive.attributes[attribIndex].name = itAttrib->first;
          destPrimitive.attributes[attribIndex].accessorIndex =
              itAttrib->second;

          attribIndex++;
        }  // for (; itAttrib != itEndAttrib; itAttrib++) {
      }    // for (size_t primitiveIndex = 0; primitiveIndex

      // derived per-primitive state
      for (size_t primitiveIndex = 0; primitiveIndex < primitiveCount;
           primitiveIndex++) {
        auto& destPrimitive = destMesh.primitives[primitiveIndex];

        {
          const auto& index_accessor =
              model.accessors[destPrimitive.indexBufferAccessorIndex];
          VKEX_ASSERT(index_accessor.count > 0);

          destPrimitive.index_buffer = m_buffers[index_accessor.bufferView];
          destPrimitive.index_count = uint32_t(index_accessor.count);

          destPrimitive.index_type = GetIndexTypeFromAccessor(index_accessor);
        }

        {
          destPrimitive.vertex_buffers.resize(BufferType::BufferTypeCount);
          destPrimitive.vertex_binding_descriptions.resize(
              BufferType::BufferTypeCount);
          destPrimitive.vertex_buffer_formats.resize(
              BufferType::BufferTypeCount);

          for (auto& attribute : destPrimitive.attributes) {
            const auto& buffer_accessor =
                model.accessors[attribute.accessorIndex];

            VkFormat buffer_format =
                GetBufferFormatFromAccessor(buffer_accessor);

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

            vkex::VertexBindingDescription desc(bufferTypeIndex,
                                                VK_VERTEX_INPUT_RATE_VERTEX);

            destPrimitive.vertex_binding_descriptions[bufferTypeIndex] = desc;
            destPrimitive.vertex_buffer_formats[bufferTypeIndex] =
                buffer_format;

          }  // for (auto& attribute
        }
      }  // for (size_t primitiveIndex = 0;
    }
  }

  {
    size_t samplerCount = model.samplers.size();
    m_samplers.resize(samplerCount);
    m_vk_samplers.resize(samplerCount);

    for (size_t samplerIndex = 0; samplerIndex < samplerCount; samplerIndex++) {
      BuildSamplerFromModelData(model.samplers[samplerIndex],
                                m_samplers[samplerIndex]);
      m_vk_samplers[samplerIndex] =
          BuildVkexSamplerFromGLTFSampler(queue, m_samplers[samplerIndex]);
    }
  }

  {
    size_t matCount = model.materials.size();
    m_materials.resize(matCount);

    for (size_t matIndex = 0; matIndex < matCount; matIndex++) {
      auto& source_material = model.materials[matIndex];

      VKEX_ASSERT(source_material.pbrMetallicRoughness.baseColorTexture.index >=
                  0);
      m_materials[matIndex].textureIndices[MaterialTextureType::BaseColor] =
          uint32_t(source_material.pbrMetallicRoughness.baseColorTexture.index);

      VKEX_ASSERT(
          source_material.pbrMetallicRoughness.metallicRoughnessTexture.index >=
          0);
      m_materials[matIndex]
          .textureIndices[MaterialTextureType::MetallicRoughness] = uint32_t(
          source_material.pbrMetallicRoughness.metallicRoughnessTexture.index);

      VKEX_ASSERT(source_material.emissiveTexture.index >= 0);
      m_materials[matIndex].textureIndices[MaterialTextureType::Emissive] =
          uint32_t(source_material.emissiveTexture.index);

      VKEX_ASSERT(source_material.occlusionTexture.index >= 0);
      m_materials[matIndex].textureIndices[MaterialTextureType::Occlusion] =
          uint32_t(source_material.occlusionTexture.index);

      VKEX_ASSERT(source_material.normalTexture.index >= 0);
      m_materials[matIndex].textureIndices[MaterialTextureType::NormalTex] =
          uint32_t(source_material.normalTexture.index);

      VKEX_ASSERT(source_material.pbrMetallicRoughness.baseColorFactor.size() ==
                  4);
      for (size_t factor_index = 0;
           factor_index <
           source_material.pbrMetallicRoughness.baseColorFactor.size();
           factor_index++) {
        m_materials[matIndex].baseColorFactor[factor_index] = float(
            source_material.pbrMetallicRoughness.baseColorFactor[factor_index]);
      }

      VKEX_ASSERT(source_material.emissiveFactor.size() == 3);
      for (size_t factor_index = 0;
           factor_index < source_material.emissiveFactor.size();
           factor_index++) {
        m_materials[matIndex].emissiveFactor[factor_index] =
            float(source_material.emissiveFactor[factor_index]);
      }

      m_materials[matIndex].metallicFactor =
          float(source_material.pbrMetallicRoughness.metallicFactor);
      m_materials[matIndex].roughnessFactor =
          float(source_material.pbrMetallicRoughness.roughnessFactor);
    }
  }

  {
    size_t texCount = model.textures.size();
    m_textures.resize(texCount);

    for (size_t texIndex = 0; texIndex < texCount; texIndex++) {
      m_textures[texIndex].samplerIndex = model.textures[texIndex].sampler;
      m_textures[texIndex].imageIndex = model.textures[texIndex].source;

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
      VkFormat texture_format =
          GetTextureFormatFromImage(sourceImage, use_sRGB);

      asset_util::CreateTexture(sourceImage.image.size(),
                                &(sourceImage.image.at(0)), sourceImage.width,
                                sourceImage.height, texture_format, queue,
                                asset_util::MEMORY_USAGE_GPU_ONLY,
                                &(m_images[imageIndex].gpuTexture));
    }
  }
}

bool GLTFModel::IsImageSRGB(const uint32_t image_index) {
  // albedo and emissive are SRGB, remainder should be linear (Normal, AO,
  // Roughness?)
  bool use_sRGB = false;

  for (const auto& mat : m_materials) {
    const auto& baseColorTexture =
        m_textures[mat.textureIndices[MaterialTextureType::BaseColor]];
    if (baseColorTexture.imageIndex == image_index) {
      use_sRGB = true;
      break;
    }

    const auto& emissiveTexture =
        m_textures[mat.textureIndices[MaterialTextureType::Emissive]];
    if (emissiveTexture.imageIndex == image_index) {
      use_sRGB = true;
      break;
    }
  }

  return use_sRGB;
}

const GLTFModel::Primitive& GLTFModel::GetPrimitive(uint32_t node_index,
                                                    uint32_t primitive_index) {
  auto meshIndex = m_nodes[node_index].meshIndex;
  return m_meshes[meshIndex].primitives[primitive_index];
}

std::vector<vkex::VertexBindingDescription>
GLTFModel::GetVertexBindingDescriptions(uint32_t node_index,
                                        uint32_t primitive_index) {
  const auto& prim = GetPrimitive(node_index, primitive_index);

  return prim.vertex_binding_descriptions;
}

std::vector<VkFormat> GLTFModel::GetVertexBufferFormats(
    uint32_t node_index, uint32_t primitive_index) {
  const auto& prim = GetPrimitive(node_index, primitive_index);

  return prim.vertex_buffer_formats;
}

// const std::vector<uint32_t>& GLTFModel::GetMaterialTextureIndices(
//    uint32_t node_index, uint32_t primitive_index) {
//  const auto& prim = GetPrimitive(node_index, primitive_index);
//
//  const auto& material = m_materials[prim.materialIndex];
//
//  return material.textureIndices;
//}

// const std::vector<GLTFModel::Sampler>& GLTFModel::GetMaterialSamplers(
//    uint32_t node_index, uint32_t primitive_index) {
//  const auto& prim = GetPrimitive(node_index, primitive_index);
//  const auto& material = m_materials[prim.materialIndex];
//
//  return material.samplers;
//}

const GLTFModel::Material& GLTFModel::GetMaterialInfo(
    uint32_t node_index, uint32_t primitive_index) {
  const auto& prim = GetPrimitive(node_index, primitive_index);
  return m_materials[prim.materialIndex];
}

const GLTFModel::Texture& GLTFModel::GetTextureInfo(uint32_t texture_index) {
  return m_textures[texture_index];
}

const GLTFModel::Image& GLTFModel::GetImageInfo(uint32_t image_index) {
  return m_images[image_index];
}

const GLTFModel::Sampler& GLTFModel::GetSamplerInfo(uint32_t sampler_index) {
  return m_samplers[sampler_index];
}

vkex::Sampler GLTFModel::GetVkSampler(uint32_t sampler_index) {
  return m_vk_samplers[sampler_index];
}

vkex::Sampler GLTFModel::GetVkSamplerFromMaterialComponent(
    uint32_t node_index, uint32_t primitive_index, MaterialTextureType type) {
  const auto& mat = GetMaterialInfo(node_index, primitive_index);
  const auto& texture_info = GetTextureInfo(mat.textureIndices[type]);
  return GetVkSampler(texture_info.samplerIndex);
}

vkex::Texture GLTFModel::GetVkTextureFromMaterialComponent(
    uint32_t node_index, uint32_t primitive_index, MaterialTextureType type) {
  const auto& mat = GetMaterialInfo(node_index, primitive_index);
  const auto& texture_info = GetTextureInfo(mat.textureIndices[type]);
  const auto& image_info = GetImageInfo(texture_info.imageIndex);
  return image_info.gpuTexture;
}

const float* GLTFModel::GetBaseColorFactor(uint32_t node_index,
                                           uint32_t primitive_index) {
  const auto& prim = GetPrimitive(node_index, primitive_index);
  const auto& material = m_materials[prim.materialIndex];

  return &material.baseColorFactor[0];
}

const float* GLTFModel::GetEmissiveFactor(uint32_t node_index,
                                          uint32_t primitive_index) {
  const auto& prim = GetPrimitive(node_index, primitive_index);
  const auto& material = m_materials[prim.materialIndex];

  return &material.emissiveFactor[0];
}

float GLTFModel::GetMetallicFactor(uint32_t node_index,
                                   uint32_t primitive_index) {
  const auto& prim = GetPrimitive(node_index, primitive_index);
  const auto& material = m_materials[prim.materialIndex];

  return material.metallicFactor;
}

float GLTFModel::GetRoughnessFactor(uint32_t node_index,
                                    uint32_t primitive_index) {
  const auto& prim = GetPrimitive(node_index, primitive_index);
  const auto& material = m_materials[prim.materialIndex];

  return material.roughnessFactor;
}

void GLTFModel::GetDebugBaseColorFactor(uint32_t node_index,
                                        uint32_t primitive_index,
                                        float** out_base_color_factor) {
  const auto& prim = GetPrimitive(node_index, primitive_index);
  auto& material = m_materials[prim.materialIndex];

  *out_base_color_factor = &material.baseColorFactor[0];
}

void GLTFModel::GetDebugEmissiveFactor(uint32_t node_index,
                                       uint32_t primitive_index,
                                       float** out_emissive_factor) {
  const auto& prim = GetPrimitive(node_index, primitive_index);
  auto& material = m_materials[prim.materialIndex];

  *out_emissive_factor = &material.emissiveFactor[0];
}

void GLTFModel::GetDebugMetallicFactor(uint32_t node_index,
                                       uint32_t primitive_index,
                                       float** out_metallic) {
  const auto& prim = GetPrimitive(node_index, primitive_index);
  auto& material = m_materials[prim.materialIndex];

  *out_metallic = &material.metallicFactor;
}

void GLTFModel::GetDebugRoughnessFactor(uint32_t node_index,
                                        uint32_t primitive_index,
                                        float** out_roughness) {
  const auto& prim = GetPrimitive(node_index, primitive_index);
  auto& material = m_materials[prim.materialIndex];

  *out_roughness = &material.roughnessFactor;
}

vkex::Buffer GLTFModel::GetIndexBuffer(uint32_t node_index,
                                       uint32_t primitive_index) {
  const auto& prim = GetPrimitive(node_index, primitive_index);

  return prim.index_buffer;
}

VkIndexType GLTFModel::GetIndexType(uint32_t node_index,
                                    uint32_t primitive_index) {
  const auto& prim = GetPrimitive(node_index, primitive_index);

  return prim.index_type;
}

void GLTFModel::GetVertexBuffers(uint32_t node_index, uint32_t primitive_index,
                                 std::vector<VkBuffer>& vertex_buffers) {
  const auto& prim = GetPrimitive(node_index, primitive_index);

  vertex_buffers.resize(BufferType::BufferTypeCount);

  vertex_buffers[BufferType::Position] =
      *(prim.vertex_buffers[BufferType::Position]);
  vertex_buffers[BufferType::Normal] =
      *(prim.vertex_buffers[BufferType::Normal]);
  vertex_buffers[BufferType::TexCoord0] =
      *(prim.vertex_buffers[BufferType::TexCoord0]);
}

uint32_t GLTFModel::GetIndexCount(uint32_t node_index,
                                  uint32_t primitive_index) {
  const auto& prim = GetPrimitive(node_index, primitive_index);

  return prim.index_count;
}