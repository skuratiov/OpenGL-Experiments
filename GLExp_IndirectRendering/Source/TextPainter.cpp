#include "framework.h"

#include <fstream>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "ShaderProgram.h"
#include "VertexBufferObject.h"

#include "TextPainter.h"

#include <ostream>

//std::ofstream logFile("fontLog.txt");

//
// Constructor / destructor
//
TextPainter::TextPainter() {
	memset((void*)m_Chars, 0, sizeof(m_Chars));
	m_textureArray = 0;

	m_textColor = m_glyphPos = m_glyphSize = m_glyphLayer = m_glyphUV = 0;
	m_uProjection = 0;
}

TextPainter::~TextPainter() {
}

BOOL TextPainter::initFont() {
  
    FT_Library ft;
    
    if (FT_Init_FreeType(&ft)) {
        return -1;
    }

	std::ifstream file(L"../Fonts/Roboto-Regular.ttf", std::ios::binary | std::ios::ate);

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
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_R8, 128, 128, 128, 0, GL_RED, GL_UNSIGNED_BYTE, 0);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	if (FT_New_Memory_Face(ft, buffer, fileSize, 0, &face) == 0)  {
		FT_Set_Pixel_Sizes(face, 0, 128);

		for (unsigned char c = 0; c < 128; c++) {
			if (FT_Load_Char(face, c, FT_LOAD_RENDER)) 
				continue;

			Character& ch = m_Chars[c];

			ch.Size = glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows);
			ch.Baseline = float(face->size->metrics.ascender >> 6);
			ch.Bearing = glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top);
			ch.Advance = face->glyph->metrics.horiAdvance >> 6;
			
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
		0.0f,0.0f, // top-left
		0.0f,1.0f, // bottom-left
		1.0f,0.0f, // top-right
		1.0f,1.0f  // bottom-right
	};

	m_textVertices.createBuffers(vertexData, sizeof(vertexData), nullptr, 0, VERTEX_DATA_FORMAT::FLOAT_VX2);
	
	m_uProjection = glGetUniformLocation(m_textShaderProgram.getProgramId(), "uProjection");

	m_textColor = glGetUniformLocation(m_textShaderProgram.getProgramId(), "textColor");
	m_glyphPos = glGetUniformLocation(m_textShaderProgram.getProgramId(), "glyphPos");
	m_glyphSize = glGetUniformLocation(m_textShaderProgram.getProgramId(), "glyphSize");
	m_glyphLayer = glGetUniformLocation(m_textShaderProgram.getProgramId(), "glyphLayer");
	m_glyphUV = glGetUniformLocation(m_textShaderProgram.getProgramId(), "glyphUV");


	return TRUE;
}

void TextPainter::cleanup() {

	m_textVertices.deleteBuffers();

	m_textShaderProgram.cleanup();

	glDeleteTextures(1, &m_textureArray);
	m_textureArray = 0;
}

void TextPainter::beginTextLayer() {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureArray);

	m_textVertices.bind();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_textShaderProgram.useProgram();

	glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(1024), static_cast<float>(768), 0.0f);
	glUniformMatrix4fv(m_uProjection, 1, GL_FALSE, glm::value_ptr(projection));
}

void TextPainter::endTextLayer() {

	m_textShaderProgram.freeProgram();

	glDisable(GL_BLEND);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

	m_textVertices.unbind();
}

void TextPainter::textOut(wchar_t* lpszMessage, float x, float y, float scale, glm::vec3 color) {
	float currScale = scale;
	float currentX = x, currentY = y;

	static glm::vec2 positions[255];
	static glm::vec2 sizes[255];
	static int layers[255];
	static glm::vec4 uvs[255];
	
	uint32_t stringPos = 0, stringSize = 0;
	for (stringPos = 0; stringPos < 255; stringPos++) {
		wchar_t c = lpszMessage[stringPos];
		if (c == 0) break;

		Character currChar = m_Chars[int(c)];
			
		float xpos = currentX + currChar.Bearing.x * currScale;
		float ypos = currentY + (currChar.Baseline - currChar.Bearing.y) * currScale;

		float w = float(currChar.Size.x) * currScale;
		float h = float(currChar.Size.y) * currScale;
	
		positions[stringPos] = glm::vec2(xpos, ypos);
		sizes[stringPos] = glm::vec2(w, h);
		layers[stringPos] = int(c);

		uvs[stringPos] = glm::vec4(
			0.0, 0.0,
			((float)(currChar.Size.x + currChar.Bearing.x) / 128.0f), 
			((float)(currChar.Size.y) / 128.0f)); // full quad UV

		currentX += static_cast<float>(currChar.Advance) * currScale;
		
		stringSize++;
	}

	const int MAX_GLYPHS = 255;

	glUniform3f(m_textColor, color.x, color.y, color.z);
	glUniform2fv(m_glyphPos, MAX_GLYPHS, &positions[0][0]);
	glUniform2fv(m_glyphSize, MAX_GLYPHS, &sizes[0][0]);
	glUniform1iv(m_glyphLayer, MAX_GLYPHS, layers);
	glUniform4fv(m_glyphUV, MAX_GLYPHS, &uvs[0][0]);
	
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, stringSize);

}
