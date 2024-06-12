#include "DisassembleGraph.h"

#include <fstream>

#include "Logger.h"

#include "HLP_Config.h"

bool DisassembleGraph::ImportPuzzle(const std::string &puzzleFilePath)
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
    auto &rootNode = _GraphNodes.emplace_back(std::make_shared<PuzzleConfig>());

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

        rootNode->AddPuzzlePiece(piece);
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

PuzzleConfig &DisassembleGraph::GetPuzzleConfig(int configNo)
{
    return *_GraphNodes[configNo];
}

void DisassembleGraph::RenderConfig(int configNo, Shader &shader, VertexBuffer &voxelModel)
{
    _GraphNodes[configNo]->Render(shader, voxelModel);
}

void DisassembleGraph::CalculateNeighborConfigs(int configNo, std::vector<std::shared_ptr<PuzzleConfig>> &neighborConfigs)
{
    _GraphNodes[configNo]->CalculateNeighborConfigs(neighborConfigs);
}
