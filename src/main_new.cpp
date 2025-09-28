#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>

// 프로젝트 헤더들
#include "camera/Camera.h"
#include "mesh/Mesh.h"
#include "shader/Shader.h"

int main() {
  // SDL2 초기화
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cerr << "SDL2 초기화 실패: " << SDL_GetError() << std::endl;
    return -1;
  }

  // OpenGL 버전 설정 (OpenGL 3.3 Core Profile)
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  // 더블 버퍼링 활성화
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

  // 창 생성
  SDL_Window *window = SDL_CreateWindow(
      "Mixture 3D Project", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800,
      600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

  if (!window) {
    std::cerr << "창 생성 실패: " << SDL_GetError() << std::endl;
    SDL_Quit();
    return -1;
  }

  // OpenGL 컨텍스트 생성
  SDL_GLContext glContext = SDL_GL_CreateContext(window);
  if (!glContext) {
    std::cerr << "OpenGL 컨텍스트 생성 실패: " << SDL_GetError() << std::endl;
    SDL_DestroyWindow(window);
    SDL_Quit();
    return -1;
  }

  // GLAD 초기화
  if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
    std::cerr << "GLAD 초기화 실패" << std::endl;
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return -1;
  }

  // OpenGL 설정
  glEnable(GL_DEPTH_TEST);
  glViewport(0, 0, 800, 600);

  // 마우스 커서 숨기기 및 캡처
  SDL_SetRelativeMouseMode(SDL_TRUE);
  SDL_ShowCursor(SDL_DISABLE);

  std::cout << "OpenGL 버전: " << glGetString(GL_VERSION) << std::endl;
  std::cout << "OpenGL 렌더러: " << glGetString(GL_RENDERER) << std::endl;

  // 셰이더 프로그램 생성
  unsigned int shaderProgram = createShaderProgram();

  // 메시들 생성
  Mesh floor = createFloor();
  std::vector<Mesh> walls;

  // 4면의 벽 생성 - 크기를 2배로 늘림
  walls.push_back(createWall("wall_front", glm::vec3(0, 0, -10),
                             glm::vec3(0, 0, 0))); // 앞쪽 벽
  walls.push_back(createWall("wall_back", glm::vec3(0, 0, 10),
                             glm::vec3(0, 180, 0))); // 뒤쪽 벽
  walls.push_back(createWall("wall_left", glm::vec3(-10, 0, 0),
                             glm::vec3(0, 90, 0))); // 왼쪽 벽
  walls.push_back(createWall("wall_right", glm::vec3(10, 0, 0),
                             glm::vec3(0, -90, 0))); // 오른쪽 벽

  // 카메라 설정 - 방 크기에 맞게 조정
  Camera camera(glm::vec3(0.0f, 3.0f, 15.0f));

  // 투영 매트릭스
  glm::mat4 projection =
      glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

  // 델타타임 계산
  Uint32 lastTime = SDL_GetTicks();

  // 메인 루프
  bool running = true;
  SDL_Event event;

  while (running) {
    // 델타타임 계산
    Uint32 currentTime = SDL_GetTicks();
    float deltaTime = (currentTime - lastTime) / 1000.0f;
    lastTime = currentTime;

    // 이벤트 처리
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT:
        running = false;
        break;
      case SDL_KEYDOWN:
        switch (event.key.keysym.sym) {
        case SDLK_ESCAPE:
          running = false;
          break;
        }
        break;
      case SDL_MOUSEMOTION: {
        // 상대 마우스 모드에서는 xrel, yrel을 직접 사용
        float xoffset = event.motion.xrel;
        float yoffset = -event.motion.yrel; // Y축 반전

        camera.processMouseMovement(xoffset, yoffset);
        break;
      }
      case SDL_WINDOWEVENT:
        if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
          int width = event.window.data1;
          int height = event.window.data2;
          glViewport(0, 0, width, height);
          // 투영 매트릭스 업데이트
          projection = glm::perspective(
              glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
          std::cout << "창 크기 변경: " << width << "x" << height << std::endl;
        }
        break;
      }
    }

    // 키 상태 지속적 체크 (WASD 이동)
    const Uint8 *keyState = SDL_GetKeyboardState(nullptr);
    if (keyState[SDL_SCANCODE_W]) {
      camera.processKeyboard(0, deltaTime); // W
    }
    if (keyState[SDL_SCANCODE_S]) {
      camera.processKeyboard(1, deltaTime); // S
    }
    if (keyState[SDL_SCANCODE_A]) {
      camera.processKeyboard(2, deltaTime); // A
    }
    if (keyState[SDL_SCANCODE_D]) {
      camera.processKeyboard(3, deltaTime); // D
    }

    // 카메라 업데이트
    camera.update(deltaTime);

    // 충돌 감지 및 처리
    std::vector<Mesh> allMeshes;
    allMeshes.push_back(floor);
    allMeshes.insert(allMeshes.end(), walls.begin(), walls.end());
    camera.handleCollision(allMeshes);

    // 배경색 설정 (#aaaaaa)
    glClearColor(0.666f, 0.666f, 0.666f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 셰이더 사용
    glUseProgram(shaderProgram);

    // 유니폼 설정
    glm::mat4 view = camera.getViewMatrix();
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE,
                       &view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1,
                       GL_FALSE, &projection[0][0]);

    // 바닥 렌더링
    glBindVertexArray(floor.VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, floor.texture);
    glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1,
                       GL_FALSE, &floor.modelMatrix[0][0]);
    glDrawElements(GL_TRIANGLES, floor.indices.size(), GL_UNSIGNED_INT, 0);

    // 벽들 렌더링
    for (const auto &wall : walls) {
      glBindVertexArray(wall.VAO);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, wall.texture);
      glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);
      glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1,
                         GL_FALSE, &wall.modelMatrix[0][0]);
      glDrawElements(GL_TRIANGLES, wall.indices.size(), GL_UNSIGNED_INT, 0);
    }

    glBindVertexArray(0);

    // 버퍼 교체
    SDL_GL_SwapWindow(window);
  }

  // 정리
  SDL_GL_DeleteContext(glContext);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
