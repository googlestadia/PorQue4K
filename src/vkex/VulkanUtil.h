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

#ifndef __VKEX_VULKAN_UTIL_H__
#define __VKEX_VULKAN_UTIL_H__

#include <vkex/Config.h>

#define VKEX_IHV_VENDOR_ID_AMD     0x1002
#define VKEX_IHV_VENDOR_ID_INTEL   0x8086
#define VKEX_IHV_VENDOR_ID_NVIDIA  0x10DE

#define VK_LAYER_LUNARG_STANDARD_VALIDATION_LAYER_NAME  "VK_LAYER_LUNARG_standard_validation"

namespace vkex {

// =================================================================================================
// Enums
// =================================================================================================

/** @enum ImageType
 *
 * WARNING: Not compatible with VkImageType
 *
 */
enum ImageType {
  ImageTypeUndefined = 0,
  ImageType1D        = 1,
  ImageType2D        = 2,
  ImageType3D        = 3,
};

/** @enum ImageViewType
 *
 * WARNING: Not compatible with VkImageViewType
 *
 */
enum ImageViewType {
  ImageViewTypeUndefined = 0,
  ImageViewType1D        = 1,
  ImageViewType2D        = 2,
  ImageViewType3D        = 3,
  ImageViewTypeCube      = 4,
  ImageViewType1DArray   = 5,
  ImageViewType2DArray   = 6,
  ImageViewTypeCubeArray = 7,
};

// =================================================================================================
// Flags
// =================================================================================================

/** @union AttachmentDescriptionFlags
 *
 */
union AttachmentDescriptionFlags {
  struct {
    bool may_alias : 1;
  } bits;
  VkAttachmentDescriptionFlags flags;

  AttachmentDescriptionFlags() : flags(0)
  {
  }
  AttachmentDescriptionFlags(const VkAttachmentDescriptionFlags& value)
  {
    flags = value;
  }
  AttachmentDescriptionFlags& operator=(const VkAttachmentDescriptionFlags& rhs)
  {
    flags = rhs;
    return *this;
  }
  operator VkAttachmentDescriptionFlags() const
  {
    return flags;
  }
};

/** @union BufferCreateFlags
 *
 */
union BufferCreateFlags {
  struct {
  } bits;
  VkBufferCreateFlags flags;

  BufferCreateFlags() : flags(0)
  {
  }
  BufferCreateFlags(const VkBufferCreateFlags& value)
  {
    flags = value;
  }
  BufferCreateFlags& operator=(const VkBufferCreateFlags& rhs)
  {
    flags = rhs;
    return *this;
  }
  operator VkBufferCreateFlags() const
  {
    return flags;
  }
};

/** @union BufferUsageFlags
 *
 */
union BufferUsageFlags {
  struct {
    bool transfer_src : 1;
    bool transfer_dst : 1;
    bool uniform_texel_buffer : 1;
    bool storage_texel_buffer : 1;
    bool uniform_buffer : 1;
    bool storage_buffer : 1;
    bool index_buffer : 1;
    bool vertex_buffer : 1;
    bool indirect_buffer : 1;
  } bits;
  VkBufferUsageFlags flags;

  BufferUsageFlags() : flags(0)
  {
  }
  BufferUsageFlags(const VkBufferUsageFlags& value)
  {
    flags = value;
  }
  BufferUsageFlags& operator=(const VkBufferUsageFlags& rhs)
  {
    flags = rhs;
    return *this;
  }
  operator VkBufferUsageFlags() const
  {
    return flags;
  }
};

/** @union CommandPoolCreateFlags
 *
 */
union CommandPoolCreateFlags {
  struct {
    bool transient : 1;
    bool reset_command_buffer : 1;
    bool _protected : 1;
  } bits;
  VkCommandPoolCreateFlags flags;

