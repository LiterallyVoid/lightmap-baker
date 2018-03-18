/* -*- mode: c++ -*- */
#pragma once

#include <string>
#include <vector>

#include "texture.h"

class Compute {
public:
  Compute();
  Compute(std::string path);
  ~Compute();

  void setData(int num, unsigned long size, void *data);
  std::vector<float> getData();
  void execute(int sizeX, int sizeY, int sizeZ);

private:
  GLuint program;
  GLuint ssboIn1, ssboIn2, texOut;
};
