#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

// 메시 구조체
struct Mesh {
  unsigned int VAO, VBO, EBO;
  std::vector<float> vertices;
  std::vector<unsigned int> indices;
  unsigned int texture;
  glm::mat4 modelMatrix;
  std::string name; // 충돌 감지를 위한 식별자

  // 충돌 감지를 위한 경계 상자
  glm::vec3 minBounds;
  glm::vec3 maxBounds;
};

// 메시 생성 함수들
Mesh createFloor();
Mesh createWall(const std::string &name, const glm::vec3 &position,
                const glm::vec3 &rotation);

// 충돌 감지 함수들
bool checkAABBCollision(const glm::vec3 &pos1, const glm::vec3 &min1,
                        const glm::vec3 &max1, const glm::vec3 &pos2,
                        const glm::vec3 &min2, const glm::vec3 &max2);
bool checkCameraCollision(const class Camera &camera, const Mesh &mesh);
void calculateBounds(Mesh &mesh);
