#include "Cube.h"
#include "../utils/TextureUtils.h"
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

// 정육면체 생성자
Cube::Cube(glm::vec3 pos, float cubeSize, float m)
    : position(pos), size(cubeSize), mass(m) {

  // 정육면체 버텍스 생성 (중심이 원점)
  float halfSize = size * 0.5f;

  // 6면의 정육면체 버텍스 (위치 + 텍스처 좌표)
  vertices = {// 앞면 (Z+)
              -halfSize, -halfSize, halfSize, 0.0f, 0.0f, halfSize, -halfSize,
              halfSize, 1.0f, 0.0f, halfSize, halfSize, halfSize, 1.0f, 1.0f,
              -halfSize, halfSize, halfSize, 0.0f, 1.0f,

              // 뒷면 (Z-)
              -halfSize, -halfSize, -halfSize, 1.0f, 0.0f, -halfSize, halfSize,
              -halfSize, 1.0f, 1.0f, halfSize, halfSize, -halfSize, 0.0f, 1.0f,
              halfSize, -halfSize, -halfSize, 0.0f, 0.0f,

              // 윗면 (Y+)
              -halfSize, halfSize, -halfSize, 0.0f, 1.0f, -halfSize, halfSize,
              halfSize, 0.0f, 0.0f, halfSize, halfSize, halfSize, 1.0f, 0.0f,
              halfSize, halfSize, -halfSize, 1.0f, 1.0f,

              // 아랫면 (Y-)
              -halfSize, -halfSize, -halfSize, 1.0f, 1.0f, halfSize, -halfSize,
              -halfSize, 0.0f, 1.0f, halfSize, -halfSize, halfSize, 0.0f, 0.0f,
              -halfSize, -halfSize, halfSize, 1.0f, 0.0f,

              // 오른쪽면 (X+)
              halfSize, -halfSize, -halfSize, 1.0f, 0.0f, halfSize, halfSize,
              -halfSize, 1.0f, 1.0f, halfSize, halfSize, halfSize, 0.0f, 1.0f,
              halfSize, -halfSize, halfSize, 0.0f, 0.0f,

              // 왼쪽면 (X-)
              -halfSize, -halfSize, -halfSize, 0.0f, 0.0f, -halfSize, -halfSize,
              halfSize, 1.0f, 0.0f, -halfSize, halfSize, halfSize, 1.0f, 1.0f,
              -halfSize, halfSize, -halfSize, 0.0f, 1.0f};

  // 인덱스 생성 (각 면마다 2개의 삼각형)
  indices = {// 앞면
             0, 1, 2, 2, 3, 0,
             // 뒷면
             4, 5, 6, 6, 7, 4,
             // 윗면
             8, 9, 10, 10, 11, 8,
             // 아랫면
             12, 13, 14, 14, 15, 12,
             // 오른쪽면
             16, 17, 18, 18, 19, 16,
             // 왼쪽면
             20, 21, 22, 22, 23, 20};

  // VAO, VBO, EBO 생성
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
               vertices.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
               indices.data(), GL_STATIC_DRAW);

  // 위치 속성
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  // 텍스처 좌표 속성
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);

  // 노란색-검은색 체크무늬 텍스처 생성
  texture = createYellowBlackCheckerboardTexture();

  // 초기 모델 매트릭스 설정
  updateModelMatrix();

  // 경계 상자 계산
  calculateBounds();

  std::cout << "정육면체 생성 완료: 위치(" << position.x << ", " << position.y
            << ", " << position.z << "), 크기: " << size << std::endl;
}

// 소멸자
Cube::~Cube() { cleanup(); }

// 물리 업데이트
void Cube::update(float deltaTime) {
  // 중력 적용
  velocity.y -= 9.8f * deltaTime;

  // 공기 저항 적용 (X, Z축에만)
  float airResistance = 0.99f;
  velocity.x *= airResistance;
  velocity.z *= airResistance;

  // 위치 업데이트
  position += velocity * deltaTime;

  // 모델 매트릭스 업데이트
  updateModelMatrix();

  // 경계 상자 업데이트
  calculateBounds();
}

// 모델 매트릭스 업데이트
void Cube::updateModelMatrix() {
  modelMatrix = glm::mat4(1.0f);
  modelMatrix = glm::translate(modelMatrix, position);
}

// 경계 상자 계산
void Cube::calculateBounds() {
  float halfSize = size * 0.5f;
  minBounds = position - glm::vec3(halfSize);
  maxBounds = position + glm::vec3(halfSize);
}

// 리소스 정리
void Cube::cleanup() {
  if (VAO) {
    glDeleteVertexArrays(1, &VAO);
    VAO = 0;
  }
  if (VBO) {
    glDeleteBuffers(1, &VBO);
    VBO = 0;
  }
  if (EBO) {
    glDeleteBuffers(1, &EBO);
    EBO = 0;
  }
  if (texture) {
    glDeleteTextures(1, &texture);
    texture = 0;
  }
}
