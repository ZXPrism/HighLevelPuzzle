#pragma once

struct Voxel
{
    Voxel(int x, int z) : _X(x), _Z(z)
    {
    }

    // coordinates in world space
    int _X;
    int _Z;
};
