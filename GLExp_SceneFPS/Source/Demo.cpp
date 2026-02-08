//
// OpenGL framework and demo boilerplate
// (c) 2026 by Sergei Kuratiov. MIT License
//

#include "Demo.h"
#include "Camera.h"
#include "Renderer.h"


//
//  Globals
//
Demo* Demo::m_pInstance = nullptr;

Renderer* g_pRenderer = Renderer::getInstance();
Camera* g_pCamera = Camera::getInstance();

//
// Constructor / destructor
//
Demo::Demo() {
}

Demo::~Demo() {
}

//	Init 
BOOL Demo::Init(LPWSTR lpCmdLine) {

	setWindowTitle(L"Indirect rendering");
	
	g_pRenderer->setupView(getViewportWidth(), getViewportHeight());
	g_pRenderer->initScene();

	return TRUE;
}

// Run
void Demo::Run(double frameTime, float fps) {

	//	Update camera
	float mouseDX, mouseDY;
	if (consumeMouseDelta(mouseDX, mouseDY)) {
		g_pCamera->onMouseMove(mouseDX, mouseDY, (float)frameTime);
	}
	g_pCamera->updateOnControls((float)frameTime);



	//	Render frame
	g_pRenderer->drawFrame(frameTime, fps);
}


// Done
void Demo::Done() {
	g_pRenderer->cleanup();
}

void Demo::onKeyDown(int key) {
	g_pCamera->onKeyDown(key);
}

void Demo::onKeyUp(int key) {
	g_pCamera->onKeyUp(key);
}

//
// Define entry point
//
DEFINE_GL_APPLICATION(Demo)