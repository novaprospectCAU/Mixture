#include <SDL2/SDL.h>
#include <cmath>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <string>
#include <vector>

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
         float pitch = 0.0f)
      : front(glm::vec3(0.0f, 0.0f, -1.0f)), movementSpeed(130.0f),
        mouseSensitivity(0.25f), zoom(45.0f), acceleration(50.0f),
        friction(10.0f) {
    this->position = position;
    this->worldUp = up;
    this->yaw = yaw;
    this->pitch = pitch;
    this->velocity = glm::vec3(10.0f);
    updateCameraVectors();
  }

  glm::mat4 getViewMatrix() {
    return glm::lookAt(position, position + front, up);
  }

  void processKeyboard(int direction, float deltaTime) {
    if (direction == 0) // W
      this->velocity += front * acceleration * deltaTime;
    if (direction == 1) // S
      this->velocity -= front * acceleration * deltaTime;
    if (direction == 2) // A
      this->velocity -= right * acceleration * deltaTime;
    if (direction == 3) // D
      this->velocity += right * acceleration * deltaTime;
  }

  void processMouseMovement(float xoffset, float yoffset,
                            bool constrainPitch = true) {
    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (constrainPitch) {
      if (pitch > 89.0f)
        pitch = 89.0f;
      if (pitch < -89.0f)
        pitch = -89.0f;
    }

    updateCameraVectors();
  }

  void update(float deltaTime) {
    // 마찰 적용
    velocity *= (1.0f - friction * deltaTime);

    // 최대 속도 제한
    float maxSpeed = movementSpeed * 2.0f;
    if (glm::length(velocity) > maxSpeed) {
      velocity = glm::normalize(velocity) * maxSpeed;
    }

    // 위치 업데이트
    position += velocity * deltaTime;
  }

  // 충돌 감지 후 위치 복원 (전방 선언 필요)
  void handleCollision(const std::vector<struct Mesh> &meshes);

private:
  void updateCameraVectors() {
    glm::vec3 newFront;
    newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    newFront.y = sin(glm::radians(pitch));
    newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(newFront);

    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
  }
};

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

// 셰이더 소스 코드
const char *vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoord;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
}
)";

const char *fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D texture1;

void main() {
    FragColor = texture(texture1, TexCoord);
}
)";

// 체크무늬 텍스처 생성
unsigned int createCheckerboardTexture(int width = 64, int height = 64) {
  unsigned int texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  std::vector<unsigned char> data(width * height * 3);
  int checkerSize = 8;

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      int checkerX = (x / checkerSize) % 2;
      int checkerY = (y / checkerSize) % 2;
      bool isWhite = (checkerX + checkerY) % 2 == 0;

      int index = (y * width + x) * 3;
      unsigned char color = isWhite ? 255 : 0;
      data[index] = color;     // R
      data[index + 1] = color; // G
      data[index + 2] = color; // B
    }
  }

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
               GL_UNSIGNED_BYTE, data.data());
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  return texture;
}

// 셰이더 컴파일
unsigned int compileShader(const char *source, GLenum type) {
  unsigned int shader = glCreateShader(type);
  glShaderSource(shader, 1, &source, nullptr);
  glCompileShader(shader);

  int success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    char infoLog[512];
    glGetShaderInfoLog(shader, 512, nullptr, infoLog);
    std::cerr << "셰이더 컴파일 실패: " << infoLog << std::endl;
  }

  return shader;
}

// 셰이더 프로그램 생성
unsigned int createShaderProgram() {
  unsigned int vertexShader =
      compileShader(vertexShaderSource, GL_VERTEX_SHADER);
  unsigned int fragmentShader =
      compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);

  unsigned int shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);

  int success;
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
  if (!success) {
    char infoLog[512];
    glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
    std::cerr << "셰이더 프로그램 링크 실패: " << infoLog << std::endl;
  }

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  return shaderProgram;
}

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
}

// 카메라 충돌 처리 구현
void Camera::handleCollision(const std::vector<Mesh> &meshes) {
  for (const auto &mesh : meshes) {
    if (checkCameraCollision(*this, mesh)) {
      // 충돌이 감지되면 이전 위치로 되돌림
      position -= velocity * 0.016f; // 대략적인 deltaTime 사용
      velocity = glm::vec3(0.0f);    // 속도 초기화
      break;
    }
  }
}

// 바닥 메시 생성
Mesh createFloor() {
  Mesh floor;
  floor.name = "floor";

  // 바닥 버텍스 (XZ 평면) - 크기를 2배로 늘림
  floor.vertices = {// 위치 (x, y, z)    텍스처 좌표 (u, v)
                    -10.0f, 0.0f,   -10.0f, 0.0f,  0.0f, 10.0f, 0.0f,
                    -10.0f, 1.0f,   0.0f,   10.0f, 0.0f, 10.0f, 1.0f,
                    1.0f,   -10.0f, 0.0f,   10.0f, 0.0f, 1.0f};

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
                   -10.0f, 0.0f,   0.0f, 0.0f,  0.0f,  10.0f, 0.0f,
                   0.0f,   2.0f,   0.0f, 10.0f, 10.0f, 0.0f,  2.0f,
                   2.0f,   -10.0f, 0.0f, 10.0f, 0.0f,  2.0f};

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

  // 마우스 관련 변수 (상대 마우스 모드 사용으로 간소화)

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