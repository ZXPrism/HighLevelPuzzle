#include "PuzzleConfig.h"

#include <format>
#include <stack>

#include <glm/gtc/matrix_transform.hpp>

#include "Logger.h"
#include "Utils.h"

int PuzzleConfig::_NoPiece = -1;
int PuzzleConfig::_Inf = 0x3f3f3f3f;

int PuzzleConfig::_DxArray[4] = {0, 0, -1, 1};
int PuzzleConfig::_DzArray[4] = {-1, 1, 0, 0};
const char *PuzzleConfig::_DirArray[4] = {"BACK", "FORWARD", "LEFT", "RIGHT"};

void PuzzleConfig::AddPuzzlePiece(std::shared_ptr<PuzzlePiece> puzzlePiece)
{
    _Data.push_back(puzzlePiece);
}

void PuzzleConfig::Render(Shader &shader, VertexBuffer &voxelModel)
{
    shader.Activate();

    glm::vec3 pos{};

    for (int x = 0; x < _SizeX; x++)
    {
        int z = 0;
        for (auto &run : _OccupiedRLEMapZ[x])
        {
            int pieceNo = run._PieceNo, length = run._Length;
            if (pieceNo != _NoPiece && !_PuzzlePieceInvisibility[pieceNo])
            {
                for (int dz = 0; dz < length; dz++)
                {
                    glm::mat4 model(1.0f);
                    pos = {x + _MinX, 0, z + dz + _MinZ}; // don't forget to map coordinates
                    shader.SetUniform("model", glm::translate(model, pos));
                    shader.SetUniform("color", _PuzzlePieceMaterials[_PuzzlePieceMaterialsMap[pieceNo]]._Color);
                    voxelModel.DrawTriangles(0, 36);
                }
            }

            z += length;
        }
    }
}

void PuzzleConfig::AssignPuzzlePieceMaterials()
{
    _PuzzlePieceInvisibility.resize(_Data.size());
    _PuzzlePieceMaterialsMap.resize(_Data.size());

    auto GenerateRandomColor = []() {
        static const std::vector<float> segments = {0, 0.4, 0.7, 1.0};
        static const std::vector<int> weight = {1, 5, 1};

        // FIXME: Why adding a static keyword here would cause access violation???
        // and why adding this annotation making the issue solved?
        // it must be the compiler's fault...
        static std::vector<float> res(3);

        RandPiecewiseDist(res, 3, segments, weight);

        return glm::vec3(res[0], res[1], res[2]);
    };

    // 1. Generate material for piece #0
    _PuzzlePieceMaterials.emplace_back(GenerateRandomColor());

    // 2. Generate materials for the rest pieces
    int n = _Data.size();
    for (int i = 1; i < n; i++)
    {
        int colorIndex = 0;
        bool flag = true;

        do
        {
            flag = true;
            for (auto adjacentPiece : _AdjacencyGraph[i])
            {
                if (colorIndex == _PuzzlePieceMaterialsMap[adjacentPiece])
                {
                    ++colorIndex;

                    if (colorIndex == _PuzzlePieceMaterials.size())
                    {
                        _PuzzlePieceMaterials.emplace_back(GenerateRandomColor());
                        break;
                    }

                    flag = false;
                    break;
                }
            }
        } while (!flag);

        _PuzzlePieceMaterialsMap[i] = colorIndex;
    }
}

void PuzzleConfig::_BuildAdjacencyGraph(std::vector<std::vector<int>> &occupiedMap)
{
    _AdjacencyGraph.resize(_Data.size());

    // initially all pieces' coordinates are in [0, sizeX) x [0, sizeZ)
    // mapped coordinates are in [sizeX, 2 * sizeX) x [sizeZ, 2 * sizeZ)
    // time complexity: O(4 * _PuzzleSizeX * _PuzzleSizeZ)

    for (int x = 0; x < _SizeX; x++)
    {
        for (int z = 0; z < _SizeZ; z++)
        {
            if (occupiedMap[x][z] != _NoPiece)
            {
                for (int d = 0; d < 4; d++)
                {
                    int nx = x + _DxArray[d];
                    int nz = z + _DzArray[d];
                    if (nx >= 0 && nx < _SizeX && nz >= 0 && nz < _SizeZ && occupiedMap[nx][nz] != _NoPiece &&
                        occupiedMap[nx][nz] != occupiedMap[x][z])
                    {
                        _AdjacencyGraph[occupiedMap[x][z]].insert(occupiedMap[nx][nz]);
                    }
                }
            }
        }
    }

    LOG_INFO("Adjacency graph building completed");
    DEBUG_SCOPE({
        int n = _Data.size();
        for (int i = 0; i < n; i++)
        {
            std::cout << i << " ->";
            for (auto &adjacentPiece : _AdjacencyGraph[i])
            {
                std::cout << ' ' << adjacentPiece;
            }
            std::cout << std::endl;
        }
    });
}

