#pragma once
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

class FreeCamera
{
public:
	FreeCamera();

	void Move(const glm::vec3 vector);
	void ProcessMouseInput(float x, float y);
	glm::mat4 GetMatrix();

	glm::vec3 position;
private:
	glm::vec3 front;
	glm::vec3 right;
	glm::vec3 up;

	float pitch = 0.0f;
	float yaw   = 0.0f;

	void UpdateVectors();
};