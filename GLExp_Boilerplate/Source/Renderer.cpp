//
// OpenGL framework and demo boilerplate
// (c) 2026 by Sergei Kuratiov. MIT License
//

#include "framework.h"

#include "Renderer.h"
#include "Image.h"

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

// Init scene
void Renderer::initScene() {
    Image image;
    image.fromDDS("../Images/metal.dds", TEXFILTER_MODE::LINEAR_ANISO);
}

// Draw frame
void Renderer::drawFrame(double frameTime) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


}
