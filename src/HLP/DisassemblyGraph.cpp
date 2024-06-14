#include "DisassemblyGraph.h"

#include <fstream>
#include <queue>

#include "Logger.h"

#include "HLP_Config.h"

bool DisassemblyGraph::ImportPuzzle(const std::string &puzzleFilePath)
{
    _GraphEdges.clear();
    _GraphNodes.clear();

    // load the puzzle file as the initial puzzle config
    std::ifstream fin(puzzleFilePath);
    if (!fin)
    {
        LOG_ERROR("Unable to open the configuation file!");
        return false;
    }

    // check if the file is valid
    int magicNumber = 0;
    fin >> magicNumber;
    if (magicNumber != cPuzzleFileMagicNumber)
    {
        LOG_ERROR("This is not a valid configuration file!");
        return false;
    }

    // load:
    // 1. the number of puzzle pieces
    // 2. the x-axis size of puzzle
    // 3. the z-axis size of puzzle
    int pieceNum = 0;
    fin >> pieceNum;
    auto &rootNode = _GraphNodes.emplace_back(std::make_shared<PuzzleConfig>(0, pieceNum));
    _GraphEdges.emplace_back();

    // load each puzzle piece
    for (int i = 0; i < pieceNum; i++)
    {
        auto piece = std::make_shared<PuzzlePiece>();

        int voxelNum = 0, x = 0, z = 0;
        fin >> voxelNum;

        for (int j = 0; j < voxelNum; j++)
        {
            fin >> x >> z;
            piece->_Voxels.emplace_back(x, z);
        }

        rootNode->AddPuzzlePiece(i, piece);
    }

    fin.close();

    // generate acceleration structures of the config
    rootNode->BuildAccelStructures();

    // assign the materials to the pieces
    // in order to distinguish them
    rootNode->AssignPuzzlePieceMaterials();

    LOG_INFO("Successfully imported puzzle with %d puzzle pieces", pieceNum);

    return true;
}

PuzzleConfig &DisassemblyGraph::GetPuzzleConfig(int configID)
{
    return *_GraphNodes[configID];
}

void DisassemblyGraph::RenderConfig(int configID, Shader &shader, VertexBuffer &voxelModel)
{
    _GraphNodes[configID]->Render(shader, voxelModel);
}

void DisassemblyGraph::CalculateNeighborConfigs(int configID, std::vector<std::shared_ptr<PuzzleConfig>> &neighborConfigs)
{
    _GraphNodes[configID]->CalculateNeighborConfigs(neighborConfigs);
}

int DisassemblyGraph::GetPuzzleConfigNum() const
{
    return _GraphNodes.size();
}

void DisassemblyGraph::Test_AddAllNeighborConfigs(int configID)
{
    std::vector<std::shared_ptr<PuzzleConfig>> neighborConfigs;
    CalculateNeighborConfigs(configID, neighborConfigs);
    for (auto neighbor : neighborConfigs)
    {
        _GraphNodes.push_back(neighbor);
    }
}

void DisassemblyGraph::BuildKernelDisassemblyGraph()
{
    if (_GraphNodes.empty())
    {
        LOG_ERROR("No puzzle cam be disassembled :( Please generate or import one.");
        return;
    }

    if (_GraphNodes.size() != 1)
    {
        LOG_ERROR("The graph should only contain one config (i.e. puzzle) before disassembling!");
        return;
    }

    std::unordered_map<int, bool> visit;
    std::queue<int> queue;
    queue.push(0); // start from config 0!!
    visit[0] = false;

    while (!queue.empty())
    {
        int frontConfig = queue.front();
        queue.pop();

        if (visit[frontConfig]) // don't visit the same node again!
        {
            continue;
        }

        if (!_GraphNodes[frontConfig]->IsFullConfig())
        {
            _TargetNodeIDs.push_back(frontConfig);
        }
        else
        {
            std::vector<int> pendingNeighbors;
            std::vector<std::shared_ptr<PuzzleConfig>> neighborConfigs;
            CalculateNeighborConfigs(frontConfig, neighborConfigs);

            int neighborCnt = neighborConfigs.size();

            for (int k = 0; k < neighborCnt; k++)
            {
                auto &neighborConfig = neighborConfigs[k];

                // the paper missed an important assumption!!
                // if found a target node, don't check other neighbors, only add the target node
                // or this function will NEVER STOP!
                if (!neighborConfig->IsFullConfig())
                {
                    pendingNeighbors.clear();
                    pendingNeighbors.push_back(k);
                    break;
                }

                pendingNeighbors.push_back(k);
            }

            for (auto pendingNeighborConfig : pendingNeighbors)
            {
                // check if the neighborConfig has already been in _GraphNodes
                // by brute force though.. I can't figure out a better approach
                bool alreadyExistConfig = false;
                int existConfigID = -1;
                int n = _GraphNodes.size();
                for (int i = 0; i < n; i++)
                {
                    if (neighborConfigs[pendingNeighborConfig]->IsEqualTo(*_GraphNodes[i]))
                    {
                        alreadyExistConfig = true;
                        existConfigID = i;
                        break;
                    }
                }

                if (alreadyExistConfig) // if neighborConfig is already in _GraphNodes, find its ID in _GraphNodes
                {
                    _GraphEdges[existConfigID].insert(frontConfig);
                    _GraphEdges[frontConfig].insert(existConfigID);
                }
                else
                {
                    int newConfigID = _GraphNodes.size();
                    _GraphNodes.push_back(neighborConfigs[pendingNeighborConfig]);
                    _GraphEdges.emplace_back();

                    _GraphEdges[newConfigID].insert(frontConfig);
                    _GraphEdges[frontConfig].insert(newConfigID);

                    visit[newConfigID] = false;

                    queue.push(newConfigID);
                }
            }
        }

        visit[frontConfig] = true;

        if (_GraphNodes.size() > 100)
        {
            return;
        }
    }
}
