// header.h : include file for standard system include files,
// or project specific include files
//

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define STRICT							
#define VC_LEANMEAN						

// Windows Header Files
#include <windows.h>
// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <stdarg.h>
#include <tchar.h>
#include <time.h>
#include <crtdbg.h>

// OpenGL Header Files
#define GLEW_STATIC
#include "Libs/glew/include/GL/glew.h"
#include "Libs/glew/include/GL/wglew.h"

#pragma comment (lib, "opengl32.lib")
#pragma comment (lib, "gdi32.lib")


#if defined(_WIN32) && !defined(_WIN64)
#pragma comment(lib, "Source/Libs/glew/lib/Release/Win32/glew32s.lib")
#endif

// win64
#if defined(_WIN32) && defined(_WIN64)
#pragma comment(lib, "Source/Libs/glew/lib/Release/x64/glew32s.lib")
#endif
