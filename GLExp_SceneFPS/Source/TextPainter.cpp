#include "framework.h"

#include <fstream>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "ShaderProgram.h"
#include "VertexBufferObject.h"

#include "TextPainter.h"

#include <ostream>

//std::ofstream logFile("fontLog.txt");

#define GLYPH_SIZE	64

void *m_uboPtr = nullptr;

struct GlyphBlock
{
	glm::vec4  glyphRect[255];   // x,y,w,h
	glm::vec4  glyphUV[255];     // u0,v0,u1,v1
	glm::ivec4 glyphLayer[255];  // x = layer
};

//
// Constructor / destructor
//
TextPainter::TextPainter() {
	memset((void*)m_Chars, 0, sizeof(m_Chars));
	m_textureArray = 0;

	m_textColor = m_glyphPos = m_glyphSize = m_glyphLayer = m_glyphUV = 0;
	m_uProjection = 0;

	m_uboGlyphs = 0;
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
		
	if (FT_New_Memory_Face(ft, buffer, fileSize, 0, &face) == 0)  {
		FT_Set_Pixel_Sizes(face, 0, GLYPH_SIZE);
			
		glGenTextures(1, &m_textureArray);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureArray);
		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_R8, GLYPH_SIZE, GLYPH_SIZE, 128, 0, GL_RED, GL_UNSIGNED_BYTE, 0);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		for (unsigned char c = 0; c < 128; c++) {
			if (FT_Load_Char(face, c, FT_LOAD_RENDER)) 
				continue;

			Character& ch = m_Chars[c];

			ch.Size = glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows);
			ch.Baseline = float(face->size->metrics.ascender >> 6);
			ch.Bearing = glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top);
			ch.Advance = face->glyph->metrics.horiAdvance >> 6;
			
			if (face->glyph->bitmap.width > GLYPH_SIZE) face->glyph->bitmap.width = GLYPH_SIZE;
			if (face->glyph->bitmap.rows > GLYPH_SIZE) face->glyph->bitmap.rows = GLYPH_SIZE;

			glTexSubImage3D(
				GL_TEXTURE_2D_ARRAY,
				0, 0, 0, int(c),
				face->glyph->bitmap.width,
				face->glyph->bitmap.rows,
				1,
				GL_RED,
				GL_UNSIGNED_BYTE,
				face->glyph->bitmap.buffer
			);
		}		
	} else {
		delete[] buffer;
		buffer = nullptr;

		FT_Done_Face(face);
		FT_Done_FreeType(ft);

		return -4;
	}

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
		
	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	delete[] buffer; 
	buffer = nullptr;

	if (TRUE != m_textShaderProgram.fromSrc("../Shaders/text.vert", "../Shaders/text.frag"))
		return -5;

	GLfloat vertexData[] = {
		0.0f, 0.0f, // 0: top-left
		1.0f, 0.0f, // 1: top-right    
		0.0f, 1.0f, // 2: bottom-left  
		1.0f, 1.0f  // 3: bottom-right
	};

	m_textVertices.createBuffers(vertexData, sizeof(vertexData), nullptr, 0, VERTEX_DATA_FORMAT::FLOAT_VX2);
	
	m_uProjection = glGetUniformLocation(m_textShaderProgram.getProgramId(), "uProjection");

	m_textColor = glGetUniformLocation(m_textShaderProgram.getProgramId(), "textColor");
	
	glGenBuffers(1, &m_uboGlyphs);
	glBindBuffer(GL_UNIFORM_BUFFER, m_uboGlyphs);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(GlyphBlock), nullptr, GL_STREAM_DRAW);

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_uboGlyphs);
		
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	return TRUE;
}

void TextPainter::cleanup() {

	m_textVertices.deleteBuffers();

	m_textShaderProgram.cleanup();

	glDeleteTextures(1, &m_textureArray);
	m_textureArray = 0;
}


void TextPainter::beginTextLayer() {
	m_textShaderProgram.useProgram();  

	glBindBuffer(GL_UNIFORM_BUFFER, m_uboGlyphs);
			
	glm::mat4 projection = glm::ortho(0.0f, 1024.0f, 768.0f, 0.0f);
	glUniformMatrix4fv(m_uProjection, 1, GL_FALSE, glm::value_ptr(projection));
		
	m_textVertices.bind();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureArray);

	// Ensure text is drawn on top of the scene:
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void TextPainter::endTextLayer() {

	m_textShaderProgram.freeProgram();

	glDisable(GL_BLEND);

	// Restore depth testing / depth write so scene rendering continues normally
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

	m_textVertices.unbind();

	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void TextPainter::textOut(wchar_t* lpszMessage, float x, float y, float scale, glm::vec3 color) {
	float currScale = scale;
	float currentX = x, currentY = y;
	
	GlyphBlock gpuData;
	memset(&gpuData, 0, sizeof(GlyphBlock));

	uint32_t stringPos = 0, stringSize = 0;
	for (stringPos = 0; stringPos < 255; stringPos++) {
		wchar_t c = lpszMessage[stringPos];
		if (c == 0) break;

		Character currChar = m_Chars[int(c)];
			
		float xpos = currentX + currChar.Bearing.x * currScale;
		float ypos = currentY + (currChar.Baseline - currChar.Bearing.y) * currScale;

		float w = float(currChar.Size.x) * currScale;
		float h = float(currChar.Size.y) * currScale;
	
		gpuData.glyphRect[stringPos] = glm::vec4(xpos, ypos, w, h);
		gpuData.glyphUV[stringPos] = glm::vec4(
			0.0, 0.0,
			((float)(currChar.Size.x + currChar.Bearing.x) / ((float)GLYPH_SIZE)),
			((float)(currChar.Size.y) / ((float)GLYPH_SIZE))); // full quad UV

		gpuData.glyphLayer[stringPos] = glm::ivec4(int(c), 0, 0, 0);
		
		currentX += static_cast<float>(currChar.Advance) * currScale;
		
		stringSize++;
	}
		
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GlyphBlock), &gpuData);
	
	glUniform3fv(m_textColor, 1, glm::value_ptr(color));

	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, stringSize);
}
