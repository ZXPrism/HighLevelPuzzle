#include "DisassemblyGraph.h"

#include <fstream>
#include <queue>
#include <stack>

#include "Logger.h"

#include "HLP_Config.h"

bool DisassemblyGraph::ImportPuzzle(const std::string &puzzleFilePath)
{
    _GraphEdges.clear();
    _GraphNodes.clear();
    _GraphNodesParents.clear();
    _TargetNodeIDs.clear();
    _DisassemblyPlan.clear();
    _MinTargetNodeDepth = 0x3f3f3f3f;
    _DisasmGraphBuilt = false;
    _PrevTargetNodeID = -1;

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
    _GraphNodesParents.push_back(-1); // rootNode has no parents..

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

void DisassemblyGraph::BuildKernelDisassemblyGraph(int configID, int relativeDepth, int fullConfigDelta)
{
    if (_GraphNodes.empty())
    {
        LOG_ERROR("No puzzle cam be disassembled :( Please generate or import one.");
        return;
    }

    int currentMinTargetNodeDepth = 0x3f3f3f3f;

    std::unordered_map<int, bool> visit;
    std::queue<int> queue;
    queue.push(configID);
    visit[configID] = false;

    while (!queue.empty())
    {
        int frontConfigID = queue.front();
        queue.pop();

        if (visit[frontConfigID]) // don't visit the same node again!
        {
            continue;
        }

        auto &frontConfig = _GraphNodes[frontConfigID];
        int currentDepth = frontConfig->GetDepth();
        if (!frontConfig->IsFullConfig(fullConfigDelta))
        {
            currentMinTargetNodeDepth = std::min(currentMinTargetNodeDepth, currentDepth);
            _TargetNodeIDs[currentDepth - relativeDepth] = frontConfigID;
        }
        else if (currentDepth < currentMinTargetNodeDepth) // pruning
        {
            std::vector<int> pendingNeighbors;
            std::vector<std::shared_ptr<PuzzleConfig>> neighborConfigs;
            CalculateNeighborConfigs(frontConfigID, neighborConfigs);

            int neighborCnt = neighborConfigs.size();

            for (int k = 0; k < neighborCnt; k++)
            {
                auto &neighborConfig = neighborConfigs[k];

                // the paper missed an important assumption!!
                // if found a target node, don't check other neighbors, only add the target node
                // or this function will NEVER STOP!
                if (!neighborConfig->IsFullConfig(fullConfigDelta))
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
                    // the depth of that "already existing" config must be the same as or shallower than current config
                    // no need to update the preceding node
                    _GraphEdges[existConfigID].insert(frontConfigID);
                    _GraphEdges[frontConfigID].insert(existConfigID);
                }
                else
                {
                    int newConfigID = _GraphNodes.size();
                    _GraphNodes.push_back(neighborConfigs[pendingNeighborConfig]);
                    _GraphEdges.emplace_back();
                    _GraphNodesParents.push_back(frontConfigID);

                    _GraphEdges[newConfigID].insert(frontConfigID);
                    _GraphEdges[frontConfigID].insert(newConfigID);

                    visit[newConfigID] = false;

                    queue.push(newConfigID);
                }
            }
        }

        visit[frontConfigID] = true;
    }

    _MinTargetNodeDepth = std::min(_MinTargetNodeDepth, currentMinTargetNodeDepth);
    _DisasmGraphBuilt = true;

    // extract the kernel disassembly plan from the root node to the shallowest target node
    std::stack<int> planStack;
    int currentNodeID = _TargetNodeIDs.begin()->second;

    while (currentNodeID != _PrevTargetNodeID)
    {
        planStack.push(currentNodeID);
        currentNodeID = _GraphNodesParents[currentNodeID];
    }
    _PrevTargetNodeID = _TargetNodeIDs.begin()->second;

    while (!planStack.empty())
    {
        _DisassemblyPlan.push_back(planStack.top());
        planStack.pop();
    }

    LOG_INFO("Extracted kernel disassembly plan. Plan size = %d", _DisassemblyPlan.size());
}

void DisassemblyGraph::BuildCompleteDisassemblyGraph()
{
    // basically, to generate a complete disassembly graph
    // we only need to recursively execute BuildKernelDisassemblyGraph
    // the different thing is:
    // 1. the meaning of "depth" should change, relative to the previous kernel disassembly graph's target node
    // 2. the criteria of "full config" should change, since every config after first kernel disassembling is not a full config

    // for simplicity I didn't disassemble those removed subassembly, only disassemble the remaining parts.

    // NOTE: always start from node #0
    BuildKernelDisassemblyGraph();
    auto prevTargetNode = _GraphNodes[_PrevTargetNodeID];
    while (prevTargetNode->GetPuzzlePieceNum() != 1)
    {
        BuildKernelDisassemblyGraph(_PrevTargetNodeID, prevTargetNode->GetDepth(), prevTargetNode->GetRemovedPieceNum());
        prevTargetNode = _GraphNodes[_PrevTargetNodeID];
    }

    _DisasmGraphBuilt = true;
}

int DisassemblyGraph::GetDisasmPlanConfigID(int planOffset)
{
    return _DisassemblyPlan[planOffset];
}

bool DisassemblyGraph::IsDisasmGraphBuilt() const
{
    return _DisasmGraphBuilt;
}

int DisassemblyGraph::GetPuzzleDifficulty() const
{
    return _MinTargetNodeDepth;
}

int DisassemblyGraph::GetDisasmPlanSize() const
{
    return _DisassemblyPlan.size();
}
