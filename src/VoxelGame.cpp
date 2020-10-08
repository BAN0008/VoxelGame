#include <SDL.h>
#include "Renderer.hpp"
#include "Log.hpp"
#include "JobSystem.hpp"
#include "Shader.hpp"
#include "FreeCamera.hpp"
#include "Chunk.hpp"
#include "TextureArray.hpp"
#include "VertexBuffer.hpp"
#include <cstdint>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/noise.hpp>
#include <functional>
#include <map>
#include <array>
#include <memory>

const char *vertexCode =
R"(#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;
layout (location = 2) in vec3 aTexCoord;

uniform mat4 uCamera;
uniform mat4 uModel;

uniform sampler2DArray uTexture;

out vec3 fragPos;
out vec3 norm;
out vec3 texCoord;

void main()
{
	fragPos     = vec3(uModel * vec4(aPos, 1.0));
	norm        = aNorm;
	texCoord    = vec3(vec2(1.0) / textureSize(uTexture, 0).xy, 1.0) * aTexCoord;
	gl_Position = uCamera * uModel * vec4(aPos, 1.0);
})";

const char *fragmentCode =
R"(#version 330 core

in      vec3 fragPos;
in      vec3 norm;
in      vec3 texCoord;

out     vec4 outColor;

uniform vec3 uCameraPos;
uniform sampler2DArray uTexture;

const vec3  lightPos = vec3(-200, 200.0, -200.0);
const float ambient  = 0.1;

void main()
{
	// vec3  color      = vec3(1.0, 0.0, 0.0);
	vec3  color      = texture(uTexture, texCoord).rgb;
	vec3  ambient    = 0.3 * color;
	vec3  lightDir   = normalize(lightPos - fragPos);

	float diff       = max(dot(lightDir, norm), 0.0);
	vec3  diffuse    = diff * color;

	vec3  viewDir    = normalize(uCameraPos - fragPos);
	vec3  reflectDir = reflect(-lightDir, norm);
	vec3  halfwayDir = normalize(lightDir + viewDir);
	float spec       = pow(max(dot(norm, halfwayDir), 0.0), 32.0);
	vec3  specular   = vec3(0.3) * spec;

	outColor = vec4(ambient + diffuse + specular, 1.0);
})";

const char *cursorVertexCode =
R"(#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aTexCoord;

uniform mat4 uTransform;

uniform sampler2DArray uTexture;

out vec3 texCoord;

void main()
{
	texCoord    = vec3(vec2(1.0) / textureSize(uTexture, 0).xy, 1.0) * aTexCoord;
	gl_Position = uTransform * vec4(aPos, 0.0, 1.0);
})";

const char *cursorFragmentCode =
R"(#version 330 core

in      vec3 texCoord;
out     vec4 outColor;

uniform sampler2DArray uTexture;

void main()
{
	outColor = texture(uTexture, texCoord);
})";

struct CursorVertex
{
	int8_t  pX, pZ;
	uint8_t tX, tY, tZ;
};

const std::array<CursorVertex, 6> cursorVertices = {
	CursorVertex{ 16,  16, 16, 16, 3},
	CursorVertex{ 16, -16, 16, 0,  3},
	CursorVertex{-16, -16, 0,  0,  3},
	CursorVertex{-16, -16, 0,  0,  3},
	CursorVertex{-16,  16, 0,  16, 3},
	CursorVertex{ 16,  16, 16, 16, 3}
};

template<uint8_t Width, uint8_t Height, uint8_t Depth, typename VoxelType, VoxelType NullVoxel>
void GenerateChunk(std::shared_ptr<Chunk<Width, Height, Depth, VoxelType, NullVoxel>> chunk, int x, int y, int z)
{
	chunk->lock.lock();
	for (uint8_t iZ = 0; iZ < Depth; iZ++) {
		for (uint8_t iX = 0; iX < Width; iX++) {
			chunk->SetVoxel(iX, 0, iZ, 1);
			const int aX = iX + (x * Width);
			const int aZ = iZ + (z * Depth);
			float noise = (24.0f) * glm::simplex(glm::vec2((float)aX / (float)(512), (float)aZ / (float)(512)));
			noise += (12.0f) * glm::simplex(glm::vec2((float)aX / (float)(64), (float)aZ / (float)(64)));
			noise += 12.0f;
			for (uint8_t iY = 0; iY <= noise + 2; iY++) {
				chunk->SetVoxel(iX, iY, iZ, 1);
			}
		}
	}
	chunk->UpdateVertices();
	chunk->lock.unlock();
}

