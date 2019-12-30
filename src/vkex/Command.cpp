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

#include "vkex/Command.h"
#include "vkex/Device.h"
#include "vkex/ToString.h"

namespace vkex {

// =================================================================================================
// CommandBuffer
// =================================================================================================
CCommandBuffer::CCommandBuffer()
{
}

CCommandBuffer::~CCommandBuffer()
{
}

vkex::Result CCommandBuffer::InternalCreate(
  const vkex::CommandBufferCreateInfo&  create_info,
  const VkAllocationCallbacks*          p_allocator
)
{
  // Copy create info
  m_create_info = create_info;

  return vkex::Result::Success;
}

vkex::Result CCommandBuffer::InternalDestroy(const VkAllocationCallbacks* p_allocator)
{
  m_create_info.vk_object = VK_NULL_HANDLE;

  return vkex::Result::Success;
}

vkex::Result CCommandBuffer::Begin(VkCommandBufferUsageFlags flags)
{
  VkCommandBufferBeginInfo vk_begin_info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
  vk_begin_info.flags             = flags;
  vk_begin_info.pInheritanceInfo  = nullptr;

  VkResult vk_result = InvalidValue<VkResult>::Value;
  VKEX_VULKAN_RESULT_CALL(
    vk_result,
    vkex::BeginCommandBuffer(
      m_create_info.vk_object,
      &vk_begin_info)
  );
  if (vk_result != VK_SUCCESS) {
    return vkex::Result(vk_result);
  }

  return vkex::Result::Success;
}

vkex::Result CCommandBuffer::End()
{
  VkResult vk_result = InvalidValue<VkResult>::Value;
  VKEX_VULKAN_RESULT_CALL(
    vk_result,
    vkex::EndCommandBuffer(
      m_create_info.vk_object)
  );
  if (vk_result != VK_SUCCESS) {
    return vkex::Result(vk_result);
  }

  return vkex::Result::Success;  
}

// -------------------------------------------------------------------------------------------------
// Command functions that mirror the vkCmd* interface
// -------------------------------------------------------------------------------------------------
void CCommandBuffer::CmdBindPipeline(VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline)
{
  VkCommandBuffer vk_command_buffer = GetVkObject();
  vkex::CmdBindPipeline(
    vk_command_buffer,
    pipelineBindPoint,
    pipeline);
}

void CCommandBuffer::CmdSetViewport(uint32_t firstViewport, uint32_t viewportCount, const VkViewport* pViewports)
{
  VkCommandBuffer vk_command_buffer = GetVkObject();
  vkex::CmdSetViewport(
    vk_command_buffer,
    firstViewport,
    viewportCount,
    pViewports);
}

void CCommandBuffer::CmdSetScissor(uint32_t firstScissor, uint32_t scissorCount, const VkRect2D* pScissors)
{
  VkCommandBuffer vk_command_buffer = GetVkObject();
  vkex::CmdSetScissor(
    vk_command_buffer,
    firstScissor,
    scissorCount,
    pScissors);
}

void CCommandBuffer::CmdSetLineWidth(float lineWidth)
{
  VkCommandBuffer vk_command_buffer = GetVkObject();
  vkex::CmdSetLineWidth(
    vk_command_buffer,
    lineWidth);
}

void CCommandBuffer::CmdSetDepthBias(float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor)
{
  VkCommandBuffer vk_command_buffer = GetVkObject();
  vkex::CmdSetDepthBias(
    vk_command_buffer,
    depthBiasConstantFactor,
    depthBiasClamp,
    depthBiasSlopeFactor);
}

void CCommandBuffer::CmdSetBlendConstants(const float blendConstants[4])
{
  VkCommandBuffer vk_command_buffer = GetVkObject();
  vkex::CmdSetBlendConstants(
    vk_command_buffer,
    blendConstants);
}

void CCommandBuffer::CmdSetDepthBounds(float minDepthBounds, float maxDepthBounds)
{
  VkCommandBuffer vk_command_buffer = GetVkObject();
  vkex::CmdSetDepthBounds(
    vk_command_buffer,
    minDepthBounds,
    maxDepthBounds);
}

void CCommandBuffer::CmdSetStencilCompareMask(VkStencilFaceFlags faceMask, uint32_t compareMask)
{
  VkCommandBuffer vk_command_buffer = GetVkObject();
  vkex::CmdSetStencilCompareMask(
    vk_command_buffer,
    faceMask,
    compareMask);
}

void CCommandBuffer::CmdSetStencilWriteMask(VkStencilFaceFlags faceMask, uint32_t writeMask)
{
  VkCommandBuffer vk_command_buffer = GetVkObject();
  vkex::CmdSetStencilWriteMask(
    vk_command_buffer,
    faceMask,
    writeMask);
}

void CCommandBuffer::CmdSetStencilReference(VkStencilFaceFlags faceMask, uint32_t reference)
{
  VkCommandBuffer vk_command_buffer = GetVkObject();
  vkex::CmdSetStencilReference(
    vk_command_buffer,
    faceMask,
    reference);
}

void CCommandBuffer::CmdBindDescriptorSets(VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets)
{
  VkCommandBuffer vk_command_buffer = GetVkObject();
  vkex::CmdBindDescriptorSets(
    vk_command_buffer,
    pipelineBindPoint,
    layout,
    firstSet,
    descriptorSetCount,
    pDescriptorSets,
    dynamicOffsetCount,
    pDynamicOffsets);
}

void CCommandBuffer::CmdBindIndexBuffer(VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType)
{
  VkCommandBuffer vk_command_buffer = GetVkObject();
  vkex::CmdBindIndexBuffer(
    vk_command_buffer,
    buffer,
    offset,
    indexType);
}

void CCommandBuffer::CmdBindVertexBuffers(uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets)
{
  VkCommandBuffer vk_command_buffer = GetVkObject();
  vkex::CmdBindVertexBuffers(
    vk_command_buffer,
    firstBinding,
    bindingCount,
    pBuffers,
    pOffsets);
}

void CCommandBuffer::CmdDraw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
  VkCommandBuffer vk_command_buffer = GetVkObject();
  vkex::CmdDraw(
    vk_command_buffer,
    vertexCount,
    instanceCount,
    firstVertex,
    firstInstance);
}

