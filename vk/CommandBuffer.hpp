/* coding: utf-8 */
/**
 * YorabaTaiju/Wakaba
 *
 * Copyright 2019-, Kaede Fujisaki
 */

#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <memory>
#include <utility>
#include <functional>

namespace vk {

class Vulkan;
class VulkanBuilder;
class CommandPool;
class Buffer;
class Framebuffer;
class GraphicsPipeline;

class CommandBuffer final {
private:
  std::weak_ptr<Vulkan> vulkan_;
  std::shared_ptr<CommandPool> commandPool_;
  VkCommandBuffer vkCommandBuffer_;
public:
  CommandBuffer() = delete;
  CommandBuffer(CommandBuffer const&) = delete;
  CommandBuffer(CommandBuffer&&) = default;
  CommandBuffer& operator=(CommandBuffer const&) = delete;
  CommandBuffer& operator=(CommandBuffer&&) = default;
  explicit CommandBuffer(std::weak_ptr<Vulkan> vulkan, std::shared_ptr<CommandPool> commandPool, VkCommandBuffer vkCommandBuffer)
  : vulkan_(std::move(vulkan))
  , commandPool_(std::move(commandPool))
  , vkCommandBuffer_(vkCommandBuffer)
  {
  }
  ~CommandBuffer() noexcept;

public:
  void recordOneshot(std::function<void(std::shared_ptr<Vulkan> const&, CommandBuffer&)>);
  void recordRenderPass(Framebuffer& framebuffer, std::function<void(std::shared_ptr<Vulkan> const&, CommandBuffer&)>);
  void record(std::function<void(std::shared_ptr<Vulkan> const&, CommandBuffer&)>);

public:
  void bindPipeline(GraphicsPipeline& graphicsPipeilne);
  void bindVertexBuffer(uint32_t bindingPoint, Buffer& buffer);
  void draw(uint32_t vertices, uint32_t instances);

public:
  void copyBufferSync(Buffer& dst, VkDeviceSize dstOffset, Buffer& src, VkDeviceSize srcOffset, VkDeviceSize size);
  void executeSync();

public:
  [[ nodiscard ]] VkCommandBuffer vkCommandBuffer() { return this->vkCommandBuffer_;}
  [[ nodiscard ]] std::shared_ptr<CommandPool> commandPool() { return this->commandPool_;}
};

}
