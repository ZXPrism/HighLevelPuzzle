#pragma once

#include <map>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "PuzzleConfig.h"

class DisassemblyGraph
{
public:
    // puzzle is either generated by the PuzzleGenerator or imported from a puzzle file
    // all data will be cleared before each generation / import
    bool ImportPuzzle(const std::string &puzzleFilePath);

    // config operations
    void RenderConfig(int configID, Shader &shader, VertexBuffer &voxelModel);
    void CalculateNeighborConfigs(int configID, std::vector<std::shared_ptr<PuzzleConfig>> &neighborConfigs);
    void BuildKernelDisassemblyGraph(int configID = 0, int relativeDepth = 0, int fullConfigDelta = 0);
    void BuildCompleteDisassemblyGraph();
    void DisassembleGraph();

    // queries
    PuzzleConfig &GetPuzzleConfig(int configID);
    int GetDisasmPlanConfigID(int planOffset);
    int GetPuzzleConfigNum() const;
    int GetDisasmPlanSize() const;
    bool IsDisasmGraphBuilt() const;
    int GetPuzzleDifficulty() const;

    // tests
    void Test_AddAllNeighborConfigs(int configID); // this action doesn't maintain edges!

private:
    std::vector<std::unordered_set<int>> _GraphEdges;
    std::vector<std::shared_ptr<PuzzleConfig>> _GraphNodes;
    std::vector<int> _GraphNodesParents;
    std::map<int, int> _TargetNodeIDs; // <depth , ID>
    std::vector<int> _DisassemblyPlan;

    int _MinTargetNodeDepth = 0x3f3f3f3f;
    bool _DisasmGraphBuilt = false;
    int _PrevTargetNodeID = -1;
};
