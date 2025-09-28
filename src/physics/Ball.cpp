#include "Ball.h"
#include "../utils/TextureUtils.h"
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

Ball::Ball(glm::vec3 pos, float r, float m)
    : position(pos), radius(r), mass(m) {
  velocity = glm::vec3(0.0f);
  angularVelocity = glm::vec3(0.0f);
  rotation = glm::vec3(0.0f);

  // 구체 메시 생성
  BallMesh::createSphere(vertices, indices, radius);

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

  // 체크무늬 텍스처 생성
  texture = BallMesh::createCheckerboardTexture();

  // 초기 모델 매트릭스
  updateModelMatrix();
}

void Ball::update(float deltaTime) {
  // 중력 적용
  velocity.y += GRAVITY * deltaTime;

  // 위치 업데이트
  position += velocity * deltaTime;

  // 구르는 모션을 위한 각속도 계산 (속도 기반)
  if (glm::length(velocity) > 0.1f) {
    // 바닥에 닿았을 때만 구르는 각속도 적용
    if (position.y <= radius + 0.1f) { // 바닥 근처에 있을 때
      float rollingSpeed =
          glm::length(glm::vec2(velocity.x, velocity.z)) / radius;
      angularVelocity.x = -velocity.z * rollingSpeed; // Z 방향 이동 -> X축 회전
      angularVelocity.z = velocity.x * rollingSpeed; // X 방향 이동 -> Z축 회전
    }
  }

  // 회전 업데이트
  rotation += angularVelocity * deltaTime;

  // 각속도 감쇠 (공기 저항)
  angularVelocity *= AIR_RESISTANCE;

  // 속도 감쇠 (마찰) - 바닥에 닿았을 때만
  if (position.y <= radius + 0.1f) {
    velocity *= FRICTION;
  }

  // 모델 매트릭스 업데이트
  updateModelMatrix();
}

void Ball::handleCollision(const glm::vec3 &normal, float restitution) {
  // 반사 벡터 계산
  float dotProduct = glm::dot(velocity, normal);
  if (dotProduct < 0) { // 접근 중일 때만 반사
    velocity = velocity - (1 + restitution) * dotProduct * normal;
  }

  // 각속도도 반사 (단순화된 모델)
  angularVelocity = glm::reflect(angularVelocity, normal) * 0.5f;
}

void Ball::updateModelMatrix() {
  modelMatrix = glm::mat4(1.0f);
  modelMatrix = glm::translate(modelMatrix, position);

  // 회전 적용 (누적 회전 기반)
  if (glm::length(rotation) > 0.001f) {
    float angle = glm::length(rotation);
    glm::vec3 axis = glm::normalize(rotation);
    modelMatrix = glm::rotate(modelMatrix, angle, axis);
  }
}

void Ball::cleanup() {
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO);
  glDeleteTextures(1, &texture);
}

// 구체 메시 생성 (구면 좌표계 사용)
void BallMesh::createSphere(std::vector<float> &vertices,
                            std::vector<unsigned int> &indices, float radius,
                            int sectors, int stacks) {
  vertices.clear();
  indices.clear();

  float sectorStep = 2 * M_PI / sectors;
  float stackStep = M_PI / stacks;
  float sectorAngle, stackAngle;

  // 버텍스 생성
  for (int i = 0; i <= stacks; ++i) {
    stackAngle = M_PI / 2 - i * stackStep;
    float xy = radius * cosf(stackAngle);
    float z = radius * sinf(stackAngle);

    for (int j = 0; j <= sectors; ++j) {
      sectorAngle = j * sectorStep;

      float x = xy * cosf(sectorAngle);
      float y = xy * sinf(sectorAngle);

      // 위치
      vertices.push_back(x);
      vertices.push_back(y);
      vertices.push_back(z);

      // 텍스처 좌표 (구면 매핑)
      float u = (float)j / sectors;
      float v = (float)i / stacks;
      vertices.push_back(u);
      vertices.push_back(v);
    }
  }

  // 인덱스 생성
  for (int i = 0; i < stacks; ++i) {
    int k1 = i * (sectors + 1);
    int k2 = k1 + sectors + 1;

    for (int j = 0; j < sectors; ++j, ++k1, ++k2) {
      if (i != 0) {
        indices.push_back(k1);
        indices.push_back(k2);
        indices.push_back(k1 + 1);
      }

      if (i != (stacks - 1)) {
        indices.push_back(k1 + 1);
        indices.push_back(k2);
        indices.push_back(k2 + 1);
      }
    }
  }
}

// 체크무늬 텍스처 생성
unsigned int BallMesh::createCheckerboardTexture() {
  const int width = 64;
  const int height = 64;
  std::vector<unsigned char> data(width * height * 3);

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      int index = (y * width + x) * 3;

      // 체크무늬 패턴
      bool isWhite = ((x / 8) + (y / 8)) % 2 == 0;

      if (isWhite) {
        data[index] = 255;     // R
        data[index + 1] = 255; // G
        data[index + 2] = 255; // B
      } else {
        data[index] = 0;     // R
        data[index + 1] = 0; // G
        data[index + 2] = 0; // B
      }
    }
  }

  unsigned int texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
               GL_UNSIGNED_BYTE, data.data());

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  return texture;
}
