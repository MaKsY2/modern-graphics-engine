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
#include <array>
#include "obj_loader/gltfLoaderTiny.hpp"

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

    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
    float cameraYaw = -90.0f;
    float cameraPitch = 0.0f;

    std::array<bool, GLFW_KEY_LAST + 1> keys{};
    bool rotating = false;
    bool firstMouse = true;

    double lastX = 0.0;
    double lastY = 0.0;

    float mouseSensetive = 0.1f;
    float moveSpeed = 3.0f;

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
            (void)scancode; (void)mods;
    auto *app = reinterpret_cast<OpenGLCubeApp*>(glfwGetWindowUserPointer(win));
    if (!app) return;

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(win, true);

    if (key >= 0 && key <= GLFW_KEY_LAST) {
        if (action == GLFW_PRESS)   app->keys[key] = true;
        if (action == GLFW_RELEASE) app->keys[key] = false;
    } });

        glfwSetMouseButtonCallback(window, [](GLFWwindow *win, int button, int action, int mods)
                                   {
    (void)mods;
    auto *app = reinterpret_cast<OpenGLCubeApp*>(glfwGetWindowUserPointer(win));
    if (!app) return;

    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            app->rotating = true;
            app->firstMouse = true; 
        } else if (action == GLFW_RELEASE) {
            app->rotating = false;
        }
    } });

        glfwSetCursorPosCallback(window, [](GLFWwindow *win, double xpos, double ypos)
                                 {
    auto *app = reinterpret_cast<OpenGLCubeApp*>(glfwGetWindowUserPointer(win));
    if (!app || !app->rotating) return;

    if (app->firstMouse) {
        app->lastX = xpos;
        app->lastY = ypos;
        app->firstMouse = false;
        return;
    }

    float xoffset = static_cast<float>(xpos - app->lastX);
    float yoffset = static_cast<float>(app->lastY - ypos);

    app->lastX = xpos;
    app->lastY = ypos;

    app->cameraYaw   += xoffset * app->mouseSensetive;
    app->cameraPitch += yoffset * app->mouseSensetive;

    if (app->cameraPitch > 89.0f)  app->cameraPitch = 89.0f;
    if (app->cameraPitch < -89.0f) app->cameraPitch = -89.0f; });
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

        loader::LoadedMeshPU data = loader::LoadGLB_ToCPU_PU("assets/power_armor.glb", false);

        const auto objectMesh = std::make_shared<utils::Mesh>();
        objectMesh->upload(data.vertices, data.indices);

        auto loadedObject = std::make_unique<utils::RenderObject>(objectMesh, shader);
        loadedObject->color = {1, 0, 0, 1};

        float lastFrame = static_cast<float>(glfwGetTime());

        while (!glfwWindowShouldClose(window))
        {

            glfwPollEvents();

            float now = (float)glfwGetTime();
            float dt = now - lastFrame;
            lastFrame = now;

            // направление взгляда из yaw/pitch
            glm::vec3 forward;
            forward.x = cos(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));
            forward.y = sin(glm::radians(cameraPitch));
            forward.z = sin(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));
            forward = glm::normalize(forward);

            glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));

            float v = moveSpeed * dt;
            if (keys[GLFW_KEY_W])
                cameraPos += forward * v;
            if (keys[GLFW_KEY_S])
                cameraPos -= forward * v;
            if (keys[GLFW_KEY_A])
                cameraPos -= right * v;
            if (keys[GLFW_KEY_D])
                cameraPos += right * v;

            auto currentTime = std::chrono::high_resolution_clock::now();
            float time = std::chrono::duration<float>(currentTime - startTime).count();

            glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            shader->use();

            glm::mat4 view = glm::lookAt(cameraPos, cameraPos + forward, glm::vec3(0.0f, 1.0f, 0.0f));

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