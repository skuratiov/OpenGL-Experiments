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

    m_nMouseDX = m_nMouseDY = 0;
}

Application::~Application() {
}

//
// initApplicationBase
//
BOOL Application::initApplicationBase(LPWSTR lpCmdLine, HINSTANCE hInstance, int nCmdShow) {

    if (!registerWindowClass(hInstance) || !initInstance(hInstance, nCmdShow) || 
        !initOpenGL(32, 24, 8) || !initRawInput()) {
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

    // Initialize timer so first measured frameTime is valid
    startFrameTimer();

    while (handleMessages()) {

        // Measure time elapsed since last frame and restart timer for next frame
        frameTime = getFrameTime();
        startFrameTimer();

        if (m_hDC && m_hGLRC) {
            Run(frameTime, m_currentFPS);
            swapBuffers();
        }

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

    destroyRawInput();

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
    wcex.cbWndExtra = sizeof(LONG_PTR);
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH) CreateSolidBrush(RGB(0, 0, 0));
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON1));

    return RegisterClassExW(&wcex);
}

// Unregister WindowClass
void Application::unregisterWindowClass() const {
    UnregisterClass(szWindowClass, m_hInstance);
}

// Init instance
BOOL Application::initInstance(HINSTANCE hInstance, int nCmdShow) {
    int nWinWidth = m_ViewportDims.right + (2 * GetSystemMetrics(SM_CXSIZEFRAME)),
        nWinHeight = m_ViewportDims.bottom + (2 * GetSystemMetrics(SM_CYSIZEFRAME)) + GetSystemMetrics(SM_CYCAPTION);

    m_hWnd = CreateWindowW(szWindowClass, szTitle, WS_DLGFRAME | WS_SYSMENU,
        CW_USEDEFAULT, 0, nWinWidth, nWinHeight, nullptr, nullptr, hInstance, this);

    if (!m_hWnd) {
        return FALSE;
    }

    if (nullptr == (m_hDC = GetDC(m_hWnd))) {
        DestroyWindow(m_hWnd);
        m_hWnd = nullptr;
        return FALSE;
    }

    
    ShowWindow(m_hWnd, nCmdShow);
    UpdateWindow(m_hWnd);

    SetForegroundWindow(m_hWnd);
    SetFocus(m_hWnd);

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

    Application* pApp = (Application*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    switch (message) {

        case WM_NCCREATE: {
            auto* cs = (CREATESTRUCT*)lParam;
            pApp = (Application*)cs->lpCreateParams;
            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pApp);
            break;
        }

        case WM_ACTIVATE: {
            if (!pApp) break;
            pApp->activeRawInput(LOWORD(wParam) != WA_INACTIVE);
            break;
        }

        case WM_PAINT: {
            if (pApp && pApp->isOpenGLInitizlized()) {
                ValidateRect(hWnd, NULL);
                return 0;
            }

            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            RECT rect;
            GetClientRect(hWnd, &rect);

            HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0));
            FillRect(hdc, &rect, blackBrush);
            DeleteObject(blackBrush);

            // Красный текст
            SetTextColor(hdc, RGB(255, 0, 0));
            SetBkMode(hdc, TRANSPARENT);
            TextOutW(hdc, 10, 10, L"Initializing OpenGL...", 22);

            EndPaint(hWnd, &ps);
        }
        break;

        case WM_ERASEBKGND:
            return 1;

        case WM_SETCURSOR: {
            SetCursor(NULL);
            return TRUE;
        }

        case WM_INPUT: {
            RAWINPUT input = {};
            UINT size = sizeof(RAWINPUT);
            if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, &input, &size, sizeof(RAWINPUTHEADER)) == size) {
                if (input.header.dwType == RIM_TYPEMOUSE) {
                    LONG dx = input.data.mouse.lLastX;
                    LONG dy = input.data.mouse.lLastY;

                    if (pApp) pApp->accumMouseDelta(dx, dy);
                }
            }

            return 0;
        }

        case WM_KEYDOWN: {
            if (pApp) pApp->onKeyDown((int)wParam);
            return 0;
        }

        case WM_KEYUP: {
            if (pApp) pApp->onKeyUp((int)wParam);
            return 0;
        }

        case WM_CLOSE:
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
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

