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

class PuzzleConfig
{
public:
    void AddPuzzlePiece(std::shared_ptr<PuzzlePiece> puzzlePiece);

    // these functions are supposed to be called ONLY ONCE for one object
    void AssignPuzzlePieceMaterials();
    void BuildAccelStructures();
    void CalculateNeighborConfigs(std::vector<std::shared_ptr<PuzzleConfig>> &neighborConfigs);

    void Render(Shader &shader, VertexBuffer &voxelModel);

public:
    // helpers, don't use them directly unless for test
    void _EnumerateSubassembly(int depth, std::set<int> &pieceNos, const std::function<void()> &callback);
    bool _ValidateSubassembly(std::set<int> &pieceNos); // through DFS
    void _BuildSubassemblyValidater();                  // through DSU
    void _CalculateBoundingBox();
    void _BuildAdjacencyGraph(std::vector<std::vector<int>> &occupiedMap);
    void _BuildOccupiedRLEMap(std::vector<std::vector<int>> &occupiedMap);
    int _CalculateMaxMovableDistance(std::set<int> &pieceNos, int diretction);

private:
    std::vector<PuzzlePieceInfo> _Data;

    // rendering: it's meaningless to generate something invisible!
    std::vector<PuzzlePieceMaterial> _PuzzlePieceMaterials;
    std::vector<int> _PuzzlePieceMaterialsMap;
    std::vector<std::uint8_t> _PuzzlePieceInvisibility;

    // accelration structures
    struct RLEInfo
    {
        int _PieceNo;
        int _Length;
    };

    std::vector<std::set<int>> _AdjacencyGraph;
    std::vector<std::vector<RLEInfo>> _OccupiedRLEMapX;
    std::vector<std::vector<RLEInfo>> _OccupiedRLEMapZ;
    std::vector<std::vector<int>> _OccupiedRLEMapPreX;
    std::vector<std::vector<int>> _OccupiedRLEMapPreZ;
    DSU _SubasmValidator;
    int _MinX, _MaxX, _MinZ, _MaxZ;
    int _SizeX, _SizeZ;

    static int _NoPiece;
    static int _Inf;

    static int _DxArray[4];
    static int _DzArray[4];
    static const char *_DirArray[4];
};
