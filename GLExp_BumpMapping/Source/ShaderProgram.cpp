#include "framework.h"
#include <fstream>
#include "ShaderProgram.h"

//
// Constructor / destructor
//
ShaderProgram::ShaderProgram() {
	m_nProgramId = 0;
}

ShaderProgram::~ShaderProgram() {
	cleanup();
}

// return TRUE on success, negative error code on failure
BOOL ShaderProgram::fromSrc(const char* lpVertShaderSrc, const char* lpFragShaderSrc) {

	GLuint nVertShaderId = 0, nFragShaderId = 0;
	
	std::ifstream vertShaderFile(lpVertShaderSrc, std::ios::binary | std::ios::ate);
	if (!vertShaderFile) return -1;
	const size_t vertSrcSize = vertShaderFile.tellg();
	if (vertSrcSize == 0) { vertShaderFile.close(); return -2; }
	vertShaderFile.seekg(0);
	uint8_t* vertShaderBuffer = new uint8_t[vertSrcSize];
	vertShaderFile.read(reinterpret_cast<char*>(vertShaderBuffer), vertSrcSize);
	if (!vertShaderFile) { delete[] vertShaderBuffer; vertShaderFile.close(); return -3; }
	vertShaderFile.close();

	std::ifstream fragShaderFile(lpFragShaderSrc, std::ios::binary | std::ios::ate);
	if (!fragShaderFile) { delete[] vertShaderBuffer;  return -3; }
	const size_t fragSrcSize = fragShaderFile.tellg();
	if (fragSrcSize == 0) { fragShaderFile.close(); return -4; }
	fragShaderFile.seekg(0);
	uint8_t* fragShaderBuffer = new uint8_t[fragSrcSize];
	fragShaderFile.read(reinterpret_cast<char*>(fragShaderBuffer), fragSrcSize);
	if (!fragShaderFile) { delete[] vertShaderBuffer; delete[] fragShaderBuffer; fragShaderFile.close(); return -5; }
	fragShaderFile.close();

	GLint nParam = 0;

	// Vertex shader
	nVertShaderId = glCreateShader(GL_VERTEX_SHADER);

	glShaderSource(nVertShaderId, 1, (const GLchar**)&vertShaderBuffer, (GLint*)&vertSrcSize);
	glCompileShader(nVertShaderId);

	glGetShaderiv(nVertShaderId, GL_COMPILE_STATUS, &nParam);

	if (GL_TRUE != nParam) {
		delete[] vertShaderBuffer;
		delete[] fragShaderBuffer;
		return -6;
	}

	// Fragment shader
	nFragShaderId = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(nFragShaderId, 1, (const GLchar**)&fragShaderBuffer, (GLint*)&fragSrcSize);
	glCompileShader(nFragShaderId);

	glGetShaderiv(nFragShaderId, GL_COMPILE_STATUS, &nParam);

	if (GL_TRUE != nParam) {
		delete[] vertShaderBuffer;
		delete[] fragShaderBuffer;
		return -7;
	}

	m_nProgramId = glCreateProgram();
	glAttachShader(m_nProgramId, nVertShaderId);
	glAttachShader(m_nProgramId, nFragShaderId);
	glLinkProgram(m_nProgramId);

	GLint success;
	glGetProgramiv(m_nProgramId, GL_LINK_STATUS, &success);
	if (!success) {
		GLchar infoLog[1024];
		glGetProgramInfoLog(m_nProgramId, sizeof(infoLog), nullptr, infoLog);
		glDeleteProgram(m_nProgramId);
		m_nProgramId = 0;

		return -8;
	}

	glDeleteShader(nVertShaderId);
	glDeleteShader(nFragShaderId);
		
	delete[] vertShaderBuffer; 
	delete[] fragShaderBuffer;
	
	return TRUE;
}

void ShaderProgram::cleanup() {
	if (m_nProgramId) {
		glDeleteProgram(m_nProgramId);
		m_nProgramId = 0;
	}
}
