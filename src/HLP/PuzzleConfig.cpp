#include "PuzzleConfig.h"

#include <stack>

#include <glm/gtc/matrix_transform.hpp>

#include "Logger.h"
#include "Utils.h"

int PuzzleConfig::_NoPiece = -1;
int PuzzleConfig::_Inf = 0x3f3f3f3f;

int PuzzleConfig::_DxArray[4] = {0, 0, -1, 1};
int PuzzleConfig::_DzArray[4] = {-1, 1, 0, 0};
const char *PuzzleConfig::_DirArray[4] = {"BACK", "FORWARD", "LEFT", "RIGHT"};

PuzzleConfig::PuzzleConfig(int depth, int originalPieceNum) : _Depth(depth), _OriginalPieceNum(originalPieceNum)
{
}

int PuzzleConfig::GetDepth() const
{
    return _Depth;
}

bool PuzzleConfig::IsFullConfig() const
{
    return _PieceIDs.size() == _OriginalPieceNum;
}

void PuzzleConfig::AddPuzzlePiece(int pieceID, std::shared_ptr<PuzzlePiece> puzzlePiece)
{
    _Data[pieceID] = PuzzlePieceInfo(puzzlePiece);
    _PieceIDs.push_back(pieceID);
}

void PuzzleConfig::AddPuzzlePiece(int pieceID, std::shared_ptr<PuzzlePiece> puzzlePiece, const PuzzlePieceState &state)
{
    _Data[pieceID] = PuzzlePieceInfo(puzzlePiece, state);
    _PieceIDs.push_back(pieceID);
}

void PuzzleConfig::AddPuzzlePiece(int pieceID, const PuzzlePieceInfo &puzzlePieceInfo)
{
    _Data[pieceID] = puzzlePieceInfo;
    _PieceIDs.push_back(pieceID);
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
            int pieceID = run._PieceID, length = run._Length;
            if (pieceID != _NoPiece)
            {
                for (int dz = 0; dz < length; dz++)
                {
                    glm::mat4 model(1.0f);
                    pos = {x + _MinX, 0, z + dz + _MinZ}; // don't forget to map coordinates
                    shader.SetUniform("model", glm::translate(model, pos));
                    shader.SetUniform("color", _PuzzlePieceMaterialsMap[pieceID]._Color);
                    voxelModel.DrawTriangles(0, 36);
                }
            }

            z += length;
        }
    }
}

void PuzzleConfig::SetPieceMaterial(int pieceID, const PuzzlePieceMaterial &material)
{
    _PuzzlePieceMaterialsMap[pieceID] = material;
}

void PuzzleConfig::AssignPuzzlePieceMaterials()
{
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

    // 2024-06-13 UPD:
    // originally I assigned different materials(colors) to adjacent pieces
    // but I found it useless now
    // because if you move some pieces and some of them which are not adjacent before will be adjacent
    // but you can't re-assign colors for them (or everything will be confusing)
    // new strategy: every puzzle a piece has different color

    int n = _Data.size();
    for (int i = 0; i < n; i++)
    {
        _PuzzlePieceMaterialsMap[_PieceIDs[i]] = PuzzlePieceMaterial(GenerateRandomColor());
    }
}

