#pragma once


class ShaderProgram {
public:
	ShaderProgram();
	virtual ~ShaderProgram();

	BOOL fromSrc(const char* lpVertShaderSrc, const char* lpFragShaderSrc);
	void cleanup();

	inline GLuint getProgramId() { return m_nProgramId; }

	inline void useProgram() { glUseProgram(m_nProgramId); }
	inline void freeProgram() { glUseProgram(0); }

private:
	GLuint m_nProgramId;
};

