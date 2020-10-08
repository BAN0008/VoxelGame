#include "FreeCamera.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Log.hpp" // Delete me

FreeCamera::FreeCamera()
{
	position = {0.0f, 0.0f, 0.0f};
	pitch = 0.0f;
	yaw   = 0.0f;
	UpdateVectors();
}

void FreeCamera::Move(const glm::vec3 vector)
{
	position   += glm::normalize(glm::vec3(right.x, 0.0f, right.z)) * vector.x;
	position.y += vector.y;
	position   += glm::normalize(glm::vec3(front.x, 0.0f, front.z)) * vector.z;
}

void FreeCamera::ProcessMouseInput(float x, float y)
{
	yaw   += x * 0.25f;
	pitch -= y * 0.25f;

	if (pitch >=  89.0f) pitch =  89.0f;
	if (pitch <= -89.0f) pitch = -89.0f;

	UpdateVectors();
}

glm::mat4 FreeCamera::GetMatrix()
{
	return glm::lookAt(position, position + front, up);
}

void FreeCamera::UpdateVectors()
{
	front = glm::normalize(glm::vec3(
		glm::cos(glm::radians(yaw)) * glm::cos(glm::radians(pitch)),
		glm::sin(glm::radians(pitch)),
		glm::sin(glm::radians(yaw)) * glm::cos(glm::radians(pitch))
	));

	right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
	up    = glm::normalize(glm::cross(right, front));
}