void PuzzleConfig::_BuildAdjacencyGraph(std::vector<std::vector<int>> &occupiedMap)
{
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
                        _AdjacencyGraph[occupiedMap[nx][nz]].insert(occupiedMap[x][z]);
                    }
                }
            }
        }
    }

    DLOG_INFO("Adjacency graph building completed");
    DEBUG_SCOPE({
        int n = _Data.size();
        for (auto pieceID : pieceIDs)
        {
            std::cout << pieceID << " ->";
            for (auto &adjacentPiece : _AdjacencyGraph[pieceID])
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
    std::set<int> subasmPieceIDs;
    _EnumerateSubassembly(0, subasmPieceIDs, [&]() {
        DLOG_INFO("Found a valid subassembly!");
        DEBUG_SCOPE({
            for (auto pieceID : subasmPieceIDs)
            {
                std::cout << '<' << pieceID << "> ";
            }
            std::cout << std::endl;
        });

        // 2. calculate the max movable distance in each direction
        for (int d = 0; d < 4; d++)
        {
            int maxMovableSteps = _CalculateMaxMovableDistance(subasmPieceIDs, d);

            DLOG_INFO("MaxMovableSteps in direction %s: %d", _DirArray[d], maxMovableSteps);

            // 3.1 remove the subassembly and label it as a target node
            if (maxMovableSteps == _Inf)
            {
                auto newConfig = neighborConfigs.emplace_back(std::make_shared<PuzzleConfig>(_Depth + 1, _OriginalPieceNum));

                int n = _Data.size();
                for (int i = 0; i < n; i++)
                {
                    if (!subasmPieceIDs.contains(_PieceIDs[i]))
                    {
                        newConfig->AddPuzzlePiece(_PieceIDs[i], _Data[_PieceIDs[i]]);
                        newConfig->SetPieceMaterial(_PieceIDs[i], _PuzzlePieceMaterialsMap[_PieceIDs[i]]);
                    }
                }

                newConfig->BuildAccelStructures();

                return; // if a piece can be removed, we don't care how it's removed
            }
            else // 3.2 for each unit distance, generate a neighborconfig
            {
                for (int dist = 1; dist <= maxMovableSteps; dist++)
                {
                    auto newConfig = neighborConfigs.emplace_back(std::make_shared<PuzzleConfig>(_Depth + 1, _OriginalPieceNum));

                    int n = _Data.size();
                    for (int i = 0; i < n; i++)
                    {
                        if (!subasmPieceIDs.contains(_PieceIDs[i]))
                        {
                            newConfig->AddPuzzlePiece(_PieceIDs[i], _Data[_PieceIDs[i]]);
                        }
                        else
                        {
                            auto state = _Data[_PieceIDs[i]]._State;
                            state._OffsetX += _DxArray[d] * dist;
                            state._OffsetZ += _DzArray[d] * dist;

                            newConfig->AddPuzzlePiece(_PieceIDs[i], _Data[_PieceIDs[i]]._Piece, state);
                        }

                        newConfig->SetPieceMaterial(_PieceIDs[i], _PuzzlePieceMaterialsMap[_PieceIDs[i]]);
                    }

                    newConfig->BuildAccelStructures();
                }
            }
        }
    });

    LOG_INFO("Neighbor config calculation completed! Found %d neighbor(s).", neighborConfigs.size());
}

void PuzzleConfig::_EnumerateSubassembly(int depth, std::set<int> &pieceIDs, const std::function<void()> &callback)
{
    // Normally enumeration on sets have exponential time complexity
    // But through correct pruning we will never reach that upper limit! (i hope so)

    if (pieceIDs.size() <= (_Data.size() + 1) / 2 && depth != 0)
    {
        callback();
    }

    if (depth == _Data.size())
    {
        return;
    }

    for (int i = depth; i < _Data.size(); i++)
    {
        bool connected =
            (pieceIDs.size() == 0) ? true : (_SubasmValidator.Find(_PieceIDs[i]) == _SubasmValidator.Find(_PieceIDs[depth - 1]));
        if (connected) // make sure the newly visited piece is "connected" to previous pieces
        {
            pieceIDs.insert(_PieceIDs[i]);
            _EnumerateSubassembly(i + 1, pieceIDs, callback);
            pieceIDs.erase(_PieceIDs[i]);
        }
    }
}

bool PuzzleConfig::_ValidateSubassembly(std::set<int> &pieceIDs)
{
    // ! deprecated !
    // since I found a better approach: use Disjoint Set Union

    // do a DFS from the first element on the _AdjacencyGraph
    // check if every element can be visited
    // time complexity: O(pieceIDs.size())

    if (pieceIDs.empty())
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

    visStack.push(*pieceIDs.begin());
    vis[*pieceIDs.begin()] = 1;

    while (!visStack.empty())
    {
        auto topPiece = visStack.top();
        visStack.pop();

        ++visCount;

        for (auto adjacentPiece : _AdjacencyGraph[topPiece])
        {
            if (pieceIDs.contains(adjacentPiece) && !vis[adjacentPiece])
            {
                vis[adjacentPiece] = 1;
                visStack.push(adjacentPiece);
            }
        }
    }

    return visCount == pieceIDs.size();
}

void PuzzleConfig::_BuildSubassemblyValidater()
{
    int n = _Data.size();

    _SubasmValidator.Init(_PieceIDs.back() + 1); // TODO: optimize it! some space is wasted!

    for (int i = 0; i < n; i++)
    {
        for (auto adjacentPiece : _AdjacencyGraph[_PieceIDs[i]])
        {
            _SubasmValidator.Unite(_PieceIDs[i], adjacentPiece);
        }
    }
}

int PuzzleConfig::_CalculateMaxMovableDistance(std::set<int> &pieceIDs, int direction)
{
    // stuck at here for two days
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
    for (auto pieceID : pieceIDs)
    {
        auto &[piece, state] = _Data[pieceID];
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
                    int pieceIDCheck = _OccupiedRLEMapX[z][offset]._PieceID;

                    if (pieceIDCheck != _NoPiece && !pieceIDs.contains(pieceIDCheck))
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
                    int pieceIDCheck = _OccupiedRLEMapZ[x][offset]._PieceID;

                    if (pieceIDCheck != _NoPiece && !pieceIDs.contains(pieceIDCheck))
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

    for (auto &[pieceID, info] : _Data)
    {
        auto &[piece, state] = info;
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

        DLOG_INFO("Constructed _OccupiedRLEMapX[%d]:", z);
        DEBUG_SCOPE({
            for (auto &run : _OccupiedRLEMapX[z])
            {
                std::cout << std::format("<{}, {}> ", run._PieceID, run._Length);
            }
            std::cout << std::endl;
        });

        DLOG_INFO("Constructed _OccupiedRLEMapPreX[%d]:", z);
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

        int n = _OccupiedRLEMapZ[x].size();
        _OccupiedRLEMapPreZ[x].resize(n + 1);
        for (int i = 0; i < n; i++)
        {
            _OccupiedRLEMapPreZ[x][i + 1] = _OccupiedRLEMapPreZ[x][i] + _OccupiedRLEMapZ[x][i]._Length;
        }

        DLOG_INFO("Constructed _OccupiedRLEMapZ[%d]:", x);
        DEBUG_SCOPE({
            for (auto &run : _OccupiedRLEMapZ[x])
            {
                std::cout << std::format("<{}, {}> ", run._PieceID, run._Length);
            }
            std::cout << std::endl;
        });

        DLOG_INFO("Constructed _OccupiedRLEMapPreZ[%d]:", x);
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
    for (auto &[pieceID, info] : _Data)
    {
        auto &[piece, state] = info;
        for (auto &voxel : piece->_Voxels)
        {
            // coordinates need to be mapped!
            int x = voxel._X + state._OffsetX - _MinX;
            int z = voxel._Z + state._OffsetZ - _MinZ;
            occupiedMap[x][z] = pieceID;
        }
    }

    // occupied rle map is used to facilitate:
    // 1. check collisions
    // 2. rendering (yes it's cache-friendly!)
    // 3. assist buding adjacency graph
    _BuildOccupiedRLEMap(occupiedMap);

    // adjacency graph is used to facilitate:
    // 1. subassembly enumeration
    _BuildAdjacencyGraph(occupiedMap);
}

std::array<int, 4> PuzzleConfig::GetPuzzleSize() const // MinX, MinZ, SizeX, Size
{
    return {_MinX, _MinZ, _SizeX, _SizeZ};
}

bool PuzzleConfig::IsEqualTo(const PuzzleConfig &rhs)
{
    if (_Data.size() != rhs._Data.size() || _SizeX != rhs._SizeX || _SizeZ != rhs._SizeZ)
    {
        return false;
    }

    for (auto &[pieceID, pieceInfo] : _Data)
    {
        auto iter = rhs._Data.find(pieceID);
        if (iter == _Data.end())
        {
            return false;
        }
        else
        {
            int relX = pieceInfo._State._OffsetX - _MinX, relZ = pieceInfo._State._OffsetZ - _MinZ;
            int rhsRelX = iter->second._State._OffsetX - rhs._MinX, rhsRelZ = iter->second._State._OffsetZ - rhs._MinZ;
            if (relX != rhsRelX || relZ != rhsRelZ)
            {
                return false;
            }
        }
    }

    return true;
}

// generate if remove then else!!!!
