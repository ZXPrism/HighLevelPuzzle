#pragma once

#include "Camera.h"
#include "DisassembleGraph.h"

class PuzzleDemonstrator
{
public:
    // init
    void Init();
    void InitVoxelModel();
    void InitShaders();

    // executed per frame
    void Tick(float dt);
    void CorrectCameraPos();
    void RenderPuzzle();
    void RenderMenu();
    void RenderMenu_FPSPanel();
    void RenderMenu_DasmPlanner();

    // miscs
    void DetectPuzzleFiles();

private:
    // disassemble planner
    DisassembleGraph _DasmGraph;

    // puzzle designer
    ;

    // puzzle info
    bool _PuzzleImported = false;
    int _CurrentConfigID = 0;
    int _PrevConfigID = -1; // used to detect if the config changes (if so, then we need to correct the position of camera)

    // miscs
    float _DeltaTime;
    std::vector<std::string> _PuzzleFiles;

    // rendering
    VertexBuffer _VoxelModel;
    Shader _BasicShader;
    Camera _Camera;
};
