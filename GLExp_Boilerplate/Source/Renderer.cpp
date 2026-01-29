//
// OpenGL framework and demo boilerplate
// (c) 2026 by Sergei Kuratiov. MIT License
//

#include "framework.h"
#include "Renderer.h"
#include "Image.h"
#include "ShaderProgram.h"

//
//  Globals
//
Renderer* Renderer::m_pInstance = nullptr;

//
// Constructor / destructor
//
Renderer::Renderer() {
    m_modelMatrix = glm::mat4(1.0);
    m_viewMatrix = glm::mat4(1.0);
    m_projectionMatrix = glm::mat4(1.0);
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
    Image image;
    image.fromDDS("../Images/metal.dds", TEXFILTER_MODE::LINEAR_ANISO);

    ShaderProgram shaderProgram;
    shaderProgram.fromSrc("../Shaders/basic.vert", "../Shaders/basic.frag");
}

// Draw frame
void Renderer::drawFrame(double frameTime) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


}
