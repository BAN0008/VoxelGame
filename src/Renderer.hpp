#pragma once
#include <SDL_video.h>

namespace Renderer
{
	bool Initialize(SDL_Window *window);
	void Cleanup();

	void SetClearColor(float r, float g, float b, float a);

	void ClearBuffer();
	void FlushBuffer();
}