  CommandPoolCreateFlags() : flags(0)
  {
  }
  CommandPoolCreateFlags(const VkCommandPoolCreateFlags& value)
  {
    flags = value;
  }
  CommandPoolCreateFlags& operator=(const VkCommandPoolCreateFlags& rhs)
  {
    flags = rhs;
    return *this;
  }
  operator VkCommandPoolCreateFlags() const
  {
    return flags;
  }
};

/** @union DescriptorPoolCreateFlags
 *
 */
union DescriptorLayoutCreateFlags {
  struct {
    bool push_descriptor_set : 1;
    bool update_after_bind_pool : 1;
  } bits;
  VkDescriptorSetLayoutCreateFlags flags;

  DescriptorLayoutCreateFlags() : flags(0)
  {
  }
  DescriptorLayoutCreateFlags(const VkDescriptorSetLayoutCreateFlags& value)
  {
    flags = value;
  }
  DescriptorLayoutCreateFlags& operator=(const VkDescriptorSetLayoutCreateFlags& rhs)
  {
    flags = rhs;
    return *this;
  }
  operator VkDescriptorSetLayoutCreateFlags() const
  {
    return flags;
  }
};

/** @union DescriptorPoolCreateFlags
 *
 */
union DescriptorPoolCreateFlags {
  struct {
    bool free_descriptor_set : 1;
    bool update_after_bind : 1;
  } bits;
  VkDescriptorPoolCreateFlags flags;

  DescriptorPoolCreateFlags() : flags(0)
  {
  }
  DescriptorPoolCreateFlags(const VkDescriptorPoolCreateFlags& value)
  {
    flags = value;
  }
  DescriptorPoolCreateFlags& operator=(const VkDescriptorPoolCreateFlags& rhs)
  {
    flags = rhs;
    return *this;
  }
  operator VkDescriptorPoolCreateFlags() const
  {
    return flags;
  }
};

/** @union FenceCreateFlags
 *
 */
union FenceCreateFlags {
  struct {
    bool signaled : 1;
  } bits;
  VkFenceCreateFlags flags;

  FenceCreateFlags() : flags(0)
  {
  }
  FenceCreateFlags(const VkFenceCreateFlags& value)
  {
    flags = value;
  }
  FenceCreateFlags& operator=(const VkFenceCreateFlags& rhs)
  {
    flags = rhs;
    return *this;
  }
  operator VkFenceCreateFlags() const
  {
    return flags;
  }
};

/** @union ImageAspectFlags
 *
 */
union ImageAspectFlags {
  struct {
    bool color_bit    : 1;
    bool depth_bit    : 1;
    bool stencil_bit  : 1;
    bool metadata_bit : 1;
    bool plane_0_bit  : 1;
    bool plane_1_bit  : 1;
    bool plane_2_bit  : 1;
  } bits;
  VkImageAspectFlags flags;

  ImageAspectFlags() : flags(0)
  {
  }
  ImageAspectFlags(const VkImageAspectFlags& value)
  {
    flags = value;
  }
  ImageAspectFlags& operator=(const VkImageAspectFlags& rhs)
  {
    flags = rhs;
    return *this;
  }
  operator VkImageAspectFlags() const
  {
    return flags;
  }
};

/** @union ImageCreateFlags
 *
 */
union ImageCreateFlags {
  struct {
    bool sparse_binding                     : 1;
    bool sparse_residency                   : 1;
    bool sparse_aliased                     : 1;
    bool mutable_format                     : 1;
    bool cube_compatible                    : 1;
    bool _2d_array_compatible               : 1;
    bool split_instance_bind_regions        : 1;
    bool block_texel_view_compatible        : 1;
    bool extended_usage                     : 1;
    bool disjoint                           : 1;
    bool alias                              : 1;
    bool _protected                         : 1;
    bool sample_locations_compatible_depth  : 1;
  } bits;
  VkImageCreateFlags flags;

