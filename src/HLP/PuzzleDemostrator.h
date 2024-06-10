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
    void InitCamera();

    // executed per frame
    void Tick(float dt);
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
    int _CurrentConfigNo = 0;

    // miscs
    float _DeltaTime;
    std::vector<std::string> _PuzzleFiles;

    // rendering
    VertexBuffer _VoxelModel;
    Shader _BasicShader;
    Camera _Camera;
};
