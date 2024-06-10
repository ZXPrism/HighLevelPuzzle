#pragma once

#include <functional>

struct GLFWwindow;

class Application
{
public:
    void Init();
    void Run(const std::function<void(float)> &callback);
    void Shutdown();

    GLFWwindow *GetWindowHandle();

    using OnCursorPosFunc = std::function<void(double, double)>;
    using OnKeyFunc = std::function<void(int, int, int, int)>;
    using OnMouseButtonFunc = std::function<void(int, int, int)>;
    using OnScrollFunc = std::function<void(double, double)>;
    using OnWindowSizeFunc = std::function<void(int, int)>;

    void InitCallbacks();

    void RegisterOnKeyFunc(OnKeyFunc func);
    void RegisterOnMouseButtonFunc(OnMouseButtonFunc func);
    void RegisterOnCursorPosFunc(OnCursorPosFunc func);
    void RegisterOnScrollFunc(OnScrollFunc func);
    void RegisterOnWindowSizeFunc(OnWindowSizeFunc func);

    void OnKey(int key, int scanCode, int action, int mods);
    void OnMouseButton(int button, int action, int mods);
    void OnCursorPos(double xPos, double yPos);
    void OnScroll(double xOffset, double yOffset);
    void OnWindowSize(int width, int height);

private:
    GLFWwindow *_pWindow;

    std::vector<OnCursorPosFunc> _OnCursorPosVec;
    std::vector<OnKeyFunc> _OnKeyVec;
    std::vector<OnMouseButtonFunc> _OnMouseButtonVec;
    std::vector<OnScrollFunc> _OnScrollVec;
    std::vector<OnWindowSizeFunc> _OnWindowSizeVec;
};

extern Application gApp;
