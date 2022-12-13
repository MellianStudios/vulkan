#ifndef MELLIANCLIENT_APP_H
#define MELLIANCLIENT_APP_H

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <memory>
#include <stdexcept>
#include "Device.h"
#include "GameObject.h"
#include "Renderer.h"
#include "RenderSystem.h"
#include "Window.h"

class App
{
public:
    static constexpr int WIDTH = 1280;
    static constexpr int HEIGHT = 720;

    App()
    {
        loadGameObjects();
    }

    App(const App &) = delete;

    App &operator=(App &&) = delete;

    void run()
    {
        RenderSystem render_system{device, renderer.getSwapChainRenderPass()};

        while (!window.shouldClose()) {
            glfwPollEvents();

            if (auto command_buffer = renderer.beginFrame()) {
                renderer.beginSwapChainRenderPass(command_buffer);
                render_system.renderGameObjects(command_buffer, game_objects);
                renderer.endSwapChainRenderPass(command_buffer);
                renderer.endFrame();
            }
        }

        vkDeviceWaitIdle(device.device());
    }

private:
    Window window{WIDTH, HEIGHT, "WoW"};
    Device device{window};
    Renderer renderer{window, device};
    std::vector<GameObject> game_objects;

    void loadGameObjects()
    {
        std::vector<Model::Vertex> vertices{
            {{0.0f,  -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f,  0.5f},  {0.0f, 1.0f, 0.0f}},
            {{-0.5f, 0.5f},  {0.0f, 0.0f, 1.0f}},
        };

        auto model = std::make_shared<Model>(device, vertices);

        auto triangle = GameObject::createGameObject();

        triangle.model = model;
        triangle.color = {.1f, .8f, .1f};
        triangle.transform_2d.translation.x = .2f;
        triangle.transform_2d.scale = {2.f, .5f};
        triangle.transform_2d.rotation = .25f * glm::two_pi<float>();

        game_objects.push_back(std::move(triangle));
    }
};

#endif //MELLIANCLIENT_APP_H
