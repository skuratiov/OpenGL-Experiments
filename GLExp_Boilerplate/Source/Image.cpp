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

#pragma pack(push)
#pragma pack(1)
struct DDS_HEADER_DXT10 {
	uint32_t dxgiFormat;
	uint32_t resourceDimension;
	uint32_t miscFlag;
	uint32_t arraySize;
	uint32_t miscFlags2;
};
#pragma pack(pop)

enum DXGI_FORMAT {
	DXGI_FORMAT_BC7_UNORM = 98,
	DXGI_FORMAT_BC7_UNORM_SRGB = 99,
};

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
//
typedef struct TKTXTextureInfo {
	unsigned char Identifier[12]; 		// File identifier
	unsigned int nEndianness;			// Endianness
	unsigned int glType; 				// The type of the image data. Values are the same as in the @p type parameter of glTexImage*D. Must be 0 for compressed images.
	unsigned int glTypeSize;			// The data type size to be used in case of endianness conversion.
	unsigned int glFormat;				// The format of the image(s). Values are the same as in the format parameter of glTexImage*D. Must be 0 for compressed images.
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
// TGA 
//
#pragma pack(push, 1)
struct TGAHeader {
	uint8_t  idLength;
	uint8_t  colorMapType;
	uint8_t  imageType;
	uint16_t colorMapFirstEntryIndex;
	uint16_t colorMapLength;
	uint8_t  colorMapEntrySize;
	uint16_t xOrigin;
	uint16_t yOrigin;
	uint16_t width;
	uint16_t height;
	uint8_t  pixelDepth;
	uint8_t  imageDescriptor;
};
#pragma pack(pop)


//
//	Constructor / destructor
//
Image::Image() {
	m_GLTexture = m_GLTarget = 0;
	m_nWidth = m_nHeight = m_nDepth = m_nDepth = 0;
	m_nCompression = m_nBitsPerPx = 0;
}

Image::~Image() {
}

BOOL Image::fromDDS(const char* lpFileName, unsigned short filterMode) {

	std::ifstream file(lpFileName, std::ios::binary | std::ios::ate);

	if (!file) return -1; 
		
	const size_t fileSize = file.tellg();
	file.seekg(0);

	if (fileSize < sizeof(DDSHeader)) { return -2; }

	uint8_t *buffer = new uint8_t[fileSize];

	file.read(reinterpret_cast<char*>(buffer), fileSize);
	if (!file) return -2; 

	file.close();

	const DDSHeader* fileHeader = (DDSHeader*)buffer;

	if (fileHeader->dwMagic != FOURCC('D', 'D', 'S', ' ')) {
		delete[] buffer;
		buffer = nullptr;
		return -3;
	}

	if (fileHeader->dwSize != 124 ||
		fileHeader->ddpfPixelFormat.dwSize != 32) {
		delete[] buffer;
		buffer = nullptr;
		return -4;
	}

	m_nWidth = fileHeader->dwWidth;
	m_nHeight = fileHeader->dwHeight;
	m_nDepth = fileHeader->dwDepth;
	m_nBitsPerPx = fileHeader->ddpfPixelFormat.dwRGBBitCount;

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

	bool hasDX10 = false;
	DDS_HEADER_DXT10* dx10 = nullptr;

	if (fileHeader->ddpfPixelFormat.dwFourCC == FOURCC('D', 'X', '1', '0')) {
		hasDX10 = true;

		if (fileSize < sizeof(DDSHeader) + sizeof(DDS_HEADER_DXT10)) {
			delete[] buffer;
			return -5; 
		}

		dx10 = (DDS_HEADER_DXT10*) (buffer + sizeof(DDSHeader));

		switch (dx10->dxgiFormat) {
		case DXGI_FORMAT_BC7_UNORM:
			m_nCompression = GL_COMPRESSED_RGBA_BPTC_UNORM;
			break;

		case DXGI_FORMAT_BC7_UNORM_SRGB:
			m_nCompression = GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM;
			break;
		
		default:
			delete[] buffer;
			buffer = nullptr;
			return -6;
		}
	}
	else {
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
	}

	uint8_t *pixelData = buffer + sizeof(DDSHeader);
	if (hasDX10)
		pixelData += sizeof(DDS_HEADER_DXT10);

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
		switch (m_nBitsPerPx) {
		case 8:
			pixelFormat = GL_RED;
			internalFmt = GL_R8;
			break;

		case 16:
			pixelFormat = GL_RG;
			internalFmt = GL_RG8;
			break;

		case 24:
			pixelFormat = GL_BGR;
			internalFmt = GL_RGB8;
			break;

		case 32:
			pixelFormat = GL_RGBA;
			internalFmt = GL_RGBA8;
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
					if (m_nCompression == GL_COMPRESSED_RGB_S3TC_DXT1_EXT)
						block = 8;
					size = ((w + 3) / 4) * block;
					if (ptr + size > bufferEnd) break;
					glCompressedTexImage1D(GL_TEXTURE_1D, mip, m_nCompression, w, 0, size, ptr);
				} else {
					uint32_t block = (m_nCompression == GL_COMPRESSED_RGB_S3TC_DXT1_EXT) ? 8 : 16;
					if (m_nCompression == GL_COMPRESSED_RGB_S3TC_DXT1_EXT)
						block = 8;
					size = ((h + 3) / 4) * block;
					if (ptr + size > bufferEnd) break;
					glCompressedTexImage1D(GL_TEXTURE_1D, mip, m_nCompression, h, 0, size, ptr);
				}
			}
			else {
				if (m_nHeight == 1) {
					size = w * (m_nBitsPerPx >> 3);
					if (ptr + size > bufferEnd) break;
					glTexImage1D(GL_TEXTURE_1D, mip, internalFmt, w, 0, pixelFormat, GL_UNSIGNED_BYTE, ptr);
				} else {
					size = h * (m_nBitsPerPx >> 3);
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
				if (m_nCompression == GL_COMPRESSED_RGB_S3TC_DXT1_EXT)
					block = 8;
				uint32_t bw = (w + 3) / 4;
				uint32_t bh = (h + 3) / 4;
				uint32_t sliceSize = bw * bh * block;
				size = sliceSize * d;
				if (ptr + size > bufferEnd) break;
				glCompressedTexImage3D(GL_TEXTURE_3D, mip, m_nCompression, w, h, d, 0, size, ptr);
			}
			else {
				size = w * h * d * (m_nBitsPerPx >> 3);
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
					if (m_nCompression == GL_COMPRESSED_RGB_S3TC_DXT1_EXT)
						block = 8;
					size = ((w + 3) / 4) * ((h + 3) / 4) * block;
					if (ptr + size > bufferEnd) break;
					glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, mip, m_nCompression, w, h, 0, size, ptr);
				} else {
					size = w * h * (m_nBitsPerPx >> 3);
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
				if (m_nCompression == GL_COMPRESSED_RGB_S3TC_DXT1_EXT)
					block = 8;
				size = ((w + 3) / 4) * ((h + 3) / 4) * block;
				if (ptr + size > bufferEnd) break;
				glCompressedTexImage2D(GL_TEXTURE_2D, mip, m_nCompression, w, h, 0, size, ptr);
			}
			else {
				size = w * h * (m_nBitsPerPx >> 3);
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

BOOL Image::fromKTX(const char* lpFileName, unsigned short filterMode) {

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
	m_nBitsPerPx = (hdr->glFormat == 0) ? 0 : 8;


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

BOOL Image::fromTGA(const char* lpFileName, unsigned short filterMode) {
	std::ifstream file(lpFileName, std::ios::binary | std::ios::ate);
	if (!file) return -1;

	std::streamsize fileSize = file.tellg();
	file.seekg(0, std::ios::beg);
	uint8_t* buffer = new uint8_t[fileSize];
	if (!file.read(reinterpret_cast<char*>(buffer), fileSize)) {
		delete[] buffer;
		return -2;
	}

	auto decodeTga = [&](uint8_t *fileContent, uint32_t fileSize,  
		uint32_t *width, uint32_t *height, uint32_t *bpp, uint8_t **pixelsPtr) {
		if (fileSize < sizeof(TGAHeader)) return -1;

		const TGAHeader* hdr = reinterpret_cast<const TGAHeader*>(fileContent);

		const size_t offset = sizeof(TGAHeader) + hdr->idLength;
		if (fileSize <= offset) return -2;

		bool isColor = hdr->imageType == 2 || hdr->imageType == 10;
		bool isGray = hdr->imageType == 3 || hdr->imageType == 11;
		bool isMapped = hdr->imageType == 1 || hdr->imageType == 9;
		bool isRLE = hdr->imageType == 9 || hdr->imageType == 10 || hdr->imageType == 11;
		bool flipY = !(hdr->imageDescriptor & 0x20);

		const size_t pixelCount = hdr->width * hdr->height;

		const uint8_t* src = fileContent + offset;
		const uint8_t* end = fileContent + fileSize;

		// palette setup
		const uint8_t* palette = nullptr;
		size_t paletteBpp = hdr->colorMapEntrySize / 8;
		size_t paletteSize = hdr->colorMapLength * paletteBpp;

		if (isMapped) {
			if (hdr->colorMapType != 1 || paletteBpp < 1 || paletteBpp > 4)
				return -3;

			palette = fileContent + sizeof(TGAHeader) + hdr->idLength;
			if (fileSize < offset + paletteSize)
				return -4;
			src += paletteSize;
		}

		uint8_t srcBytes =
			isMapped ? 1 :
			isGray ? 1 :
			hdr->pixelDepth / 8;

		uint8_t dstBytes =
			isMapped ? 3 :
			isGray ? 1 :
			hdr->pixelDepth / 8;

		uint8_t* pixels = new uint8_t[pixelCount * dstBytes];
		size_t i = 0;

		auto decodeMappedColor = [&](uint8_t index, uint8_t* out) {

			const uint8_t* c = palette + index * paletteBpp;

			if (paletteBpp == 3 || paletteBpp == 4) {
				out[0] = c[0]; // B
				out[1] = c[1]; // G
				out[2] = c[2]; // R
			}
			else if (paletteBpp == 2) {
				// 16-bit 555
				uint16_t v = *(uint16_t*)c;

				out[0] = ((v >> 0) & 31) << 3;
				out[1] = ((v >> 5) & 31) << 3;
				out[2] = ((v >> 10) & 31) << 3;
			}
			else if (paletteBpp == 1) {
				out[0] = out[1] = out[2] = c[0];
			}
		};

		if (isRLE) {
			while (i < pixelCount && src < end) {
				uint8_t packet = *src++;
				bool rle = (packet & 0x80) != 0;
				uint8_t count = (packet & 0x7F) + 1;

				if (rle) {
					if (src + srcBytes > end)
						break;

					const uint8_t* pixelData = src;
					src += srcBytes;

					for (uint8_t j = 0; j < count && i < pixelCount; ++j, ++i) {
						if (isMapped) {
							decodeMappedColor(pixelData[0], &pixels[i * dstBytes]);
						} else {
							std::memcpy(&pixels[i * dstBytes],pixelData, dstBytes);
						}
					}
				} else {
					// raw packet
					for (uint8_t j = 0; j < count && i < pixelCount; ++j, ++i) {
						if (src + srcBytes > end)
							break;

						if (isMapped) {
							decodeMappedColor(*src,
								&pixels[i * dstBytes]);
							src += 1; 
						} else {
							std::memcpy(&pixels[i * dstBytes], src, dstBytes);
							src += srcBytes;
						}
					}
				}
			}
		}
		else {
			// Non-RLE (raw)
			for (; i < pixelCount && src < end; ++i) {
				if (isMapped) {
					if (src + 1 > end) break;

					uint8_t index = *src++;
					decodeMappedColor(index, &pixels[i * dstBytes]);
				} else {
					if (src + srcBytes > end) break;
					std::memcpy(&pixels[i * dstBytes], src, dstBytes);
					src += srcBytes;
				}
			}
		}

		if (i != pixelCount) {
			delete[] pixels;
			return -4;
		}

		*width = hdr->width;
		*height = hdr->height;
		
		if (isMapped)
			*bpp = 24;
		else if (isGray)
			*bpp = 8;
		else
			*bpp = hdr->pixelDepth;

		uint32_t bytesPerPx = (*bpp) >> 3;

		if (flipY) {
			uint32_t rowSize = (*width) * bytesPerPx;

			for (uint32_t y = 0; y < *height / 2; y++) {
				uint8_t* row1 = pixels + y * rowSize;
				uint8_t* row2 = pixels + (*height - 1 - y) * rowSize;

				for (uint32_t i = 0; i < rowSize; i++)
					std::swap(row1[i], row2[i]);
			}
		}

		*pixelsPtr = pixels;

		return TRUE;
	};

	uint8_t* pixelMap = nullptr;

	if (decodeTga(buffer, fileSize, &m_nWidth, &m_nHeight, &m_nBitsPerPx, &pixelMap) != TRUE) {
		delete[] buffer;
		return -3;
	}

	delete[] buffer; buffer = nullptr;

	// Init OpenGL texture
	if ((m_nHeight == 1) || (m_nWidth == 1)) {
		m_GLTarget = GL_TEXTURE_1D;
	}
	else {
		m_GLTarget = GL_TEXTURE_2D;
	}

	glGenTextures(1, &m_GLTexture);
	glBindTexture(m_GLTarget, m_GLTexture);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
	glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	const uint8_t mipCount = 1;
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

	GLenum pixelFormat = 0;
	GLenum internalFmt = 0;

	switch (m_nBitsPerPx) {
	case 8:
		pixelFormat = GL_RED;
		internalFmt = GL_R8;
		break;

	case 16:
		pixelFormat = GL_RG;
		internalFmt = GL_RG8;
		break;

	case 24:
		pixelFormat = GL_BGR;
		internalFmt = GL_RGB8;
		break;

	case 32:
		pixelFormat = GL_RGBA;
		internalFmt = GL_RGBA8 ;
		break;

	default:
		pixelFormat = GL_LUMINANCE;
		internalFmt = GL_LUMINANCE8;
	}

	if (m_GLTarget == GL_TEXTURE_1D) {
		if (m_nHeight == 1) {
			glTexImage1D(GL_TEXTURE_1D, 0, internalFmt, m_nWidth, 0, pixelFormat, GL_UNSIGNED_BYTE, pixelMap);
		} else {
			glTexImage1D(GL_TEXTURE_1D, 0, internalFmt, m_nHeight, 0, pixelFormat, GL_UNSIGNED_BYTE, pixelMap);
		}
	} else {
		glTexImage2D(GL_TEXTURE_2D, 0, internalFmt, m_nWidth, m_nHeight, 0, pixelFormat, GL_UNSIGNED_BYTE, pixelMap);
	}

	if (useMips) {
		glGenerateMipmap(m_GLTarget);
	}

	delete[] pixelMap; pixelMap = nullptr;

	return TRUE; 
}

//
// Private
//

void Image::genCheckboard() {

}
