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

    gTimer.DispatchLoopTask(1.0f, [&]() { DetectPuzzleFiles(); });
}

void PuzzleDemonstrator::CorrectCameraPos()
{
    auto &config = _DasmGraph.GetPuzzleConfig(_CurrentConfigID);
    auto [minX, minZ, sizeX, sizeZ] = config.GetPuzzleSize();
    _Camera.SetCameraPos({minX + sizeX / 2.0, 15.0f, minZ + sizeZ / 2.0 + 0.01f});
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
#ifdef MY_DEBUG
    ui::internal::ShowDemoWindow();
#endif

    ui::StandardWindow("HLP Demo Menu", [&]() {
        RenderMenu_FPSPanel();

        if (ImGui::CollapsingHeader("DISASSEMBLY PLANNER"))
        {
            RenderMenu_DasmPlanner();
        }

        if (ImGui::CollapsingHeader("PUZZLE GENERATOR (WIP)"))
        {
            ;
        }

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
            _CurrentPlanOffset = 0;
            _PrevConfigID = -1;
        }
    }
    ImGui::SameLine();
    ui::HelpMarker("Put puzzle files in \"resources\" folder");

    if (_PuzzleImported)
    {
        std::string currentPuzzlePath = _PuzzleFiles[selected];
        currentPuzzlePath = currentPuzzlePath.substr(0, currentPuzzlePath.find('.')); // strip the extension
        ImGui::Text("Loaded Puzzle: <%s>", currentPuzzlePath.c_str());

        auto &currentConfig = _DasmGraph.GetPuzzleConfig(_CurrentConfigID);
        auto configSize = currentConfig.GetPuzzleSize();
        auto depth = currentConfig.GetDepth();

        ImGui::SeparatorText("Current Config Info");
        {
            ImGui::Text("ID: #%d", _CurrentConfigID);
            ImGui::Text("MinX = %d, MinZ = %d, SizeX = %d, SizeZ = %d", configSize[0], configSize[1], configSize[2], configSize[3]);
            ImGui::Text("Depth: %d", depth);

            bool isFullConfig = currentConfig.IsFullConfig();
            ImGui::Checkbox("Full Config", &isFullConfig);

            // test: check if two config is equal
            static int equalityCheckIDs[2] = {0, 0};
            bool equalityCheckRes =
                (_DasmGraph.GetPuzzleConfig(equalityCheckIDs[0]).IsEqualTo(_DasmGraph.GetPuzzleConfig(equalityCheckIDs[1])));
            ImGui::Checkbox("Check Equality", &equalityCheckRes);
            ImGui::SameLine();
            ImGui::InputInt2("", equalityCheckIDs);

            int configNum = _DasmGraph.GetPuzzleConfigNum();
            if (ImGui::Button("Prev Config"))
            {
                _CurrentConfigID = (_CurrentConfigID - 1 + configNum) % configNum;
            }
            ImGui::SameLine();

            if (ImGui::Button("Next Config"))
            {
                _CurrentConfigID = (_CurrentConfigID + 1) % configNum;
            }
        }

        ImGui::SeparatorText("Disassemble Puzzle");
        {
            if (!_DasmGraph.IsDisasmGraphBuilt())
            {
                if (ImGui::Button("Disassemble [Kernel]"))
                {
                    _DasmGraph.BuildKernelDisassemblyGraph();
                }

                if (ImGui::Button("Disassemble [Complete]"))
                {
                    _DasmGraph.BuildCompleteDisassemblyGraph();
                }
            }

            if (_DasmGraph.IsDisasmGraphBuilt())
            {
                int disasmPlanSize = _DasmGraph.GetDisasmPlanSize();
                if (ImGui::Button("Prev Disasm Config"))
                {
                    _CurrentPlanOffset = (_CurrentPlanOffset - 1 + disasmPlanSize) % disasmPlanSize;
                    _CurrentConfigID = _DasmGraph.GetDisasmPlanConfigID(_CurrentPlanOffset);
                }
                ImGui::SameLine();

                if (ImGui::Button("Next Disasm Config"))
                {
                    _CurrentPlanOffset = (_CurrentPlanOffset + 1) % disasmPlanSize;
                    _CurrentConfigID = _DasmGraph.GetDisasmPlanConfigID(_CurrentPlanOffset);
                }

                ImGui::Text("Difficulty: %d", _DasmGraph.GetPuzzleDifficulty());
            }
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
