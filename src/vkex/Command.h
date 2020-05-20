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

#ifndef __VKEX_COMMAND_H__
#define __VKEX_COMMAND_H__

#include <vkex/Config.h>
#include <vkex/Traits.h>
#include <vkex/VulkanUtil.h>

namespace vkex {

// =================================================================================================
// CommandBuffer
// =================================================================================================

/** @struct CommandBufferCreateInfo 
 *
 */
struct CommandBufferCreateInfo {
  VkCommandBuffer vk_object;
};

/** @class ICommandBuffer
 *
 */ 
class CCommandBuffer {
public:
  CCommandBuffer();
  ~CCommandBuffer();

  /** @fn operator VkBuffer()
   *
   */
  operator VkCommandBuffer() const { 
    return m_create_info.vk_object; 
  }

  /** @fn GetVkObject
   *
   */
  VkCommandBuffer GetVkObject() const { 
    return m_create_info.vk_object; 
  }

  /** @fn GetPool
   *
   */
  vkex::CommandPool GetPool() const {
    return m_pool;
  }

  vkex::Result Begin(VkCommandBufferUsageFlags flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
  vkex::Result End();

  // -----------------------------------------------------------------------------------------------
  // Command functions that mirror the vkCmd* interface
  // -----------------------------------------------------------------------------------------------
  void  CmdBindPipeline(VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline);
  void  CmdSetViewport(uint32_t firstViewport, uint32_t viewportCount, const VkViewport* pViewports);
  void  CmdSetScissor(uint32_t firstScissor, uint32_t scissorCount, const VkRect2D* pScissors);
  void  CmdSetLineWidth(float lineWidth);
  void  CmdSetDepthBias(float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor);
  void  CmdSetBlendConstants(const float blendConstants[4]);
  void  CmdSetDepthBounds(float minDepthBounds, float maxDepthBounds);
  void  CmdSetStencilCompareMask(VkStencilFaceFlags faceMask, uint32_t compareMask);
  void  CmdSetStencilWriteMask(VkStencilFaceFlags faceMask, uint32_t writeMask);
  void  CmdSetStencilReference(VkStencilFaceFlags faceMask, uint32_t reference);
  void  CmdBindDescriptorSets(VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets);
  void  CmdBindIndexBuffer(VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType);
  void  CmdBindVertexBuffers(uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets);
  void  CmdDraw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
  void  CmdDrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);
  void  CmdDrawIndirect(VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride);
  void  CmdDrawIndexedIndirect(VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride);
  void  CmdDispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
  void  CmdDispatchIndirect(VkBuffer buffer, VkDeviceSize offset);
  void  CmdCopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions);
  void  CmdCopyImage(VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy* pRegions);
  void  CmdBlitImage(VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit* pRegions, VkFilter filter);
  void  CmdCopyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions);
  void  CmdCopyImageToBuffer(VkImage srcImage, VkImageLayout srcImageLayout, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions);
  void  CmdUpdateBuffer(VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize dataSize, const void* pData);
  void  CmdFillBuffer(VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size, uint32_t data);
  void  CmdClearColorImage(VkImage image, VkImageLayout imageLayout, const VkClearColorValue* pColor, uint32_t rangeCount, const VkImageSubresourceRange* pRanges);
  void  CmdClearDepthStencilImage(VkImage image, VkImageLayout imageLayout, const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount, const VkImageSubresourceRange* pRanges);
  void  CmdClearAttachments(uint32_t attachmentCount, const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects);
  void  CmdResolveImage(VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageResolve* pRegions);
  void  CmdSetEvent(VkEvent event, VkPipelineStageFlags stageMask);
  void  CmdResetEvent(VkEvent event, VkPipelineStageFlags stageMask);
  void  CmdWaitEvents(uint32_t eventCount, const VkEvent* pEvents, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers);
  void  CmdPipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers);
  void  CmdBeginQuery(VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags);
  void  CmdEndQuery(VkQueryPool queryPool, uint32_t query);
  void  CmdResetQueryPool(VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount);
  void  CmdWriteTimestamp(VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool, uint32_t query);
  void  CmdCopyQueryPoolResults(VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags);
  void  CmdPushConstants(VkPipelineLayout layout, VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* pValues);
  void  CmdBeginRenderPass(const VkRenderPassBeginInfo* pRenderPassBegin, VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE);
  void  CmdNextSubpass(VkSubpassContents contents);
  void  CmdEndRenderPass();
  void  CmdExecuteCommands(uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers);

  // -----------------------------------------------------------------------------------------------
  // Command functions with convenience parameters
  // -----------------------------------------------------------------------------------------------
  void  CmdBindPipeline(vkex::ComputePipeline pipeline);
  void  CmdBindPipeline(vkex::GraphicsPipeline pipeline);
  void  CmdSetViewport(uint32_t firstViewport, const std::vector<VkViewport>* pViewports);
  void  CmdSetViewport(const VkRect2D& area, float minDepth = 0.0f, float maxDepth = 1.0f);
  void  CmdSetScissor(uint32_t firstScissor, const std::vector<VkRect2D>* pScissors);
  void  CmdSetScissor(const VkRect2D& area);
  void  CmdSetBlendConstants(float bc0, float bc1, float bc2, float bc3);
  void  CmdBindDescriptorSets(VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, const std::vector<VkDescriptorSet>& descriptorSets, const std::vector<uint32_t>* pDynamicOffsets = nullptr);
  void  CmdBindIndexBuffer(vkex::Buffer buffer, VkDeviceSize offset, VkIndexType indexType);
  void  CmdBindVertexBuffers(uint32_t firstBinding, const std::vector<VkBuffer>* pBuffers, const VkDeviceSize* pOffsets);
  void  CmdBindVertexBuffers(vkex::Buffer buffer, VkDeviceSize offset = 0);
  void  CmdCopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, const std::vector<VkBufferCopy>* pRegions);
  void  CmdCopyImage(VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, const std::vector<VkImageCopy>* pRegions);
  void  CmdBlitImage(VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, const std::vector<VkImageBlit>* pRegions, VkFilter filter);
  void  CmdCopyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, const std::vector<VkBufferImageCopy>* pRegions);
  void  CmdCopyImageToBuffer(VkImage srcImage, VkImageLayout srcImageLayout, VkBuffer dstBuffer, const std::vector<VkBufferImageCopy>* pRegions);
  void  CmdUpdateBuffer(VkBuffer dstBuffer, VkDeviceSize dstOffset, const std::vector<uint8_t>* pData);
  void  CmdClearColorImage(VkImage image, VkImageLayout imageLayout, const VkClearColorValue* pColor, const std::vector<VkImageSubresourceRange>* pRanges);
  void  CmdClearDepthStencilImage(VkImage image, VkImageLayout imageLayout, const VkClearDepthStencilValue* pDepthStencil, const std::vector<VkImageSubresourceRange>* pRanges);
  void  CmdClearAttachments(uint32_t attachmentCount, const VkClearAttachment* pAttachments, const std::vector<VkClearRect>* pRects);
  void  CmdResolveImage(VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, const std::vector<VkImageResolve>* pRegions);
  void  CmdWaitEvents(const std::vector<VkEvent>* pEvents, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, const std::vector<VkMemoryBarrier>* pMemoryBarriers, const std::vector<VkBufferMemoryBarrier>* pBufferMemoryBarriers, const std::vector<VkImageMemoryBarrier>* pImageMemoryBarriers);
  void  CmdPipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, const std::vector<VkMemoryBarrier>* pMemoryBarriers, const std::vector<VkBufferMemoryBarrier>* pBufferMemoryBarriers, const std::vector<VkImageMemoryBarrier>* pImageMemoryBarriers);
  void  CmdBeginQuery(vkex::QueryPool queryPool, uint32_t query, VkQueryControlFlags flags);
  void  CmdEndQuery(vkex::QueryPool queryPool, uint32_t query);
  void  CmdResetQueryPool(vkex::QueryPool queryPool, uint32_t firstQuery, uint32_t queryCount);
  void  CmdWriteTimestamp(VkPipelineStageFlagBits pipelineStage, vkex::QueryPool queryPool, uint32_t query);
  void  CmdCopyQueryPoolResults(vkex::QueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags);
  void  CmdPushConstants(VkPipelineLayout layout, VkShaderStageFlags stageFlags, uint32_t offset, const std::vector<uint8_t>* pValues);
  void  CmdBeginRenderPass(const vkex::RenderPass renderPass, uint32_t clearValueCount, const VkClearValue* pClearValues, VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE, void* pNext = nullptr);
  void  CmdBeginRenderPass(const vkex::RenderPass renderPass, const std::vector<VkClearValue>* pClearValues, VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE, void* pNext = nullptr);
  void  CmdBeginRenderPass(const vkex::RenderPass renderPass, VkSubpassContents contents = VK_SUBPASS_CONTENTS_INLINE);
  void  CmdExecuteCommands(const std::vector<VkCommandBuffer>* pCommandBuffers);

  void  CmdTransitionImageLayout(VkImage image, VkImageAspectFlags aspectMask, uint32_t baseMipLevel, uint32_t levelCount, uint32_t baseArrayLayer, uint32_t layerCount, VkImageLayout oldLayout, VkImageLayout newLayout, VkPipelineStageFlags newPipelineStage);
  void  CmdTransitionImageLayout(vkex::Texture texture, VkImageLayout oldLayout, VkImageLayout newLayout, VkPipelineStageFlags newPipelineStage);

