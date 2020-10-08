#pragma once
#include <glad/glad.h>

namespace Utility
{
	GLenum GetFormat(int channels);
	GLenum GetInternalFormat(int channels);
}