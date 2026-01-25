#include "Demo.h"

//
//  Globals
//
Demo* Demo::m_pInstance = nullptr;


//
// Constructor / destructor
//
Demo::Demo() {
}

Demo::~Demo() {
}


//
//	Init 
//
BOOL Demo::Init(LPWSTR lpCmdLine) {
	return TRUE;
}


//
// Run
//
void Demo::Run(double frameTime, float fps) {
}


//
// Done
//
void Demo::Done() {
}


//
// Define entry point
//
DEFINE_GL_APPLICATION(Demo)