  ImageCreateFlags() : flags(0)
  {
  }
  ImageCreateFlags(const VkImageCreateFlags& value)
  {
    flags = value;
  }
  ImageCreateFlags& operator=(const VkImageCreateFlags& rhs)
  {
    flags = rhs;
    return *this;
  }
  operator VkImageCreateFlags() const
  {
    return flags;
  }
};

/** @union ImageUsageFlags
 *
 */
union ImageUsageFlags {
  struct {
    bool transfer_src : 1;
    bool transfer_dst : 1;
    bool sampled : 1;
    bool storage : 1;
    bool color_attachment : 1;
    bool depth_stencil_attachment : 1;
    bool transient_attachment : 1;
    bool input_attachment : 1;
  } bits;
  VkImageUsageFlags flags;

  ImageUsageFlags() : flags(0)
  {
  }
  ImageUsageFlags(const VkImageUsageFlags& value)
  {
    flags = value;
  }
  ImageUsageFlags& operator=(const VkImageUsageFlags& rhs)
  {
    flags = rhs;
    return *this;
  }
  operator VkImageUsageFlags() const
  {
    return flags;
  }
};

/** @union ImageViewCreateFlags
 *
 */
union ImageViewCreateFlags {
  struct {
  } bits;
  VkImageViewCreateFlags flags;

  ImageViewCreateFlags() : flags(0)
  {
  }
  ImageViewCreateFlags(const VkImageViewCreateFlags& value)
  {
    flags = value;
  }
  ImageViewCreateFlags& operator=(const VkImageViewCreateFlags& rhs)
  {
    flags = rhs;
    return *this;
  }
  operator VkImageViewCreateFlags() const
  {
    return flags;
  }
};

/** @union QueueFlags
 *
 */
union QueueFlags {
  struct {
    bool graphics : 1;
    bool compute : 1;
    bool transfer : 1;
    bool sparse_binding : 1;
    bool _protected : 1;
  } bits;
  VkQueueFlags flags;

  QueueFlags() : flags(0)
  {
  }
  QueueFlags(const VkQueueFlags& value)
  {
    flags = value;
  }
  QueueFlags& operator=(const VkQueueFlags& rhs)
  {
    flags = rhs;
    return *this;
  }
  operator VkQueueFlags() const
  {
    return flags;
  }
};

/** @union RenderPassCreateFlags
 *
 */
union RenderPassCreateFlags {
  struct {
  } bits;
  VkRenderPassCreateFlags flags;

  RenderPassCreateFlags() : flags(0)
  {
  }
  RenderPassCreateFlags(const VkRenderPassCreateFlags& value)
  {
    flags = value;
  }
  RenderPassCreateFlags& operator=(const VkRenderPassCreateFlags& rhs)
  {
    flags = rhs;
    return *this;
  }
  operator VkRenderPassCreateFlags() const
  {
    return flags;
  }
};

/** @union SamplerCreateFlags
 *
 */
union SamplerCreateFlags {
  struct {
  } bits;
  VkSamplerCreateFlags flags;

  SamplerCreateFlags() : flags(0)
  {
  }
  SamplerCreateFlags(const VkSamplerCreateFlags& value)
  {
    flags = value;
  }
  SamplerCreateFlags& operator=(const VkSamplerCreateFlags& rhs)
  {
    flags = rhs;
    return *this;
  }
  operator VkSamplerCreateFlags() const
  {
    return flags;
  }
};

/** @union SemaphoreCreateFlags
 *
 */
union SemaphoreCreateFlags {
  struct {
    bool signaled : 1;
  } bits;
  VkSemaphoreCreateFlags flags;

  SemaphoreCreateFlags() : flags(0)
  {
  }
  SemaphoreCreateFlags(const VkSemaphoreCreateFlags& value)
  {
    flags = value;
  }
  SemaphoreCreateFlags& operator=(const VkSemaphoreCreateFlags& rhs)
  {
    flags = rhs;
    return *this;
  }
  operator VkSemaphoreCreateFlags() const
  {
    return flags;
  }
};

/** @union ShaderStageFlags
 *
 */
union ShaderStageFlags {
  struct {
    bool vs : 1;
    bool hs : 1;
    bool ds : 1;
    bool gs : 1;
    bool ps : 1;
    bool cs : 1;
  } bits;
  VkShaderStageFlags flags;