BOOL Application::initOpenGL(BYTE nColorBits, BYTE nDepthBits, BYTE nStencilBits, BOOL isSync) {
   
    WNDCLASS wc = {};
    wc.lpfnWndProc = DefWindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"DummyGLWindow";

    RegisterClass(&wc);

    HWND dummyWnd = CreateWindow(L"DummyGLWindow", L"Dummy", 0,
        0, 0, 1, 1, NULL, NULL, wc.hInstance, NULL);

    if (!dummyWnd) return FALSE;

    HDC dummyDC = GetDC(dummyWnd);
    if (!dummyDC) {
        DestroyWindow(dummyWnd);
        return FALSE;
    }

    PIXELFORMATDESCRIPTOR dummyPFD = { 
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

    int dummyFormat = ChoosePixelFormat(dummyDC, &dummyPFD);
    if (!dummyFormat || !SetPixelFormat(dummyDC, dummyFormat, &dummyPFD)) {
        ReleaseDC(dummyWnd, dummyDC);
        UnregisterClass(wc.lpszClassName, wc.hInstance);
        DestroyWindow(dummyWnd);
        return FALSE;
    }

    HGLRC dummyRC = wglCreateContext(dummyDC);
    if (!dummyRC || !wglMakeCurrent(dummyDC, dummyRC)) {
        // cleanup
        if (dummyRC) wglDeleteContext(dummyRC);
        ReleaseDC(dummyWnd, dummyDC);
        UnregisterClass(wc.lpszClassName, wc.hInstance);
        DestroyWindow(dummyWnd);
        return FALSE;
    }

    if (glewInit() != GLEW_OK) {
        // cleanup
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(dummyRC);
        ReleaseDC(dummyWnd, dummyDC);
        DestroyWindow(dummyWnd);
        return FALSE;
    }

    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(dummyRC);

    ReleaseDC(dummyWnd, dummyDC);
    UnregisterClass(wc.lpszClassName, wc.hInstance);
    DestroyWindow(dummyWnd);

    int finalPixelFormat = 0;
    PIXELFORMATDESCRIPTOR finalPFD = {};
    UINT numFormats = 0;

    if (wglewIsSupported("WGL_ARB_pixel_format") && wglewIsSupported("WGL_ARB_multisample")) {
        const int samples_try[] = { 8, 4, 0 };

        for (int msaa : samples_try) {
            int attribs[] = {
                WGL_DRAW_TO_WINDOW_ARB,     GL_TRUE,
                WGL_SUPPORT_OPENGL_ARB,     GL_TRUE,
                WGL_DOUBLE_BUFFER_ARB,      GL_TRUE,
                WGL_ACCELERATION_ARB,       WGL_FULL_ACCELERATION_ARB,  
                WGL_PIXEL_TYPE_ARB,         WGL_TYPE_RGBA_ARB,
                WGL_COLOR_BITS_ARB,         nColorBits,
                WGL_RED_BITS_ARB,           8,
                WGL_GREEN_BITS_ARB,         8,
                WGL_BLUE_BITS_ARB,          8,
                WGL_ALPHA_BITS_ARB,         nColorBits >= 32 ? 8 : 0,
                WGL_DEPTH_BITS_ARB,         nDepthBits,
                WGL_STENCIL_BITS_ARB,       nStencilBits,
                WGL_SAMPLE_BUFFERS_ARB,     msaa > 0 ? GL_TRUE : GL_FALSE,
                WGL_SAMPLES_ARB,            msaa,
                0, 0
            };

            float fattribs[] = { 0,0 };

            if (wglChoosePixelFormatARB(m_hDC, attribs, fattribs, 1, &finalPixelFormat, &numFormats) && numFormats >= 1) {
                DescribePixelFormat(m_hDC, finalPixelFormat, sizeof(finalPFD), &finalPFD);
                break;
            }
        }
    }

    if (finalPixelFormat == 0) {
        PIXELFORMATDESCRIPTOR fallback = {  };
        finalPixelFormat = ChoosePixelFormat(m_hDC, &fallback);
        if (!finalPixelFormat) return FALSE;
        DescribePixelFormat(m_hDC, finalPixelFormat, sizeof(finalPFD), &finalPFD);
    }


    if (!SetPixelFormat(m_hDC, finalPixelFormat, &finalPFD)) {
        DWORD err = GetLastError();
        return FALSE;
    }

    HGLRC finalRC = NULL;

    if (wglewIsSupported("WGL_ARB_create_context")) {
        int ctxAttrs[] = {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
            WGL_CONTEXT_MINOR_VERSION_ARB, 5,  
            WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            WGL_CONTEXT_FLAGS_ARB,         WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
            0
        };

        finalRC = wglCreateContextAttribsARB(m_hDC, 0, ctxAttrs);
    }

    if (!finalRC) {
        finalRC = wglCreateContext(m_hDC);
        if (!finalRC) return FALSE;
    }

    if (!wglMakeCurrent(m_hDC, finalRC)) {
        wglDeleteContext(finalRC);
        return FALSE;
    }

    m_hGLRC = finalRC;

    GLint samples = 0;
    glGetIntegerv(GL_SAMPLES, &samples);

    if (wglewIsSupported("WGL_EXT_swap_control")) {
        wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
        if (wglSwapIntervalEXT) wglSwapIntervalEXT(isSync);
    }

    const GLubyte* vendorStr = glGetString(GL_VENDOR);
    const GLubyte* rendererStr = glGetString(GL_RENDERER);
    const GLubyte* versionStr = glGetString(GL_VERSION);

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

void Application::centerCursor() const {
    RECT rc;
    GetClientRect(m_hWnd, &rc);

    POINT pt;
    pt.x = (rc.right - rc.left) / 2;
    pt.y = (rc.bottom - rc.top) / 2;

    ClientToScreen(m_hWnd, &pt);

    SetCursorPos(pt.x, pt.y);
}

BOOL Application::initRawInput() {

    centerCursor();

    RAWINPUTDEVICE rid = {};

    rid.usUsagePage = 0x01; // Generic Desktop
    rid.usUsage = 0x02;     // Mouse
    rid.dwFlags = RIDEV_INPUTSINK | RIDEV_NOLEGACY;

    rid.hwndTarget = m_hWnd;

    if (!RegisterRawInputDevices(&rid, 1, sizeof(rid))) {
        DWORD err = GetLastError();

#ifdef _DEBUG
        wchar_t buf[256];
        swprintf(buf, 256, L"RegisterRawInputDevices failed: %lu", err);
        MessageBox(m_hWnd, buf, L"RawInput Error", MB_ICONERROR);
#endif
        return FALSE;
    }

    for (int i = 0; i < 10; i++) {
        if (ShowCursor(FALSE) < 0) break;
    }

    // Clear any residual deltas created during initialization (prevent startup drift)
    m_nMouseDX = 0;
    m_nMouseDY = 0;

    return TRUE;
}

void Application::destroyRawInput() {
    RAWINPUTDEVICE rid = {};

    rid.usUsagePage = 0x01;
    rid.usUsage = 0x02;

    rid.dwFlags = RIDEV_REMOVE;
    rid.hwndTarget = NULL;

    RegisterRawInputDevices(&rid, 1, sizeof(rid));

    for (int i = 0; i < 10; i++) {
        if (ShowCursor(true) >= 0) break;
    }
}

void Application::accumMouseDelta(LONG dx, LONG dy) {
    m_nMouseDX += dx;
    m_nMouseDY += dy;
}

bool Application::consumeMouseDelta(float &dx, float &dy) {
    dx = static_cast<float>(m_nMouseDX);
    dy = static_cast<float>(m_nMouseDY);

    // consume the accumulated deltas so they are not reused next frame
    m_nMouseDX = 0;
    m_nMouseDY = 0;

    // Small noise threshold to ignore 1-pixel jitter from raw input
    const float NOISE_THRESH = 0.5f;
    if (fabsf(dx) < NOISE_THRESH) dx = 0.f;
    if (fabsf(dy) < NOISE_THRESH) dy = 0.f;

    return (dx != 0.f || dy != 0.f);
 }