#pragma once

#include "Ball.h"
#include <glm/glm.hpp>
#include <vector>

// 전방 선언
class Cube;

// 충돌 감지 및 처리 클래스
class CollisionSystem {
public:
  // 공과 AABB 충돌 감지 (바닥, 벽)
  static bool checkBallAABBCollision(const Ball &ball,
                                     const glm::vec3 &minBounds,
                                     const glm::vec3 &maxBounds);

  // 공과 카메라 충돌 감지
  static bool checkBallCameraCollision(const Ball &ball,
                                       const glm::vec3 &cameraPos,
                                       float cameraRadius = 0.5f);

  // 공과 공 충돌 감지
  static bool checkBallBallCollision(const Ball &ball1, const Ball &ball2);

  // 충돌 반응 처리
  static void handleBallAABBCollision(Ball &ball, const glm::vec3 &minBounds,
                                      const glm::vec3 &maxBounds);
  static void handleBallCameraCollision(Ball &ball, const glm::vec3 &cameraPos,
                                        float cameraRadius = 0.5f);
  static void handleBallBallCollision(Ball &ball1, Ball &ball2);

  // 공과 방 경계 충돌 처리
  static void handleRoomCollisions(Ball &ball,
                                   const std::vector<struct Mesh> &walls,
                                   const struct Mesh &floor);

  // 정육면체와 AABB 충돌 감지
  static bool checkCubeAABBCollision(const Cube &cube,
                                     const glm::vec3 &minBounds,
                                     const glm::vec3 &maxBounds);

  // 정육면체와 방 경계 충돌 처리
  static void handleCubeRoomCollisions(Cube &cube,
                                       const std::vector<struct Mesh> &walls,
                                       const struct Mesh &floor);

  // 정육면체와 구 충돌 감지
  static bool checkCubeBallCollision(const Cube &cube, const Ball &ball);

  // 정육면체와 카메라 충돌 감지
  static bool checkCubeCameraCollision(const Cube &cube,
                                       const glm::vec3 &cameraPos,
                                       float cameraRadius = 0.5f);

  // 정육면체와 구 충돌 처리
  static void handleCubeBallCollision(Cube &cube, Ball &ball);

  // 정육면체와 카메라 충돌 처리
  static void handleCubeCameraCollision(Cube &cube, const glm::vec3 &cameraPos,
                                        float cameraRadius = 0.5f);

private:
  // AABB와 구 충돌 감지 헬퍼 함수
  static glm::vec3 closestPointOnAABB(const glm::vec3 &point,
                                      const glm::vec3 &minBounds,
                                      const glm::vec3 &maxBounds);
  static float distanceToAABB(const glm::vec3 &point,
                              const glm::vec3 &minBounds,
                              const glm::vec3 &maxBounds);
};