void CCommandBuffer::CmdDrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
  VkCommandBuffer vk_command_buffer = GetVkObject();
  vkex::CmdDrawIndexed(
    vk_command_buffer,
    indexCount,
    instanceCount,
    firstIndex,
    vertexOffset,
    firstInstance);
}

void CCommandBuffer::CmdDrawIndirect(VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride)
{
  VkCommandBuffer vk_command_buffer = GetVkObject();
  vkex::CmdDrawIndirect(
    vk_command_buffer,
    buffer,
    offset,
    drawCount,
    stride);
}

void CCommandBuffer::CmdDrawIndexedIndirect(VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride)
{
  VkCommandBuffer vk_command_buffer = GetVkObject();
  vkex::CmdDrawIndexedIndirect(
    vk_command_buffer,
    buffer,
    offset,
    drawCount,
    stride);
}

void CCommandBuffer::CmdDispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
  VkCommandBuffer vk_command_buffer = GetVkObject();
  vkex::CmdDispatch(
    vk_command_buffer,
    groupCountX,
    groupCountY,
    groupCountZ);
}

void CCommandBuffer::CmdDispatchIndirect(VkBuffer buffer, VkDeviceSize offset)
{
  VkCommandBuffer vk_command_buffer = GetVkObject();
  vkex::CmdDispatchIndirect(
    vk_command_buffer,
    buffer,
    offset);
}

void CCommandBuffer::CmdCopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferCopy* pRegions)
{
  VkCommandBuffer vk_command_buffer = GetVkObject();
  vkex::CmdCopyBuffer(
    vk_command_buffer,
    srcBuffer,
    dstBuffer,
    regionCount,
    pRegions);
}

void CCommandBuffer::CmdCopyImage(VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy* pRegions)
{
  VkCommandBuffer vk_command_buffer = GetVkObject();
  vkex::CmdCopyImage(
    vk_command_buffer,
    srcImage,
    srcImageLayout,
    dstImage,
    dstImageLayout,
    regionCount,
    pRegions);
}

void CCommandBuffer::CmdBlitImage(VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit* pRegions, VkFilter filter)
{
  VkCommandBuffer vk_command_buffer = GetVkObject();
  vkex::CmdBlitImage(
    vk_command_buffer,
    srcImage,
    srcImageLayout,
    dstImage,
    dstImageLayout,
    regionCount,
    pRegions,
    filter);
}

void CCommandBuffer::CmdCopyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy* pRegions)
{
  VkCommandBuffer vk_command_buffer = GetVkObject();
  vkex::CmdCopyBufferToImage(
    vk_command_buffer,
    srcBuffer,
    dstImage,
    dstImageLayout,
    regionCount,
    pRegions);
}

void CCommandBuffer::CmdCopyImageToBuffer(VkImage srcImage, VkImageLayout srcImageLayout, VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy* pRegions)
{
  VkCommandBuffer vk_command_buffer = GetVkObject();
  vkex::CmdCopyImageToBuffer(
    vk_command_buffer,
    srcImage,
    srcImageLayout,
    dstBuffer,
    regionCount,
    pRegions);
}

