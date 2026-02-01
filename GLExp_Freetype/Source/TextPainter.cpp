#include "framework.h"

#include <fstream>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "ShaderProgram.h"
#include "VertexBufferObject.h"

#include "TextPainter.h"


//
// Constructor / destructor
//
TextPainter::TextPainter() {
	memset((void*)m_Chars, 0, sizeof(m_Chars));
	memset((void*)m_currentStringGlifs, 0, sizeof(m_currentStringGlifs));

	m_textureArray = 0;
}

TextPainter::~TextPainter() {
}

BOOL TextPainter::initFont() {
  
    FT_Library ft;
    
    if (FT_Init_FreeType(&ft)) {
        return -1;
    }

	std::ifstream file(L"../Fonts/SourceCodePro-Regular.ttf", std::ios::binary | std::ios::ate);

	if (!file) { FT_Done_FreeType(ft); return -2; }

	const FT_Long fileSize = (FT_Long)file.tellg();
	file.seekg(0);

	uint8_t* buffer = new uint8_t[fileSize];

	file.read(reinterpret_cast<char*>(buffer), fileSize);
	if (!file) { delete[] buffer; FT_Done_FreeType(ft); return -3; }

	file.close();

	// Lets init
	FT_Face face;

	glGenTextures(1, &m_textureArray);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureArray);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_R8, 64, 64, 128, 0, GL_RED, GL_UNSIGNED_BYTE, 0);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	if (FT_New_Memory_Face(ft, buffer, fileSize, 0, &face) == 0)  {
		FT_Set_Pixel_Sizes(face, 0, 64);


		for (unsigned char c = 0; c < 128; c++) {
			if (FT_Load_Char(face, c, FT_LOAD_RENDER)) 
				continue;

			Character& ch = m_Chars[c];

			ch.Size = glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows);
			ch.Bearing = glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top);
			ch.Advance = static_cast<unsigned int>(face->glyph->advance.x);

			glTexSubImage3D(
				GL_TEXTURE_2D_ARRAY,
				0, 0, 0, int(c),
				face->glyph->bitmap.width,
				face->glyph->bitmap.rows, 1,
				GL_RED,
				GL_UNSIGNED_BYTE,
				face->glyph->bitmap.buffer
			);

			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
	}

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
		
	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	delete[] buffer; 
	buffer = nullptr;

	if (TRUE != m_textShaderProgram.fromSrc("../Shaders/text.vert", "../Shaders/text.frag"))
		return -4;

	GLfloat vertexData[] = {
	   0.0f,1.0f,
	   0.0f,0.0f,
	   1.0f,1.0f,
	   1.0f,0.0f,
	};

	m_textVertices.createBuffers(vertexData, sizeof(vertexData), nullptr, 0, VERTEX_DATA_FORMAT::FLOAT_VX2);
	
	return TRUE;
}

void TextPainter::cleanup() {

	m_textVertices.deleteBuffers();

	m_textShaderProgram.cleanup();

	glDeleteTextures(1, &m_textureArray);
	m_textureArray = 0;
}


void TextPainter::textOut(wchar_t* lpszMessage, float x, float y, float scale, glm::vec3 color) {
	scale = scale * 48.0f / 256.0f;
	float copyX = x;
	m_textShaderProgram.useProgram();

	glUniform3f(glGetUniformLocation(m_textShaderProgram.getProgramId(), "textColor"), color.x, color.y, color.z);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureArray);

	m_textVertices.bind();



	m_textVertices.unbind();
}

/*
https://github.com/johnWRS/LearnOpenGLTextRenderingImprovement/blob/main/text_rendering.cpp
*/