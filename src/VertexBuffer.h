#pragma once

#include <utility>
#include <vector>

class VertexBuffer
{
public:
    VertexBuffer &SetData(float *pData, unsigned cntFloats);
    VertexBuffer &UpdateData(unsigned offsetFloats, unsigned newDataSizeFloats, float *pNewData);
    VertexBuffer &SetUsage(unsigned location, unsigned nFloats);
    VertexBuffer &EndSetUsage();

    void DrawWireframeTriangles(unsigned verticesOffset, unsigned verticesCnt);
    void DrawTriangles(unsigned verticesOffset, unsigned verticesCnt);
    void DrawLines(unsigned verticesOffset, unsigned verticesCnt);

private:
    unsigned _VAO, _VBO;

    bool _BufferCreated = false;
    unsigned _TotalOffset = 0;
    unsigned _VBOSize = 0;
    std::vector<std::pair<unsigned, unsigned>> _VerticesUsages;
};
