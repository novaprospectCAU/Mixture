#include "TextureUtils.h"
#include <vector>

// 체크무늬 텍스처 생성
unsigned int createCheckerboardTexture(int width, int height) {
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

// 노란색-검은색 체크무늬 텍스처 생성
unsigned int createYellowBlackCheckerboardTexture(int width, int height) {
  unsigned int texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  std::vector<unsigned char> data(width * height * 3);
  int checkerSize = 8;

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      int checkerX = (x / checkerSize) % 2;
      int checkerY = (y / checkerSize) % 2;
      bool isYellow = (checkerX + checkerY) % 2 == 0;

      int index = (y * width + x) * 3;
      if (isYellow) {
        data[index] = 255;     // R (노란색)
        data[index + 1] = 255; // G (노란색)
        data[index + 2] = 0;   // B (노란색)
      } else {
        data[index] = 0;     // R (검은색)
        data[index + 1] = 0; // G (검은색)
        data[index + 2] = 0; // B (검은색)
      }
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
