//
// OpenGL framework and demo boilerplate
// (c) 2026 by Sergei Kuratiov. MIT License
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
#include <cstdint>

// OpenGL Header Files
#define GLEW_STATIC
#include "Libs/glew/include/GL/glew.h"
#include "Libs/glew/include/GL/wglew.h"

#pragma comment (lib, "opengl32.lib")
#pragma comment (lib, "gdi32.lib")


#if defined(_WIN32) && !defined(_WIN64)
#pragma comment(lib, "Source/Libs/glew/lib/Release/Win32/glew32s.lib")

#if defined(_DEBUG)
#pragma comment(lib, "Source/Libs/freetype/lib/x86/Debug/freetype.lib")
#elif
#pragma comment(lib, "Source/Libs/freetype/lib/x86/Release/freetype.lib")
#endif // DEBUG
#endif

// win64
#if defined(_WIN32) && defined(_WIN64)
#pragma comment(lib, "Source/Libs/glew/lib/Release/x64/glew32s.lib")

#if defined(_DEBUG)
#pragma comment(lib, "Source/Libs/freetype/lib/x64/Debug/freetype.lib")
#else
#pragma comment(lib, "Source/Libs/freetype/lib/x64/Release/freetype.lib")
#endif

#endif

#define GLEW_STATIC
#include "Libs/glew/include/GL/glew.h"
#include "Libs/glew/include/GL/wglew.h"


#include "Libs/glm/glm.hpp"
#include "Libs/glm/gtc/matrix_transform.hpp"
#include "Libs/glm/gtc/type_ptr.hpp"