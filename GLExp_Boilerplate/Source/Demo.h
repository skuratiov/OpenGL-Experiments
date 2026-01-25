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

	BOOL Init(LPWSTR lpCmdLine);
	void Run(double frameTime, float fps);
	void Done();
		
protected:
	Demo();

private:
	static Demo *m_pInstance;
};

