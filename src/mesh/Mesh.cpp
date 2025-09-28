#include "Mesh.h"
#include "../camera/Camera.h"
#include "../utils/TextureUtils.h"
#include <algorithm>
#include <iostream>

// 충돌 감지 함수들
bool checkAABBCollision(const glm::vec3 &pos1, const glm::vec3 &min1,
                        const glm::vec3 &max1, const glm::vec3 &pos2,
                        const glm::vec3 &min2, const glm::vec3 &max2) {
  glm::vec3 worldMin1 = pos1 + min1;
  glm::vec3 worldMax1 = pos1 + max1;
  glm::vec3 worldMin2 = pos2 + min2;
  glm::vec3 worldMax2 = pos2 + max2;

  return (worldMin1.x <= worldMax2.x && worldMax1.x >= worldMin2.x) &&
         (worldMin1.y <= worldMax2.y && worldMax1.y >= worldMin2.y) &&
         (worldMin1.z <= worldMax2.z && worldMax1.z >= worldMin2.z);
}

// 카메라와 메시 간의 충돌 감지
bool checkCameraCollision(const Camera &camera, const Mesh &mesh) {
  // 카메라를 작은 구로 가정 (반지름 0.5)
  glm::vec3 cameraMin = camera.position - glm::vec3(0.5f);
  glm::vec3 cameraMax = camera.position + glm::vec3(0.5f);

  // 월드 좌표로 변환된 경계 상자와 직접 비교
  return (cameraMin.x <= mesh.maxBounds.x && cameraMax.x >= mesh.minBounds.x) &&
         (cameraMin.y <= mesh.maxBounds.y && cameraMax.y >= mesh.minBounds.y) &&
         (cameraMin.z <= mesh.maxBounds.z && cameraMax.z >= mesh.minBounds.z);
}

// 경계 상자 계산 함수 (모델 변환 적용)
void calculateBounds(Mesh &mesh) {
  if (mesh.vertices.empty())
    return;

  // 모든 버텍스를 월드 좌표로 변환
  std::vector<glm::vec3> worldVertices;
  for (size_t i = 0; i < mesh.vertices.size(); i += 5) {
    glm::vec3 localPos(mesh.vertices[i], mesh.vertices[i + 1],
                       mesh.vertices[i + 2]);
    glm::vec4 worldPos = mesh.modelMatrix * glm::vec4(localPos, 1.0f);
    worldVertices.push_back(glm::vec3(worldPos));
  }

  if (worldVertices.empty())
    return;

  float minX = worldVertices[0].x, maxX = worldVertices[0].x;
  float minY = worldVertices[0].y, maxY = worldVertices[0].y;
  float minZ = worldVertices[0].z, maxZ = worldVertices[0].z;

  for (const auto &vertex : worldVertices) {
    minX = std::min(minX, vertex.x);
    maxX = std::max(maxX, vertex.x);
    minY = std::min(minY, vertex.y);
    maxY = std::max(maxY, vertex.y);
    minZ = std::min(minZ, vertex.z);
    maxZ = std::max(maxZ, vertex.z);
  }

  mesh.minBounds = glm::vec3(minX, minY, minZ);
  mesh.maxBounds = glm::vec3(maxX, maxY, maxZ);

  // 디버그 출력
  std::cout << "Mesh " << mesh.name << " bounds: min(" << minX << ", " << minY
            << ", " << minZ << ") max(" << maxX << ", " << maxY << ", " << maxZ
            << ")" << std::endl;
}

// 바닥 메시 생성
Mesh createFloor() {
  Mesh floor;
  floor.name = "floor";

  // 바닥 버텍스 (XZ 평면) - 크기를 2배로 늘림
  floor.vertices = {// 위치 (x, y, z)    텍스처 좌표 (u, v)
                    -10.0f, 0.0f, -10.0f, 0.0f, 0.0f,  // 왼쪽 아래
                    10.0f,  0.0f, -10.0f, 1.0f, 0.0f,  // 오른쪽 아래
                    10.0f,  0.0f, 10.0f,  1.0f, 1.0f,  // 오른쪽 위
                    -10.0f, 0.0f, 10.0f,  0.0f, 1.0f}; // 왼쪽 위

  floor.indices = {0, 1, 2, 2, 3, 0};

  // VAO, VBO, EBO 생성
  glGenVertexArrays(1, &floor.VAO);
  glGenBuffers(1, &floor.VBO);
  glGenBuffers(1, &floor.EBO);

  glBindVertexArray(floor.VAO);

  glBindBuffer(GL_ARRAY_BUFFER, floor.VBO);
  glBufferData(GL_ARRAY_BUFFER, floor.vertices.size() * sizeof(float),
               floor.vertices.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, floor.EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               floor.indices.size() * sizeof(unsigned int),
               floor.indices.data(), GL_STATIC_DRAW);

  // 위치 속성
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  // 텍스처 좌표 속성
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);

  // 텍스처 생성
  floor.texture = createCheckerboardTexture();

  // 모델 매트릭스 (단위 행렬)
  floor.modelMatrix = glm::mat4(1.0f);

  // 경계 상자 계산
  calculateBounds(floor);

  return floor;
}

// 벽 메시 생성
Mesh createWall(const std::string &name, const glm::vec3 &position,
                const glm::vec3 &rotation) {
  Mesh wall;
  wall.name = name;

  // 벽 버텍스 (XY 평면) - 크기를 2배로 늘림
  wall.vertices = {// 위치 (x, y, z)    텍스처 좌표 (u, v)
                   -10.0f, 0.0f,  0.0f, 0.0f, 0.0f,  // 왼쪽 아래
                   10.0f,  0.0f,  0.0f, 1.0f, 0.0f,  // 오른쪽 아래
                   10.0f,  20.0f, 0.0f, 1.0f, 1.0f,  // 오른쪽 위
                   -10.0f, 20.0f, 0.0f, 0.0f, 1.0f}; // 왼쪽 위

  wall.indices = {0, 1, 2, 2, 3, 0};

  // VAO, VBO, EBO 생성
  glGenVertexArrays(1, &wall.VAO);
  glGenBuffers(1, &wall.VBO);
  glGenBuffers(1, &wall.EBO);

  glBindVertexArray(wall.VAO);

  glBindBuffer(GL_ARRAY_BUFFER, wall.VBO);
  glBufferData(GL_ARRAY_BUFFER, wall.vertices.size() * sizeof(float),
               wall.vertices.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, wall.EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               wall.indices.size() * sizeof(unsigned int), wall.indices.data(),
               GL_STATIC_DRAW);

  // 위치 속성
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  // 텍스처 좌표 속성
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);

  // 텍스처 생성
  wall.texture = createCheckerboardTexture();

  // 모델 매트릭스 (위치와 회전 적용)
  wall.modelMatrix = glm::mat4(1.0f);
  wall.modelMatrix = glm::translate(wall.modelMatrix, position);
  wall.modelMatrix = glm::rotate(wall.modelMatrix, glm::radians(rotation.x),
                                 glm::vec3(1, 0, 0));
  wall.modelMatrix = glm::rotate(wall.modelMatrix, glm::radians(rotation.y),
                                 glm::vec3(0, 1, 0));
  wall.modelMatrix = glm::rotate(wall.modelMatrix, glm::radians(rotation.z),
                                 glm::vec3(0, 0, 1));

  // 경계 상자 계산
  calculateBounds(wall);

  return wall;
}
