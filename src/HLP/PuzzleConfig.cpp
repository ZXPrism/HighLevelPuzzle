#include "PuzzleConfig.h"

#include <stack>

#include <glm/gtc/matrix_transform.hpp>

#include "Logger.h"
#include "Utils.h"

PuzzleConfig::PuzzleConfig(int puzzleSizeX, int puzzleSizeZ) : _PuzzleSizeX(puzzleSizeX), _PuzzleSizeZ(puzzleSizeZ)
{
    LOG_INFO("Constructed puzzle config with sizeX = %d, sizeZ = %d", puzzleSizeX, puzzleSizeZ);
}

void PuzzleConfig::AddPuzzlePiece(std::shared_ptr<PuzzlePiece> puzzlePiece)
{
    _Data.push_back(puzzlePiece);
}

void PuzzleConfig::Render(Shader &shader, VertexBuffer &voxelModel)
{
    shader.Activate();

    int prevWireframeState = 0;
    glm::vec3 pos{};

    for (int x = 0; x < 3 * _PuzzleSizeX; x++)
    {
        for (int z = 0; z < 3 * _PuzzleSizeZ; z++)
        {
            glm::mat4 model(1.0f);
            pos = {x - _PuzzleSizeX, 0, z - _PuzzleSizeZ}; // don't forget to map coordinates
            shader.SetUniform("model", glm::translate(model, pos));

            int pieceNo = _OccupiedMap[x][z];
            if (pieceNo != _NoPiece && !_PuzzlePieceInvisibility[pieceNo])
            {
                shader.SetUniform("color", _PuzzlePieceMaterials[_PuzzlePieceMaterialsMap[pieceNo]]._Color);
                if (prevWireframeState)
                {
                    shader.SetUniform("wireframe", 0);
                    prevWireframeState = 0;
                }
            }
            else if (!prevWireframeState)
            {
                shader.SetUniform("wireframe", 1);
                prevWireframeState = 1;
            }

            voxelModel.DrawTriangles(0, 36);
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

void PuzzleConfig::BuildAdjacencyGraph()
{
    static const int dxArray[4] = {0, 0, -1, 1};
    static const int dzArray[4] = {-1, 1, 0, 0};

    _AdjacencyGraph.resize(_Data.size());

    // initially all pieces' coordinates are in [0, sizeX) x [0, sizeZ)
    // mapped coordinates are in [sizeX, 2 * sizeX) x [sizeZ, 2 * sizeZ)
    // time complexity: O(4 * _PuzzleSizeX * _PuzzleSizeZ)

    for (int x = _PuzzleSizeX; x < 2 * _PuzzleSizeX; x++)
    {
        for (int z = _PuzzleSizeZ; z < 2 * _PuzzleSizeZ; z++)
        {
            if (_OccupiedMap[x][z] != _NoPiece)
            {
                for (int d = 0; d < 4; d++)
                {
                    int nx = x + dxArray[d];
                    int nz = z + dzArray[d];
                    if (_OccupiedMap[nx][nz] != _NoPiece && _OccupiedMap[nx][nz] != _OccupiedMap[x][z])
                    {
                        _AdjacencyGraph[_OccupiedMap[x][z]].insert(_OccupiedMap[nx][nz]);
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

void PuzzleConfig::BuildOccupiedMap()
{
    // occupied graph is 8x bigger than the original puzzle to simplify edge conditions
    // since the puzzle won't be very large (restricted by the computational cost)
    // such design is reasonable..I think.
    // time complexity: O(total number of voxels)

    _OccupiedMap = std::vector<std::vector<int>>(3 * _PuzzleSizeX, std::vector<int>(3 * _PuzzleSizeZ, _NoPiece));
    int n = _Data.size();
    for (int i = 0; i < n; i++)
    {
        auto &[piece, state] = _Data[i];
        for (auto &voxel : piece->_Voxels)
        {
            // NOTE: coordinates need to be mapped to fit into the occupied map
            int x = voxel._X + state._OffsetX + _PuzzleSizeX;
            int z = voxel._Z + state._OffsetZ + _PuzzleSizeZ;
            _OccupiedMap[x][z] = i;
        }
    }
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
                std::cout << pieceNo << ' ';
            }
            std::cout << std::endl;
        });

        // 2. calculate the max number movable steps
        // 3. for each step, generate a neighborconfig
    });
}

void PuzzleConfig::_EnumerateSubassembly(int depth, std::set<int> &pieceNos, const std::function<void()> &callback)
{
    // Normally enumeration on sets have exponential time complexity
    // But through correct pruning we will never reach that upper limit! (i hope so)

    if (depth != 0)
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

int PuzzleConfig::_MaxMovableDistance(int pieceNo)
{
    return 0;
}