void CCommandBuffer::CmdUpdateBuffer(VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize dataSize, const void* pData)
{
  VkCommandBuffer vk_command_buffer = GetVkObject();
  vkex::CmdUpdateBuffer(
    vk_command_buffer,
    dstBuffer,
    dstOffset,
    dataSize,
    pData);
}

void CCommandBuffer::CmdFillBuffer(VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size, uint32_t data)
{
  VkCommandBuffer vk_command_buffer = GetVkObject();
  vkex::CmdFillBuffer(
    vk_command_buffer,
    dstBuffer,
    dstOffset,
    size,
    data);
}

void CCommandBuffer::CmdClearColorImage(VkImage image, VkImageLayout imageLayout, const VkClearColorValue* pColor, uint32_t rangeCount, const VkImageSubresourceRange* pRanges)
{
  VkCommandBuffer vk_command_buffer = GetVkObject();
  vkex::CmdClearColorImage(
    vk_command_buffer,
    image,
    imageLayout,
    pColor,
    rangeCount,
    pRanges);
}

void CCommandBuffer::CmdClearDepthStencilImage(VkImage image, VkImageLayout imageLayout, const VkClearDepthStencilValue* pDepthStencil, uint32_t rangeCount, const VkImageSubresourceRange* pRanges)
{
  VkCommandBuffer vk_command_buffer = GetVkObject();
  vkex::CmdClearDepthStencilImage(
    vk_command_buffer,
    image,
    imageLayout, 
    pDepthStencil, 
    rangeCount, 
    pRanges);
}

void CCommandBuffer::CmdClearAttachments(uint32_t attachmentCount, const VkClearAttachment* pAttachments, uint32_t rectCount, const VkClearRect* pRects)
{
  VkCommandBuffer vk_command_buffer = GetVkObject();
  vkex::CmdClearAttachments(
    vk_command_buffer,
    attachmentCount,
    pAttachments,
    rectCount,
    pRects);
}

void CCommandBuffer::CmdResolveImage(VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageResolve* pRegions)
{
  VkCommandBuffer vk_command_buffer = GetVkObject();
  vkex::CmdResolveImage(
    vk_command_buffer,
    srcImage,
    srcImageLayout,
    dstImage,
    dstImageLayout,
    regionCount,
    pRegions);
}

void CCommandBuffer::CmdSetEvent(VkEvent event, VkPipelineStageFlags stageMask)
{
  VkCommandBuffer vk_command_buffer = GetVkObject();
  vkex::CmdSetEvent(
    vk_command_buffer,
    event,
    stageMask);
}

void CCommandBuffer::CmdResetEvent(VkEvent event, VkPipelineStageFlags stageMask)
{
  VkCommandBuffer vk_command_buffer = GetVkObject();
  vkex::CmdResetEvent(
    vk_command_buffer,
    event,
    stageMask);
}

void CCommandBuffer::CmdWaitEvents(uint32_t eventCount, const VkEvent* pEvents, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers)
{
  VkCommandBuffer vk_command_buffer = GetVkObject();
  vkex::CmdWaitEvents(
    vk_command_buffer,
    eventCount,
    pEvents,
    srcStageMask,
    dstStageMask,
    memoryBarrierCount,
    pMemoryBarriers,
    bufferMemoryBarrierCount,
    pBufferMemoryBarriers,
    imageMemoryBarrierCount,
    pImageMemoryBarriers);
}

void CCommandBuffer::CmdPipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier* pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier* pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier* pImageMemoryBarriers)
{
  VkCommandBuffer vk_command_buffer = GetVkObject();
  vkex::CmdPipelineBarrier(
    vk_command_buffer,
    srcStageMask,
    dstStageMask,
    dependencyFlags,
    memoryBarrierCount, 
    pMemoryBarriers,
    bufferMemoryBarrierCount,
    pBufferMemoryBarriers,
    imageMemoryBarrierCount, 
    pImageMemoryBarriers);
}

void CCommandBuffer::CmdBeginQuery(VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags)
{
  VkCommandBuffer vk_command_buffer = GetVkObject();
  vkex::CmdBeginQuery(
    vk_command_buffer,
    queryPool,
    query,
    flags);
}

void CCommandBuffer::CmdEndQuery(VkQueryPool queryPool, uint32_t query)
{
  VkCommandBuffer vk_command_buffer = GetVkObject();
  vkex::CmdEndQuery(
    vk_command_buffer,
    queryPool,
    query);
}

void CCommandBuffer::CmdResetQueryPool(VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount)
{
  VkCommandBuffer vk_command_buffer = GetVkObject();
  vkex::CmdResetQueryPool(
    vk_command_buffer,
    queryPool,
    firstQuery,
    queryCount);
}

