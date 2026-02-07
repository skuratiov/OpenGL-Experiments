//
// OpenGL framework and demo boilerplate
// (c) 2026 by Sergei Kuratiov. MIT License
//

#pragma once
#include "Application.h"

class Demo :
    public Application {


public:
	static Demo* getInstance() {
		return (!m_pInstance) ?
			m_pInstance = new Demo() : m_pInstance;
	}

	Demo(const Demo&) = delete;
	virtual ~Demo();

	BOOL Init(LPWSTR );
	void Run(double, float);
	void Done();
		
	void onMouseDelta(LONG, LONG);
	void onKeyDown(int);
	void onKeyUp(int);

protected:
	Demo();

private:
	static Demo *m_pInstance;
};

