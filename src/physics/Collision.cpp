#include "Collision.h"
#include "../mesh/Cube.h"
#include "../mesh/Mesh.h"
#include <algorithm>
#include <iostream>

// 공과 AABB 충돌 감지
bool CollisionSystem::checkBallAABBCollision(const Ball &ball,
                                             const glm::vec3 &minBounds,
                                             const glm::vec3 &maxBounds) {
  glm::vec3 closestPoint =
      closestPointOnAABB(ball.position, minBounds, maxBounds);
  float distance = glm::length(ball.position - closestPoint);
  return distance < ball.radius;
}

// 공과 카메라 충돌 감지
bool CollisionSystem::checkBallCameraCollision(const Ball &ball,
                                               const glm::vec3 &cameraPos,
                                               float cameraRadius) {
  float distance = glm::length(ball.position - cameraPos);
  return distance < (ball.radius + cameraRadius);
}

// 공과 공 충돌 감지
bool CollisionSystem::checkBallBallCollision(const Ball &ball1,
                                             const Ball &ball2) {
  float distance = glm::length(ball1.position - ball2.position);
  return distance < (ball1.radius + ball2.radius);
}

// AABB와 가장 가까운 점 찾기
glm::vec3 CollisionSystem::closestPointOnAABB(const glm::vec3 &point,
                                              const glm::vec3 &minBounds,
                                              const glm::vec3 &maxBounds) {
  glm::vec3 closest;
  closest.x = std::max(minBounds.x, std::min(point.x, maxBounds.x));
  closest.y = std::max(minBounds.y, std::min(point.y, maxBounds.y));
  closest.z = std::max(minBounds.z, std::min(point.z, maxBounds.z));
  return closest;
}

// AABB까지의 거리 계산
float CollisionSystem::distanceToAABB(const glm::vec3 &point,
                                      const glm::vec3 &minBounds,
                                      const glm::vec3 &maxBounds) {
  glm::vec3 closest = closestPointOnAABB(point, minBounds, maxBounds);
  return glm::length(point - closest);
}

// 공과 AABB 충돌 반응 처리
void CollisionSystem::handleBallAABBCollision(Ball &ball,
                                              const glm::vec3 &minBounds,
                                              const glm::vec3 &maxBounds) {
  glm::vec3 closestPoint =
      closestPointOnAABB(ball.position, minBounds, maxBounds);
  glm::vec3 normal = glm::normalize(ball.position - closestPoint);

  // 공이 AABB 안에 완전히 들어간 경우를 처리
  if (glm::length(ball.position - closestPoint) < 0.001f) {
    // 가장 가까운 면을 찾아서 반사
    float distToMinX = ball.position.x - minBounds.x;
    float distToMaxX = maxBounds.x - ball.position.x;
    float distToMinY = ball.position.y - minBounds.y;
    float distToMaxY = maxBounds.y - ball.position.y;
    float distToMinZ = ball.position.z - minBounds.z;
    float distToMaxZ = maxBounds.z - ball.position.z;

    float minDist = std::min({distToMinX, distToMaxX, distToMinY, distToMaxY,
                              distToMinZ, distToMaxZ});

    if (minDist == distToMinX)
      normal = glm::vec3(-1, 0, 0);
    else if (minDist == distToMaxX)
      normal = glm::vec3(1, 0, 0);
    else if (minDist == distToMinY)
      normal = glm::vec3(0, -1, 0);
    else if (minDist == distToMaxY)
      normal = glm::vec3(0, 1, 0);
    else if (minDist == distToMinZ)
      normal = glm::vec3(0, 0, -1);
    else if (minDist == distToMaxZ)
      normal = glm::vec3(0, 0, 1);
  }

  // 공을 AABB 표면으로 이동
  ball.position = closestPoint + normal * ball.radius;

  // 속도 반사
  ball.handleCollision(normal, 0.7f);
}

// 공과 카메라 충돌 반응 처리
void CollisionSystem::handleBallCameraCollision(Ball &ball,
                                                const glm::vec3 &cameraPos,
                                                float cameraRadius) {
  glm::vec3 normal = glm::normalize(ball.position - cameraPos);

  // 공을 카메라에서 충분히 멀리 이동
  ball.position = cameraPos + normal * (ball.radius + cameraRadius + 0.1f);

  // 카메라의 움직임에 따라 공에 속도 추가 (카메라가 공을 밀어내는 효과)
  float pushForce = 5.0f; // 밀어내는 힘
  ball.velocity += normal * pushForce;

  // 속도 반사
  ball.handleCollision(normal, 0.8f);

  std::cout << "Ball-Camera collision! Ball pos: (" << ball.position.x << ", "
            << ball.position.y << ", " << ball.position.z << ")" << std::endl;
}

