#pragma once

#include <memory>
#include <vector>

#include <glm/glm.hpp>

#include "Voxel.h"

struct PuzzlePiece
{
    std::vector<Voxel> _Voxels;
};

struct PuzzlePieceState
{
    int _OffsetX = 0;
    int _OffsetZ = 0;
};

struct PuzzlePieceInfo
{
    PuzzlePieceInfo(std::shared_ptr<PuzzlePiece> piece) : _Piece(piece)
    {
    }

    std::shared_ptr<PuzzlePiece> _Piece;
    PuzzlePieceState _State;
};

struct PuzzlePieceMaterial
{
    PuzzlePieceMaterial() = default;
    PuzzlePieceMaterial(glm::vec3 &&color) noexcept : _Color(color)
    {
    }

    glm::vec3 _Color;
};
