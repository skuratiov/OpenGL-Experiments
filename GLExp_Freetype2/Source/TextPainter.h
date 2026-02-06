#pragma once


struct Character {
	glm::ivec2 Size;
	glm::ivec2 Bearing;
	int Advance;
	float u0, v0;
	float u1, v1;
};


class TextPainter {
public:
	TextPainter();
	~TextPainter();

	BOOL initFont();
	void cleanup();

	void beginTextLayer();
	void textOut(wchar_t* lpszMessage, float x, float y, float scale, glm::vec3 color);
	void endTextLayer();

private:
	GLuint m_fontAtlas;
	
	Character m_Chars[128];

	GLuint m_textColor, m_uProjection;

	ShaderProgram m_textShaderProgram;
	VertexBufferObject m_textVertices;
};

