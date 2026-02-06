#include "framework.h"

#include <fstream>
#include <vector>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "ShaderProgram.h"
#include "VertexBufferObject.h"
#include "TextPainter.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

static const int ATLAS_W = 512;
static const int ATLAS_H = 512;

struct TextVertex {
    glm::vec2 pos;
    glm::vec2 uv;
};

const int MAX_CHARS = 256;
const int VERTS_PER_CHAR = 6;

static TextVertex verts[MAX_CHARS * VERTS_PER_CHAR];

//
// Constructor / destructor
//
TextPainter::TextPainter() {
    memset(m_Chars, 0, sizeof(m_Chars));

    m_fontAtlas = 0;

    m_uProjection = -1;
    m_textColor = -1;
}

TextPainter::~TextPainter() {
}

BOOL TextPainter::initFont() {

    FT_Library ft;

    if (FT_Init_FreeType(&ft)) return FALSE;

    std::ifstream file("../Fonts/Roboto-Regular.ttf", std::ios::binary | std::ios::ate);

    if (!file) {
        FT_Done_FreeType(ft);
        return FALSE;
    }

    size_t fileSize = file.tellg();
    file.seekg(0);

    uint8_t* buffer = new uint8_t[fileSize];

    file.read((char*)buffer, fileSize);
    file.close();

    
    FT_Face face;

    if (FT_New_Memory_Face(ft, buffer, (FT_Long)fileSize, 0, &face)) {
        FT_Done_FreeType(ft);
        return FALSE;
    }

    FT_Set_Pixel_Sizes(face, 0, 64);

    struct GlyphInfo {
        int w, h;
        int x, y;
    };

    GlyphInfo glyphs[128] = {};

    for (int c = 0; c < 128; c++) {

        if (FT_Load_Char(face, c, FT_LOAD_DEFAULT))
            continue;

        FT_GlyphSlot g = face->glyph;

        glyphs[c].w = g->metrics.width >> 6;
        glyphs[c].h = g->metrics.height >> 6;

        Character& ch = m_Chars[c];

        ch.Bearing = {
            g->bitmap_left,
            g->bitmap_top
        };

        ch.Advance = g->advance.x >> 6;
    }

    int x = 0;
    int y = 0;
    int rowH = 0;

    for (int c = 0; c < 128; c++) {

        if (x + glyphs[c].w >= ATLAS_W) {
            x = 0;
            y += rowH + 2;
            rowH = 0;
        }

        glyphs[c].x = x;
        glyphs[c].y = y;

        x += glyphs[c].w + 2;

        rowH = std::max(rowH, glyphs[c].h);
    }

    glGenTextures(1, &m_fontAtlas);
    glBindTexture(GL_TEXTURE_2D, m_fontAtlas);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, ATLAS_W, ATLAS_H, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (int c = 0; c < 128; c++) {

        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            continue;

        FT_GlyphSlot g = face->glyph;
        FT_Bitmap& bmp = g->bitmap;

        if (!bmp.buffer || bmp.width == 0 || bmp.rows == 0)
            continue;

        glTexSubImage2D(GL_TEXTURE_2D, 0, glyphs[c].x, glyphs[c].y, bmp.width, bmp.rows, 
            GL_RED, GL_UNSIGNED_BYTE, bmp.buffer);

        // UV + size
        Character& ch = m_Chars[c];

        ch.Size = { bmp.width, bmp.rows };

        ch.u0 = (float)glyphs[c].x / (float)ATLAS_W;
        ch.u1 = (float)(glyphs[c].x + bmp.width) / (float)ATLAS_W;

        ch.v0 = (float)(glyphs[c].y + bmp.rows) / (float)ATLAS_H;
        ch.v1 = (float)glyphs[c].y / (float)ATLAS_H;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);

    FT_Done_Face(face);
    FT_Done_FreeType(ft);


    if (!m_textShaderProgram.fromSrc("../Shaders/text.vert", "../Shaders/text.frag")){
        delete[] buffer; buffer = nullptr;
        return FALSE;
    }

    const int MAX_CHARS = 256;
    const int MAX_VERTS = MAX_CHARS * 6;

    TextVertex* emptyData = new TextVertex[MAX_VERTS];
    memset((void*)emptyData, 0, MAX_VERTS * sizeof(TextVertex));

    m_textVertices.createBuffers(
        emptyData,
        (GLsizei)(MAX_VERTS * sizeof(TextVertex)),
        nullptr,
        0,
        VERTEX_DATA_FORMAT::FLOAT_VX2UV2,
        true // dynamic
    );

    delete[] emptyData;
    emptyData = nullptr;

    GLuint prog = m_textShaderProgram.getProgramId();

    m_uProjection = glGetUniformLocation(prog, "uProjection");
    m_textColor = glGetUniformLocation(prog, "textColor");

    return TRUE;
}


void TextPainter::cleanup() {
    m_textVertices.deleteBuffers();

    m_textShaderProgram.cleanup();

    if (m_fontAtlas)
        glDeleteTextures(1, &m_fontAtlas);

    m_fontAtlas = 0;
}


void TextPainter::beginTextLayer() {
    m_textShaderProgram.useProgram();

    glm::mat4 proj = glm::ortho(0.0f, 1024.0f, 0.0f, 768.0f);
    glUniformMatrix4fv(m_uProjection, 1, GL_FALSE, glm::value_ptr(proj));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_fontAtlas);

    m_textVertices.bind();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


void TextPainter::endTextLayer() {
    glDisable(GL_BLEND);

    glBindTexture(GL_TEXTURE_2D, 0);

    m_textVertices.unbind();

    m_textShaderProgram.freeProgram();
}


void TextPainter::textOut(wchar_t* text, float x, float y,float scale,glm::vec3 color) {
             
    int vertCount = 0;

    float xpos = x;
    float ypos = 738 - y;

    for (int i = 0; text[i] && vertCount + VERTS_PER_CHAR <= MAX_CHARS * VERTS_PER_CHAR; i++) {
        unsigned char c = (unsigned char)text[i];
        Character& ch = m_Chars[c];

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
     
        float x0 = xpos + ch.Bearing.x * scale;
        float y0 = ypos - (ch.Size.y - ch.Bearing.y) * scale;

        float x1 = x0 + w;
        float y1 = y0 + h;

        verts[vertCount++] = { {x0, y0}, {ch.u0, ch.v0} };
        verts[vertCount++] = { {x1, y0}, {ch.u1, ch.v0} };
        verts[vertCount++] = { {x1, y1}, {ch.u1, ch.v1} };

        verts[vertCount++] = { {x0, y0}, {ch.u0, ch.v0} };
        verts[vertCount++] = { {x1, y1}, {ch.u1, ch.v1} };
        verts[vertCount++] = { {x0, y1}, {ch.u0, ch.v1} };
        
        xpos += ch.Advance * scale;
    }

    m_textVertices.update(verts, vertCount, sizeof(TextVertex));

    glUniform3fv(m_textColor, 1, glm::value_ptr(color));

    m_textVertices.draw();
}
