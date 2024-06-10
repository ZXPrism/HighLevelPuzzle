#include "Application.h"

#include <chrono>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

#include "Config.h"
#include "Timer.h"
#include "UI.h"

Application gApp;

void Application::Init()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    _pWindow = glfwCreateWindow(cWindowWidth, cWindowHeight, cpWindowTitle, nullptr, nullptr);
    glfwMakeContextCurrent(_pWindow);
    glfwSwapInterval(1);

    auto vidmode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    glfwSetWindowPos(_pWindow, (vidmode->width - cWindowWidth) / 2, (vidmode->height - cWindowHeight) / 2);

    gladLoadGL();
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glViewport(0, 0, cWindowWidth, cWindowHeight);

    InitCallbacks();
    RegisterOnWindowSizeFunc([&](int width, int height) { glViewport(0, 0, width, height); });

    ui::Init(_pWindow);
}

GLFWwindow *Application::GetWindowHandle()
{
    return _pWindow;
}

void Application::Run(const std::function<void(float)> &callback)
{
    namespace ch = std::chrono;

    auto t1 = ch::steady_clock::now();
    float dt = 0.0f;

    while (!glfwWindowShouldClose(_pWindow))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        gTimer.Tick(dt);

        ui::NewFrame();
        callback(dt);
        ui::Render();

        glfwPollEvents();
        glfwSwapBuffers(_pWindow);

        auto t2 = ch::steady_clock::now();
        dt = ch::duration_cast<ch::microseconds>(t2 - t1).count() / 1000000.0;
        t1 = t2;
    }
}

void Application::Shutdown()
{
    ui::Shutdown();

    glfwDestroyWindow(_pWindow);
    glfwTerminate();
}

void Application::InitCallbacks()
{
    glfwSetWindowUserPointer(_pWindow, this);

    glfwSetCursorPosCallback(_pWindow, [](GLFWwindow *window, double xPos, double yPos) {
        ((Application *)glfwGetWindowUserPointer(window))->OnCursorPos(xPos, yPos);
    });
    glfwSetKeyCallback(_pWindow, [](GLFWwindow *window, int key, int scanCode, int action, int mods) {
        ((Application *)glfwGetWindowUserPointer(window))->OnKey(key, scanCode, action, mods);
    });
    glfwSetMouseButtonCallback(_pWindow, [](GLFWwindow *window, int button, int action, int mods) {
        ((Application *)glfwGetWindowUserPointer(window))->OnMouseButton(button, action, mods);
    });
    glfwSetScrollCallback(_pWindow, [](GLFWwindow *window, double xOffset, double yOffset) {
        ((Application *)glfwGetWindowUserPointer(window))->OnScroll(xOffset, yOffset);
    });
    glfwSetWindowSizeCallback(_pWindow, [](GLFWwindow *window, int width, int height) {
        ((Application *)glfwGetWindowUserPointer(window))->OnWindowSize(width, height);
    });
}

void Application::RegisterOnCursorPosFunc(OnCursorPosFunc func)
{
    _OnCursorPosVec.push_back(func);
}

void Application::RegisterOnKeyFunc(OnKeyFunc func)
{
    _OnKeyVec.push_back(func);
}

void Application::RegisterOnMouseButtonFunc(OnMouseButtonFunc func)
{
    _OnMouseButtonVec.push_back(func);
}

void Application::RegisterOnScrollFunc(OnScrollFunc func)
{
    _OnScrollVec.push_back(func);
}

void Application::RegisterOnWindowSizeFunc(OnWindowSizeFunc func)
{
    _OnWindowSizeVec.push_back(func);
}

void Application::OnCursorPos(double xPos, double yPos)
{
    for (auto &fn : _OnCursorPosVec)
        fn(xPos, yPos);
}

void Application::OnKey(int key, int scanCode, int action, int mods)
{
    for (auto &fn : _OnKeyVec)
        fn(key, scanCode, action, mods);
}

void Application::OnMouseButton(int button, int action, int mods)
{
    for (auto &fn : _OnMouseButtonVec)
        fn(button, action, mods);
    ;
}

void Application::OnScroll(double xOffset, double yOffset)
{
    for (auto &fn : _OnScrollVec)
        fn(xOffset, yOffset);
}

void Application::OnWindowSize(int width, int height)
{
    for (auto &fn : _OnWindowSizeVec)
        fn(width, height);
}
