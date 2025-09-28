#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

// 전방 선언
struct Mesh;

// 카메라 클래스
class Camera {
public:
  glm::vec3 position;
  glm::vec3 front;
  glm::vec3 up;
  glm::vec3 right;
  glm::vec3 worldUp;

  float yaw;
  float pitch;

  float movementSpeed;
  float mouseSensitivity;
  float zoom;

  // 가속도 관련
  glm::vec3 velocity;
  float acceleration;
  float friction;

  Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
         glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = -90.0f,
         float pitch = 0.0f);

  glm::mat4 getViewMatrix();
  void processKeyboard(int direction, float deltaTime);
  void processMouseMovement(float xoffset, float yoffset,
                            bool constrainPitch = true);
  void update(float deltaTime);
  void handleCollision(const std::vector<Mesh> &meshes);

private:
  void updateCameraVectors();
};
