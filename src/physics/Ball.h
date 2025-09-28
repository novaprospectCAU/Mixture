#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

// 공의 물리 속성을 관리하는 클래스
class Ball {
public:
  // 물리 속성
  glm::vec3 position;        // 위치
  glm::vec3 velocity;        // 속도
  glm::vec3 angularVelocity; // 각속도
  glm::vec3 rotation;        // 누적 회전 (라디안)
  float radius;              // 반지름
  float mass;                // 질량

  // 렌더링 속성
  unsigned int VAO, VBO, EBO;
  std::vector<float> vertices;
  std::vector<unsigned int> indices;
  unsigned int texture;
  glm::mat4 modelMatrix;

  // 생성자
  Ball(glm::vec3 pos, float r, float m = 1.0f);

  // 물리 업데이트
  void update(float deltaTime);

  // 충돌 처리
  void handleCollision(const glm::vec3 &normal, float restitution = 0.8f);

  // 렌더링 준비
  void updateModelMatrix();

  // 정리
  void cleanup();

private:
  // 중력 상수
  static constexpr float GRAVITY = -9.81f;
  static constexpr float FRICTION = 0.98f;       // 마찰 계수
  static constexpr float AIR_RESISTANCE = 0.99f; // 공기 저항
};

// 구체 메시 생성 함수
class BallMesh {
public:
  static void createSphere(std::vector<float> &vertices,
                           std::vector<unsigned int> &indices, float radius,
                           int sectors = 20, int stacks = 20);
  static unsigned int createCheckerboardTexture();
};