void CCommandBuffer::CmdWriteTimestamp(VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool, uint32_t query)
{
  VkCommandBuffer vk_command_buffer = GetVkObject();
  vkex::CmdWriteTimestamp(
    vk_command_buffer,
    pipelineStage,
    queryPool,
    query);
}

void CCommandBuffer::CmdCopyQueryPoolResults(VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags)
{
  VkCommandBuffer vk_command_buffer = GetVkObject();
  vkex::CmdCopyQueryPoolResults(
    vk_command_buffer,
    queryPool,
    firstQuery,
    queryCount,
    dstBuffer,
    dstOffset,
    stride,
    flags);
}

void CCommandBuffer::CmdPushConstants(VkPipelineLayout layout, VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size, const void* pValues)
{
  VkCommandBuffer vk_command_buffer = GetVkObject();
  vkex::CmdPushConstants(
    vk_command_buffer,
    layout,
    stageFlags,
    offset,
    size,
    pValues);
}

void CCommandBuffer::CmdBeginRenderPass(const VkRenderPassBeginInfo* pRenderPassBegin, VkSubpassContents contents)
{
  VkCommandBuffer vk_command_buffer = GetVkObject();
  vkex::CmdBeginRenderPass(
    vk_command_buffer,
    pRenderPassBegin,
    contents);
}

void CCommandBuffer::CmdNextSubpass(VkSubpassContents contents)
{
  VkCommandBuffer vk_command_buffer = GetVkObject();
  vkex::CmdNextSubpass(
    vk_command_buffer,
    contents);
}

void CCommandBuffer::CmdEndRenderPass()
{
  VkCommandBuffer vk_command_buffer = GetVkObject();
  vkex::CmdEndRenderPass(
    vk_command_buffer);
}

void CCommandBuffer::CmdExecuteCommands(uint32_t commandBufferCount, const VkCommandBuffer* pCommandBuffers)
{
  VkCommandBuffer vk_command_buffer = GetVkObject();
  vkex::CmdExecuteCommands(
    vk_command_buffer,
    commandBufferCount,
    pCommandBuffers);
}

// -----------------------------------------------------------------------------------------------
// Command functions with convenience parameters
// -----------------------------------------------------------------------------------------------
void CCommandBuffer::CmdBindPipeline(vkex::ComputePipeline pipeline)
{
  this->CmdBindPipeline(VK_PIPELINE_BIND_POINT_COMPUTE, *pipeline);
}

void CCommandBuffer::CmdBindPipeline(vkex::GraphicsPipeline pipeline)
{
  this->CmdBindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline);
}

void CCommandBuffer::CmdSetViewport(uint32_t firstViewport, const std::vector<VkViewport>* pViewports)
{
  this->CmdSetViewport(firstViewport, CountU32(*pViewports), DataPtr(*pViewports));
}

void CCommandBuffer::CmdSetViewport(const VkRect2D& area, float minDepth, float maxDepth)
{
  float x       = static_cast<float>(area.offset.x);
  float y       = static_cast<float>(area.offset.y);
  float width   = static_cast<float>(area.extent.width);
  float height  = static_cast<float>(area.extent.height);

  VkViewport view_port = {};
  view_port.x        = x;
  view_port.y        = height;
  view_port.width    = width;
  view_port.height   = -height;
  view_port.minDepth = minDepth;
  view_port.maxDepth = maxDepth;
  this->CmdSetViewport(0, 1, &view_port);
}

void CCommandBuffer::CmdSetScissor(uint32_t firstScissor, const std::vector<VkRect2D>* pScissors)
{
  this->CmdSetScissor(firstScissor, CountU32(*pScissors), DataPtr(*pScissors));
}

void CCommandBuffer::CmdSetScissor(const VkRect2D& area)
{
  this->CmdSetScissor(0, 1, &area);
}

void CCommandBuffer::CmdSetBlendConstants(float bc0, float bc1, float bc2, float bc3)
{
}

void CCommandBuffer::CmdBindDescriptorSets(VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, const std::vector<VkDescriptorSet>& descriptorSets, const std::vector<uint32_t>* pDynamicOffsets)
{
  this->CmdBindDescriptorSets(
    pipelineBindPoint, 
    layout, 
    firstSet,
    CountU32(descriptorSets),
    DataPtr(descriptorSets),
    (pDynamicOffsets != nullptr ? CountU32(*pDynamicOffsets) : 0),
    (pDynamicOffsets != nullptr ? DataPtr(*pDynamicOffsets)  : nullptr));
}

