//
// OpenGL framework and demo boilerplate
// (c) 2026 by Sergei Kuratiov. MIT License
//

#include"framework.h"
#include "Renderer.h"

//
//  Globals
//
Renderer* Renderer::m_pInstance = nullptr;

//
// Constructor / destructor
//
Renderer::Renderer() {
}

Renderer::~Renderer() {
}


// drawFrame()
void Renderer::drawFrame(double frameTime) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


}
