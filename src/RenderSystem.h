#ifndef MELLIANCLIENT_RENDERSYSTEM_H
#define MELLIANCLIENT_RENDERSYSTEM_H

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <memory>
#include <stdexcept>
#include "Device.h"
#include "GameObject.h"
#include "Pipeline.h"

struct SimplePushConstantData
{
    glm::mat2 transform{1.f};
    glm::vec2 offset;
    alignas(16) glm::vec3 color;
};

class RenderSystem
{
public:
    RenderSystem(Device &device, VkRenderPass render_pass) : device{device}
    {
        createPipelineLayout();
        createPipeline(render_pass);
    }

    ~RenderSystem()
    {
        vkDestroyPipelineLayout(device.device(), pipeline_layout, nullptr);
    }

    RenderSystem(const RenderSystem &) = delete;

    RenderSystem &operator=(RenderSystem &&) = delete;

    void renderGameObjects(VkCommandBuffer command_buffer, std::vector<GameObject> &game_objects)
    {
        pipeline->bind(command_buffer);

        for (auto &object: game_objects) {
            object.transform_2d.rotation = glm::mod(object.transform_2d.rotation + .01f, glm::two_pi<float>());

            SimplePushConstantData push{};

            push.offset = object.transform_2d.translation;
            push.color = object.color;
            push.transform = object.transform_2d.mat2();

            vkCmdPushConstants(
                command_buffer,
                pipeline_layout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(SimplePushConstantData),
                &push
            );

            object.model->bind(command_buffer);
            object.model->draw(command_buffer);
        }
    }

private:
    Device &device;
    std::unique_ptr<Pipeline> pipeline;
    VkPipelineLayout pipeline_layout;

    void createPipelineLayout()
    {
        VkPushConstantRange push_constant_range;

        push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        push_constant_range.offset = 0;
        push_constant_range.size = sizeof(SimplePushConstantData);

        VkPipelineLayoutCreateInfo pipeline_layout_info{};

        pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_info.setLayoutCount = 0;
        pipeline_layout_info.pSetLayouts = nullptr;
        pipeline_layout_info.pushConstantRangeCount = 1;
        pipeline_layout_info.pPushConstantRanges = &push_constant_range;

        if (vkCreatePipelineLayout(device.device(), &pipeline_layout_info, nullptr, &pipeline_layout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout");
        }
    }

    void createPipeline(VkRenderPass render_pass)
    {
        assert(pipeline_layout != nullptr && "cannot create pipeline before pipeline layout");

        PipelineConfigInfo pipeline_config{};

        Pipeline::defaultPipelineConfigInfo(pipeline_config);

        pipeline_config.render_pass = render_pass;
        pipeline_config.pipeline_layout = pipeline_layout;

        pipeline = std::make_unique<Pipeline>(
            device,
            pipeline_config,
            "../../src/Shaders/shader.vert.spv",
            "../../src/Shaders/shader.frag.spv"
        );
    }
};

#endif //MELLIANCLIENT_RENDERSYSTEM_H