void CCommandBuffer::CmdBindIndexBuffer(vkex::Buffer buffer, VkDeviceSize offset, VkIndexType indexType)
{
  VkBuffer vk_buffer = *buffer;
  this->CmdBindIndexBuffer(vk_buffer, offset, indexType);
}

void CCommandBuffer::CmdBindVertexBuffers(uint32_t firstBinding, const std::vector<VkBuffer>* pBuffers, const VkDeviceSize* pOffsets)
{
}

void CCommandBuffer::CmdBindVertexBuffers(vkex::Buffer buffer, VkDeviceSize offset)
{
  VkBuffer vk_buffer = *buffer;
  this->CmdBindVertexBuffers(0, 1, &vk_buffer, &offset);
}

void CCommandBuffer::CmdCopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, const std::vector<VkBufferCopy>* pRegions)
{
}

void CCommandBuffer::CmdCopyImage(VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, const std::vector<VkImageCopy>* pRegions)
{
}

void CCommandBuffer::CmdBlitImage(VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, const std::vector<VkImageBlit>* pRegions, VkFilter filter)
{
}

void CCommandBuffer::CmdCopyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, const std::vector<VkBufferImageCopy>* pRegions)
{
}

void CCommandBuffer::CmdCopyImageToBuffer(VkImage srcImage, VkImageLayout srcImageLayout, VkBuffer dstBuffer, const std::vector<VkBufferImageCopy>* pRegions)
{
}

void CCommandBuffer::CmdUpdateBuffer(VkBuffer dstBuffer, VkDeviceSize dstOffset, const std::vector<uint8_t>* pData)
{
}

void CCommandBuffer::CmdClearColorImage(VkImage image, VkImageLayout imageLayout, const VkClearColorValue* pColor, const std::vector<VkImageSubresourceRange>* pRanges)
{
}

void CCommandBuffer::CmdClearDepthStencilImage(VkImage image, VkImageLayout imageLayout, const VkClearDepthStencilValue* pDepthStencil, const std::vector<VkImageSubresourceRange>* pRanges)
{
}

void CCommandBuffer::CmdClearAttachments(uint32_t attachmentCount, const VkClearAttachment* pAttachments, const std::vector<VkClearRect>* pRects)
{
}

void CCommandBuffer::CmdResolveImage(VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, const std::vector<VkImageResolve>* pRegions)
{
}

void CCommandBuffer::CmdWaitEvents(const std::vector<VkEvent>* pEvents, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, const std::vector<VkMemoryBarrier>* pMemoryBarriers, const std::vector<VkBufferMemoryBarrier>* pBufferMemoryBarriers, const std::vector<VkImageMemoryBarrier>* pImageMemoryBarriers)
{
}

void CCommandBuffer::CmdPipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, const std::vector<VkMemoryBarrier>* pMemoryBarriers, const std::vector<VkBufferMemoryBarrier>* pBufferMemoryBarriers, const std::vector<VkImageMemoryBarrier>* pImageMemoryBarriers)
{
}

void CCommandBuffer::CmdBeginQuery(vkex::QueryPool queryPool, uint32_t query, VkQueryControlFlags flags)
{
  this->CmdBeginQuery(
    queryPool->GetVkObject(),
    query,
    flags);
}

void CCommandBuffer::CmdEndQuery(vkex::QueryPool queryPool, uint32_t query)
{
  this->CmdEndQuery(
    queryPool->GetVkObject(),
    query);
}

void CCommandBuffer::CmdResetQueryPool(vkex::QueryPool queryPool, uint32_t firstQuery, uint32_t queryCount)
{
  this->CmdResetQueryPool(
    queryPool->GetVkObject(),
    firstQuery,
    queryCount);
}

void CCommandBuffer::CmdWriteTimestamp(VkPipelineStageFlagBits pipelineStage, vkex::QueryPool queryPool, uint32_t query)
{
  this->CmdWriteTimestamp(
    pipelineStage,
    queryPool->GetVkObject(),
    query);
}

void CCommandBuffer::CmdCopyQueryPoolResults(vkex::QueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags)
{
  this->CmdCopyQueryPoolResults(
    queryPool->GetVkObject(),
    firstQuery,
    queryCount,
    dstBuffer,
    dstOffset,
    stride,
    flags);
}

void CCommandBuffer::CmdPushConstants(VkPipelineLayout layout, VkShaderStageFlags stageFlags, uint32_t offset, const std::vector<uint8_t>* pValues)
{
}

