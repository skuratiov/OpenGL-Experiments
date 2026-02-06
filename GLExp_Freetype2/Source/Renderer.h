//
// OpenGL framework and demo boilerplate
// (c) 2026 by Sergei Kuratiov. MIT License
//

#pragma once

#include "Libs/glm/glm.hpp"
#include "Libs/glm/gtc/matrix_transform.hpp"
#include "Libs/glm/gtc/type_ptr.hpp"

class Renderer {
public:
	static Renderer* getInstance() {
		return (!m_pInstance) ?
			m_pInstance = new Renderer() : m_pInstance;
	}
	Renderer(const Renderer&) = delete;
	virtual ~Renderer();

	void setupView(long, long);
	void initScene();
	void drawFrame(double, float);
	void cleanup();

private:
	static Renderer* m_pInstance;

	glm::mat4 m_modelMatrix, m_viewMatrix, m_projectionMatrix;

	float m_rotationAngle;

protected:
	Renderer();
};

