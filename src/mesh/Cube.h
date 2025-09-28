#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

class Cube {
public:
  // 정육면체 속성
  glm::vec3 position;
  glm::vec3 velocity;
  float size; // 정육면체의 한 변의 길이
  float mass;

  // OpenGL 객체들
  unsigned int VAO, VBO, EBO;
  std::vector<float> vertices;
  std::vector<unsigned int> indices;
  unsigned int texture;
  glm::mat4 modelMatrix;

  // 충돌 감지를 위한 경계 상자
  glm::vec3 minBounds;
  glm::vec3 maxBounds;

  // 생성자
  Cube(glm::vec3 pos, float cubeSize, float m);

  // 소멸자
  ~Cube();

  // 물리 업데이트
  void update(float deltaTime);

  // 모델 매트릭스 업데이트
  void updateModelMatrix();

  // 경계 상자 계산
  void calculateBounds();

  // 리소스 정리
  void cleanup();
};
