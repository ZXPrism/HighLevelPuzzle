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
    bool operator==(const PuzzlePieceState &rhs)
    {
        return _OffsetX == rhs._OffsetX && _OffsetZ == rhs._OffsetZ;
    }

    int _OffsetX = 0;
    int _OffsetZ = 0;
};

struct PuzzlePieceInfo
{
    PuzzlePieceInfo() = default;

    explicit PuzzlePieceInfo(std::shared_ptr<PuzzlePiece> piece) : _Piece(piece)
    {
    }

    PuzzlePieceInfo(std::shared_ptr<PuzzlePiece> piece, PuzzlePieceState state) : _Piece(piece), _State(state)
    {
    }

    std::shared_ptr<PuzzlePiece> _Piece;
    PuzzlePieceState _State;
};

struct PuzzlePieceMaterial
{
    PuzzlePieceMaterial() = default;
    explicit PuzzlePieceMaterial(glm::vec3 &&color) noexcept : _Color(color)
    {
    }

    glm::vec3 _Color;
};
