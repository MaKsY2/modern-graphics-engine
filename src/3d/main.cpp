#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <cmath>

//==================== FABRIK ====================
static float dist3(const glm::vec3 &a, const glm::vec3 &b) { return glm::length(a - b); }
static glm::vec3 norm3(const glm::vec3 &v)
{
    float l = glm::length(v);
    return l > 1e-8f ? v / l : glm::vec3(0);
}

void fabrik3D(std::vector<glm::vec3> &joints,
              const std::vector<float> &L,
              const glm::vec3 &root,
              glm::vec3 target,
              int maxIter,
              float tol)
{
    const int n = (int)joints.size();
    if (n < 2)
        return;

    float totalLen = 0.f;
    for (float l : L)
        totalLen += l;

    if (dist3(root, target) > totalLen)
    {
        joints[0] = root;
        for (int i = 0; i < n - 1; i++)
        {
            glm::vec3 dir = norm3(target - joints[i]);
            joints[i + 1] = joints[i] + dir * L[i];
        }
        return;
    }

    for (int it = 0; it < maxIter; ++it)
    {
        // backward
        joints[n - 1] = target;
        for (int i = n - 2; i >= 0; --i)
        {
            glm::vec3 dir = norm3(joints[i] - joints[i + 1]);
            joints[i] = joints[i + 1] + dir * L[i];
        }
        // forward
        joints[0] = root;
        for (int i = 0; i < n - 1; ++i)
        {
            glm::vec3 dir = norm3(joints[i + 1] - joints[i]);
            joints[i + 1] = joints[i] + dir * L[i];
        }
        if (dist3(joints.back(), target) < tol)
            break;
    }
}

//==================== utils ====================
static std::string loadFile(const char *path)
{
    std::ifstream f(path);
    if (!f)
    {
        std::cerr << "Failed to open: " << path << std::endl;
        return {};
    }
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}
static GLuint compileShader(GLenum type, const char *src)
{
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        char log[4096];
        glGetShaderInfoLog(s, 4096, nullptr, log);
        std::cerr << "Shader error:\n"
                  << log << std::endl;
    }
    return s;
}
static GLuint makeProgram(const std::string &vs, const std::string &fs)
{
    GLuint v = compileShader(GL_VERTEX_SHADER, vs.c_str());
    GLuint f = compileShader(GL_FRAGMENT_SHADER, fs.c_str());
    GLuint p = glCreateProgram();
    glAttachShader(p, v);
    glAttachShader(p, f);
    glLinkProgram(p);
    GLint ok = 0;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok)
    {
        char log[4096];
        glGetProgramInfoLog(p, 4096, nullptr, log);
        std::cerr << "Link error:\n"
                  << log << std::endl;
    }
    glDeleteShader(v);
    glDeleteShader(f);
    return p;
}

//==================== Camera (orbit) ====================
struct OrbitCamera
{
    float radius = 8.0f;
    float yaw = glm::radians(45.0f);
    float pitch = glm::radians(25.0f);
    glm::vec3 target{0, 1, 0};

    glm::mat4 view() const
    {
        float x = target.x + radius * std::cos(pitch) * std::cos(yaw);
        float y = target.y + radius * std::sin(pitch);
        float z = target.z + radius * std::cos(pitch) * std::sin(yaw);
        return glm::lookAt(glm::vec3(x, y, z), target, glm::vec3(0, 1, 0));
    }
};

