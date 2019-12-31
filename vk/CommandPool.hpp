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
#include "../util/Shared.hpp"

namespace vk {

class Vulkan;

class CommandPool {
private:
  std::weak_ptr<Vulkan> vulkan_;
  VkCommandPool vkCommandPool_;
public:
  explicit CommandPool(std::weak_ptr<Vulkan> vulkan, VkCommandPool vkObj)
  : vulkan_(std::move(vulkan))
  , vkCommandPool_(vkObj)
  {

  }
  ~CommandPool() noexcept;

public:
  [[ nodiscard ]] VkCommandPool vkCommandPool() {
    return this->vkCommandPool_;
  }
public:
  ENABLE_SHARED_HELPER
};

}
