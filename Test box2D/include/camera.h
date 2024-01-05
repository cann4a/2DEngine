#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Camera {
public:

	glm::vec3 position;
	glm::vec3 front_dir = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 up_dir = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 direction = glm::vec3(0.0f, 0.0f, 0.0f);

	float speed= 5.0f;

	float yaw = -90.0f;
	float pitch = 0.0f;

	const float sensitivity = 0.1f;

	Camera() {
		position = glm::vec3(0.0f, 0.0f, 0.0f);
		computeCameraDirection();
	}
	Camera(glm::vec3 pos) {
		position = pos;
		computeCameraDirection();
	}

	void setposition(float x, float y, float z) {
		position.x = x;
		position.y = y;
		position.z = z;
	}
	void setposition(glm::vec3 pos) {
		position.x = pos.x;
		position.y = pos.y;
		position.z = pos.z;
	}
	void setCameraSpeed(float speed) {
		speed= speed;
	}
	void computeCameraDirection() {
		direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		direction.y = sin(glm::radians(pitch));
		direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		front_dir = glm::normalize(direction);
	}
};

#endif