// 공과 공 충돌 반응 처리
void CollisionSystem::handleBallBallCollision(Ball &ball1, Ball &ball2) {
  glm::vec3 normal = glm::normalize(ball1.position - ball2.position);

  // 공들을 분리
  float overlap = ball1.radius + ball2.radius -
                  glm::length(ball1.position - ball2.position);
  glm::vec3 separation = normal * overlap * 0.5f;
  ball1.position += separation;
  ball2.position -= separation;

  // 속도 교환 (탄성 충돌)
  glm::vec3 relativeVelocity = ball1.velocity - ball2.velocity;
  float velocityAlongNormal = glm::dot(relativeVelocity, normal);

  if (velocityAlongNormal > 0)
    return; // 이미 분리 중

  float restitution = 0.8f;
  float impulse = -(1 + restitution) * velocityAlongNormal;
  impulse /= (1.0f / ball1.mass + 1.0f / ball2.mass);

  glm::vec3 impulseVector = impulse * normal;
  ball1.velocity += impulseVector / ball1.mass;
  ball2.velocity -= impulseVector / ball2.mass;
}

// 공과 방 경계 충돌 처리
void CollisionSystem::handleRoomCollisions(
    Ball &ball, const std::vector<struct Mesh> &walls,
    const struct Mesh &floor) {
  // 바닥과의 충돌
  if (checkBallAABBCollision(ball, floor.minBounds, floor.maxBounds)) {
    handleBallAABBCollision(ball, floor.minBounds, floor.maxBounds);
  }

  // 벽들과의 충돌
  for (const auto &wall : walls) {
    if (checkBallAABBCollision(ball, wall.minBounds, wall.maxBounds)) {
      handleBallAABBCollision(ball, wall.minBounds, wall.maxBounds);
    }
  }
}

// 정육면체와 AABB 충돌 감지
bool CollisionSystem::checkCubeAABBCollision(const Cube &cube,
                                             const glm::vec3 &minBounds,
                                             const glm::vec3 &maxBounds) {
  return (cube.minBounds.x <= maxBounds.x && cube.maxBounds.x >= minBounds.x) &&
         (cube.minBounds.y <= maxBounds.y && cube.maxBounds.y >= minBounds.y) &&
         (cube.minBounds.z <= maxBounds.z && cube.maxBounds.z >= minBounds.z);
}

// 정육면체와 방 경계 충돌 처리
void CollisionSystem::handleCubeRoomCollisions(
    Cube &cube, const std::vector<struct Mesh> &walls,
    const struct Mesh &floor) {

  // 바닥과의 충돌 (정육면체가 바닥 아래로 떨어지는 경우)
  if (cube.position.y - cube.size * 0.5f <= floor.maxBounds.y) {
    // 정육면체를 바닥 위로 이동
    cube.position.y = floor.maxBounds.y + cube.size * 0.5f;
    cube.velocity.y = 0.0f; // Y 속도 제거

    // 마찰력 적용 (X, Z 속도 감소)
    cube.velocity.x *= 0.95f;
    cube.velocity.z *= 0.95f;
  }

  // 벽들과의 충돌
  for (const auto &wall : walls) {
    // 정육면체가 벽과 겹치는지 확인
    if (checkCubeAABBCollision(cube, wall.minBounds, wall.maxBounds)) {
      // 정육면체를 벽에서 밀어냄
      glm::vec3 cubeCenter = cube.position;
      glm::vec3 wallCenter = (wall.minBounds + wall.maxBounds) * 0.5f;
      glm::vec3 direction = cubeCenter - wallCenter;

      // 가장 가까운 축을 찾아서 반사
      float distX = std::abs(direction.x);
      float distY = std::abs(direction.y);
      float distZ = std::abs(direction.z);

      if (distX >= distY && distX >= distZ) {
        // X축 충돌
        if (direction.x > 0) {
          cube.position.x = wall.maxBounds.x + cube.size * 0.5f;
        } else {
          cube.position.x = wall.minBounds.x - cube.size * 0.5f;
        }
        cube.velocity.x *= -0.7f; // X 속도 반사
      } else if (distY >= distX && distY >= distZ) {
        // Y축 충돌
        if (direction.y > 0) {
          cube.position.y = wall.maxBounds.y + cube.size * 0.5f;
        } else {
          cube.position.y = wall.minBounds.y - cube.size * 0.5f;
        }
        cube.velocity.y *= -0.7f; // Y 속도 반사
      } else {
        // Z축 충돌
        if (direction.z > 0) {
          cube.position.z = wall.maxBounds.z + cube.size * 0.5f;
        } else {
          cube.position.z = wall.minBounds.z - cube.size * 0.5f;
        }
        cube.velocity.z *= -0.7f; // Z 속도 반사
      }
    }
  }

  // 방 경계를 벗어나는 경우 강제로 되돌리기
  float roomSize = 10.0f;
  float halfSize = cube.size * 0.5f;

  // X축 경계 체크
  if (cube.position.x - halfSize < -roomSize) {
    cube.position.x = -roomSize + halfSize;
    cube.velocity.x = std::abs(cube.velocity.x) * 0.7f; // 반대 방향으로 반사
  } else if (cube.position.x + halfSize > roomSize) {
    cube.position.x = roomSize - halfSize;
    cube.velocity.x = -std::abs(cube.velocity.x) * 0.7f; // 반대 방향으로 반사
  }

  // Z축 경계 체크
  if (cube.position.z - halfSize < -roomSize) {
    cube.position.z = -roomSize + halfSize;
    cube.velocity.z = std::abs(cube.velocity.z) * 0.7f; // 반대 방향으로 반사
  } else if (cube.position.z + halfSize > roomSize) {
    cube.position.z = roomSize - halfSize;
    cube.velocity.z = -std::abs(cube.velocity.z) * 0.7f; // 반대 방향으로 반사
  }

  // Y축 경계 체크 (천장)
  if (cube.position.y + halfSize > 20.0f) {
    cube.position.y = 20.0f - halfSize;
    cube.velocity.y = -std::abs(cube.velocity.y) * 0.7f; // 아래로 반사
  }
}

