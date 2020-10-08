#include "Utility.hpp"
#include "Log.hpp"

namespace Utility
{
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
}