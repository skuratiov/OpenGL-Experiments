//
// OpenGL framework and demo boilerplate
// (c) 2026 by Sergei Kuratiov. MIT License
//

#include "framework.h"
#include "Camera.h"
#include "Demo.h"
#include "Renderer.h"
#include "Image.h"
#include "ShaderProgram.h"
#include "VertexBufferObject.h"
#include "VertexBufferObjectIndirect.h"
#include "TextPainter.h"
#include "Scene.h"

//
//  Globals
//
Renderer* Renderer::m_pInstance = nullptr;

TextPainter textPainter;
ShaderProgram shaderProgram;

// Default fallback textures (created once)
static GLuint s_defaultWhiteTex = 0;
static GLuint s_defaultNormalTex = 0;

static void ensureDefaultTextures() {
	if (s_defaultWhiteTex == 0) {
		glGenTextures(1, &s_defaultWhiteTex);
		glBindTexture(GL_TEXTURE_2D, s_defaultWhiteTex);
		unsigned char white[4] = { 255,255,255,255 };
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	if (s_defaultNormalTex == 0) {
		glGenTextures(1, &s_defaultNormalTex);
		glBindTexture(GL_TEXTURE_2D, s_defaultNormalTex);
		// default normal (0.5,0.5,1.0) -> 128,128,255
		unsigned char normal[4] = { 128,128,255,255 };
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, normal);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

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
    shaderProgram.cleanup();
    if (s_defaultWhiteTex) { glDeleteTextures(1, &s_defaultWhiteTex); s_defaultWhiteTex = 0; }
    if (s_defaultNormalTex) { glDeleteTextures(1, &s_defaultNormalTex); s_defaultNormalTex = 0; }
}

// Init view
void Renderer::setupView(long width, long height) {

    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glCullFace(GL_BACK);
    glFrontFace(GL_CW);
    glEnable(GL_CULL_FACE);

    float aspectRatio = ((float)width / (float)height);
    m_projectionMatrix = glm::perspective(glm::radians(90.0f), aspectRatio, 0.1f, 1000.0f);
}

// Init scene
void Renderer::initScene() {

    Demo* pDemo = Demo::getInstance();

    this->cleanup();
    textPainter.initFont();

    // Show immediate loading text
    textPainter.beginTextLayer();
	textPainter.textOut((wchar_t*)L"Loading scene...", 10, 10, 0.25f, glm::vec3(1.0f, 0.0f, 0.0f));
    textPainter.endTextLayer();
	pDemo->swapBuffers();

    Scene* pScene = Scene::getInstance();
	if (!pScene) return;

	BOOL res = pScene->fromOBJ("../Scene/TestBox.obj");
	if (!res) {
		textPainter.beginTextLayer();
		textPainter.textOut((wchar_t*)L"Failed to load scene", 10, 30, 0.25f, glm::vec3(1.0f, 0.0f, 0.0f));
		textPainter.endTextLayer();
		pDemo->swapBuffers();
		return;
	}

	// Ensure default fallback textures exist (1x1 white and normal)
	ensureDefaultTextures();

	// Load shader program used for scene rendering
	if (shaderProgram.fromSrc("../Shaders/bumpmap.vert", "../Shaders/bumpmap.frag") == TRUE) {
		m_diffuseLoc = glGetUniformLocation(shaderProgram.getProgramId(), "diffuseMap");
		m_normalLoc = glGetUniformLocation(shaderProgram.getProgramId(), "normalMap");
		m_lightLoc = glGetUniformLocation(shaderProgram.getProgramId(), "lightPos");
		m_modelMatrixLoc = glGetUniformLocation(shaderProgram.getProgramId(), "modelMatrix");
		m_mvpMatrixLoc = glGetUniformLocation(shaderProgram.getProgramId(), "mvpMatrix");
	} else {
		// Shader failed — inform user
		textPainter.beginTextLayer();
		textPainter.textOut((wchar_t*)L"Failed to compile/link shader", 10, 50, 0.25f, glm::vec3(1.0f, 0.0f, 0.0f));
		textPainter.endTextLayer();
		pDemo->swapBuffers();
	}

    // Reset camera state to avoid carrying input/delta from app startup
    Camera* pCamera = Camera::getInstance();
    if (pCamera) {
        pCamera->resetState();
    }

    textPainter.beginTextLayer();
	textPainter.textOut((wchar_t*)L"Scene loaded. Ready to render.", 10, 10, 0.25f, glm::vec3(0.0f, 1.0f, 0.0f));
    textPainter.endTextLayer();
	pDemo->swapBuffers();
}

// Draw frame
void Renderer::drawFrame(double frameTime, float fps) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Camera* pCamera = Camera::getInstance();
	pCamera->getViewMatrixUpdated(m_viewMatrix);
       
    m_modelMatrix = glm::mat4(1.0f);

    // no automatic rotation — keep model static
    glm::mat4 modelRot = glm::mat4(1.0f);

    // Use shader if available
    if (shaderProgram.getProgramId()) {
        shaderProgram.useProgram();

        // Set per-frame uniforms
        glm::mat4 mvpBase = m_projectionMatrix * m_viewMatrix;

        // Light position in world space
        glUniform3f(m_lightLoc, 0.0f, 0.0f, 4.0f);

        Scene* pScene = Scene::getInstance();
        if (pScene) {
            size_t meshCount = pScene->getMeshCount();
            for (size_t i = 0; i < meshCount; ++i) {
                Mesh* mesh = pScene->getMesh(i);
                if (!mesh || !mesh->vbo) continue;

                // Per-mesh model matrix (here we just use rotating model for all meshes)
                glm::mat4 modelMatrix = modelRot;
                glm::mat4 mvpMatrix = mvpBase * modelMatrix;

                // Bind diffuse texture (unit 0) and normal map (unit 1). Use defaults if material lacks textures.
                glActiveTexture(GL_TEXTURE0);
                if (mesh->material && mesh->material->diffuseTex) {
                    mesh->material->diffuseTex->bindTexture();
                } else {
                    glBindTexture(GL_TEXTURE_2D, s_defaultWhiteTex);
                }

                glActiveTexture(GL_TEXTURE1);
                if (mesh->material && mesh->material->normalMap) {
                    mesh->material->normalMap->bindTexture();
                } else {
                    glBindTexture(GL_TEXTURE_2D, s_defaultNormalTex);
                }

                // Set sampler uniforms
                glUniform1i(m_diffuseLoc, 0);
                glUniform1i(m_normalLoc, 1);

                // Set matrices
                glUniformMatrix4fv(m_modelMatrixLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
                glUniformMatrix4fv(m_mvpMatrixLoc, 1, GL_FALSE, glm::value_ptr(mvpMatrix));

                // Draw the mesh (indirect VBO). VertexBufferObjectIndirect provides drawIndirect()
                mesh->vbo->drawIndirect();

                // Unbind textures for cleanliness
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, 0);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, 0);
            }
        }

        shaderProgram.freeProgram();
    }
    
    // Overlay FPS
    wchar_t fpsStr[64];
    swprintf(fpsStr, 32, L"FPS: %.2f", fps);
    textPainter.beginTextLayer();
    textPainter.textOut(fpsStr, 10, 10, 0.25f, glm::vec3(1.0, 0.0f, 0.0f));

    swprintf(fpsStr, 64, L"Camera(YPR): %.2f %.2f %.2f", pCamera->getYaw(), pCamera->getPitch(), pCamera->getRoll());
    textPainter.textOut(fpsStr, 10, 26, 0.25f, glm::vec3(1.0, 1.0f, 1.0f));

    swprintf(fpsStr, 64, L"Camera(POS): %.2f %.2f %.2f", pCamera->getPosition().x,
        pCamera->getPosition().y, pCamera->getPosition().z);
    textPainter.textOut(fpsStr, 10, 42, 0.25f, glm::vec3(1.0, 1.0f, 1.0f));

    textPainter.endTextLayer();
}

//
void Renderer::cleanup() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    Camera* pCamera = Camera::getInstance();
    
    m_viewMatrix = glm::mat4(1.0f);
    m_modelMatrix = glm::mat4(1.0f);

    textPainter.cleanup();
}