int main()
{
    // GLFW
    if (!glfwInit())
    {
        std::cerr << "glfwInit failed\n";
        return 1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    GLFWwindow *win = glfwCreateWindow(1280, 800, "FABRIK OpenGL", nullptr, nullptr);
    if (!win)
    {
        std::cerr << "window failed\n";
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(win);
    glfwSwapInterval(1);

    // GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "glewInit failed\n";
        return 1;
    }
    glGetError(); // съесть спонтанный INVALID_ENUM после glewInit на core profile

    // Shaders
    std::string lineVS = loadFile("shaders/line.vert");
    std::string lineFS = loadFile("shaders/line.frag");
    std::string pointVS = loadFile("shaders/point.vert");
    std::string pointFS = loadFile("shaders/point.frag");

    GLuint progLine = makeProgram(lineVS, lineFS);
    GLuint progPoint = makeProgram(pointVS, pointFS);

    GLint uMVP_line = glGetUniformLocation(progLine, "uMVP");
    GLint uMVP_point = glGetUniformLocation(progPoint, "uMVP");
    GLint uColor_point = glGetUniformLocation(progPoint, "uColor");

    // Geometry buffers (динамически обновляемые)
    GLuint vaoLine = 0, vboLine = 0;
    glGenVertexArrays(1, &vaoLine);
    glGenBuffers(1, &vboLine);
    glBindVertexArray(vaoLine);
    glBindBuffer(GL_ARRAY_BUFFER, vboLine);
    glBufferData(GL_ARRAY_BUFFER, 1024 * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW); // небольшой запас
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);
    glBindVertexArray(0);

    GLuint vaoPoint = 0, vboPoint = 0;
    glGenVertexArrays(1, &vaoPoint);
    glGenBuffers(1, &vboPoint);
    glBindVertexArray(vaoPoint);
    glBindBuffer(GL_ARRAY_BUFFER, vboPoint);
    glBufferData(GL_ARRAY_BUFFER, 32 * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);
    glBindVertexArray(0);

    // FABRIK chain (условный «скелет руки»)
    std::vector<glm::vec3> joints = {
        {0, 1, 0}, {0, 2, 0}, {0, 3, 0}, {0, 4, 0} // 4 сустава = 3 сегмента
    };
    std::vector<float> L = {1.f, 1.f, 1.f};
    glm::vec3 root = joints[0];
    glm::vec3 target(2.f, 2.5f, 0.f);

    // Camera + input
    OrbitCamera cam;
    glfwSetWindowUserPointer(win, &cam);
    glfwSetCursorPosCallback(win, [](GLFWwindow *w, double x, double y)
                             {
        OrbitCamera* c = (OrbitCamera*)glfwGetWindowUserPointer(w);
        static double px=x, py=y;
        int pressed = glfwGetMouseButton(w, GLFW_MOUSE_BUTTON_LEFT);
        if (pressed==GLFW_PRESS){
            float dx = float(x - px);
            float dy = float(y - py);
            c->yaw   -= dx * 0.005f;
            c->pitch -= dy * 0.005f;
            c->pitch = glm::clamp(c->pitch, -1.3f, 1.3f);
        }
        px=x; py=y; });
    glfwSetScrollCallback(win, [](GLFWwindow *w, double, double yoff)
                          {
        OrbitCamera* c = (OrbitCamera*)glfwGetWindowUserPointer(w);
        c->radius = glm::clamp(c->radius - float(yoff), 2.0f, 40.0f); });

    glEnable(GL_DEPTH_TEST);
    glPointSize(12.f);

    while (!glfwWindowShouldClose(win))
    {
        glfwPollEvents();

        // управление target: I/J/K/L по XZ, U/O по Y
        if (glfwGetKey(win, GLFW_KEY_J) == GLFW_PRESS)
            target.x -= 0.03f;
        if (glfwGetKey(win, GLFW_KEY_L) == GLFW_PRESS)
            target.x += 0.03f;
        if (glfwGetKey(win, GLFW_KEY_I) == GLFW_PRESS)
            target.z -= 0.03f;
        if (glfwGetKey(win, GLFW_KEY_K) == GLFW_PRESS)
            target.z += 0.03f;
        if (glfwGetKey(win, GLFW_KEY_U) == GLFW_PRESS)
            target.y += 0.03f;
        if (glfwGetKey(win, GLFW_KEY_O) == GLFW_PRESS)
            target.y -= 0.03f;

        // решаем IK
        fabrik3D(joints, L, root, target, 20, 1e-3f);

        // матрицы
        int w, h;
        glfwGetFramebufferSize(win, &w, &h);
        float aspect = (h > 0) ? float(w) / float(h) : 1.f;
        glm::mat4 proj = glm::perspective(glm::radians(45.f), aspect, 0.1f, 100.f);
        glm::mat4 view = cam.view();
        glm::mat4 model(1.0f);
        glm::mat4 mvp = proj * view * model;

        // заливаем буферы
        // линия (кости)
        glBindBuffer(GL_ARRAY_BUFFER, vboLine);
        glBufferSubData(GL_ARRAY_BUFFER, 0, joints.size() * sizeof(glm::vec3), joints.data());

        // точки: все суставы + последним — таргет
        std::vector<glm::vec3> pts = joints;
        pts.push_back(target);
        glBindBuffer(GL_ARRAY_BUFFER, vboPoint);
        glBufferSubData(GL_ARRAY_BUFFER, 0, pts.size() * sizeof(glm::vec3), pts.data());

        // рендер
        glViewport(0, 0, w, h);
        glClearColor(0.08f, 0.09f, 0.12f, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // линии
        glUseProgram(progLine);
        glUniformMatrix4fv(uMVP_line, 1, GL_FALSE, &mvp[0][0]);
        glBindVertexArray(vaoLine);
        glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)joints.size());

        // точки (суставы) — зелёные
        glUseProgram(progPoint);
        glUniformMatrix4fv(uMVP_point, 1, GL_FALSE, &mvp[0][0]);
        glUniform3f(uColor_point, 0.2f, 1.0f, 0.2f);
        glBindVertexArray(vaoPoint);
        glDrawArrays(GL_POINTS, 0, (GLsizei)joints.size());

        // target — красный (последний элемент pts)
        glUniform3f(uColor_point, 1.0f, 0.2f, 0.2f);
        glDrawArrays(GL_POINTS, (GLint)joints.size(), 1);

        glfwSwapBuffers(win);
    }

    // cleanup
    glDeleteBuffers(1, &vboLine);
    glDeleteVertexArrays(1, &vaoLine);
    glDeleteBuffers(1, &vboPoint);
    glDeleteVertexArrays(1, &vaoPoint);
    glDeleteProgram(progLine);
    glDeleteProgram(progPoint);

    glfwDestroyWindow(win);
    glfwTerminate();
    return 0;
}
