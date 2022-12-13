#ifndef MELLIANCLIENT_RENDERER_H
#define MELLIANCLIENT_RENDERER_H

#include <cassert>
#include <memory>
#include <stdexcept>
#include "Device.h"
#include "SwapChain.h"
#include "Window.h"

class Renderer
{
public:
    Renderer(Window &window, Device &device) : window{window}, device{device}
    {
        recreateSwapChain();
        createCommandBuffers();
    }

    ~Renderer()
    {
        freeCommandBuffers();
    }

    Renderer(const Renderer &) = delete;

    Renderer &operator=(Renderer &&) = delete;

    bool isFrameInProgress() const
    {
        return is_frame_started;
    }

    VkCommandBuffer getCurrentCommandBuffer() const
    {
        assert(is_frame_started && "cannot get command buffer when frame not in progress");

        return command_buffers[current_frame_index];
    }

    VkRenderPass getSwapChainRenderPass() const
    {
        return swap_chain->getRenderPass();
    }

    VkCommandBuffer beginFrame()
    {
        assert(!is_frame_started && "cannot call beginFrame while already in progress");

        auto result = swap_chain->acquireNextImage(&current_image_index);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();

            return nullptr;
        }

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image");
        }

        is_frame_started = true;

        auto command_buffer = getCurrentCommandBuffer();

        VkCommandBufferBeginInfo begin_info{};

        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer");
        }

        return command_buffer;
    }

    void endFrame()
    {
        assert(is_frame_started && "cannot call endFrame while frame is not in progress");

        auto command_buffer = getCurrentCommandBuffer();

        if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer");
        }

        auto result = swap_chain->submitCommandBuffers(&command_buffer, &current_image_index);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.wasWindowResized()) {
            window.resetWindowResizeFlag();
            recreateSwapChain();
        } else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image");
        }

        is_frame_started = false;
        current_frame_index = (current_frame_index + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT;
    }

    void beginSwapChainRenderPass(VkCommandBuffer command_buffer)
    {
        assert(is_frame_started && "cannot call beginSwapChainRenderPass if frame is not in progress");
        assert(
            command_buffer == getCurrentCommandBuffer()
            &&
            "cannot begin render pass on command buffer from a different frame"
        );

        VkRenderPassBeginInfo render_pass_info{};

        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_info.renderPass = swap_chain->getRenderPass();
        render_pass_info.framebuffer = swap_chain->getFrameBuffer(current_image_index);

        render_pass_info.renderArea.offset = {0, 0};
        render_pass_info.renderArea.extent = swap_chain->getSwapChainExtent();

        std::array<VkClearValue, 2> clear_values{};

        clear_values[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
        clear_values[1].depthStencil = {1.0f, 0};

        render_pass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
        render_pass_info.pClearValues = clear_values.data();

        vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};

        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(swap_chain->getSwapChainExtent().width);
        viewport.height = static_cast<float>(swap_chain->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{{0, 0}, swap_chain->getSwapChainExtent()};

        vkCmdSetViewport(command_buffer, 0, 1, &viewport);
        vkCmdSetScissor(command_buffer, 0, 1, &scissor);
    }

    void endSwapChainRenderPass(VkCommandBuffer command_buffer)
    {
        assert(is_frame_started && "cannot call endSwapChainRenderPass if frame is not in progress");
        assert(
            command_buffer == getCurrentCommandBuffer()
            &&
            "cannot end render pass on command buffer from a different frame"
        );

        vkCmdEndRenderPass(command_buffer);
    }

    int getFrameIndex() const
    {
        assert(is_frame_started && "cannot get frame index when frame is not in progress");

        return current_frame_index;
    }

private:
    Window &window;
    Device &device;
    std::unique_ptr<SwapChain> swap_chain;
    std::vector<VkCommandBuffer> command_buffers;
    uint32_t current_image_index;
    int current_frame_index{0};
    bool is_frame_started{false};

    void createCommandBuffers()
    {
        command_buffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo alloc_info{};

        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandPool = device.getCommandPool();
        alloc_info.commandBufferCount = static_cast<uint32_t>(command_buffers.size());

        if (vkAllocateCommandBuffers(device.device(), &alloc_info, command_buffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers");
        }
    }

    void freeCommandBuffers()
    {
        vkFreeCommandBuffers(
            device.device(),
            device.getCommandPool(),
            static_cast<uint32_t>(command_buffers.size()),
            command_buffers.data()
        );

        command_buffers.clear();
    }

    void recreateSwapChain()
    {
        auto extent = window.getExtent();

        while (extent.width == 0 || extent.height == 0) {
            extent = window.getExtent();
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(device.device());

        if (swap_chain == nullptr) {
            swap_chain = std::make_unique<SwapChain>(device, extent);
        } else {
            std::shared_ptr<SwapChain> old_swap_chain = std::move(swap_chain);

            swap_chain = std::make_unique<SwapChain>(device, extent, old_swap_chain);

            if (!old_swap_chain->compareSwapFormats(*swap_chain.get())) {
                throw std::runtime_error("swap chain image(or depth) format has changed");
            }

            if (swap_chain->imageCount() != command_buffers.size()) {
                freeCommandBuffers();
                createCommandBuffers();
            }
        }
    }
};

#endif //MELLIANCLIENT_RENDERER_H
