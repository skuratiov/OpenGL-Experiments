//
// OpenGL framework and demo boilerplate
// (c) 2026 by Sergei Kuratiov. MIT License
//

#include "Application.h"
#include "resource.h"

extern "C" {
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001; // NVIDIA
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1; // AMD
}


//
//	Constants
//
// Global Variables:
WCHAR szTitle[] = L"OpenGL experiments";                    // The title bar text
WCHAR szWindowClass[] = L"OPENGL_DEMO";                     // the main window class name

//
//	Constructor / Desctructor
//
Application::Application() {
    m_hInstance = nullptr;
    m_hWnd = nullptr;
    m_hDC = nullptr;
    m_hGLRC = nullptr;

    m_ViewportDims.left = m_ViewportDims.top = 0;
    m_ViewportDims.bottom = 768;
    m_ViewportDims.right = 1024;

    m_startTime.QuadPart = 0;
    m_timerFrequency.QuadPart = 0;

    m_frameCounter = 0;
    m_timeAccumulator = 0.0f;
}

Application::~Application() {
}

//
// initApplicationBase
//
BOOL Application::initApplicationBase(LPWSTR lpCmdLine, HINSTANCE hInstance, int nCmdShow) {

    if (!registerWindowClass(hInstance) || !initInstance(hInstance, nCmdShow) || !initOpenGL(32, 24, 8)) {
        return FALSE;
    }

    this->Init(lpCmdLine);

    return TRUE;
}

//
// runApplicationBase
//
void Application::runApplicationBase() {
    double frameTime = 0.0;

    while (handleMessages()) {

        startFrameTimer();
    
        Run(frameTime, m_currentFPS);

        if (m_hDC) SwapBuffers(m_hDC);

        frameTime = getFrameTime();

        m_frameCounter++;
        m_timeAccumulator += frameTime;

        if (m_frameCounter >= 50) {
            m_currentFPS = 50.0f / static_cast<float>(m_timeAccumulator);
            m_timeAccumulator = 0.0;
            m_frameCounter = 0;
        }
    }
}

//
// cleanupApplicationBase
//
void Application::cleanupApplicationBase() {
    this->Done();

    destroyOpenGL();

    unregisterWindowClass();
    
    destroyInstance();
}

//
// Private methods
//

//  registerWindowClass
ATOM Application::registerWindowClass(HINSTANCE hInstance) {
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_RAYCAST));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

// Unregister WindowClass
void Application::unregisterWindowClass() {
    UnregisterClass(szWindowClass, m_hInstance);
}

// Init instance
BOOL Application::initInstance(HINSTANCE hInstance, int nCmdShow) {
    int nWinWidth = m_ViewportDims.right + (2 * GetSystemMetrics(SM_CXSIZEFRAME)),
        nWinHeight = m_ViewportDims.bottom + (2 * GetSystemMetrics(SM_CYSIZEFRAME)) + GetSystemMetrics(SM_CYCAPTION);

    m_hWnd = CreateWindowW(szWindowClass, szTitle, WS_DLGFRAME | WS_SYSMENU,
        CW_USEDEFAULT, 0, nWinWidth, nWinHeight, nullptr, nullptr, hInstance, nullptr);

    if (!m_hWnd) {
        return FALSE;
    }

    ShowWindow(m_hWnd, nCmdShow);
    UpdateWindow(m_hWnd);

    SetForegroundWindow(m_hWnd);
    SetFocus(m_hWnd);

    if (nullptr == (m_hDC = GetDC(m_hWnd))) {
        DestroyWindow(m_hWnd);
        m_hWnd = nullptr;
        return FALSE;
    }

    return TRUE;
}

// Destroy instance
void Application::destroyInstance() {
    if (m_hWnd) {
        DestroyWindow(m_hWnd);
        m_hWnd = nullptr;
    }
}


// WndProc
LRESULT CALLBACK  Application::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

    switch (message) {
    case WM_KEYDOWN:
        switch (wParam) {
        case VK_UP: {
            break;
        }
        case VK_DOWN: {
            break;
        }
        case VK_LEFT: {
            break;
        }
        case VK_RIGHT: {
            break;
        }
        default: {
            break;
        }
        }
        break;

    case WM_KEYUP:
        switch (wParam) {
        case VK_UP: {
            break;
        }
        case VK_DOWN: {
            break;
        }
        case VK_LEFT: {
            break;
        }
        case VK_RIGHT: {
            break;
        }
        default: {
            break;
        }
        }
        break;

    case WM_CLOSE:
    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

// handleMessages
BOOL Application::handleMessages() {
    // Message control
    static MSG msg;

    if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
        if (WM_QUIT == msg.message || WM_CLOSE == msg.message) {
            return FALSE;
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return TRUE;
}


BOOL Application::initOpenGL(BYTE nColorBits, BYTE nDepthBits, BYTE nStencilBits) {
    int nPfdNumber = 0;
    HGLRC hContextGL3x = 0;

    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        nColorBits,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        nDepthBits,
        nStencilBits,
        0,
        PFD_MAIN_PLANE,
        0, 0, 0, 0
    };

    // Choose a pixel format
    if (0 == (nPfdNumber = ChoosePixelFormat(m_hDC, &pfd))) return FALSE;

    // Set the pixel format for the device context
    if (FALSE == SetPixelFormat(m_hDC, nPfdNumber, &pfd)) return FALSE;

    // Describe it
    if (0 == DescribePixelFormat(m_hDC, nPfdNumber, sizeof(PIXELFORMATDESCRIPTOR), &pfd)) return -4;

    // Create OpenGL context
    if (NULL == (m_hGLRC = wglCreateContext(m_hDC))) return FALSE;

    // Make it current
    if (FALSE == wglMakeCurrent(m_hDC, m_hGLRC)) return FALSE;

    if (GLEW_OK != glewInit()) {
        return FALSE;
    }

    int attribList[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 0,
        WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
        0, 0
    };

    // Create OpenGL >=3.x context
    HGLRC hGLRC = NULL;
    if (wglewIsSupported("WGL_ARB_create_context")) {
        if (NULL != (hGLRC = wglCreateContextAttribsARB(m_hDC, 0, attribList))) {
            wglMakeCurrent(NULL, NULL);
            wglDeleteContext(m_hGLRC);

            m_hGLRC = hGLRC;
            if (FALSE == wglMakeCurrent(m_hDC, m_hGLRC)) return FALSE;
        }
    }
    else {
        return FALSE;
    }
       

    return TRUE;
}

void Application::destroyOpenGL() {
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(m_hGLRC);

    ReleaseDC(m_hWnd, m_hDC);
    m_hDC = NULL;
}

void Application::setWindowTitle(LPCWSTR szTitle) const {
    if (!m_hWnd || !szTitle) return;

    WCHAR buffer[256]{};

    int len = GetWindowTextW(m_hWnd, buffer, 256);

    if (len <= 0 || len >= 253) return;

    wcscat_s(buffer, L": ");
    wcscat_s(buffer, szTitle);

    SetWindowTextW(m_hWnd, buffer);
}