int main(int argc, char **argv)
{
	const int windowWidth  = 1280;
	const int windowHeight = 720;

	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		Log::Error(std::string("VoxelGame::main: Failed to initialize SDL2 - ") + SDL_GetError());
		return 0;
	}

	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

	SDL_ClearError();
	SDL_Window *window = SDL_CreateWindow("VoxelGame", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, SDL_WINDOW_OPENGL);
	if (!window) {
		Log::Error(std::string("VoxelGame::main: Failed to create window - ") + SDL_GetError());
	}

	bool running = window && Renderer::Initialize(window);
	Renderer::SetClearColor((1.0f / 255.0f) * 135.0f, (1.0f / 255.0f) * 206.0f, (1.0f / 255.0f) * 235.0f, (1.0f / 255.0f) * 255.0f);

	Shader *shader = new Shader(vertexCode, fragmentCode, "Test Shader");

	Shader *cursorShader = new Shader(cursorVertexCode, cursorFragmentCode, "Cursor Shader");
	FreeCamera *camera = new FreeCamera();

	glm::mat4 projection = glm::perspectiveFov(45.0f, 1280.0f, 720.0f, 0.1f, 1000.0f);
	SDL_bool isCaptured = SDL_FALSE;

	const int chunkWidth  = 64;
	const int chunkHeight = 64;
	const int chunkDepth  = 64;

	TextureArray *texture_atlas = new TextureArray("../res/texture_atlas.png", 4);
	texture_atlas->Bind(0);
	shader->SetUniformInt("uTexture", 0);
	cursorShader->SetUniformInt("uTexture", 0);

	VertexBuffer *cursorVertexBuffer = new VertexBuffer({VertexType::Int8_2, VertexType::Uint8_3});
	cursorVertexBuffer->UpdateVertices(cursorVertices.data(), cursorVertices.size());

	// const float loadDistance   = 256.0f;
	// const float renderDistance = 160.0f;
	const float loadDistance   = 512.0f;
	const float renderDistance = 384.0f;
	std::map<uint64_t, std::shared_ptr<Chunk<chunkWidth, chunkHeight, chunkDepth, uint8_t, 0>>> chunks;

	JobSystem::StartThreads();

	auto keyState = SDL_GetKeyboardState(nullptr);

	while (running) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:
					running = false;
					break;
				case SDL_MOUSEMOTION:
					if (isCaptured) camera->ProcessMouseInput(event.motion.xrel, event.motion.yrel);
					break;
				case SDL_MOUSEBUTTONDOWN:
					if (isCaptured && event.button.button == SDL_BUTTON_LEFT) {
						glm::vec3 direction = camera->front;
						for (float i = 0.0f; i < 256.0f; i += 0.01f) {
							glm::vec3 pos = camera->position + (direction * i);
							pos.x = floor(pos.x);
							pos.y = floor(pos.y);
							pos.z = floor(pos.z);

							int chunkX = floor((double)pos.x / (double)chunkWidth);
							int chunkZ = floor((double)pos.z / (double)chunkDepth);

							int localX = (int)pos.x % chunkWidth;
							int localZ = (int)pos.z % chunkDepth;
							if (localX < 0) localX += chunkWidth;
							if (localZ < 0) localZ += chunkDepth;

							uint64_t chunkIndex;
							*(reinterpret_cast<int*>(&chunkIndex) + 0) = chunkX;
							*(reinterpret_cast<int*>(&chunkIndex) + 1) = chunkZ;

							if (chunks.count(chunkIndex) > 0) {
								auto chunk = chunks[chunkIndex];
								chunk->lock.lock();
								if (chunk->TestPos(localX, pos.y, localZ)) {
									chunk->SetVoxel(localX, pos.y, localZ, 0);
									chunk->modified = true;
									chunk->UpdateVertices();
									chunk->lock.unlock();
									break;
								}
								chunk->lock.unlock();
							}
						}
					}
					break;
				case SDL_KEYDOWN:
					if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE && !event.key.repeat) {
						isCaptured = isCaptured == SDL_TRUE ? SDL_FALSE : SDL_TRUE;
						SDL_CaptureMouse(isCaptured);
						SDL_SetRelativeMouseMode(isCaptured);
						// SDL_ShowCursor(isCaptured == SDL_TRUE ? SDL_FALSE : SDL_TRUE);
					}
					break;
			}
		}

		glm::vec3 movement = {0.0f, 0.0f, 0.0f};
		// float speed = 0.5f;
		float speed = 3.0f;
		if (keyState[SDL_SCANCODE_SPACE])  movement.y += speed;
		if (keyState[SDL_SCANCODE_LSHIFT]) movement.y -= speed;
		if (keyState[SDL_SCANCODE_W])      movement.z += speed;
		if (keyState[SDL_SCANCODE_S])      movement.z -= speed;
		if (keyState[SDL_SCANCODE_D])      movement.x += speed;
		if (keyState[SDL_SCANCODE_A])      movement.x -= speed;
		camera->Move(movement);

		// Unload chunks
		for (auto it = chunks.cbegin(); it != chunks.cend();) {
			int x = *(reinterpret_cast<const int*>(&it->first) + 0);
			int z = *(reinterpret_cast<const int*>(&it->first) + 1);
			if (glm::length(glm::vec2(x * chunkWidth, z * chunkDepth) - glm::vec2(camera->position.x, camera->position.z)) > loadDistance) {
				if (it->second.use_count() == 1 && it->second->lock.try_lock()) {
					if (it->second->modified) {
						it->second->lock.unlock();
						it++;
					}
					else {
						it->second->lock.unlock();
						chunks.erase(it++);
					}
				}
				else it++;
			}
			else it++;
		}

		// Load chunks
		{
			int z1 = round((camera->position.z - loadDistance) / chunkDepth);
			int z2 = round((camera->position.z + loadDistance) / chunkDepth);
			int x1 = round((camera->position.x - loadDistance) / chunkWidth);
			int x2 = round((camera->position.x + loadDistance) / chunkWidth);
			for (int iZ = z1; iZ < z2; iZ++) {
				for (int iX = x1; iX < x2; iX++) {
					if (glm::length(glm::vec2(iX * chunkWidth, iZ * chunkDepth) - glm::vec2(camera->position.x, camera->position.z)) <= loadDistance) {
						uint64_t index;
						*(reinterpret_cast<int*>(&index) + 0) = iX;
						*(reinterpret_cast<int*>(&index) + 1) = iZ;
						if (chunks.count(index) == 0) {
							chunks[index] = std::make_shared<Chunk<chunkWidth, chunkHeight, chunkDepth, uint8_t, 0>>();
							const JobSystem::Job &job = std::bind(GenerateChunk<chunkWidth, chunkHeight, chunkDepth, uint8_t, 0>, chunks[index], iX, 0, iZ);
							JobSystem::AddJob(job);
						}
					}
				}
			}
		}

		Renderer::ClearBuffer();

		shader->Bind();

		shader->SetUniformVec3("uCameraPos", camera->position);
		// shader->SetUniformMat4("uCamera", projection * camera->GetMatrix() * glm::scale(glm::mat4(1.0f), glm::vec3(0.25f, 0.25f, 0.25f)));
		// shader->SetUniformMat4("uCamera", projection * camera->GetMatrix() * glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f)));
		shader->SetUniformMat4("uCamera", projection * camera->GetMatrix());

		// Render chunks
		{
			int z1 = round((camera->position.z - renderDistance) / chunkDepth);
			int z2 = round((camera->position.z + renderDistance) / chunkDepth);
			int x1 = round((camera->position.x - renderDistance) / chunkWidth);
			int x2 = round((camera->position.x + renderDistance) / chunkWidth);
			for (int iZ = z1; iZ < z2; iZ++) {
				for (int iX = x1; iX < x2; iX++) {
					if (glm::length(glm::vec2(iX * chunkWidth, iZ * chunkDepth) - glm::vec2(camera->position.x, camera->position.z)) <= renderDistance) {
						uint64_t index;
						*(reinterpret_cast<int*>(&index) + 0) = iX;
						*(reinterpret_cast<int*>(&index) + 1) = iZ;
						auto chunk = chunks[index];
						if (chunk && chunk->lock.try_lock()) {
							shader->SetUniformMat4("uModel", glm::translate(glm::mat4(1.0f), glm::vec3(iX * chunkWidth, 0.0f, iZ * chunkDepth)));
							chunk->Render();
							chunk->lock.unlock();
						}
					}
				}
			}
		}

		cursorShader->Bind();
		cursorShader->SetUniformMat4("uTransform", glm::ortho(0.0f, 1280.0f, 720.0f, 0.0f) * glm::translate(glm::mat4(1.0f), glm::vec3(1280.0f / 2.0f, 720.0f / 2.0f, 0.0f)));
		cursorVertexBuffer->Render();

		Renderer::FlushBuffer();
	}

	JobSystem::StopThreads();
	chunks.clear();

	delete cursorVertexBuffer;
	delete texture_atlas;
	delete camera;
	delete cursorShader;
	delete shader;

	Renderer::Cleanup();
	if (window) SDL_DestroyWindow(window);

	SDL_Quit();
	return 0;
}