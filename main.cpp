/* coding: utf-8 */
/**
 * YorabaTaiju/Wakaba
 *
 * Copyright 2019-, Kaede Fujisaki
 */

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <memory>

#include "util/Logger.hpp"
#include "vk/Vulkan.hpp"
#include "vk/Device.hpp"
#include "vk/builder/VulkanBuilder.hpp"
#include "vk/builder/VertexBufferBuilder.hpp"
#include "vk/builder/GraphicsPipelineBuilder.hpp"
#include "vk/builder/RenderPassBuilder.hpp"
#include "vk/Util.hpp"
#include "vk/RenderPass.hpp"
#include "taiju/shaders/Triangle.hpp"
#include "vk/buffer/VertexBuffer.hpp"
#include "vk/command/CommandPool.hpp"
#include "vk/command/PrimaryCommandBuffer.hpp"
#include "vk/command/SecondaryCommandBuffer.hpp"
#include "vk/Framebuffer.hpp"
#include "vk/image/SwapchainImage.hpp"
#include "vk/builder/FramebufferBuilder.hpp"
#include "vk/builder/RenderingDispatcherBuilder.hpp"
#include "vk/Swapchain.hpp"
#include "vk/helper/Bridge.hpp"

static int main_impl(util::Logger& log);
static int mainLoop(util::Logger& log, const std::shared_ptr<vk::Vulkan>& vulkan);

int main() {
  util::Logger log;

  log.setLevel(util::Logger::DEBUG);
  try {
    return main_impl(log);
  } catch (std::exception &e) {
    fprintf(stderr, "Unhandled exception: \"%s\"\n", e.what());
    return -255;
  } catch (...) {
    fprintf(stderr, "Unknown exception!\n");
    return -255;
  }
}

static int main_impl(util::Logger& log) {
  // Initialise GLFW
  if (!glfwInit()) {
    log.fatal("Failed to initialize GLFW");
    return -1;
  }
  log.debug("GLFW Initialized.");

  if (!glfwVulkanSupported()) {
    log.fatal("Vulkan not supported");
    return -1;
  }
  log.debug("Vulkan is supported.");

  {
    std::shared_ptr<vk::Vulkan> vulkan = vk::VulkanBuilder(log, "YorabaTaiju", 1920, 1080).build();
    try {
      mainLoop(log, vulkan);
    } catch (std::exception& e) {
      log.error(e.what());
    } catch(...) {
      log.error("Got unknown error!");
    }
  }

  glfwTerminate();
  return 0;
}

static int mainLoop(util::Logger& log, std::shared_ptr<vk::Vulkan> const& vulkan) {
  // Ensure we can capture the escape key being pressed below
  glfwSetInputMode(vulkan->window(), GLFW_STICKY_KEYS, GL_TRUE);

  auto device = vulkan->createDevice();
  auto swapchain = device->createSwapchain();
  auto bridge = vk::Bridge(device);

  auto renderPassBuilder = vk::RenderPassBuilder(device);
  renderPassBuilder.addSubPass().addColor(0);
  renderPassBuilder.addAttachment(swapchain->vkSwapchainFormat().format).loadOpClear().storeOpStore();

  auto vert = device->createShader<taiju::shaders::Triangle::Vert>();
  auto frag = device->createShader<taiju::shaders::Triangle::Frag>();
  auto renderPass = renderPassBuilder.build();
  auto gfxPipeline = vk::GraphicsPipelineBuilder(device, renderPass)
      .addVertexStage(vert)
      .addFragmentStage(frag).build();

  std::vector<taiju::shaders::Triangle::Vert::Input> vertInputs = {
      {
          .pos = {0.0f, -0.5f},
          .color = {1.0f, 0.0f, 0.0f},
      },
      {
          .pos = {0.5f, 0.5f},
          .color = {0.0f, 1.0f, 0.0f},
      },
      {
          .pos = {-0.5f, 0.5f},
          .color = {0.0f, 0.0f, 1.0f},
      },
  };

  auto vertBuffer = vk::VertexBufferBuilder(device, vertInputs.size() * sizeof(taiju::shaders::Triangle::Vert::Input)).build();
  vertBuffer.update(bridge, vertInputs);

  std::vector<vk::Framebuffer> framebuffers;
  for(std::shared_ptr<vk::SwapchainImage>& image : swapchain->images()) {
    framebuffers.emplace_back(
        vk::FramebufferBuilder(device, vulkan->width(), vulkan->height(), renderPass)
            .addColor(image, {0.0f, 0.0f, 0.0f, 1.0f})
            .build());
  }

  auto dispatcher = vk::RenderingDispatcherBuilder(device, swapchain).build();

  do {
    dispatcher.dispatch([&](std::shared_ptr<vk::CommandPool> const& cmdPool, uint32_t frameIndex) -> void {
      vk::PrimaryCommandBuffer cmd = cmdPool->createPrimaryBuffer();
      std::vector<vk::SecondaryCommandBuffer> subCmds;
      subCmds.emplace_back(cmdPool->createSecondaryBuffer());
      subCmds[0].record(framebuffers[frameIndex], [&]() -> void {
        subCmds[0].bindPipeline(gfxPipeline);
        subCmds[0].bindVertexBuffer(0, vertBuffer.buffer());
        subCmds[0].draw(3, 1);
      });

      //
      cmd.recordRenderPass(framebuffers[frameIndex], subCmds);
      dispatcher.submit(std::move(subCmds[0]));
      dispatcher.submit(std::move(cmd));
    });

    // Swap buffers
    glfwSwapBuffers(vulkan->window());
    glfwPollEvents();
  } // Check if the ESC key was pressed or the window was closed
  while (glfwGetKey(vulkan->window(), GLFW_KEY_ESCAPE) != GLFW_PRESS &&
         glfwWindowShouldClose(vulkan->window()) == 0);

  return 0;
}