private:
  friend class CCommandPool;
  friend class IObjectStorageFunctions;

  /** @fn InternalCreate
   *
   */
  vkex::Result InternalCreate(
    const vkex::CommandBufferCreateInfo&  create_info,
    const VkAllocationCallbacks*          p_allocator
  );

  /** @fn InternalDestroy
   *
   */
  vkex::Result InternalDestroy(const VkAllocationCallbacks* p_allocator);

  /** @fn SetPool
   *
   */
  void SetPool(vkex::CommandPool pool) {
    m_pool = pool;
  }

private:
  vkex::CommandPool             m_pool = nullptr;
  vkex::CommandBufferCreateInfo m_create_info = {};
};

// =================================================================================================
// CommandPool
// =================================================================================================

/** @struct CommandBufferAllocateInfo 
 *
 */
struct CommandBufferAllocateInfo {
  uint32_t  command_buffer_count;
};

/** @struct CommandPoolCreateInfo 
 *
 */
struct CommandPoolCreateInfo {
  CommandPoolCreateFlags  flags;
  uint32_t                queue_family_index;
};

/** @class ICommandPool
 *
 */ 
class CCommandPool
  : public IDeviceObject,
    protected IObjectStorageFunctions
{
public:
  CCommandPool();
  ~CCommandPool();

  /** @fn operator VkBuffer()
   *
   */
  operator VkCommandPool() const { 
    return m_vk_object; 
  }

  /** @fn GetVkObject
   *
   */
  VkCommandPool GetVkObject() const { 
    return m_vk_object; 
  }

  /** @fn AllocateCommandBuffers
   *
   */
  vkex::Result AllocateCommandBuffers(
    const vkex::CommandBufferAllocateInfo&  allocate_info, 
    vkex::CommandBuffer*                    p_command_buffers
  );
  
  /** @fn AllocateCommandBuffers
   *
   */
  vkex::Result AllocateCommandBuffers(
    const vkex::CommandBufferAllocateInfo&  allocate_info, 
    std::vector<vkex::CommandBuffer>*       p_command_buffers
  );

  /** @fn AllocateCommandBuffer
   *
   */
  vkex::Result AllocateCommandBuffer(
    const vkex::CommandBufferAllocateInfo&  allocate_info, 
    vkex::CommandBuffer*                    p_command_buffer
  );

  /** @fn FreeCommandBuffers
   *
   */
  void FreeCommandBuffers(
    uint32_t                    command_buffer_count, 
    const vkex::CommandBuffer*  p_command_buffers
  );

  /** @fn FreeCommandBuffers
   *
   */
  void FreeCommandBuffers(const std::vector<vkex::CommandBuffer>* p_command_buffers);

  /** @fn FreeCommandBuffer
   *
   */
  void FreeCommandBuffer(const vkex::CommandBuffer command_buffer);

private:
  friend class CDevice;
  friend class IObjectStorageFunctions;

  /** @fn InternalCreate
   *
   */
  vkex::Result InternalCreate(
    const vkex::CommandPoolCreateInfo&  create_info,
    const VkAllocationCallbacks*        p_allocator
  );

  /** @fn InternalDestroy
   *
   */
  vkex::Result InternalDestroy(const VkAllocationCallbacks* p_allocator);

private:
  vkex::CommandPoolCreateInfo                   m_create_info = {};
  VkCommandPoolCreateInfo                       m_vk_create_info = {};
  VkCommandPool                                 m_vk_object = VK_NULL_HANDLE;
  std::vector<std::unique_ptr<CCommandBuffer>>  m_stored_command_buffers;
};

} // namespace vkex

#endif // __VKEX_COMMAND_H__