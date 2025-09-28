#pragma once

#include <glad/glad.h>

// 텍스처 유틸리티 함수들
unsigned int createCheckerboardTexture(int width = 64, int height = 64);
unsigned int createYellowBlackCheckerboardTexture(int width = 64,
                                                  int height = 64);
