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

Image image, normalmap;
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
        glm::vec3(0.0f, 0.0f, 4.0f), 
        glm::vec3(0.0f, 0.0f, 0.0f), 
        glm::vec3(0.0f, 1.0f, 0.0f)  
    );


}

// Init scene
void Renderer::initScene() {
    //image.fromDDS("../Images/Wood.dds", TEXFILTER_MODE::LINEAR_ANISO);
    //normalmap.fromDDS("../Images/WoodDOT3.dds", TEXFILTER_MODE::LINEAR_ANISO);

    image.fromTGA("../Images/bricks.tga", TEXFILTER_MODE::LINEAR_ANISO);
    normalmap.fromTGA("../Images/bricks_n.tga", TEXFILTER_MODE::LINEAR_ANISO);

    shaderProgram.fromSrc("../Shaders/bumpmap.vert", "../Shaders/bumpmap.frag");
    
    struct Vertex {
        float x, y, z;      // position
        float u, v;         // texcoord
        float nx, ny, nz;   // normal
        float tx, ty, tz;   // tangent
    };

    Vertex verts[] = {

        // Front (+Z)
        {-1,-1, 1,  0,0,  0,0,1,  1,0,0},
        { 1,-1, 1,  1,0,  0,0,1,  1,0,0},
        { 1, 1, 1,  1,1,  0,0,1,  1,0,0},
        {-1, 1, 1,  0,1,  0,0,1,  1,0,0},

        // Back (-Z)
        { 1,-1,-1,  0,0,  0,0,-1, -1,0,0},
        {-1,-1,-1,  1,0,  0,0,-1, -1,0,0},
        {-1, 1,-1,  1,1,  0,0,-1, -1,0,0},
        { 1, 1,-1,  0,1,  0,0,-1, -1,0,0},

        // Left (-X)
        {-1,-1,-1,  0,0,  -1,0,0,  0,0,1},
        {-1,-1, 1,  1,0,  -1,0,0,  0,0,1},
        {-1, 1, 1,  1,1,  -1,0,0,  0,0,1},
        {-1, 1,-1,  0,1,  -1,0,0,  0,0,1},

        // Right (+X)
        { 1,-1, 1,  0,0,  1,0,0,  0,0,-1},
        { 1,-1,-1,  1,0,  1,0,0,  0,0,-1},
        { 1, 1,-1,  1,1,  1,0,0,  0,0,-1},
        { 1, 1, 1,  0,1,  1,0,0,  0,0,-1},

        // Top (+Y)
        {-1,1, 1,  0,0,  0,1,0,  1,0,0},
        { 1,1, 1,  1,0,  0,1,0,  1,0,0},
        { 1,1,-1,  1,1,  0,1,0,  1,0,0},
        {-1,1,-1,  0,1,  0,1,0,  1,0,0},

        // Bottom (-Y)
        {-1,-1,-1,  0,0,  0,-1,0,  1,0,0},
        { 1,-1,-1,  1,0,  0,-1,0,  1,0,0},
        { 1,-1, 1,  1,1,  0,-1,0,  1,0,0},
        {-1,-1, 1,  0,1,  0,-1,0,  1,0,0},
    };

    
    GLuint indices[] = {
        0, 1, 2, 2, 3, 0,       // Front
        4, 5, 6, 6, 7, 4,       // Back
        8, 9, 10, 10, 11, 8,    // Left
        12, 13, 14, 14, 15, 12, // Right
        16, 17, 18, 18, 19, 16, // Top
        20, 21, 22, 22, 23, 20  // Bottom
    };

    cube.createBuffers(verts, sizeof(verts), indices, sizeof(indices), VERTEX_DATA_FORMAT::FLOAT_VX3UV2NR3TN3);
}

// Draw frame
void Renderer::drawFrame(double frameTime) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_rotationAngle += (float) frameTime * 50.0f;
    m_rotationAngle = fmod(m_rotationAngle, 360.0f);

    m_modelMatrix = glm::mat4(1.0f);
    m_modelMatrix = glm::scale(m_modelMatrix, glm::vec3(1.0f));
    m_modelMatrix = glm::translate(m_modelMatrix, glm::vec3(0.0f, 0.0f, 1.0f));
    m_modelMatrix = glm::rotate(m_modelMatrix, glm::radians(m_rotationAngle), glm::vec3(1.0f, 0.0f, 0.0f));
    m_modelMatrix = glm::rotate(m_modelMatrix, glm::radians(m_rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    m_modelMatrix = glm::rotate(m_modelMatrix, glm::radians(m_rotationAngle), glm::vec3(0.0f, 0.0f, 1.0f));

    glActiveTexture(GL_TEXTURE0);
    image.bindTexture();

    glActiveTexture(GL_TEXTURE1);
    normalmap.bindTexture();

    shaderProgram.useProgram();

    GLuint diffuseLoc = glGetUniformLocation(shaderProgram.getProgramId(), "diffuseMap");
    glUniform1i(diffuseLoc, 0); // GL_TEXTURE0

    GLuint normalLoc = glGetUniformLocation(shaderProgram.getProgramId(), "normalMap");
    glUniform1i(normalLoc, 1); // GL_TEXTURE1

    GLuint lightLoc = glGetUniformLocation(shaderProgram.getProgramId(), "lightPos");
    glUniform3f(lightLoc, 0.0f, 0.0f, 10.0f);
       
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