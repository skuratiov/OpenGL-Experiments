#pragma once
class ShaderProgram {
public:
	ShaderProgram();
	virtual ~ShaderProgram();

	BOOL fromSrc(const char* lpVertShaderSrc, const char* lpFragShaderSrc);
	void cleanup();

	inline void useProgram(GLuint nProgramID) { glUseProgram(m_nProgramId); }

private:
	GLuint m_nProgramId;
};

