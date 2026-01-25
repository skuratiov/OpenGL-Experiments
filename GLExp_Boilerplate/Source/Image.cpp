//
// OpenGL framework and demo boilerplate
// (c) 2026 by Sergei Kuratiov. MIT License
//

#include "framework.h"
#include <fstream>
#include <algorithm>
#include "Image.h"


//
//	 DDSHeader
//
#pragma pack(push)
#pragma pack(1)
struct DDSHeader {
	unsigned int dwMagic;
	unsigned int dwSize;
	unsigned int dwFlags;
	unsigned int dwHeight;
	unsigned int dwWidth;
	unsigned int dwPitchOrLinearSize;
	unsigned int dwDepth;
	unsigned int dwMipMapCount;
	unsigned int dwReserved[11];

	struct {
		unsigned int dwSize;
		unsigned int dwFlags;
		unsigned int dwFourCC;
		unsigned int dwRGBBitCount;
		unsigned int dwRBitMask;
		unsigned int dwGBitMask;
		unsigned int dwBBitMask;
		unsigned int dwRGBAlphaBitMask;
	} ddpfPixelFormat;

	struct {
		unsigned int dwCaps1;
		unsigned int dwCaps2;
		unsigned int Reserved[2];
	} ddsCaps;

	unsigned int dwReserved2;

};
#pragma pack(pop)

#define DDPF_ALPHAPIXELS 0x00000001 
#define DDPF_FOURCC      0x00000004 
#define DDPF_RGB         0x00000040

#define DDSD_CAPS        0x00000001
#define DDSD_HEIGHT      0x00000002
#define DDSD_WIDTH       0x00000004
#define DDSD_PITCH       0x00000008
#define DDSD_PIXELFORMAT 0x00001000
#define DDSD_MIPMAPCOUNT 0x00020000
#define DDSD_LINEARSIZE  0x00080000
#define DDSD_DEPTH       0x00800000

#define DDSCAPS_COMPLEX  0x00000008 
#define DDSCAPS_TEXTURE  0x00001000 
#define DDSCAPS_MIPMAP   0x00400000 

#define DDSCAPS2_CUBEMAP 0x00000200 
#define DDSCAPS2_VOLUME  0x00200000 

#define DDSCAPS2_CUBEMAP_POSITIVEX 0x00000400
#define DDSCAPS2_CUBEMAP_NEGATIVEX 0x00000800
#define DDSCAPS2_CUBEMAP_POSITIVEY 0x00001000
#define DDSCAPS2_CUBEMAP_NEGATIVEY 0x00002000
#define DDSCAPS2_CUBEMAP_POSITIVEZ 0x00004000
#define DDSCAPS2_CUBEMAP_NEGATIVEZ 0x00008000
#define DDSCAPS2_CUBEMAP_ALL_FACES (DDSCAPS2_CUBEMAP_POSITIVEX | DDSCAPS2_CUBEMAP_NEGATIVEX | DDSCAPS2_CUBEMAP_POSITIVEY | DDSCAPS2_CUBEMAP_NEGATIVEY | DDSCAPS2_CUBEMAP_POSITIVEZ | DDSCAPS2_CUBEMAP_NEGATIVEZ)

#define FOURCC(a,b,c,d)((d<<24)|(c<<16)|(b<<8)|(a))

//
// KTX defs
// taken from etcpack.cxx by Ericsson AB 2005.
//
#define KTX_IDENTIFIER_REF  { 0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A }

#define KTX_ENDIAN_REF      		0x04030201
#define KTX_ENDIAN_REF_REV  		0x01020304

#define KTX_GL_UNPACK_ALIGNMENT 	0x04

#ifndef GL_ETC1_RGB8_OES
enum {
	GL_ETC1_RGB8_OES = 0x8d64
};
#endif

const unsigned char KTXFileIdentifier[] = KTX_IDENTIFIER_REF;

