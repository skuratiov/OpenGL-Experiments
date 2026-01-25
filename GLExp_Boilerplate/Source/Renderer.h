//
// OpenGL framework and demo boilerplate
// (c) 2026 by Sergei Kuratiov. MIT License
//

#pragma once

class Renderer {
public:
	static Renderer* getInstance() {
		return (!m_pInstance) ?
			m_pInstance = new Renderer() : m_pInstance;
	}
	Renderer(const Renderer&) = delete;
	virtual ~Renderer();

	void drawFrame(double );

private:
	static Renderer* m_pInstance;

protected:
	Renderer();
};

