//
// OpenGL framework and demo boilerplate
// (c) 2026 by Sergei Kuratiov. MIT License
//

#include "Demo.h"
#include "Renderer.h"

//
//  Globals
//
Demo* Demo::m_pInstance = nullptr;

Renderer* g_pRenderer = Renderer::getInstance();

//
// Constructor / destructor
//
Demo::Demo() {
}

Demo::~Demo() {
}

//	Init 
BOOL Demo::Init(LPWSTR lpCmdLine) {
	
	g_pRenderer->setupView(getViewportWidth(), getViewportHeight());
	g_pRenderer->initScene();

	return TRUE;
}

// Run
void Demo::Run(double frameTime, float fps) {

	g_pRenderer->drawFrame(frameTime);
}


// Done
void Demo::Done() {
}


//
// Define entry point
//
DEFINE_GL_APPLICATION(Demo)