void CCommandBuffer::CmdBeginRenderPass(const vkex::RenderPass renderPass, uint32_t clearValueCount, const VkClearValue* pClearValues, VkSubpassContents contents)
{
  const VkRect2D& fullRenderArea = renderPass->GetFullRenderArea();
  VkRenderPassBeginInfo vk_begin_info = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
    vk_begin_info.renderPass      = renderPass->GetVkObject();
    vk_begin_info.framebuffer     = renderPass->GetVkFramebufferObject();
    vk_begin_info.renderArea      = fullRenderArea;
    vk_begin_info.clearValueCount = clearValueCount;
    vk_begin_info.pClearValues    = pClearValues;
  this->CmdBeginRenderPass(
    &vk_begin_info,
    contents);
}

void CCommandBuffer::CmdBeginRenderPass(const vkex::RenderPass renderPass, const std::vector<VkClearValue>* pClearValues, VkSubpassContents contents)
{
  this->CmdBeginRenderPass(
    renderPass,
    CountU32(*pClearValues),
    DataPtr(*pClearValues),
    contents);
}

void CCommandBuffer::CmdBeginRenderPass(const vkex::RenderPass renderPass, VkSubpassContents contents)
{
  const std::vector<VkClearValue>& clearValues = renderPass->GetClearValues();
  this->CmdBeginRenderPass(
    renderPass,
    CountU32(clearValues),
    DataPtr(clearValues),
    contents);
}

void CCommandBuffer::CmdExecuteCommands(const std::vector<VkCommandBuffer>* pCommandBuffers)
{
}

void CCommandBuffer::CmdTransitionImageLayout(VkImage image, VkImageAspectFlags aspectMask, uint32_t baseMipLevel, uint32_t levelCount, uint32_t baseArrayLayer, uint32_t layerCount, VkImageLayout oldLayout, VkImageLayout newLayout, VkPipelineStageFlags newPipelineStage)
{
  VKEX_ASSERT(newLayout != VK_IMAGE_LAYOUT_UNDEFINED);
  VKEX_ASSERT(newLayout != VK_IMAGE_LAYOUT_PREINITIALIZED);
  if ((newLayout == VK_IMAGE_LAYOUT_UNDEFINED) || (newLayout == VK_IMAGE_LAYOUT_PREINITIALIZED)) {
    VKEX_ASSERT_MSG(false, "Invalid destination image layout");
  }

  VkPipelineStageFlags src_stage_mask     = 0;
  VkPipelineStageFlags dst_stage_mask     = 0;
  VkDependencyFlags    dependency_flags   = 0;

  VkImageMemoryBarrier barrier            = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
  barrier.oldLayout                       = VK_IMAGE_LAYOUT_GENERAL;
  barrier.newLayout                       = VK_IMAGE_LAYOUT_GENERAL;
  barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
  barrier.image                           = image;
  barrier.subresourceRange.aspectMask     = aspectMask;
  barrier.subresourceRange.baseMipLevel   = baseMipLevel;
  barrier.subresourceRange.levelCount     = levelCount,
  barrier.subresourceRange.baseArrayLayer = baseArrayLayer;
  barrier.subresourceRange.layerCount     = layerCount;

  switch (oldLayout) {
    default: VKEX_ASSERT_MSG(false, "unsupported image layout"); break;

    case VK_IMAGE_LAYOUT_UNDEFINED: {
      src_stage_mask        = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
      barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
      barrier.oldLayout     = VK_IMAGE_LAYOUT_UNDEFINED;
    } break;

    case VK_IMAGE_LAYOUT_GENERAL: {
      // @TODO: This may need tweaking.
      src_stage_mask        = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
      barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
      barrier.oldLayout     = VK_IMAGE_LAYOUT_GENERAL;
    } break;

    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: {
      src_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      barrier.srcAccessMask =
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
      barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    } break;

    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL: {
      src_stage_mask        = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
      barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
      barrier.oldLayout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    } break;

    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL: {
      src_stage_mask        = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
      barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
      barrier.oldLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    } break;

    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: {
      src_stage_mask        = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
      barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
      barrier.oldLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    } break;

    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL: {
      src_stage_mask        = VK_PIPELINE_STAGE_TRANSFER_BIT;
      barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
      barrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    } break;

    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: {
      src_stage_mask        = VK_PIPELINE_STAGE_TRANSFER_BIT;
      barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      barrier.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    } break;

    case VK_IMAGE_LAYOUT_PREINITIALIZED: {
      // @TODO: This may need tweaking.
      src_stage_mask        = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
      barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
      barrier.oldLayout     = VK_IMAGE_LAYOUT_PREINITIALIZED;
    } break;

    case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL: {
      // @TODO: This may need tweaking.
      src_stage_mask        = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
      barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
      barrier.oldLayout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    } break;

    case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL: {
      // @TODO: This may need tweaking.
      src_stage_mask        = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
      barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
      barrier.oldLayout     = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL;
    } break;

    case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: {
      src_stage_mask        = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
      barrier.srcAccessMask = 0;
      barrier.oldLayout     = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    } break;
  }

  switch (newLayout) {
    default:
      VKEX_ASSERT_MSG(false, "unsupported image layout");
      break;

      //
      // Note: VK_IMAGE_LAYOUT_UNDEFINED cannot be a destination layout.
      //
      // case VK_IMAGE_LAYOUT_UNDEFINED: break;

    case VK_IMAGE_LAYOUT_GENERAL: {
      dst_stage_mask        = newPipelineStage;
      barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
      barrier.newLayout     = VK_IMAGE_LAYOUT_GENERAL;
    } break;

    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: {
      dst_stage_mask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
      barrier.newLayout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    } break;

    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL: {
      dst_stage_mask        = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
      barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
      barrier.newLayout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    } break;

    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL: {
      dst_stage_mask        = newPipelineStage;
      barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
      barrier.newLayout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    } break;

    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: {
      dst_stage_mask        = newPipelineStage;
      barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
      barrier.newLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    } break;

    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL: {
      dst_stage_mask        = VK_PIPELINE_STAGE_TRANSFER_BIT;
      barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
      barrier.newLayout     = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    } break;

    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL: {
      dst_stage_mask        = VK_PIPELINE_STAGE_TRANSFER_BIT;
      barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      barrier.newLayout     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    } break;

    //
    // Note: VK_IMAGE_LAYOUT_UNDEFINED cannot be a destination layout.
    //
    // case VK_IMAGE_LAYOUT_PREINITIALIZED: break;

    case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL: {
      dst_stage_mask        = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
      barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
      barrier.newLayout     = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL;
    } break;

    case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL: {
      dst_stage_mask        = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
      barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
      barrier.newLayout     = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL;
    } break;

    case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: {
      dst_stage_mask        = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
      barrier.dstAccessMask = 0;
      barrier.newLayout     = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    } break;
  } 

  this->CmdPipelineBarrier(src_stage_mask, dst_stage_mask, dependency_flags, 0, nullptr, 0, nullptr, 1, &barrier);
}

