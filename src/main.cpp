#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "utils/shader.hpp"
#include "utils/render_object.hpp"
#include "utils/primitives/sphere.hpp"
#include "utils/primitives/cube.hpp"

#include <iostream>
#include <vector>
#include <chrono>
#include <memory>

#define LOG(msg) std::cout << "[INFO] " << msg << std::endl
#define LOG_ERROR(msg) std::cerr << "[ERROR] " << msg << std::endl

class OpenGLCubeApp
{
public:
    void run()
    {
        initWindow();
        initOpenGL();
        mainLoop();
        cleanup();
    }

private:
    GLFWwindow *window;
    std::shared_ptr<Shader> shader;

    glm::vec3 cameraPos = glm::vec3(2.0f, 2.0f, 2.0f);
    float cameraYaw = -45.0f;
    float cameraPitch = -35.0f;

    const int WIDTH = 1280;
    const int HEIGHT = 800;

    void initWindow()
    {
        LOG("Initializing GLFW window...");
        if (!glfwInit())
        {
            throw std::runtime_error("Failed to initialize GLFW");
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

        window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL Cube", nullptr, nullptr);
        if (!window)
        {
            glfwTerminate();
            throw std::runtime_error("Failed to create GLFW window");
        }

        glfwMakeContextCurrent(window);
        glfwSetWindowUserPointer(window, this);
        LOG("Window created successfully");
    }

    void initOpenGL()
    {
        LOG("Initializing OpenGL...");

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            throw std::runtime_error("Failed to initialize GLAD");
        }

        LOG("OpenGL Version: " << glGetString(GL_VERSION));
        LOG("GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION));

        shader = std::make_shared<Shader>("shaders/vert.vert", "shaders/frag.frag");

        glEnable(GL_DEPTH_TEST);

        LOG("OpenGL initialization complete!");
    }

    void mainLoop()
    {
        LOG("Entering main loop...");
        LOG("Camera position: (" << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z << ")");

        glfwSetKeyCallback(window, [](GLFWwindow *win, int key, int scancode, int action, int mods)
                           {
            auto app = reinterpret_cast<OpenGLCubeApp*>(glfwGetWindowUserPointer(win));
            if (action == GLFW_PRESS || action == GLFW_REPEAT) {
                float speed = 0.1f;
                glm::vec3 forward;
                forward.x = cos(glm::radians(app->cameraYaw)) * cos(glm::radians(app->cameraPitch));
                forward.y = sin(glm::radians(app->cameraPitch));
                forward.z = sin(glm::radians(app->cameraYaw)) * cos(glm::radians(app->cameraPitch));
                forward = glm::normalize(forward);
                glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
                
                if (key == GLFW_KEY_W) app->cameraPos += forward * speed;
                if (key == GLFW_KEY_S) app->cameraPos -= forward * speed;
                if (key == GLFW_KEY_A) app->cameraPos -= right * speed;
                if (key == GLFW_KEY_D) app->cameraPos += right * speed;
                if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(win, true);
            } });

        glfwSetCursorPosCallback(window, [](GLFWwindow *win, double xpos, double ypos)
                                 {
            static double lastX = 640, lastY = 400;
            static bool firstMouse = true;
            
            if (glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
                if (firstMouse) {
                    lastX = xpos;
                    lastY = ypos;
                    firstMouse = false;
                }
                
                float xoffset = xpos - lastX;
                float yoffset = lastY - ypos;
                lastX = xpos;
                lastY = ypos;
                

                // это пиздец с reinterpret_cast, но работает 
                auto app = reinterpret_cast<OpenGLCubeApp*>(glfwGetWindowUserPointer(win));
                app->cameraYaw += xoffset * 0.1f;
                app->cameraPitch += yoffset * 0.1f;
                
                if (app->cameraPitch > 89.0f) app->cameraPitch = 89.0f;
                if (app->cameraPitch < -89.0f) app->cameraPitch = -89.0f;
            } else {
                firstMouse = true;
            } });

        std::cout << "\nControls:\n";
        std::cout << "  W/A/S/D - Move camera\n";
        std::cout << "  Right Mouse + Drag - Rotate camera\n";
        std::cout << "  ESC - Exit\n\n";

        auto startTime = std::chrono::high_resolution_clock::now();

        std::vector<std::unique_ptr<utils::RenderObject>> scene;

        auto sphere1 = std::make_unique<primitives::Sphere>(1.0f, 64, 128, shader);
        sphere1->transform.position = {0, 0, 0};
        sphere1->transform.dirty = true;
        sphere1->color = {1, 1, 0, 1};
        scene.push_back(std::move(sphere1));

        auto cube1 = std::make_unique<primitives::Cube>(glm::vec3(1.0f, 1.0f, 1.0f), shader);
        cube1->transform.position = {2, 0, 0};
        cube1->transform.dirty = true;
        cube1->color = {1, 0, 0, 1};
        scene.push_back(std::move(cube1));

        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();

            auto currentTime = std::chrono::high_resolution_clock::now();
            float time = std::chrono::duration<float>(currentTime - startTime).count();

            glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            shader->use();

            glm::vec3 cubeCenter(0.0f, 0.0f, 0.0f);
            glm::mat4 view = glm::lookAt(cameraPos, cubeCenter, glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);

            shader->setMat4("view", view);
            shader->setMat4("proj", proj);

            float angle = time * glm::radians(3.0f);

            for (auto &obj : scene)
            {
                obj->transform.rotation = glm::angleAxis(angle, glm::vec3(0, 1, 0));
                obj->transform.dirty = true;

                obj->draw(proj * view);
            }

            glfwSwapBuffers(window);
        }

        LOG("Exiting main loop");
    }

    void cleanup()
    {
        LOG("Cleaning up...");
        shader.reset();
        glfwDestroyWindow(window);
        glfwTerminate();
        LOG("Cleanup complete");
    }
};

int main()
{
    OpenGLCubeApp app;

    try
    {
        app.run();
    }
    catch (const std::exception &e)
    {
        LOG_ERROR(e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}