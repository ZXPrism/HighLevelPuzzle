#pragma once

#include <array>
#include <functional>
#include <memory>
#include <set>
#include <unordered_set>
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
    explicit PuzzleConfig(int depth, int originalPieceNum);

    // construct the config, these function should be called first before other operations
    void AddPuzzlePiece(int pieceID, const PuzzlePieceInfo &puzzlePieceInfo);
    void AddPuzzlePiece(int pieceID, std::shared_ptr<PuzzlePiece> puzzlePiece);
    void AddPuzzlePiece(int pieceID, std::shared_ptr<PuzzlePiece> puzzlePiece, const PuzzlePieceState &state);

    // --> ONLY used when enumerating neighbor configs, to keep materials unchanged
    void SetPieceMaterial(int pieceID, const PuzzlePieceMaterial &material);

    // these functions are supposed to be called ONLY ONCE for one object
    void AssignPuzzlePieceMaterials();
    void BuildAccelStructures();
    void CalculateNeighborConfigs(std::vector<std::shared_ptr<PuzzleConfig>> &neighborConfigs);

    // rendering
    void Render(Shader &shader, VertexBuffer &voxelModel);

    // queries
    int GetDepth() const;
    bool IsFullConfig(int delta = 0) const;
    std::array<int, 4> GetPuzzleSize() const; // MinX, MinZ, SizeX, SizeZ
    int GetPuzzlePieceNum() const;
    int GetRemovedPieceNum() const;
    bool IsEqualTo(const PuzzleConfig &rhs);

public:
    // helpers, don't use them directly unless for test
    void _EnumerateSubassembly(int depth, std::set<int> &pieceIDs, const std::function<void()> &callback);
    bool _ValidateSubassembly(std::set<int> &pieceIDs); // through DFS
    void _BuildSubassemblyValidater();                  // through DSU
    void _CalculateBoundingBox();
    void _BuildAdjacencyGraph(std::vector<std::vector<int>> &occupiedMap);
    void _BuildOccupiedRLEMap(std::vector<std::vector<int>> &occupiedMap);
    int _CalculateMaxMovableDistance(std::set<int> &pieceIDs, int diretction);

private:
    std::unordered_map<int, PuzzlePieceInfo> _Data;
    std::vector<int> _PieceIDs; // mainly used for enumerating subassemblies

    // values for query
    int _Depth = 0;
    int _OriginalPieceNum = 0;

    // rendering: it's meaningless to generate something invisible!
    std::unordered_map<int, PuzzlePieceMaterial> _PuzzlePieceMaterialsMap;

    // accelration structures
    struct RLEInfo
    {
        int _PieceID;
        int _Length;
    };
    std::unordered_map<int, std::unordered_set<int>> _AdjacencyGraph;
    std::vector<std::vector<RLEInfo>> _OccupiedRLEMapX;
    std::vector<std::vector<RLEInfo>> _OccupiedRLEMapZ;
    std::vector<std::vector<int>> _OccupiedRLEMapPreX;
    std::vector<std::vector<int>> _OccupiedRLEMapPreZ;
    DSU _SubasmValidator;
    int _MinX, _MaxX, _MinZ, _MaxZ;
    int _SizeX, _SizeZ;

    // constants
    static int _NoPiece;
    static int _Inf;

    static int _DxArray[4];
    static int _DzArray[4];
    static const char *_DirArray[4];
};
