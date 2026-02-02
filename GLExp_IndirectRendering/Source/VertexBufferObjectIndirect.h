#pragma once
#include "VertexBufferObject.h"

class VertexBufferObjectIndirect : public VertexBufferObject {
public:
    VertexBufferObjectIndirect();
    ~VertexBufferObjectIndirect();

    void createIndirect(void*, GLsizei, void*, GLsizei, uint16_t);
    void updateCommand();
    void drawIndirect() const;
    void deleteIndirect();

private:

    struct DrawElementsIndirectCommand
    {
        GLuint count;
        GLuint instanceCount;
        GLuint firstIndex;
        GLuint baseVertex;
        GLuint baseInstance;
    };

    DrawElementsIndirectCommand m_cmd;
    GLuint m_indirectBuffer;
};