//
//	KTX file header structure
//	Taken from ktx.h by Georg Kolling, Imagination Technology and Mark Callow, HI Corporation, original (c) 2010 The Khronos Group, Inc.
//  Modified by myself
//
typedef struct TKTXTextureInfo {
	unsigned char Identifier[12]; 		// File identifier
	unsigned int nEndianness;			// Endianness
	unsigned int glType; 				// The type of the image data. Values are the same as in the @p type parameter of glTexImage*D. Must be 0 for compressed images.
	unsigned int glTypeSize;			// The data type size to be used in case of endianness conversion.
	unsigned int glFormat;				//  The format of the image(s). Values are the same as in the format parameter of glTexImage*D. Must be 0 for compressed images.
	unsigned int glInternalFormat;		// The internal format of the image(s). Values are the same as for the internalformat parameter of glTexImage*2D.
	unsigned int glBaseInternalFormat;	// The base internalformat of the image(s).
	unsigned int pixelWidth; 			// Width of the image for texture level 0, in pixels.
	unsigned int pixelHeight; 			// Height of the texture image for level 0, in pixels. Must be 0 for 1D textures.
	unsigned int pixelDepth;			// Depth of the texture image for level 0, in pixels. Must be 0 for 1D, 2D and cube textures.
	unsigned int numberOfArrayElements; // The number of array elements. Must be 0 if not an array texture.
	unsigned int numberOfFaces; 		// The number of cubemap faces. Cubemap faces must be provided in the order: +X, -X, +Y, -Y, +Z, -Z.
	unsigned int numberOfMipmapLevels;	// The number of mipmap levels.
	unsigned int bytesOfKeyValueData;	// Metadata size
} TKTXTextureInfo;

typedef struct TKTXImageInfo {
	unsigned int size;			// Size of the image data in bytes.
	unsigned char* data;  		// Pointer to the image data.
} TKTXImageInfo;


//
//	Constructor / destructor
//
Image::Image() {
	m_GLTexture = m_GLTarget = 0;
	m_nWidth = m_nHeight = m_nDepth = m_nDepth = 0;
	m_nCompression = m_nChannels = 0;
}

Image::~Image() {
}

void Image::genCheckboard() {
}