void PuzzleConfig::CalculateNeighborConfigs(std::vector<std::shared_ptr<PuzzleConfig>> &neighborConfigs)
{
    // 0. build acceleration structure
    _BuildSubassemblyValidater();

    // 1. enumerate subassemblies
    std::set<int> subasmPieceNos;
    _EnumerateSubassembly(0, subasmPieceNos, [&]() {
        LOG_INFO("Found a valid subassembly!");
        DEBUG_SCOPE({
            for (auto pieceNo : subasmPieceNos)
            {
                std::cout << '<' << pieceNo << "> ";
            }
            std::cout << std::endl;
        });

        // 2. calculate the max movable distance in each direction
        for (int d = 0; d < 4; d++)
        {
            int maxMovableSteps = _CalculateMaxMovableDistance(subasmPieceNos, d);

            LOG_INFO("MaxMovableSteps in direction %s: %d", _DirArray[d], maxMovableSteps);

            // 3.
            if (maxMovableSteps == _Inf) // remove the subassembly and label it as a target node
            {
                ;
            }
            else // for each unit distance, generate a neighborconfig
            {
                for (int dist = 1; dist <= maxMovableSteps; dist++)
                {
                    ;
                }
            }
        }
    });
}

void PuzzleConfig::_EnumerateSubassembly(int depth, std::set<int> &pieceNos, const std::function<void()> &callback)
{
    // Normally enumeration on sets have exponential time complexity
    // But through correct pruning we will never reach that upper limit! (i hope so)

    if (pieceNos.size() <= (_Data.size() + 1) / 2 && depth != 0)
    {
        callback();
    }

    if (depth == _Data.size())
    {
        return;
    }

    for (int i = depth; i < _Data.size(); i++)
    {
        bool connected = (_SubasmValidator.Find(i) == _SubasmValidator.Find(std::max(0, depth - 1)));
        if (connected) // make sure the newly visited piece is "connected" to previous pieces
        {
            pieceNos.insert(i);
            _EnumerateSubassembly(i + 1, pieceNos, callback);
            pieceNos.erase(i);
        }
    }
}

bool PuzzleConfig::_ValidateSubassembly(std::set<int> &pieceNos)
{
    // ! deprecated !
    // since I found a better approach: use Disjoint Set Union

    // do a DFS from the first element on the _AdjacencyGraph
    // check if every element can be visited
    // time complexity: O(pieceNos.size())

    if (pieceNos.empty())
    {
        return false;
    }

    std::vector<std::uint8_t> vis(_Data.size());
    std::stack<int> visStack;
    int visCount = 0;

    // stuck here for a while because of the positioning of vis[..] = 1
    // now I got it:
    // when you try to use stack / queue to realize DFS / BFS
    // FIRST consider their recursive form!

    visStack.push(*pieceNos.begin());
    vis[*pieceNos.begin()] = 1;

    while (!visStack.empty())
    {
        auto topPiece = visStack.top();
        visStack.pop();

        ++visCount;

        for (auto adjacentPiece : _AdjacencyGraph[topPiece])
        {
            if (pieceNos.contains(adjacentPiece) && !vis[adjacentPiece])
            {
                vis[adjacentPiece] = 1;
                visStack.push(adjacentPiece);
            }
        }
    }

    return visCount == pieceNos.size();
}

void PuzzleConfig::_BuildSubassemblyValidater()
{
    int n = _Data.size();

    _SubasmValidator.Init(n);

    for (int i = 0; i < n; i++)
    {
        for (auto &adjacentPiece : _AdjacencyGraph[i])
        {
            _SubasmValidator.Unite(i, adjacentPiece);
        }
    }
}

