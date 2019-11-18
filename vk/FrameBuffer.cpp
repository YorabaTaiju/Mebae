//
// Created by kaede on 2019/11/13.
//

#include "FrameBuffer.hpp"
#include "Vulkan.hpp"

namespace vk {

vk::FrameBuffer::~FrameBuffer() {
  std::shared_ptr<Vulkan> vulkan =  vulkan_.lock();
  if(vulkan) {
    vkDestroyFramebuffer(vulkan->device(), this->vkObj_, nullptr);
  }
}

}