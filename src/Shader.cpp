#include "Shader.hpp"
#include "Log.hpp"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

Shader::Shader(const char *vertexCode, const char *fragmentCode, const char *name)
{
	char infoLog[4096];
	int  success;

	vertexID = glCreateShader(GL_VERTEX_SHADER);
	if (GLAD_GL_KHR_debug && name) glObjectLabel(GL_SHADER, vertexID, -1, (std::string(name) + " (VS)").c_str());
	glShaderSource(vertexID, 1, &vertexCode, nullptr);
	glCompileShader(vertexID);
	glGetShaderiv(vertexID, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertexID, 4096, nullptr, infoLog);
		Log::Error(std::string("Shader: Failed to compile vertex shader - ") + infoLog);
	}

	fragmentID = glCreateShader(GL_FRAGMENT_SHADER);
	if (GLAD_GL_KHR_debug && name) glObjectLabel(GL_SHADER, fragmentID, -1, (std::string(name) + " (FS)").c_str());
	glShaderSource(fragmentID, 1, &fragmentCode, nullptr);
	glCompileShader(fragmentID);
	glGetShaderiv(fragmentID, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragmentID, 4096, nullptr, infoLog);
		Log::Error(std::string("Shader: Failed to compile fragment shader - ") + infoLog);
	}

	programID = glCreateProgram();
	if (GLAD_GL_KHR_debug && name) glObjectLabel(GL_PROGRAM, programID, -1, name);
	glAttachShader(programID, vertexID);
	glAttachShader(programID, fragmentID);
	glLinkProgram(programID);
	glGetProgramiv(programID, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(programID, 4096, nullptr, infoLog);
		Log::Error(std::string("Shader: Failed to link shader program - ") + infoLog);
	}
}

Shader::~Shader()
{
	glDeleteProgram(programID);
	glDeleteShader(vertexID);
	glDeleteShader(fragmentID);
}

void Shader::Bind()
{
	glUseProgram(programID);
}

void Shader::SetUniformInt(const char *name, const int value)
{
	#ifndef ARB_DIRECT_STATE_ACCESS
		GLint currentProgram = 0;
		glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
		glUseProgram(programID);
		glUniform1i(glGetUniformLocation(programID, name), value);
	#else
		glProgramUniform1i(programID, glGetUniformLocation(programID, name), value);
	#endif
	#ifndef ARB_DIRECT_STATE_ACCESS
		glUseProgram(currentProgram);
	#endif
}

void Shader::SetUniformVec3(const char *name, const glm::vec3 &value)
{
	#ifndef ARB_DIRECT_STATE_ACCESS
		GLint currentProgram = 0;
		glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
		glUseProgram(programID);
		glUniform3fv(glGetUniformLocation(programID, name), 1, glm::value_ptr(value));
	#else
		glProgramUniform3fv(programID, glGetUniformLocation(programID, name), 1, glm::value_ptr(value));
	#endif
	#ifndef ARB_DIRECT_STATE_ACCESS
		glUseProgram(currentProgram);
	#endif
}

void Shader::SetUniformMat4(const char *name, const glm::mat4 &value)
{
	#ifndef ARB_DIRECT_STATE_ACCESS
		GLint currentProgram = 0;
		glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
		glUseProgram(programID);
		glUniformMatrix4fv(glGetUniformLocation(programID, name), 1, GL_FALSE, glm::value_ptr(value));
	#else
		glProgramUniformMatrix4fv(programID, glGetUniformLocation(programID, name), 1, GL_FALSE, glm::value_ptr(value));
	#endif
	#ifndef ARB_DIRECT_STATE_ACCESS
		glUseProgram(currentProgram);
	#endif
}