  ShaderStageFlags() : flags(0)
  {
  }
  ShaderStageFlags(const VkShaderStageFlags& value)
  {
    flags = value;
  }
  ShaderStageFlags& operator=(const VkShaderStageFlags& rhs)
  {
    flags = rhs;
    return *this;
  }
  operator VkShaderStageFlags() const
  {
    return flags;
  }
};

/** @union QueryPipelineStatisticFlagBits
 *
 */
union QueryPipelineStatisticFlags {
  struct {
    bool input_assembly_vertices : 1;
    bool input_assembly_primitives : 1;
    bool vertex_shader_invocations : 1;
    bool geometry_shader_invocations : 1;
    bool geometry_shader_primitives : 1;
    bool clipping_invocations : 1;
    bool clipping_primitives : 1;
    bool fragment_shader_invocations : 1;
    bool tessellation_control_shader_patches : 1;
    bool tessellation_evaluation_shader_invocations : 1;
    bool compute_shader_invocations : 1;
  } bits;
  VkQueryPipelineStatisticFlags flags;

  QueryPipelineStatisticFlags() : flags(0)
  {
  }

  QueryPipelineStatisticFlags(const VkQueryPipelineStatisticFlags& value)
  {
    flags = value;
  }

  QueryPipelineStatisticFlags& operator=(const VkQueryPipelineStatisticFlags & rhs)
  {
    flags = rhs;
    return *this;
  }

  operator VkQueryPipelineStatisticFlags() const
  {
    return flags;
  }
};

// =================================================================================================
// Descriptor Pool Sizes
// =================================================================================================

/** @union union DescriptorPoolSizes {
 *
 */
union DescriptorPoolSizes {
  struct {
    uint32_t sampler;
    uint32_t combined_image_sampler;
    uint32_t sampled_image;
    uint32_t storage_image;
    uint32_t uniform_texel_buffer;
    uint32_t storage_texel_buffer;
    uint32_t uniform_buffer;
    uint32_t storage_buffer;
    uint32_t uniform_buffer_dynamic;
    uint32_t storage_buffer_dynamic;
    uint32_t input_attachment;
  };
  uint32_t sizes[VK_DESCRIPTOR_TYPE_RANGE_SIZE];

  DescriptorPoolSizes& operator+=(const vkex::DescriptorPoolSizes& rhs)
  {
    this->sampler += rhs.sampler;
    this->combined_image_sampler += rhs.combined_image_sampler;
    this->sampled_image += rhs.sampled_image;
    this->storage_image += rhs.storage_image;
    this->uniform_texel_buffer += rhs.uniform_texel_buffer;
    this->storage_texel_buffer += rhs.storage_texel_buffer;
    this->uniform_buffer += rhs.uniform_buffer;
    this->storage_buffer += rhs.storage_buffer;
    this->uniform_buffer_dynamic += rhs.uniform_buffer_dynamic;
    this->storage_buffer_dynamic += rhs.storage_buffer_dynamic;
    this->input_attachment += rhs.input_attachment;
    return *this;
  }

