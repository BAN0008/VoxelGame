#include "Renderer.hpp"
#include "Log.hpp"
#include <SDL_video.h>
#include <glad/glad.h>

namespace Renderer
{
	static SDL_Window   *window    = nullptr;
	static SDL_GLContext glContext = nullptr;

	void debugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
	{
		Log::Error(message);
	}
}

bool Renderer::Initialize(SDL_Window *window)
{
	::Renderer::window = window;

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	#ifdef DEBUG
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
	#endif

	#ifdef ARB_DIRECT_STATE_ACCESS
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
	#else
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	#endif

	SDL_ClearError();
	glContext = SDL_GL_CreateContext(window);
	if (!glContext) {
		Log::Error(std::string("Renderer::Initialize: Failed to create OpenGL context - ") + SDL_GetError());
		return false;
	}
	if (!gladLoadGL()) {
		Log::Error("Renderer::Initialize: Failed to initialize OpenGL");
		return false;
	}

	Log::Info(std::string("OpenGL version: ") + reinterpret_cast<const char *>(glGetString(GL_VERSION)));

	#ifdef ARB_DIRECT_STATE_ACCESS
		const int requiredMajor = 4;
		const int requiredMinor = 5;
	#else
		const int requiredMajor = 3;
		const int requiredMinor = 3;
	#endif

	int major = 0;
	int minor = 0;

	glGetIntegerv(GL_MAJOR_VERSION, &major);
	glGetIntegerv(GL_MINOR_VERSION, &minor);

	if (major < requiredMajor || (major == requiredMajor && minor < requiredMinor)) {
		Log::Error(std::string("OpenGL ") + std::to_string(requiredMajor) + "." + std::to_string(requiredMinor) + " is required. Version " + std::to_string(major) + "." + std::to_string(minor) + " available");
		return false;
	}

	#ifdef DEBUG
		if (GLAD_GL_KHR_debug) {
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallback(debugMessageCallback, nullptr);
		}
	#endif

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	return true;
}

void Renderer::Cleanup()
{
	if (glContext) SDL_GL_DeleteContext(glContext);
}

void Renderer::SetClearColor(float r, float g, float b, float a)
{
	glClearColor(r, g, b, a);
}

void Renderer::ClearBuffer()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::FlushBuffer()
{
	SDL_GL_SwapWindow(window);
}