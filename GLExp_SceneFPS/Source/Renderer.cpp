//
// OpenGL framework and demo boilerplate
// (c) 2026 by Sergei Kuratiov. MIT License
//

#include "framework.h"
#include "Camera.h"
#include "Renderer.h"
#include "Image.h"
#include "ShaderProgram.h"
#include "VertexBufferObject.h"
#include "VertexBufferObjectIndirect.h"
#include "TextPainter.h"

//
//  Globals
//
Renderer* Renderer::m_pInstance = nullptr;

TextPainter textPainter;

//
// Constructor / destructor
//
Renderer::Renderer() {
    m_modelMatrix = glm::mat4(1.0);
    m_viewMatrix = glm::mat4(1.0);
    m_projectionMatrix = glm::mat4(1.0);

    m_rotationAngle = 0.0f;

    m_diffuseLoc = m_normalLoc = m_lightLoc = m_modelMatrixLoc = m_mvpMatrixLoc = 0;
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
}

// Init scene
void Renderer::initScene() {
      

    textPainter.initFont();
}

// Draw frame
void Renderer::drawFrame(double frameTime, float fps) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Camera* pCamera = Camera::getInstance();
	pCamera->getViewMatrixUpdated(m_viewMatrix);
       
    m_modelMatrix = glm::mat4(1.0f);
   
    /*
	*  Draw scene using MVP matrices and shaders here
    */

       
    
    wchar_t fpsStr[32];
    swprintf(fpsStr, 32, L"FPS: %.2f", fps);
    textPainter.beginTextLayer();
    textPainter.textOut(fpsStr, 10, 10, 0.25, glm::vec3(1.0, 0.0f, 0.0f));
    textPainter.endTextLayer();
}

//
void Renderer::cleanup() {

    textPainter.cleanup();
}