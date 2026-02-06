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
    float x, y;
    float u, v;
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

    if (FT_Init_FreeType(&ft))
        return FALSE;

    std::ifstream file("../Fonts/Roboto-Regular.ttf",
        std::ios::binary | std::ios::ate);

    if (!file) {
        FT_Done_FreeType(ft);
        return FALSE;
    }

    size_t size = file.tellg();
    file.seekg(0);

    std::vector<uint8_t> buffer(size);
    file.read((char*)buffer.data(), size);

    file.close();

    FT_Face face;

    if (FT_New_Memory_Face(ft, buffer.data(), (FT_Long)size, 0, &face)) {
        FT_Done_FreeType(ft);
        return FALSE;
    }

    FT_Set_Pixel_Sizes(face, 0, 64);

    struct TempGlyph {
        int w, h;
        std::vector<unsigned char> data;
    };

    TempGlyph temp[128];

    for (int c = 0; c < 128; c++) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            continue;

        auto& bmp = face->glyph->bitmap;

        temp[c].w = bmp.width;
        temp[c].h = bmp.rows;

        temp[c].data.resize(bmp.width * bmp.rows);

        memcpy(temp[c].data.data(),
            bmp.buffer,
            temp[c].data.size());

        // metrics
        Character& ch = m_Chars[c];

        ch.Size = { bmp.width, bmp.rows };

        ch.Bearing = {
            face->glyph->bitmap_left,
            face->glyph->bitmap_top
        };

        ch.Advance = face->glyph->advance.x >> 6;
    }
    
    struct Pos { int x, y; };

    Pos pos[128];

    int x = 0;
    int y = 0;
    int rowH = 0;

    for (int c = 0; c < 128; c++) {
        if (x + temp[c].w >= ATLAS_W) {
            x = 0;
            y += rowH + 2;
            rowH = 0;
        }

        pos[c].x = x;
        pos[c].y = y;

        x += temp[c].w + 2;

        rowH = std::max(rowH, temp[c].h);
    }

    glGenTextures(1, &m_fontAtlas);
    glBindTexture(GL_TEXTURE_2D, m_fontAtlas);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, ATLAS_W, ATLAS_H, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (int c = 0; c < 128; c++) {
        if (temp[c].w == 0 || temp[c].h == 0)
            continue;

        glTexSubImage2D(GL_TEXTURE_2D, 0, pos[c].x, pos[c].y, temp[c].w, temp[c].h, GL_RED,
            GL_UNSIGNED_BYTE,
            temp[c].data.data());

        // UV
        Character& ch = m_Chars[c];
        
        ch.u0 = (float)pos[c].x / (float)ATLAS_W;
       // ch.v0 = (float)pos[c].y / ATLAS_H;
        ch.v0 = (float)(pos[c].y + temp[c].h) / (float)ATLAS_H;
        

        ch.u1 = (float)(pos[c].x + temp[c].w) / (float)ATLAS_W;
      //  ch.v1 = (float)(pos[c].y + temp[c].h) / ATLAS_H;
        ch.v1 = (float)pos[c].y / (float)ATLAS_H;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);

    FT_Done_Face(face);
    FT_Done_FreeType(ft);


    if (!m_textShaderProgram.fromSrc("../Shaders/text.vert", "../Shaders/text.frag")) {
        return FALSE;
    }

    const int MAX_CHARS = 256;
    const int MAX_VERTS = MAX_CHARS * 6;
    const int VERT_SIZE = sizeof(float) * 4;

    std::vector<float> emptyData;
    emptyData.resize(MAX_VERTS * 4);

    m_textVertices.createBuffers(
        emptyData.data(),
        (GLsizei)(emptyData.size() * sizeof(float)),
        nullptr,
        0,
        VERTEX_DATA_FORMAT::FLOAT_VX2UV2,
        true // dynamic
    );
      
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

        verts[vertCount++] = { x0, y0, ch.u0, ch.v0 };
        verts[vertCount++] = { x1, y0, ch.u1, ch.v0 };
        verts[vertCount++] = { x1, y1, ch.u1, ch.v1 };

        verts[vertCount++] = { x0, y0, ch.u0, ch.v0 };
        verts[vertCount++] = { x1, y1, ch.u1, ch.v1 };
        verts[vertCount++] = { x0, y1, ch.u0, ch.v1 };
        
        xpos += ch.Advance * scale;
    }

    m_textVertices.update(verts, vertCount, sizeof(TextVertex));

    glUniform3fv(m_textColor, 1, glm::value_ptr(color));

    m_textVertices.draw();
}
