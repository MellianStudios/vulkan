#ifndef MELLIANCLIENT_WINDOW_H
#define MELLIANCLIENT_WINDOW_H

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <string>

class Window
{
public:
    Window(int w, int h, std::string name) : width{w}, height{h}, name{name}
    {
        initWindow();
    }

    ~Window()
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    bool shouldClose()
    {
        return glfwWindowShouldClose(window);
    }

    void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface)
    {
        if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface");
        }
    }

    VkExtent2D getExtent()
    {
        return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
    }

    bool wasWindowResized()
    {
        return frame_buffer_resize;
    }

    void resetWindowResizeFlag()
    {
        frame_buffer_resize = false;
    }

private:
    int width;
    int height;
    bool frame_buffer_resize = false;
    std::string name;
    GLFWwindow *window;

    void initWindow()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);

        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, frameBufferResizeCallback);
    }

    static void frameBufferResizeCallback(GLFWwindow *window, int width, int height)
    {
        auto new_window = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));

        new_window->frame_buffer_resize = true;
        new_window->width = width;
        new_window->height = height;
    }
};

#endif //MELLIANCLIENT_WINDOW_H
