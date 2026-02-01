#pragma once


struct Character {
	glm::ivec2   Size;      // Size of glyph
	glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
	unsigned int Advance;   // Horizontal offset to advance to next glyph
};

struct GlyphInstance {
	unsigned int layer;
	glm::vec2 pos;
	glm::vec2 size;
	glm::vec4 uv;   // u0,v0,u1,v1
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

	GlyphInstance m_currentStringGlifs[2000];

	ShaderProgram m_textShaderProgram;
	VertexBufferObject m_textVertices;
};

