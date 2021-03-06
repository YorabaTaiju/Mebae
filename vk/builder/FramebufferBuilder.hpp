/* coding: utf-8 */
/**
 * YorabaTaiju/Wakaba
 *
 * Copyright 2019-, Kaede Fujisaki
 */

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <memory>
#include <vector>
#include <optional>

namespace vk {

class Device;
class RenderPass;
class Image;
class Framebuffer;

class FramebufferBuilder {
private:
  std::shared_ptr<Device> const device_;
  std::shared_ptr<RenderPass> const renderPass_;
  uint32_t const width_;
  uint32_t const height_;
private:
  std::vector<std::shared_ptr<Image>> images_;
  std::vector<VkClearValue> clears_;
public:
  FramebufferBuilder() = delete;
  FramebufferBuilder(FramebufferBuilder&&) = delete;
  FramebufferBuilder(FramebufferBuilder const&) = delete;
  FramebufferBuilder& operator=(FramebufferBuilder&&) = delete;
  FramebufferBuilder& operator=(FramebufferBuilder const&) = delete;

public:
  explicit FramebufferBuilder(std::shared_ptr<Device> device, uint32_t width, uint32_t height, std::shared_ptr<RenderPass> renderPass);
  FramebufferBuilder& addColor(std::shared_ptr<Image> image, std::array<float, 4> const& clearValue);
  FramebufferBuilder& addDepth(std::shared_ptr<Image> image, float clearValue);
  FramebufferBuilder& addStencil(std::shared_ptr<Image> image, uint32_t value);

  Framebuffer build();
};

}
