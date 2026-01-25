#pragma once
#include "framework.h"

class Application {
public:
	Application();
	virtual ~Application();

	BOOL initApplicationBase(LPWSTR lpCmdLine, HINSTANCE hInstance, int nCmdShow);
	void runApplicationBase();
	void cleanupApplicationBase();


protected:
	virtual BOOL Init(LPWSTR lpCmdLine) = 0;
	virtual void Run(double frameTime, float fps) = 0;
	virtual void Done() = 0;

private:
	HINSTANCE m_hInstance;
	HWND m_hWnd;
	HDC m_hDC;
	HGLRC m_hGLRC;
	RECT m_ViewportDims;

	int m_frameCounter = 0;
	double m_timeAccumulator = 0.0;
	float m_currentFPS = 0.0f;

	LARGE_INTEGER m_startTime, m_timerFrequency;

	ATOM registerWindowClass(HINSTANCE hInstance);
	void unregisterWindowClass();

	BOOL initInstance(HINSTANCE hInstance, int nCmdShow);
	void destroyInstance();

	BOOL handleMessages();

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	inline void startFrameTimer() {
        QueryPerformanceFrequency(&m_timerFrequency); 
        QueryPerformanceCounter(&m_startTime);
	}

	inline double getFrameTime() {
		LARGE_INTEGER endTime;
		QueryPerformanceCounter(&endTime);

        if (endTime.QuadPart < m_startTime.QuadPart) {
            endTime.QuadPart += m_timerFrequency.QuadPart;  
        }

		return static_cast<double>(endTime.QuadPart - m_startTime.QuadPart) / m_timerFrequency.QuadPart;
	}
};

//
// Application entry point defs for Win32/x64
//
#define DEFINE_GL_APPLICATION(App) \
	int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,\
						  _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {\
	UNREFERENCED_PARAMETER(hPrevInstance);\
	UNREFERENCED_PARAMETER(lpCmdLine);\
	\
	_CrtSetDbgFlag(0);\
	\
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);\
	\
	App *pApp = pApp->getInstance();\
	\
	pApp->initApplicationBase(lpCmdLine,hInstance,nCmdShow);\
	pApp->runApplicationBase();\
	pApp->cleanupApplicationBase();\
	\
	return (int)EXIT_SUCCESS;\
};