#include "UI.h"

#include <filesystem>
#include <iostream>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>

#include "Config.h"

namespace ui {
    namespace internal {
        void PresetStyle()
        {
            auto &style = ImGui::GetStyle();
            auto &colors = style.Colors;

            colors[ImGuiCol_TitleBg] = {0.113f, 0.113f, 0.113f, 1.0f};
            colors[ImGuiCol_TitleBgActive] = {0.187f, 0.187f, 0.187f, 1.0f};

            colors[ImGuiCol_Tab] = {0.113f, 0.113f, 0.113f, 1.0f};
            colors[ImGuiCol_TabUnfocusedActive] = {0.113f, 0.113f, 0.113f, 1.0f};
            colors[ImGuiCol_TabActive] = {0.5f, 0.5f, 0.5f, 1.0f};
            colors[ImGuiCol_TabHovered] = {0.3f, 0.3f, 0.3f, 1.0f};

            colors[ImGuiCol_Header] = {0.5f, 0.5f, 0.5f, 1.0f};
            colors[ImGuiCol_HeaderHovered] = {0.7f, 0.7f, 0.7f, 1.0f};

            colors[ImGuiCol_FrameBg] = {0.3f, 0.3f, 0.3f, 1.0f};

            colors[ImGuiCol_Button] = {0.5f, 0.5f, 0.8f, 1.0f};

            style.WindowPadding = {8, 10};
            style.FramePadding = {8, 8};
            style.ItemSpacing = {8, 8};
            style.ItemInnerSpacing = {8, 8};
            style.ScrollbarSize = 16;
            style.FrameRounding = 4;
            style.WindowRounding = 8;
        }

        void ShowDemoWindow()
        {
            ImGui::ShowDemoWindow();
        }

        void LoadFonts(const char *pFontPath)
        {
            auto &io = ImGui::GetIO();

            ImFontConfig fontConfig;
            fontConfig.MergeMode = true;
            fontConfig.PixelSnapH = true;
            io.Fonts->AddFontFromFileTTF(pFontPath, cDefaultFontSize, NULL, io.Fonts->GetGlyphRangesDefault());
        }

    } // namespace internal

    void Init(GLFWwindow *pWindow)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui_ImplGlfw_InitForOpenGL(pWindow, true);
        ImGui_ImplOpenGL3_Init(cpGlslVersion);
        ImGui::StyleColorsDark();

        internal::PresetStyle();
        internal::LoadFonts();
    }

    void NewFrame()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void Render()
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void Shutdown()
    {
        ImGui_ImplGlfw_Shutdown();
        ImGui_ImplOpenGL3_Shutdown();
        ImGui::DestroyContext();
    }

    void FileBrowser(const std::string &title, bool *p_open, std::string &selectedPath, bool &fileSelected)
    {
        namespace fs = std::filesystem;

        static bool initialized = false;
        static std::string initialPath;
        static std::string currentPath;

        fileSelected = false;

        if (!initialized)
        {
            initialPath = fs::current_path().string();
            currentPath = initialPath;
            initialized = true;
        }

        if (*p_open)
        {
            ImGui::OpenPopup(title.c_str());
            *p_open = false;
        }

        ImGui::PushStyleVar(ImGuiStyleVar_WindowTitleAlign, ImVec2(0.5f, 0.5f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

        if (ImGui::BeginPopupModal(title.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
        {
            ImVec2 displaySize = ImGui::GetIO().DisplaySize;
            ImVec2 windowSize = ImGui::GetWindowSize();
            ImVec2 centerPos = ImVec2((displaySize.x - windowSize.x) * 0.5f, (displaySize.y - windowSize.y) * 0.5f);
            ImGui::SetWindowPos(centerPos);

            if (!currentPath.empty())
            {
                ImGui::Text("Current Path: %s", currentPath.c_str());
                ImGui::Separator();
            }

            if (ImGui::Button("Home"))
            {
                currentPath = initialPath;
            }
            ImGui::SameLine();

            if (currentPath != fs::current_path().root_path().string())
            {
                if (ImGui::Button(".."))
                {
                    currentPath = fs::path(currentPath).parent_path().string();
                }
            }

            ImGui::Separator();

            for (const auto &entry : fs::directory_iterator(currentPath))
            {
                const auto &path = entry.path();
                std::string filename = path.filename().string();

                if (entry.is_directory())
                {
                    if (ImGui::Selectable((filename + "/").c_str(), false, ImGuiSelectableFlags_DontClosePopups))
                    {
                        currentPath = path.string();
                    }
                }
                else
                {
                    if (ImGui::Selectable(filename.c_str(), false, ImGuiSelectableFlags_DontClosePopups))
                    {
                        selectedPath = path.string();
                        fileSelected = true;
                        ImGui::CloseCurrentPopup();
                    }
                }
            }

            ImGui::Separator();

            if (ImGui::Button("Close"))
            {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        ImGui::PopStyleVar(2);
    }

    void StandardWindow(const std::string &title, const std::function<void()> &callback)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowTitleAlign, ImVec2(0.5f, 0.5f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

        ImGui::Begin(title.c_str());
        callback();
        ImGui::End();

        ImGui::PopStyleVar(2);
    }

    void HelpMarker(const std::string &text)
    {
        ImGui::TextDisabled("(?)");
        if (ImGui::BeginItemTooltip())
        {
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            ImGui::TextUnformatted(text.c_str());
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }

    void Combo(int &selected, std::vector<std::string> &items, const std::string &preview)
    {
        if (ImGui::BeginCombo("", preview.c_str()))
        {
            for (int i = 0; i < items.size(); i++)
            {
                const bool is_selected = (selected == i);

                if (ImGui::Selectable(items[i].c_str(), is_selected))
                {
                    selected = i;
                }

                if (is_selected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
    }

} // namespace ui
