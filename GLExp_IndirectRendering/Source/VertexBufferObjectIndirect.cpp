#include "framework.h"
#include "VertexBufferObjectIndirect.h"

//
//  Constructor / destructor
//
VertexBufferObjectIndirect::VertexBufferObjectIndirect() {
    m_indirectBuffer = 0;
    memset(&m_cmd, 0, sizeof(m_cmd));
}

VertexBufferObjectIndirect::~VertexBufferObjectIndirect() {
    deleteIndirect();
}

void VertexBufferObjectIndirect::createIndirect(void* vertexData, GLsizei vertexDataSize,
    void* indexData, GLsizei indexDataSize, uint16_t format) {
    
    createBuffers(vertexData, vertexDataSize, indexData, indexDataSize, format);
    
    if (!m_hasIndex) return; 

    m_cmd.count = m_nIndexCount;
    m_cmd.instanceCount = 1;
    m_cmd.firstIndex = 0;
    m_cmd.baseVertex = 0;
    m_cmd.baseInstance = 0;

    bind();

    glGenBuffers(1, &m_indirectBuffer);

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirectBuffer);

    glBufferData(
        GL_DRAW_INDIRECT_BUFFER,
        sizeof(m_cmd),
        &m_cmd,
        GL_DYNAMIC_DRAW
    );

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
}

void VertexBufferObjectIndirect::updateCommand() {
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirectBuffer);

    glBufferSubData(
        GL_DRAW_INDIRECT_BUFFER,
        0,
        sizeof(m_cmd),
        &m_cmd
    );

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
}

void VertexBufferObjectIndirect::drawIndirect() const {
    if (!m_hasIndex || m_indirectBuffer == 0)
        return;

    glBindVertexArray(m_nVertexArrayId);

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_indirectBuffer);

    glDrawElementsIndirect(
        GL_TRIANGLES,
        GL_UNSIGNED_INT,
        nullptr
    );

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
}


void VertexBufferObjectIndirect::deleteIndirect() {
   
    if (m_indirectBuffer) {
        glDeleteBuffers(1, &m_indirectBuffer);
        m_indirectBuffer = 0;
    }

    deleteBuffers();
}
