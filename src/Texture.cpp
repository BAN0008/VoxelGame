#include "Texture.hpp"
#include "Log.hpp"
#include <glad/glad.h>
#include <stb_image.h>

GLenum GetFormat(int channels)
{
	switch (channels) {
		case 1:  return GL_RED;
		case 2:  return GL_RG;
		case 3:  return GL_RGB;
		case 4:  return GL_RGBA;
		default:
			Log::Error("Texture: Unsupported number of channels");
			return 0;
	}
}

GLenum GetInternalFormat(int channels)
{
	switch (channels) {
		case 1:  return GL_R8;
		case 2:  return GL_RG8;
		case 3:  return GL_RGB8;
		case 4:  return GL_RGBA8;
		default:
			Log::Error("Texture: Unsupported number of channels");
			return 0;
	}
}

Texture::Texture(const char *fileName)
{
	#ifdef ARB_DIRECT_STATE_ACCESS
		glCreateTextures(GL_TEXTURE_2D, 1, &textureID);

		// glTextureParameteri(textureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// glTextureParameteri(textureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(textureID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(textureID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		int channels = 0;
		auto imageData = stbi_load(fileName, &width, &height, &channels, 0);

		glTextureStorage2D(textureID, 1, GetInternalFormat(channels), width, height);
		glTextureSubImage2D(textureID, 0, 0, 0, width, height, GetFormat(channels), GL_UNSIGNED_BYTE, imageData);
		stbi_image_free(imageData);
	#else
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		int channels = 0;
		auto imageData = stbi_load(fileName, &width, &height, &channels, 0);

		glTexImage2D(GL_TEXTURE_2D, 0, GetFormat(channels), width, height, 0, GetFormat(channels), GL_UNSIGNED_BYTE, imageData);
		stbi_image_free(imageData);
	#endif
}

Texture::~Texture()
{
	glDeleteTextures(1, &textureID);
}

void Texture::Bind(int slot)
{
	#ifdef ARB_DIRECT_STATE_ACCESS
		glBindTextureUnit(slot, textureID);
	#else
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, textureID);
	#endif
}