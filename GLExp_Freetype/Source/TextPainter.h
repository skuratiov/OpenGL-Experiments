#pragma once


struct Character {
	glm::ivec2   Size;      // Size of glyph
	glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
	float Baseline;         // Baseline
	unsigned int Advance;			// Horizontal offset to advance to next glyph
};

class TextPainter {
public:
	TextPainter();
	~TextPainter();

	BOOL initFont();
	void cleanup();

	void textOut(wchar_t* lpszMessage, float x, float y, float scale, glm::vec3 color);

private:
	GLuint m_textureArray;

	Character m_Chars[128];

	ShaderProgram m_textShaderProgram;
	VertexBufferObject m_textVertices;
};

