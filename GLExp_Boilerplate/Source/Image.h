//
// OpenGL framework and demo boilerplate
// (c) 2026 by Sergei Kuratiov. MIT License
//
#pragma once

enum TEXFILTER_MODE {
	NEAREST,
	LINEAR,
	LINEAR_MIPMAP_NEAREST,
	NEAREST_MIPMAP_LINEAR,
	LINEAR_MIPMAP_LINEAR,
	LINEAR_ANISO,
	LINEAR_MIPMAP_LINEAR_ANISO
};

class Image {
public:
	Image();
	virtual ~Image();

	BOOL fromTGA(const char *, unsigned short, bool maxQuality = true);
	BOOL fromDDS(const char *, unsigned short , bool maxQuality = true);
	BOOL fromKTX(const char*, unsigned short , bool maxQuality = true);

private:
	GLenum m_GLTarget;
	GLuint m_GLTexture;

	unsigned int m_nWidth, m_nHeight, m_nDepth;
	unsigned int m_nBitsPerPx, m_nCompression;

	void genCheckboard();
};

