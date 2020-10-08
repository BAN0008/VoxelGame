#include "TextureArray.hpp"
#include "Utility.hpp"
#include <stb_image.h>
#include <glad/glad.h>

using namespace Utility;

TextureArray::TextureArray(const char *fileName, int depth) : depth(depth)
{
	#ifdef ARB_DIRECT_STATE_ACCESS
		glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &textureID);

		// glTextureParameteri(textureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// glTextureParameteri(textureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(textureID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(textureID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		int channels = 0;
		auto imageData = stbi_load(fileName, &width, &height, &channels, 0);
		height /= depth;

		glTextureStorage3D(textureID, 1, GetInternalFormat(channels), width, height, depth);
		for (int i = 0; i < depth; i++) {
			glTextureSubImage3D(textureID, 0, 0, 0, i, width, height, 1, GetFormat(channels), GL_UNSIGNED_BYTE, imageData + (width * height * channels * i));
		}
		stbi_image_free(imageData);
	#else
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);

		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		int channels = 0;
		auto imageData = stbi_load(fileName, &width, &height, &channels, 0);
		height /= depth;

		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GetFormat(channels), width, height, depth, 0, GetFormat(channels), GL_UNSIGNED_BYTE, nullptr);
		for (int i = 0; i < depth; i++) {
			glTexSubImage3D(textureID, 0, 0, 0, i, width, height, 1, GetFormat(channels), GL_UNSIGNED_BYTE, imageData + (width * height * channels * i));
		}
		stbi_image_free(imageData);
	#endif
}

TextureArray::~TextureArray()
{
	glDeleteTextures(1, &textureID);
}

void TextureArray::Bind(int slot)
{
	#ifdef ARB_DIRECT_STATE_ACCESS
		glBindTextureUnit(slot, textureID);
	#else
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D_ARRAY, textureID);
	#endif
}