// 정육면체와 구 충돌 감지
bool CollisionSystem::checkCubeBallCollision(const Cube &cube,
                                             const Ball &ball) {
  // 구의 중심이 정육면체의 AABB 내부에 있는지 확인
  glm::vec3 closestPoint =
      closestPointOnAABB(ball.position, cube.minBounds, cube.maxBounds);
  float distance = glm::length(ball.position - closestPoint);
  return distance < ball.radius;
}

// 정육면체와 카메라 충돌 감지
bool CollisionSystem::checkCubeCameraCollision(const Cube &cube,
                                               const glm::vec3 &cameraPos,
                                               float cameraRadius) {
  // 카메라를 구로 가정하고 정육면체와의 충돌 감지
  glm::vec3 closestPoint =
      closestPointOnAABB(cameraPos, cube.minBounds, cube.maxBounds);
  float distance = glm::length(cameraPos - closestPoint);
  return distance < cameraRadius;
}

// 정육면체와 구 충돌 처리
void CollisionSystem::handleCubeBallCollision(Cube &cube, Ball &ball) {
  // 구의 중심이 정육면체의 AABB 내부에 있는지 확인
  glm::vec3 closestPoint =
      closestPointOnAABB(ball.position, cube.minBounds, cube.maxBounds);
  glm::vec3 normal = glm::normalize(ball.position - closestPoint);

  // 구를 정육면체에서 밀어냄
  float overlap = ball.radius - glm::length(ball.position - closestPoint);
  if (overlap > 0) {
    ball.position = closestPoint + normal * ball.radius;

    // 구에 반사 속도 적용
    ball.handleCollision(normal, 0.8f);

    // 정육면체도 반대 방향으로 밀려나게 함
    glm::vec3 cubeForce = -normal * 2.0f; // 정육면체에 힘 적용
    cube.velocity += cubeForce;
  }
}

// 정육면체와 카메라 충돌 처리
void CollisionSystem::handleCubeCameraCollision(Cube &cube,
                                                const glm::vec3 &cameraPos,
                                                float cameraRadius) {
  // 카메라를 구로 가정하고 정육면체와의 충돌 감지
  glm::vec3 closestPoint =
      closestPointOnAABB(cameraPos, cube.minBounds, cube.maxBounds);
  glm::vec3 normal = glm::normalize(cameraPos - closestPoint);

  // 정육면체를 카메라에서 밀어냄
  float overlap = cameraRadius - glm::length(cameraPos - closestPoint);
  if (overlap > 0) {
    // 정육면체를 카메라에서 충분히 멀리 이동
    cube.position =
        cameraPos + normal * (cube.size * 0.5f + cameraRadius + 0.1f);

    // 정육면체에 반사 속도 적용
    cube.velocity += normal * 3.0f; // 카메라가 정육면체를 밀어내는 효과
  }
}