  DescriptorPoolSizes& operator*=(uint32_t rhs)
  {
    this->sampler *= rhs;
    this->combined_image_sampler *= rhs;
    this->sampled_image *= rhs;
    this->storage_image *= rhs;
    this->uniform_texel_buffer *= rhs;
    this->storage_texel_buffer *= rhs;
    this->uniform_buffer *= rhs;
    this->storage_buffer *= rhs;
    this->uniform_buffer_dynamic *= rhs;
    this->storage_buffer_dynamic *= rhs;
    this->input_attachment *= rhs;
    return *this;
  }
};

// =================================================================================================
// Vertex Description
// =================================================================================================

class VertexBindingDescription;

/** @class VertexAttribute
 *
 */
class VertexAttributeDescription {
public:
  VertexAttributeDescription()
  {
  }
  VertexAttributeDescription(VkFormat format, uint32_t binding, const std::string& name = "");
  VertexAttributeDescription(
    uint32_t           location,
    uint32_t           binding,
    VkFormat           format,
    uint32_t           offset,
    const std::string& name = "");
  VertexAttributeDescription(
    const VkVertexInputAttributeDescription& description,
    const std::string&                       name = "");
  ~VertexAttributeDescription()
  {
  }
  const std::string& GetName() const
  {
    return m_name;
  }
  const VkVertexInputAttributeDescription& GetDescription() const
  {
    return m_description;
  }

private:
  friend class vkex::VertexBindingDescription;
  VkVertexInputAttributeDescription m_description;
  std::string                       m_name;
};

/** @class VertexDescription
 *
 */
class VertexBindingDescription {
public:
  VertexBindingDescription()
  {
  }
  VertexBindingDescription(
    uint32_t          binding,
    VkVertexInputRate vertex_input_rate = VK_VERTEX_INPUT_RATE_VERTEX);
  VertexBindingDescription(
    uint32_t                                       binding,
    VkVertexInputRate                              vertex_input_rate,
    const std::vector<VertexAttributeDescription>& attributes);
  ~VertexBindingDescription()
  {
  }
  uint32_t GetBinding() const
  {
    return m_description.binding;
  }
  const VkVertexInputBindingDescription& GetDescription() const
  {
    return m_description;
  }
  const std::vector<VertexAttributeDescription>& GetAttributes() const
  {
    return m_attributes;
  }
  void AddAttribute(const VertexAttributeDescription& attribute);
  void AddAttribute(uint32_t location, VkFormat format, const std::string& name = "");

private:
  void UpdateOffsetsAndStride();

private:
  VkVertexInputBindingDescription         m_description = {};
  std::vector<VertexAttributeDescription> m_attributes;
};

// =================================================================================================
// ShaderInterface
// =================================================================================================
class ShaderInterface {
public:
  struct Binding {
    std::string      name;
    uint32_t         set_number;
    uint32_t         binding_number;
    VkDescriptorType descriptor_type;
    uint32_t         descriptor_count;
    ShaderStageFlags stage_flags;
  };

  struct Set {
    uint32_t                              set_number;
    std::vector<ShaderInterface::Binding> bindings;
  };

  ShaderInterface();
  ~ShaderInterface();

  /** @fn AddBinding
   *
   */
  vkex::Result AddBinding(const vkex::ShaderInterface::Binding& binding);

  /**@n AddBindings
   *
   */
  void AddBindings(const vkex::ShaderInterface& interface);

  /** @fn GetSetNumbers
   *
   */
  vkex::DescriptorPoolSizes GetDescriptorPoolSizes() const;

  /** @fn GetSetNumbers
   *
   */
  std::vector<uint32_t> GetSetNumbers() const;

  /** @fn GetDescriptorSets
   *
   */
  const std::vector<vkex::ShaderInterface::Set>& GetSets() const;

  /** @fn GetDescriptorSet
   *
   */
  vkex::ShaderInterface::Set GetSet(uint32_t set_number) const;

  /** @fn GetDescriptorType
   *
   */
  VkDescriptorType GetDescriptorType(uint32_t set_number, uint32_t binding_number) const;

private:
  friend class CShaderModule;
  friend class CShaderProgram;

  VertexBindingDescription                m_vertex_bindings;
  std::vector<vkex::ShaderInterface::Set> m_descriptor_sets;
};

// =================================================================================================
// ShaderArguments
// =================================================================================================

enum {
  kMaxAllSets = 0xFFFFFFFF
};

struct AssignedDescriptorSet {
  uint32_t              set_number;
  vkex::DescriptorSet   descriptor_set;  
};

class ShaderArguments {
public:
  ShaderArguments();
  ShaderArguments(const std::vector<AssignedDescriptorSet>& assigned_sets);
  ~ShaderArguments();

  vkex::Result AssignSet(uint32_t set_number, vkex::DescriptorSet descriptor_set);

  /** @fn AssignDescriptor
   *
   */
  vkex::Result AssignDescriptor(uint32_t set_number, uint32_t binding_number, const vkex::Buffer constant_buffer, uint32_t array_element = 0);