int PuzzleConfig::_CalculateMaxMovableDistance(std::set<int> &pieceNos, int direction)
{
    // stuck at here for a couple of hours
    // I am too dumb to figure this out..but eventually did it!
    // KEY idea: a subassembly can be removed *if and only if* no other pieces block its way in the moving direction!

    // now the problem is how to implement the idea above.
    // add more assumptions (restrictions) on the puzzle may be a good choice to solve this
    // but that will make the puzzle less fun!
    // in order to "efficiently" (both in time and space) do this , I proposed an approach with bounding box + RLE
    // inspired by an article read months ago: https://0fps.net/2012/01/14/an-analysis-of-minecraft-like-engines/

    int maxMovableDistance = _Inf;
    int dx = _DxArray[direction], dz = _DzArray[direction];
    bool removable = true;

    // we can check the max movable distance of each voxel in the subassembly
    for (auto pieceNo : pieceNos)
    {
        auto &[piece, state] = _Data[pieceNo];
        for (auto &voxel : piece->_Voxels)
        {
            int x = voxel._X + state._OffsetX - _MinX;
            int z = voxel._Z + state._OffsetZ - _MinZ;
            if (dx != 0)
            {
                // first locate the piece in RLEMap
                int RLEMapXSize_Z = _OccupiedRLEMapX[z].size();
                int offset =
                    std::upper_bound(_OccupiedRLEMapPreX[z].begin(), _OccupiedRLEMapPreX[z].end(), x) - _OccupiedRLEMapPreX[z].begin() - 1;

                while (offset >= 0 && offset < RLEMapXSize_Z)
                {
                    int pieceNoCheck = _OccupiedRLEMapX[z][offset]._PieceNo;

                    if (pieceNoCheck != _NoPiece && !pieceNos.contains(pieceNoCheck))
                    {
                        int blockCoord =
                            _OccupiedRLEMapPreX[z][offset + 1] -
                            _OccupiedRLEMapX[z][offset]._Length; // coordinate of the first voxel that blocks the way of current voxel
                        if (dx < 0)
                        {
                            blockCoord = _OccupiedRLEMapPreX[z][offset + 1] - 1;
                        }
                        maxMovableDistance = std::min(maxMovableDistance, std::abs(x - blockCoord) - 1);
                        removable = false;
                        break;
                    }

                    offset += dx;
                }
            }
            else // dz != 0, similar
            {
                int RLEMapZSize_X = _OccupiedRLEMapZ[x].size();
                int offset =
                    std::upper_bound(_OccupiedRLEMapPreZ[x].begin(), _OccupiedRLEMapPreZ[x].end(), z) - _OccupiedRLEMapPreZ[x].begin() - 1;

                while (offset >= 0 && offset < RLEMapZSize_X)
                {
                    int pieceNoCheck = _OccupiedRLEMapZ[x][offset]._PieceNo;

                    if (pieceNoCheck != _NoPiece && !pieceNos.contains(pieceNoCheck))
                    {
                        int blockCoord = _OccupiedRLEMapPreZ[x][offset + 1] - _OccupiedRLEMapZ[x][offset]._Length;
                        if (dz < 0)
                        {
                            blockCoord = _OccupiedRLEMapPreZ[x][offset + 1] - 1;
                        }
                        maxMovableDistance = std::min(maxMovableDistance, std::abs(z - blockCoord) - 1);
                        removable = false;
                        break;
                    }

                    offset += dz;
                }
            }
        }
    }

    return removable ? _Inf : maxMovableDistance;
}

void PuzzleConfig::_CalculateBoundingBox()
{
    _MinX = _Inf;
    _MinZ = _Inf;
    _MaxX = -_Inf;
    _MaxZ = -_Inf;

    for (auto &[piece, state] : _Data)
    {
        for (auto &voxel : piece->_Voxels)
        {
            int x = voxel._X + state._OffsetX;
            int z = voxel._Z + state._OffsetZ;

            _MinX = std::min(_MinX, x);
            _MinZ = std::min(_MinZ, z);
            _MaxX = std::max(_MaxX, x);
            _MaxZ = std::max(_MaxZ, z);
        }
    }

    _SizeX = _MaxX - _MinX + 1;
    _SizeZ = _MaxZ - _MinZ + 1;
}

