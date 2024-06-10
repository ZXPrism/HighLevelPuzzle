#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <set>
#include <vector>

#include "Shader.h"
#include "VertexBuffer.h"

#include "PuzzlePiece.h"
#include "Shader.h"
#include "Utils.h"
#include "VertexBuffer.h"

// Design Note:
// A "passive" structure
// It don't execute any function actively
class PuzzleConfig
{
public:
    PuzzleConfig(int puzzleSizeX, int puzzleSizeZ);

    void AddPuzzlePiece(std::shared_ptr<PuzzlePiece> puzzlePiece);

    void AssignPuzzlePieceMaterials();
    void BuildAdjacencyGraph();
    void BuildOccupiedMap();

    void CalculateNeighborConfigs(std::vector<std::shared_ptr<PuzzleConfig>> &neighborConfigs);

    void Render(Shader &shader, VertexBuffer &voxelModel);

public:
    // helpers, don't use them directly unless for test
    void _EnumerateSubassembly(int depth, std::set<int> &pieceNos, const std::function<void()> &callback);
    bool _ValidateSubassembly(std::set<int> &pieceNos); // through DFS
    void _BuildSubassemblyValidater();                  // through DSU
    int _MaxMovableDistance(int pieceNo);

private:
    std::vector<PuzzlePieceInfo> _Data;
    int _PuzzleSizeX, _PuzzleSizeZ;

    // rendering: it's meaningless to generate something invisible!
    std::vector<PuzzlePieceMaterial> _PuzzlePieceMaterials;
    std::vector<int> _PuzzlePieceMaterialsMap;
    std::vector<std::uint8_t> _PuzzlePieceInvisibility;

    // accelration structures
    std::vector<std::set<int>> _AdjacencyGraph;
    std::vector<std::vector<int>> _OccupiedMap;
    DSU _SubasmValidator;

    const int _NoPiece = -1;
    const int _InfDistance = 0x3f3f3f3f;
};