  /** @fn AssignDescriptor
   *
   */
  vkex::Result AssignDescriptor(uint32_t set_number, uint32_t binding_number, const vkex::Texture texture, uint32_t array_element = 0);

  /** @fn AssignDescriptor
   *
   */
  vkex::Result AssignDescriptor(uint32_t set_number, uint32_t binding_number, const vkex::Sampler sampler, uint32_t array_element = 0);

  /** @fn GetVkDescriptorSets
   *
   */
  std::vector<VkDescriptorSet> GetVkDescriptorSets(uint32_t first_set_number = 0, uint32_t set_count = kMaxAllSets) const;

private:
  void SortSets();

private:
  std::vector<AssignedDescriptorSet>  m_assigned_sets;
};

// =================================================================================================
// DisplayInfo
// =================================================================================================
struct DisplayInfo {
#if defined(VKEX_LINUX_WAYLAND)
  struct wl_display* display;
#elif defined(VKEX_LINUX_XCB)
  xcb_connection_t* connection;
  xcb_visualid_t    visual_id;
#elif defined(VKEX_LINUX_XLIB)
  Display* dpy;
  VisualID visual_id;
#endif
};

// =================================================================================================
// Utility Functions
// =================================================================================================

/** @fn DetermineAspectMask
 *
 */
vkex::ImageAspectFlags DetermineAspectMask(VkFormat format);

/** @fn ComponentMapping
 *
 */
VkComponentMapping ComponentMapping(
  VkComponentSwizzle r,
  VkComponentSwizzle g,
  VkComponentSwizzle b,
  VkComponentSwizzle a);

/** @fn ComponentMappingRGBA
 *
 */
VkComponentMapping ComponentMappingRGBA();

/** @fn ImageSubresourceRange
 *
 */
VkImageSubresourceRange ImageSubresourceRange(
  VkImageAspectFlags aspect_mask,
  uint32_t           base_mip_level    = 0,
  uint32_t           mip_level_count   = 1,
  uint32_t           base_array_layer  = 0,
  uint32_t           array_layer_count = 1);

/** @fn FormatComponentType
 *
 */
vkex::ComponentType FormatComponentType(VkFormat format);

/** @fn FormatComponentSize
 *
 */
uint32_t FormatComponentSize(VkFormat format);

/** @fn FormatComponentCount
 *
 */
uint32_t FormatComponentCount(VkFormat format);

/** @fn FormatSize
 *
 */
uint32_t FormatSize(VkFormat format);

// =================================================================================================
// Image Layout Transition Functions
// =================================================================================================
vkex::Result TransitionImageLayout(
  vkex::Queue          queue,
  vkex::Image          image,
  VkImageLayout        old_layout,
  VkImageLayout        new_layout,
  VkPipelineStageFlags new_pipeline_stage);

vkex::Result TransitionImageLayout(
  vkex::Queue           queue,
  vkex::Texture         texture,
  VkImageLayout         old_layout,
  VkImageLayout         new_layout,
  VkPipelineStageFlags  new_shader_stage);

// =================================================================================================
// Buffer/Image Copy Functions
// =================================================================================================
vkex::Result CopyResource(
  vkex::Queue         queue,
  vkex::Buffer        src,
  vkex::Buffer        dst,
  uint32_t            region_count,
  const VkBufferCopy* p_regions);

vkex::Result CopyResource(
  vkex::Queue         queue,
  VkDeviceSize        src_size,
  const void*         p_src_data,
  vkex::Buffer        dst);

vkex::Result CopyResource(
  vkex::Queue        queue,
  vkex::Image        src,
  vkex::Image        dst,
  uint32_t           region_count,
  const VkImageCopy* p_regions);

vkex::Result CopyResource(
  vkex::Queue              queue,
  vkex::Buffer             src,
  vkex::Image              dst,
  uint32_t                 region_count,
  const VkBufferImageCopy* p_regions);

} // namespace vkex

#endif // __VKEX_VULKAN_UTIL_H__