void PuzzleConfig::_BuildOccupiedRLEMap(std::vector<std::vector<int>> &occupiedMap)
{
    _OccupiedRLEMapX.resize(_SizeZ);
    _OccupiedRLEMapPreX.resize(_SizeZ);
    for (int z = 0; z < _SizeZ; z++)
    {
        int prev = 0;
        for (int x = 1; x < _SizeX; x++)
        {
            if (occupiedMap[x - 1][z] != occupiedMap[x][z])
            {
                _OccupiedRLEMapX[z].push_back({occupiedMap[x - 1][z], x - prev});
                prev = x;
            }
        }

        // process the last segment
        _OccupiedRLEMapX[z].push_back({occupiedMap[prev][z], _SizeX - prev});

        int n = _OccupiedRLEMapX[z].size();
        _OccupiedRLEMapPreX[z].resize(n + 1);
        for (int i = 0; i < n; i++)
        {
            _OccupiedRLEMapPreX[z][i + 1] = _OccupiedRLEMapPreX[z][i] + _OccupiedRLEMapX[z][i]._Length;
        }

        LOG_INFO("Constructed _OccupiedRLEMapX[%d]:", z);
        DEBUG_SCOPE({
            for (auto &run : _OccupiedRLEMapX[z])
            {
                std::cout << std::format("<{}, {}> ", run._PieceNo, run._Length);
            }
            std::cout << std::endl;
        });

        LOG_INFO("Constructed _OccupiedRLEMapPreX[%d]:", z);
        DEBUG_SCOPE({
            for (auto &val : _OccupiedRLEMapPreX[z])
            {
                std::cout << val << ' ';
            }
            std::cout << std::endl;
        });
    }

    // similar
    _OccupiedRLEMapZ.resize(_SizeX);
    _OccupiedRLEMapPreZ.resize(_SizeX);
    for (int x = 0; x < _SizeX; x++)
    {
        int prev = 0;
        for (int z = 1; z < _SizeZ; z++)
        {
            if (occupiedMap[x][z - 1] != occupiedMap[x][z])
            {
                _OccupiedRLEMapZ[x].push_back({occupiedMap[x][z - 1], z - prev});
                prev = z;
            }
        }

        // process the last segment
        _OccupiedRLEMapZ[x].push_back({occupiedMap[x][prev], _SizeZ - prev});

        int n = _OccupiedRLEMapZ[x].size(), run = 0;
        _OccupiedRLEMapPreZ[x].resize(n + 1);
        for (int i = 0; i < n; i++)
        {
            _OccupiedRLEMapPreZ[x][i + 1] = _OccupiedRLEMapPreZ[x][i] + _OccupiedRLEMapZ[x][i]._Length;
        }

        LOG_INFO("Constructed _OccupiedRLEMapZ[%d]:", x);
        DEBUG_SCOPE({
            for (auto &run : _OccupiedRLEMapZ[x])
            {
                std::cout << std::format("<{}, {}> ", run._PieceNo, run._Length);
            }
            std::cout << std::endl;
        });

        LOG_INFO("Constructed _OccupiedRLEMapPreZ[%d]:", x);
        DEBUG_SCOPE({
            for (auto &val : _OccupiedRLEMapPreZ[x])
            {
                std::cout << val << ' ';
            }
            std::cout << std::endl;
        });
    }
}

void PuzzleConfig::BuildAccelStructures()
{
    _CalculateBoundingBox();

    std::vector<std::vector<int>> occupiedMap(_SizeX, std::vector<int>(_SizeZ, _NoPiece));
    int n = _Data.size();
    for (int i = 0; i < n; i++)
    {
        auto &[piece, state] = _Data[i];
        for (auto &voxel : piece->_Voxels)
        {
            // coordinates need to be mapped!
            int x = voxel._X + state._OffsetX - _MinX;
            int z = voxel._Z + state._OffsetZ - _MinZ;
            occupiedMap[x][z] = i;
        }
    }

    // occupied rle map is used to facilitate:
    // 1. check collisions
    // 2. rendering (yes it's cache-friendly!)
    // 3. assist buding adjacency graph
    _BuildOccupiedRLEMap(occupiedMap);

    // adjacency graph is used to facilitate:
    // 1. material assignment
    // 2. subassembly enumeration
    _BuildAdjacencyGraph(occupiedMap);
}
