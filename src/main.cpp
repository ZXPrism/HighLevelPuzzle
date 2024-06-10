#include <imgui.h>

#include "Application.h"

#include "HLP/PuzzleDemostrator.h"

int main(int argc, char *argv[])
{
    gApp.Init();

    PuzzleDemonstrator puzzleDemo;
    puzzleDemo.Init();

    gApp.Run([&](float dt) {
        puzzleDemo.Tick(dt);
        puzzleDemo.RenderMenu();
        puzzleDemo.RenderPuzzle();
    });

    gApp.Shutdown();

    return 0;
}