BOOL Image::fromDDS(const char* lpFileName, uint16_t filterMode, bool maxQuality) {

	std::ifstream file(lpFileName, std::ios::binary | std::ios::ate);

	if (!file) return -1; 
		
	const size_t fileSize = file.tellg();
	file.seekg(0);

	if (fileSize < sizeof(DDSHeader)) { return -2; }

	uint8_t *buffer = new uint8_t[fileSize];

	file.read(reinterpret_cast<char*>(buffer), fileSize);
	if (!file) return -3; 

	file.close();

	const DDSHeader* fileHeader = (DDSHeader*)buffer;

	if (fileHeader->dwMagic != FOURCC('D', 'D', 'S', ' ')) {
		delete[] buffer;
		buffer = nullptr;
		return -5;
	}

	if (fileHeader->dwSize != 124 ||
		fileHeader->ddpfPixelFormat.dwSize != 32) {
		delete[] buffer;
		buffer = nullptr;
		return -6;
	}

	m_nWidth = fileHeader->dwWidth;
	m_nHeight = fileHeader->dwHeight;
	m_nDepth = fileHeader->dwDepth;
	m_nChannels = fileHeader->ddpfPixelFormat.dwRGBBitCount;

	uint32_t mipCount = fileHeader->dwMipMapCount;
	if (mipCount == 0) mipCount = 1;

	if ((fileHeader->ddsCaps.dwCaps2 & DDSCAPS2_CUBEMAP) &&
		(fileHeader->ddsCaps.dwCaps2 & DDSCAPS2_CUBEMAP_ALL_FACES)) {
		m_GLTarget = GL_TEXTURE_CUBE_MAP;
	}
	else if ((fileHeader->dwFlags & DDSD_DEPTH) &&
		(fileHeader->ddsCaps.dwCaps2 & DDSCAPS2_VOLUME)) {
		m_GLTarget = GL_TEXTURE_3D;
		m_nDepth = fileHeader->dwDepth ? fileHeader->dwDepth : 1;
	}
	else if ((m_nHeight == 1) || (m_nWidth == 1)) {
		m_GLTarget = GL_TEXTURE_1D;
	}
	else {
		m_GLTarget = GL_TEXTURE_2D;
	}


	m_nCompression = 0; 
	switch (fileHeader->ddpfPixelFormat.dwFourCC) {
	case FOURCC('D', 'X', 'T', '1'):
		m_nCompression = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
		break;

	case FOURCC('D', 'X', 'T', '3'):
		m_nCompression = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		break;

	case FOURCC('D', 'X', 'T', '5'):
		m_nCompression = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		break;
	}

	uint8_t *pixelData = buffer + sizeof(DDSHeader);

	glGenTextures(1, &m_GLTexture);
	glBindTexture(m_GLTarget, m_GLTexture);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	GLenum pixelFormat = 0;
	GLenum internalFmt = 0;

	if (!m_nCompression) {
		switch (m_nChannels) {
		case 8:
			pixelFormat = GL_ALPHA;
			internalFmt = maxQuality ? GL_ALPHA8 : GL_ALPHA;
			break;

		case 16:
			pixelFormat = GL_LUMINANCE_ALPHA;
			internalFmt = GL_LUMINANCE_ALPHA;
			break;

		case 24:
			pixelFormat = GL_BGR;
			internalFmt = maxQuality ? GL_RGB8 : GL_RGB;
			break;

		case 32:
			pixelFormat = GL_RGBA;
			internalFmt = maxQuality ? GL_RGBA8 : GL_RGBA;
			break;

		default:
			pixelFormat = GL_LUMINANCE;
			internalFmt = GL_LUMINANCE8;
		}
	}

	bool useMips = false;

	switch (filterMode) {
	case TEXFILTER_MODE::LINEAR:
		glTexParameteri(m_GLTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(m_GLTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		break;

	case TEXFILTER_MODE::LINEAR_MIPMAP_NEAREST:
		if (mipCount > 1) {
			glTexParameteri(m_GLTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(m_GLTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		} else {
			glTexParameteri(m_GLTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(m_GLTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
		break;

	case TEXFILTER_MODE::LINEAR_MIPMAP_LINEAR:
		if (mipCount > 1) {
			glTexParameteri(m_GLTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(m_GLTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		} else {
			glTexParameteri(m_GLTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(m_GLTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		break;

	case TEXFILTER_MODE::LINEAR_ANISO:
		GLfloat texureMaxAniso;
		glTexParameteri(m_GLTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(m_GLTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &texureMaxAniso);

		if (texureMaxAniso > 0.0f)
			glTexParameterf(m_GLTarget,	GL_TEXTURE_MAX_ANISOTROPY_EXT, texureMaxAniso);
		break;

	default:
		glTexParameteri(m_GLTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(m_GLTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	if (mipCount == 1 &&
		(filterMode == TEXFILTER_MODE::LINEAR_MIPMAP_LINEAR ||
		filterMode == TEXFILTER_MODE::LINEAR_MIPMAP_NEAREST)) {
		useMips = true;
	}

	GLsizei w = m_nWidth;
	GLsizei h = m_nHeight;

	uint8_t* ptr = pixelData;
	
	uint32_t size = 0;
	uint32_t mip = 0;

	uint8_t* bufferEnd = buffer + fileSize;

	if (m_GLTarget == GL_TEXTURE_1D) {	
		for (mip = 0; mip < mipCount; mip++) {
			if (m_nCompression) {
				if (m_nHeight == 1) {
					uint32_t block = (m_nCompression == GL_COMPRESSED_RGB_S3TC_DXT1_EXT) ? 8 : 16;
					size = ((w + 3) / 4) * block;
					if (ptr + size > bufferEnd) break;
					glCompressedTexImage1D(GL_TEXTURE_1D, mip, m_nCompression, w, 0, size, ptr);
				} else {
					uint32_t block = (m_nCompression == GL_COMPRESSED_RGB_S3TC_DXT1_EXT) ? 8 : 16;
					size = ((h + 3) / 4) * block;
					if (ptr + size > bufferEnd) break;
					glCompressedTexImage1D(GL_TEXTURE_1D, mip, m_nCompression, h, 0, size, ptr);
				}
			}
			else {
				if (m_nHeight == 1) {
					size = w * (m_nChannels >> 3);
					if (ptr + size > bufferEnd) break;
					glTexImage1D(GL_TEXTURE_1D, mip, internalFmt, w, 0, pixelFormat, GL_UNSIGNED_BYTE, ptr);
				} else {
					size = h * (m_nChannels >> 3);
					if (ptr + size > bufferEnd) break;
					glTexImage1D(GL_TEXTURE_1D, mip, internalFmt, h, 0, pixelFormat, GL_UNSIGNED_BYTE, ptr);
				}
			}

			w = std::max<GLsizei>(1u, w / 2);
			h = std::max<GLsizei>(1u, h / 2);

			ptr += size;
		}

	}
	else if (m_GLTarget == GL_TEXTURE_3D) {
		GLsizei d = m_nDepth;

		for (mip = 0; mip < mipCount; mip++) {

			if (m_nCompression) {
				uint32_t block = (m_nCompression == GL_COMPRESSED_RGB_S3TC_DXT1_EXT) ? 8 : 16;
				uint32_t bw = (w + 3) / 4;
				uint32_t bh = (h + 3) / 4;
				uint32_t sliceSize = bw * bh * block;
				size = sliceSize * d;
				if (ptr + size > bufferEnd) break;
				glCompressedTexImage3D(GL_TEXTURE_3D, mip, m_nCompression, w, h, d, 0, size, ptr);
			}
			else {
				size = w * h * d * (m_nChannels >> 3);
				if (ptr + size > bufferEnd) break;
				glTexImage3D(GL_TEXTURE_3D, mip, internalFmt, w, h, d, 0, pixelFormat, GL_UNSIGNED_BYTE, ptr);
			}

			ptr += size;

			w = std::max<GLsizei>(1u, w / 2);
			h = std::max<GLsizei>(1u, h / 2);
			d = std::max<GLsizei>(1u, d / 2);
		}
	}
	else if (m_GLTarget == GL_TEXTURE_CUBE_MAP) {
		for (uint32_t face = 0; face < 6; face++) {
			w = m_nWidth;
			h = m_nHeight;

			for (mip = 0; mip < mipCount; mip++) {
				if (m_nCompression) {
					uint32_t block = (m_nCompression == GL_COMPRESSED_RGB_S3TC_DXT1_EXT) ? 8 : 16;
					size = ((w + 3) / 4) * ((h + 3) / 4) * block;
					if (ptr + size > bufferEnd) break;
					glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, mip, m_nCompression, w, h, 0, size, ptr);
				} else {
					size = w * h * (m_nChannels >> 3);
					if (ptr + size > bufferEnd) break;
					glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, mip, internalFmt, w, h, 0, pixelFormat, GL_UNSIGNED_BYTE, ptr);
				}

				w = std::max<GLsizei>(1u, w / 2);
				h = std::max<GLsizei>(1u, h / 2);

				ptr += size;
			}

		}
	}
	else { // GL_TEXTURE_2D
		for (mip = 0; mip < mipCount; mip++) {
			if (m_nCompression) {
				uint32_t block = (m_nCompression == GL_COMPRESSED_RGB_S3TC_DXT1_EXT) ? 8 : 16;
				size = ((w + 3) / 4) * ((h + 3) / 4) * block;
				if (ptr + size > bufferEnd) break;
				glCompressedTexImage2D(GL_TEXTURE_2D, mip, m_nCompression, w, h, 0, size, ptr);
			}
			else {
				size = w * h * (m_nChannels >> 3);
				if (ptr + size > bufferEnd) break;
				glTexImage2D(GL_TEXTURE_2D, mip, internalFmt, w, h, 0, pixelFormat, GL_UNSIGNED_BYTE, ptr);
			}

			ptr += size;

			w = std::max<GLsizei>(1u, w / 2);
			h = std::max<GLsizei>(1u, h / 2);
		}
	}

	if (useMips) {
		glGenerateMipmap(m_GLTarget);
	}

	delete[] buffer; 
	buffer = nullptr;
	
	return TRUE;
}

BOOL Image::fromKTX(const char* lpFileName, uint16_t filterMode, bool maxQuality) {

	std::ifstream file(lpFileName, std::ios::binary | std::ios::ate);
	if (!file) return -1;

	const size_t fileSize = file.tellg();
	file.seekg(0);

	if (fileSize < sizeof(TKTXTextureInfo)) return -2;

	uint8_t* buffer = new uint8_t[fileSize];
	file.read(reinterpret_cast<char*>(buffer), fileSize);
	if (!file) {
		delete[] buffer;
		return -3;
	}
	file.close();

	TKTXTextureInfo* hdr = reinterpret_cast<TKTXTextureInfo*>(buffer);

	const unsigned char ktxID[12] = {
		0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB,
		0x0D, 0x0A, 0x1A, 0x0A
	};
	for (int i = 0; i < 12; i++)
		if (hdr->Identifier[i] != ktxID[i]) {
			delete[] buffer;
			return -4;
		}

	if (hdr->nEndianness != 0x04030201) {
		delete[] buffer;
		return -5;
	}

	m_nWidth = hdr->pixelWidth;
	m_nHeight = hdr->pixelHeight;
	m_nDepth = hdr->pixelDepth;

	uint32_t mipCount = hdr->numberOfMipmapLevels;
	if (mipCount == 0) mipCount = 1;

	if (hdr->numberOfFaces == 6)
		m_GLTarget = GL_TEXTURE_CUBE_MAP;
	else if (m_nDepth > 0 && hdr->numberOfFaces == 1 && hdr->numberOfArrayElements == 0)
		m_GLTarget = GL_TEXTURE_3D;
	else if (m_nHeight > 0 && hdr->numberOfFaces == 1)
		m_GLTarget = GL_TEXTURE_2D;
	else if (m_nHeight == 0 && m_nDepth == 0)
		m_GLTarget = GL_TEXTURE_1D;
	else {
		delete[] buffer;
		return -6; 
	}

	m_nCompression = (hdr->glType == 0) ? hdr->glInternalFormat : 0;
	m_nChannels = (hdr->glFormat == 0) ? 0 : 8; 


	uint8_t* ptr = buffer + sizeof(TKTXTextureInfo) + hdr->bytesOfKeyValueData;
	uint8_t* bufferEnd = buffer + fileSize;

	glGenTextures(1, &m_GLTexture);
	glBindTexture(m_GLTarget, m_GLTexture);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	bool useMips = (mipCount == 1 &&
		(filterMode == TEXFILTER_MODE::LINEAR_MIPMAP_LINEAR ||
			filterMode == TEXFILTER_MODE::LINEAR_MIPMAP_NEAREST));

	switch (filterMode) {
	case TEXFILTER_MODE::LINEAR:
		glTexParameteri(m_GLTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(m_GLTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		break;
	case TEXFILTER_MODE::LINEAR_MIPMAP_NEAREST:
		glTexParameteri(m_GLTarget, GL_TEXTURE_MIN_FILTER, (mipCount > 1) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
		glTexParameteri(m_GLTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		break;
	case TEXFILTER_MODE::LINEAR_MIPMAP_LINEAR:
		glTexParameteri(m_GLTarget, GL_TEXTURE_MIN_FILTER, (mipCount > 1) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
		glTexParameteri(m_GLTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		break;
	default:
		glTexParameteri(m_GLTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(m_GLTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	for (uint32_t mip = 0; mip < mipCount; mip++) {
		if (ptr >= bufferEnd) break;

		uint32_t imageSize = *reinterpret_cast<uint32_t*>(ptr);
		ptr += 4; 

		if (ptr + imageSize > bufferEnd) break;

		if (m_GLTarget == GL_TEXTURE_1D) {
			glCompressedTexImage1D(GL_TEXTURE_1D, mip, m_nCompression, m_nWidth, 0, imageSize, ptr);
		}
		else if (m_GLTarget == GL_TEXTURE_2D) {
			glCompressedTexImage2D(GL_TEXTURE_2D, mip, m_nCompression, m_nWidth, m_nHeight, 0, imageSize, ptr);
		}
		else if (m_GLTarget == GL_TEXTURE_3D) {
			glCompressedTexImage3D(GL_TEXTURE_3D, mip, m_nCompression, m_nWidth, m_nHeight, m_nDepth, 0, imageSize, ptr);
		}
		else if (m_GLTarget == GL_TEXTURE_CUBE_MAP) {
			for (int face = 0; face < 6; face++) {
				glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, mip, m_nCompression, m_nWidth, m_nHeight, 0, imageSize / 6, ptr + face * (imageSize / 6));
			}
		}

		ptr += imageSize;
		m_nWidth = std::max<GLsizei>(1u, m_nWidth / 2);
		m_nHeight = std::max<GLsizei>(1u, m_nHeight / 2);
		m_nDepth = std::max<GLsizei>(1u, m_nDepth / 2);
	}

	if (useMips) glGenerateMipmap(m_GLTarget);

	delete[] buffer;
	return TRUE;
}


void Image::fromTGA(const char* lpFileName) {
}