void CCommandBuffer::CmdTransitionImageLayout(vkex::Texture texture, VkImageLayout oldLayout, VkImageLayout newLayout, VkPipelineStageFlags newPipelineStage)
{
  this->CmdTransitionImageLayout(
    *(texture->GetImage()),
    texture->GetAspectFlags(),
    0,
    texture->GetMipLevels(),
    0,
    texture->GetArrayLayers(),
    oldLayout,
    newLayout,
    newPipelineStage);
}

// =================================================================================================
// CommandPool
// =================================================================================================
CCommandPool::CCommandPool()
{
}

CCommandPool::~CCommandPool()
{
}

vkex::Result CCommandPool::InternalCreate(
  const vkex::CommandPoolCreateInfo&  create_info,
  const VkAllocationCallbacks*        p_allocator
)
{
  // Copy create info
  m_create_info = create_info;

  // Vulkan create info
  {
    m_vk_create_info = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
    m_vk_create_info.flags            = m_create_info.flags.flags;
    m_vk_create_info.queueFamilyIndex = m_create_info.queue_family_index;
  }

  // Create Vulkan object
  {
    VkResult vk_result = InvalidValue<VkResult>::Value;
    VKEX_VULKAN_RESULT_CALL(
      vk_result,
      vkex::CreateCommandPool(
        *m_device,
        &m_vk_create_info,
        p_allocator,
        &m_vk_object)
    );
    if (vk_result != VK_SUCCESS) {
      return vkex::Result(vk_result);
    }
  }

  return vkex::Result::Success;
}

vkex::Result CCommandPool::InternalDestroy(const VkAllocationCallbacks* p_allocator)
{
  // Collect command buffers
  std::vector<vkex::CommandBuffer> command_buffers;
  for (auto& obj : m_stored_command_buffers) {
    command_buffers.push_back(obj.get());
  }
  // Free command buffers
  FreeCommandBuffers(&command_buffers);

  if (m_vk_object != VK_NULL_HANDLE) {
    vkex::DestroyCommandPool(
      *m_device,
      m_vk_object,
      p_allocator);

    m_vk_object = VK_NULL_HANDLE;
  }

  return vkex::Result::Success;
}

