/* coding: utf-8 */
/**
 * YorabaTaiju/Wakaba
 *
 * Copyright 2019-, Kaede Fujisaki
 */

#include "BufferBuilder.hpp"
#include "../Vulkan.hpp"
#include "../buffer/Buffer.hpp"

namespace vk {

BufferBuilder::BufferBuilder(std::shared_ptr<Vulkan> vulkan)
:vulkan_(std::move(vulkan))
{
  this->vkBufferCreateInfo_ = {
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .size = 0,
      .usage = 0,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 0, // TODO?
      .pQueueFamilyIndices = nullptr, // TODO?
  };
}

std::shared_ptr<Buffer> BufferBuilder::build() {
  VkBuffer vkBuffer;
  this->vkBufferCreateInfo_.size = this->size_;
  this->vkBufferCreateInfo_.usage = this->usages_;
  if (vkCreateBuffer(vulkan_->vkDevice(), &vkBufferCreateInfo_, nullptr, &vkBuffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to create buffer!");
  }
  return std::make_shared<Buffer>(vulkan_, vkBuffer, size_);
}

BufferBuilder& BufferBuilder::setSize(VkDeviceSize const size) {
  this->size_ = size;
  return *this;
}

BufferBuilder &BufferBuilder::addUsages(VkBufferUsageFlags usage) {
  this->usages_ |= usage;
  return *this;
}

BufferBuilder &BufferBuilder::setUsages(VkBufferUsageFlags usages) {
  this->usages_ = usages;
  return *this;
}

BufferBuilder &BufferBuilder::clearUsages() {
  this->usages_ = 0;
  return *this;
}

}