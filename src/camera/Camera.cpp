#include "Camera.h"
#include "../mesh/Mesh.h"
#include <cmath>

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : front(glm::vec3(0.0f, 0.0f, -1.0f)), movementSpeed(200.0f),
      mouseSensitivity(0.25f), zoom(45.0f), acceleration(80.0f),
      friction(10.0f) {
  this->position = position;
  this->worldUp = up;
  this->yaw = yaw;
  this->pitch = pitch;
  this->velocity = glm::vec3(10.0f);
  updateCameraVectors();
}

glm::mat4 Camera::getViewMatrix() {
  return glm::lookAt(position, position + front, up);
}

void Camera::processKeyboard(int direction, float deltaTime) {
  if (direction == 0) // W
    this->velocity += front * acceleration * deltaTime;
  if (direction == 1) // S
    this->velocity -= front * acceleration * deltaTime;
  if (direction == 2) // A
    this->velocity -= right * acceleration * deltaTime;
  if (direction == 3) // D
    this->velocity += right * acceleration * deltaTime;
}

void Camera::processMouseMovement(float xoffset, float yoffset,
                                  bool constrainPitch) {
  xoffset *= mouseSensitivity;
  yoffset *= mouseSensitivity;

  yaw += xoffset;
  pitch += yoffset;

  if (constrainPitch) {
    if (pitch > 89.0f)
      pitch = 89.0f;
    if (pitch < -89.0f)
      pitch = -89.0f;
  }

  updateCameraVectors();
}

void Camera::update(float deltaTime) {
  // 마찰 적용
  velocity *= (1.0f - friction * deltaTime);

  // 최대 속도 제한
  float maxSpeed = movementSpeed * 2.0f;
  if (glm::length(velocity) > maxSpeed) {
    velocity = glm::normalize(velocity) * maxSpeed;
  }

  // 위치 업데이트
  position += velocity * deltaTime;
}

void Camera::handleCollision(const std::vector<Mesh> &meshes) {
  glm::vec3 newPosition = position;
  bool collisionDetected = false;

  for (const auto &mesh : meshes) {
    if (checkCameraCollision(*this, mesh)) {
      collisionDetected = true;

      // 각 축별로 충돌을 확인하고 해당 축의 이동만 되돌림
      glm::vec3 testPos = position - velocity * 0.016f;

      // X축 충돌 확인
      Camera testCameraX = *this;
      testCameraX.position = glm::vec3(testPos.x, position.y, position.z);
      if (!checkCameraCollision(testCameraX, mesh)) {
        newPosition.x = testPos.x;
      }

      // Y축 충돌 확인
      Camera testCameraY = *this;
      testCameraY.position = glm::vec3(position.x, testPos.y, position.z);
      if (!checkCameraCollision(testCameraY, mesh)) {
        newPosition.y = testPos.y;
      }

      // Z축 충돌 확인
      Camera testCameraZ = *this;
      testCameraZ.position = glm::vec3(position.x, position.y, testPos.z);
      if (!checkCameraCollision(testCameraZ, mesh)) {
        newPosition.z = testPos.z;
      }

      // 충돌이 감지된 축의 속도를 0으로 설정
      if (newPosition.x == position.x)
        velocity.x = 0.0f;
      if (newPosition.y == position.y)
        velocity.y = 0.0f;
      if (newPosition.z == position.z)
        velocity.z = 0.0f;
    }
  }

  if (collisionDetected) {
    position = newPosition;
  }
}

void Camera::updateCameraVectors() {
  glm::vec3 newFront;
  newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
  newFront.y = sin(glm::radians(pitch));
  newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
  front = glm::normalize(newFront);

  right = glm::normalize(glm::cross(front, worldUp));
  up = glm::normalize(glm::cross(right, front));
}