vkex::Result CCommandPool::AllocateCommandBuffers(
  const vkex::CommandBufferAllocateInfo&  allocate_info, 
  vkex::CommandBuffer*                    p_command_buffers
)
{
  if (allocate_info.command_buffer_count == 0) {
    return vkex::Result::ErrorCommandBufferCountMustNotBeZero;
  }

  VkCommandBufferAllocateInfo vk_allocate_info = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
  vk_allocate_info.commandPool        = m_vk_object;
  vk_allocate_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  vk_allocate_info.commandBufferCount = allocate_info.command_buffer_count;

  std::vector<VkCommandBuffer> vk_command_buffers(allocate_info.command_buffer_count);
  VkResult vk_result = InvalidValue<VkResult>::Value;
  VKEX_VULKAN_RESULT_CALL(
    vk_result,
    vkex::AllocateCommandBuffers(
      *m_device,
      &vk_allocate_info,
      vk_command_buffers.data())
  );
  if (vk_result != VK_SUCCESS) {
    return vkex::Result(vk_result);
  }

  std::vector<vkex::CommandBuffer> command_buffers;
  vkex::Result vkex_result = vkex::Result::Undefined;
  for (uint32_t i = 0; i < allocate_info.command_buffer_count; ++i) {
    vkex::CommandBufferCreateInfo command_buffer_create_info = {};
    command_buffer_create_info.vk_object = vk_command_buffers[i];
    
    vkex::CommandBuffer command_buffer = nullptr;
    vkex_result = CreateObject<CCommandBuffer>(
      command_buffer_create_info,
      nullptr,
      m_stored_command_buffers,
      &CCommandBuffer::SetPool,
      this,
      &command_buffer);

    if (!vkex_result) {
      break;
    }

    command_buffers.push_back(command_buffer);
  }

  if (!vkex_result) {
    this->FreeCommandBuffers(
      CountU32(command_buffers),
      DataPtr(command_buffers));
    return vkex_result;
  }

  for (uint32_t i = 0; i < allocate_info.command_buffer_count; ++i) {
    vkex::CommandBuffer command_buffer = command_buffers[i];
    p_command_buffers[i] = command_buffer;
  }
  
  return vkex::Result::Success;
}
  
vkex::Result CCommandPool::AllocateCommandBuffers(
  const vkex::CommandBufferAllocateInfo&  allocate_info, 
  std::vector<vkex::CommandBuffer>*       p_command_buffers
)
{
  if (allocate_info.command_buffer_count == 0) {
    return vkex::Result::ErrorCommandBufferCountMustNotBeZero;
  }

  std::vector<vkex::CommandBuffer> command_buffers(allocate_info.command_buffer_count);
  vkex::Result vkex_result = AllocateCommandBuffers(
    allocate_info,
    command_buffers.data());
  
  if (!vkex_result) {
    return vkex_result;
  }

  for (auto& command_buffer : command_buffers) {
    p_command_buffers->push_back(command_buffer);
  }

  return vkex::Result::Success;
}

vkex::Result CCommandPool::AllocateCommandBuffer(
  const vkex::CommandBufferAllocateInfo&  allocate_info, 
  vkex::CommandBuffer*                    p_command_buffer
)
{
  if (allocate_info.command_buffer_count != 1) {
    return vkex::Result::ErrorCommandBufferCountMustBeOne;
  }

  std::vector<vkex::CommandBuffer> command_buffers(allocate_info.command_buffer_count);
  vkex::Result vkex_result = AllocateCommandBuffers(
    allocate_info,
    command_buffers.data());
  
  if (!vkex_result) {
    return vkex_result;
  }

  *p_command_buffer = command_buffers[0];

  return vkex::Result::Success;
}

void CCommandPool::FreeCommandBuffers(
  uint32_t                    command_buffer_count, 
  const vkex::CommandBuffer*  p_command_buffers
)
{
  std::vector<VkCommandBuffer> vk_command_buffers;
  for (uint32_t i = 0; i < command_buffer_count; ++i) {
    vkex::CommandBuffer command_buffer = p_command_buffers[i];
    // Copy Vulkan object
    vk_command_buffers.push_back(command_buffer->GetVkObject());
    // Destroy the stored object
    DestroyObject<CCommandBuffer>(
      m_stored_command_buffers,      
      command_buffer,
      nullptr);
  }

  vkex::FreeCommandBuffers(
    *m_device,
    m_vk_object,
    CountU32(vk_command_buffers),
    DataPtr(vk_command_buffers));
}

void CCommandPool::FreeCommandBuffers(const std::vector<vkex::CommandBuffer>* p_command_buffers)
{
  this->FreeCommandBuffers(
    CountU32(*p_command_buffers),
    DataPtr(*p_command_buffers));
}

void CCommandPool::FreeCommandBuffer(const vkex::CommandBuffer command_buffer)
{
  this->FreeCommandBuffers(
    1,
    &command_buffer);  
}

} // namespace vkex