#include "PuzzleDemostrator.h"

#include <fstream>

#include <imgui.h>

#include "Logger.h"
#include "Timer.h"
#include "UI.h"
#include "Utils.h"

#include "HLP_Config.h"

void PuzzleDemonstrator::Init()
{
    InitVoxelModel();
    InitShaders();

    gTimer.DispatchTask(1.0f, [&]() { DetectPuzzleFiles(); });
}

void PuzzleDemonstrator::CorrectCameraPos()
{
    auto &config = _DasmGraph.GetPuzzleConfig(_CurrentConfigID);
    auto [minX, minZ, sizeX, sizeZ] = config.GetPuzzleSize();
    _Camera.SetCameraPos({minX + sizeX / 2.0, 10.0f, minZ + sizeZ / 2.0 + 0.01f});
    _Camera.LookAt({minX + sizeX / 2.0, 0.0f, minZ + sizeZ / 2.0});
}

void PuzzleDemonstrator::InitVoxelModel()
{
    float vertices[] = {-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.5f,  -0.5f, -0.5f, 1.0f, 0.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f,
                        0.5f,  0.5f,  -0.5f, 1.0f, 1.0f, -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

                        -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, 0.5f,  -0.5f, 0.5f,  1.0f, 0.0f, 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
                        0.5f,  0.5f,  0.5f,  1.0f, 1.0f, -0.5f, 0.5f,  0.5f,  0.0f, 1.0f, -0.5f, -0.5f, 0.5f,  0.0f, 0.0f,

                        -0.5f, 0.5f,  0.5f,  1.0f, 0.0f, -0.5f, 0.5f,  -0.5f, 1.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
                        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, -0.5f, 0.5f,  0.5f,  1.0f, 0.0f,

                        0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f, 0.5f,  -0.5f, -0.5f, 0.0f, 1.0f,
                        0.5f,  -0.5f, -0.5f, 0.0f, 1.0f, 0.5f,  -0.5f, 0.5f,  0.0f, 0.0f, 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

                        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.5f,  -0.5f, -0.5f, 1.0f, 1.0f, 0.5f,  -0.5f, 0.5f,  1.0f, 0.0f,
                        0.5f,  -0.5f, 0.5f,  1.0f, 0.0f, -0.5f, -0.5f, 0.5f,  0.0f, 0.0f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,

                        -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f, 0.5f,  0.5f,  -0.5f, 1.0f, 1.0f, 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
                        0.5f,  0.5f,  0.5f,  1.0f, 0.0f, -0.5f, 0.5f,  0.5f,  0.0f, 0.0f, -0.5f, 0.5f,  -0.5f, 0.0f, 1.0f};

    _VoxelModel.SetData(vertices, sizeof(vertices) / sizeof(float)).SetUsage(0, 3).SetUsage(1, 2).EndSetUsage();
}

void PuzzleDemonstrator::InitShaders()
{
    ShaderInfo shaderInfo;
    shaderInfo._VertexShaderPath = cpBasicShaderVSPath;
    shaderInfo._FragmentShaderPath = cpBasicShaderFSPath;
    _BasicShader.Init(shaderInfo);

    _BasicShader.Activate();
    _BasicShader.SetUniform("view", _Camera.GetViewMatrix());
    _BasicShader.SetUniform("proj", _Camera.GetProjectionMatrix());
}

void PuzzleDemonstrator::Tick(float dt)
{
    _DeltaTime = dt;
    _Camera.Update(dt);

    _BasicShader.Activate();
    _BasicShader.SetUniform("view", _Camera.GetViewMatrix());

    if (_PuzzleImported && _PrevConfigID != _CurrentConfigID)
    {
        _PrevConfigID = _CurrentConfigID;
        CorrectCameraPos();
    }
}

void PuzzleDemonstrator::RenderPuzzle()
{
    if (_PuzzleImported)
    {
        _DasmGraph.RenderConfig(_CurrentConfigID, _BasicShader, _VoxelModel);
    }
}

void PuzzleDemonstrator::RenderMenu()
{
    ui::internal::ShowDemoWindow();

    ui::StandardWindow("Menu", [&]() {
        ImGui::SeparatorText("MISCS");
        RenderMenu_FPSPanel();

        ImGui::SeparatorText("DISASSEMBLY PLANNER");
        RenderMenu_DasmPlanner();

        ImGui::SeparatorText("PUZZLE GENERATOR");

        ImGui::End();
    });
}

void PuzzleDemonstrator::RenderMenu_FPSPanel()
{
    static float fpsArr[64];
    static int fpsArrWriteIdx = 0;

    int currentFPS = std::floor(1.0f / _DeltaTime);

    fpsArr[fpsArrWriteIdx] = currentFPS;

    if (fpsArrWriteIdx == 64)
        fpsArrWriteIdx = 0;
    else
        ++fpsArrWriteIdx;

    ImGui::PlotLines("", fpsArr, IM_ARRAYSIZE(fpsArr));
    ImGui::SameLine();
    ImGui::Text("FPS: %d", currentFPS);
}

void PuzzleDemonstrator::RenderMenu_DasmPlanner()
{
    static int selected = 0;

    // display puzzle file selction menu
    if (_PuzzleFiles.empty())
    {
        ui::Combo(selected, _PuzzleFiles, "(no puzzle files found )");
    }
    else
    {
        ui::Combo(selected, _PuzzleFiles, _PuzzleFiles[selected]);
    }

    ImGui::SameLine();

    if (ImGui::Button("IMPORT") && !_PuzzleFiles.empty())
    {
        std::string puzzleFilePath = (fs::path(cPuzzleFileFolder) / _PuzzleFiles[selected]).string();
        if (_DasmGraph.ImportPuzzle(puzzleFilePath)) // in case the import fails
        {
            _PuzzleImported = true;
            _CurrentConfigID = 0;
            _PrevConfigID = -1;
        }
    }
    ImGui::SameLine();
    ui::HelpMarker("Put puzzle files in \"resources\" folder");

    if (_PuzzleImported)
    {
        std::string currentPuzzlePath = _PuzzleFiles[selected];
        currentPuzzlePath = currentPuzzlePath.substr(0, currentPuzzlePath.find('.')); // strip the extension
        ImGui::Text("Current Puzzle: <%s>", currentPuzzlePath.c_str());
        ImGui::Text("Current Config: #%d", _CurrentConfigID);

        // if (ImGui::Button("Show Neighbor Configs"))
        // {
        //     _DasmGraph.Test_AddAllNeighborConfigs(_CurrentConfigID);
        // }

        // int configNum = _DasmGraph.GetPuzzleConfigNum();
        // if (ImGui::Button("Next Config"))
        // {
        //     _CurrentConfigID = (_CurrentConfigID + 1) % configNum;
        // }

        if (ImGui::Button("Build Kernel Disassembly Graph"))
        {
            _DasmGraph.BuildKernelDisassemblyGraph();
        }
    }
}

void PuzzleDemonstrator::DetectPuzzleFiles()
{
    _PuzzleFiles.clear();

    TraverseFolder(cPuzzleFileFolder, [&](const fs::path &filePath) {
        std::ifstream fin(filePath);
        int magicNumber = 0;
        fin >> magicNumber;
        fin.close();
        if (magicNumber == cPuzzleFileMagicNumber)
        {
            _PuzzleFiles.push_back(filePath.filename().string());
        }
    });
}
