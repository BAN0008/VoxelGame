#pragma once
#include <glm/mat4x4.hpp>

class Shader
{
public:
	Shader(const char *vertexCode, const char *fragmentCode, const char *name = nullptr);
	~Shader();

	void Bind();
	void SetUniformInt(const char *name, const int value);
	void SetUniformVec3(const char *name, const glm::vec3 &value);
	void SetUniformMat4(const char *name, const glm::mat4 &value);
private:
	unsigned int programID  = 0;
	unsigned int vertexID   = 0;
	unsigned int fragmentID = 0;
};