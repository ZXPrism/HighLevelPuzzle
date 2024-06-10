#include "VertexBuffer.h"

#include <iostream>

#include <glad/glad.h>

VertexBuffer &VertexBuffer::SetData(float *pData, unsigned cntFloats)
{
    if (!_BufferCreated)
    {
        _VBOSize = cntFloats;

        glGenVertexArrays(1, &_VAO);
        glBindVertexArray(_VAO);

        glGenBuffers(1, &_VBO);
        glBindBuffer(GL_ARRAY_BUFFER, _VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * cntFloats, pData, GL_STATIC_DRAW);

        glBindVertexArray(0);
        _BufferCreated = true;
    }
    else
    {
        glDeleteVertexArrays(1, &_VAO);
        glDeleteBuffers(1, &_VBO);
        _BufferCreated = false;

        return SetData(pData, cntFloats);
    }

    return *this;
}

VertexBuffer &VertexBuffer::UpdateData(unsigned offsetFloats, unsigned newDataSizeFloats, float *pNewData)
{
    if (offsetFloats + newDataSizeFloats >= _VBOSize)
    {
        std::cout << "fatal: Too much new data for the vertex buffer!" << std::endl;
        return *this;
    }

    glBindBuffer(GL_ARRAY_BUFFER, _VBO);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(float) * offsetFloats, sizeof(float) * newDataSizeFloats, pNewData);

    return *this;
}

VertexBuffer &VertexBuffer::SetUsage(unsigned location, unsigned nFloats)
{
    if (_BufferCreated)
    {
        _VerticesUsages.push_back({location, nFloats});
        _TotalOffset += nFloats;
    }
    else
    {
        std::cout << "fatal: You must call 'SetData' before setting usages!" << std::endl;
    }

    return *this;
}

VertexBuffer &VertexBuffer::EndSetUsage()
{
    unsigned preOffset = 0;

    glBindVertexArray(_VAO);
    for (auto &[location, nFloats] : _VerticesUsages)
    {
        glVertexAttribPointer(location, nFloats, GL_FLOAT, GL_FALSE, _TotalOffset * sizeof(float), (void *)(preOffset * sizeof(float)));
        glEnableVertexAttribArray(location);
        preOffset += nFloats;
    }
    _VerticesUsages.resize(0);

    return *this;
}

void VertexBuffer::DrawTriangles(unsigned verticesOffset, unsigned verticesCnt)
{
    glBindVertexArray(_VAO);
    glDrawArrays(GL_TRIANGLES, verticesOffset, verticesCnt);
}

void VertexBuffer::DrawWireframeTriangles(unsigned verticesOffset, unsigned verticesCnt)
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void VertexBuffer::DrawLines(unsigned verticesOffset, unsigned verticesCnt)
{
    glBindVertexArray(_VAO);
    glDrawArrays(GL_LINE_STRIP, verticesOffset, verticesCnt);
}
