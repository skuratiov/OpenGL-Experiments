//
// OpenGL framework and demo boilerplate
// (c) 2026 by Sergei Kuratiov. MIT License
//

#include "framework.h"
#include "Renderer.h"
#include "Image.h"
#include "ShaderProgram.h"
#include "VertexBufferObject.h"

//
//  Globals
//
Renderer* Renderer::m_pInstance = nullptr;

Image image;
ShaderProgram shaderProgram;
VertexBufferObject cube;

//
// Constructor / destructor
//
Renderer::Renderer() {
    m_modelMatrix = glm::mat4(1.0);
    m_viewMatrix = glm::mat4(1.0);
    m_projectionMatrix = glm::mat4(1.0);

    m_rotationAngle = 0.0f;
}

Renderer::~Renderer() {
}

// Init view
void Renderer::setupView(long width, long height) {

    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    float aspectRatio = ((float)width / (float)height);
    m_projectionMatrix = glm::perspective(glm::radians(90.0f), aspectRatio, 0.1f, 1000.0f);

    m_viewMatrix = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 5.0f), 
        glm::vec3(0.0f, 0.0f, 0.0f), 
        glm::vec3(0.0f, 1.0f, 0.0f)  
    );


}

// Init scene
void Renderer::initScene() {
    image.fromDDS("../Images/metal.dds", TEXFILTER_MODE::LINEAR_ANISO);

    shaderProgram.fromSrc("../Shaders/basic.vert", "../Shaders/basic.frag");
    
    struct Vertex { float x, y, z, u, v; };
    Vertex verts[] = {
        // Front face (+Z)
        {-1, -1,  1, 0, 0}, { 1, -1,  1, 1, 0},
        { 1,  1,  1, 1, 1}, {-1,  1,  1, 0, 1},
        // Back face (-Z)
        { 1, -1, -1, 0, 0}, {-1, -1, -1, 1, 0},
        {-1,  1, -1, 1, 1}, { 1,  1, -1, 0, 1},
        // Left face (-X)
        {-1, -1, -1, 0, 0}, {-1, -1,  1, 1, 0},
        {-1,  1,  1, 1, 1}, {-1,  1, -1, 0, 1},
        // Right face (+X)
        { 1, -1,  1, 0, 0}, { 1, -1, -1, 1, 0},
        { 1,  1, -1, 1, 1}, { 1,  1,  1, 0, 1},
        // Top face (+Y)
        {-1,  1,  1, 0, 0}, { 1,  1,  1, 1, 0},
        { 1,  1, -1, 1, 1}, {-1,  1, -1, 0, 1},
        // Bottom face (-Y)
        {-1, -1, -1, 0, 0}, { 1, -1, -1, 1, 0},
        { 1, -1,  1, 1, 1}, {-1, -1,  1, 0, 1},
    };
    
    GLuint indices[] = {
        0, 1, 2, 2, 3, 0,       // Front
        4, 5, 6, 6, 7, 4,       // Back
        8, 9, 10, 10, 11, 8,    // Left
        12, 13, 14, 14, 15, 12, // Right
        16, 17, 18, 18, 19, 16, // Top
        20, 21, 22, 22, 23, 20  // Bottom
    };


    cube.createBuffers(verts, sizeof(verts), indices, sizeof(indices), VERTEX_DATA_FORMAT::FLOAT_VX3UV2);
}

// Draw frame
void Renderer::drawFrame(double frameTime) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_rotationAngle += (float) frameTime * 50.0f;
    if (m_rotationAngle > 360.0f) { m_rotationAngle -= 360.0f; }

    m_modelMatrix = glm::mat4(1.0f);
    m_modelMatrix = glm::translate(m_modelMatrix, glm::vec3(0.0f, 0.0f, 1.0f));
    m_modelMatrix = glm::rotate(m_modelMatrix, glm::radians(m_rotationAngle), glm::vec3(1.0f, 0.0f, 0.0f));
    m_modelMatrix = glm::rotate(m_modelMatrix, glm::radians(m_rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    m_modelMatrix = glm::rotate(m_modelMatrix, glm::radians(m_rotationAngle), glm::vec3(0.0f, 0.0f, 1.0f));
    m_modelMatrix = glm::scale(m_modelMatrix, glm::vec3(1.0f));

    image.bindTexture();

    shaderProgram.useProgram();

    GLuint projectionLoc = glGetUniformLocation(shaderProgram.getProgramId(), "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(m_projectionMatrix));

    GLuint modelLoc = glGetUniformLocation(shaderProgram.getProgramId(), "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m_modelMatrix));

    GLuint viewLoc = glGetUniformLocation(shaderProgram.getProgramId(), "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(m_viewMatrix));


    cube.draw();
    cube.unbind();
}

//
void Renderer::cleanup() {

    cube.deleteBuffers();
}