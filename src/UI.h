#pragma once

#include "Config.h"

#include <functional>
#include <string>

struct GLFWwindow;

namespace ui {
    namespace internal {
        void PresetStyle();
        void LoadFonts(const char *pFontPath = cpDefaultFontPath);

#ifdef MY_DEBUG
        void ShowDemoWindow();
#endif

    } // namespace internal

    void Init(GLFWwindow *pWindow);
    void NewFrame();
    void Render();
    void Shutdown();

    // widgets
    void FileBrowser(const std::string &title, bool *pOpen, std::string &selectedPath, bool &fileSelected);
    void StandardWindow(const std::string &title, const std::function<void()> &callback);
    void Combo(int &selected, std::vector<std::string> &items, const std::string &preview);

    // helpers
    void HelpMarker(const std::string &text);

} // namespace ui
