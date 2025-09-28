#pragma once

#include <glad/glad.h>
#include <string>

// 셰이더 소스 코드
extern const char *vertexShaderSource;
extern const char *fragmentShaderSource;

// 셰이더 함수들
unsigned int compileShader(const char *source, GLenum type);
unsigned int createShaderProgram();
