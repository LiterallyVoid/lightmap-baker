/* -*- mode: c++ -*- */
#pragma once

#include <string>
#include <vector>

#include "texture.h"
#include "compute.h"

class Map {
public:
  Map(std::string path, std::string outPath);
  ~Map();

  void draw();
  void calcNormals();
  void calcLightmapUV();
  void compileLightmap();

  bool rayFaceCollision(int face, double start[3], double end[3], double &time);
  bool castRay(double start[3], double end[3], double &minTime, int ignore);

public:
  struct Vertex {
    double pos[3];
    double lightmapUV[2];
    double uv[2];
  };

  struct Face {
    int side;
    double normal[3];
    double distance;
    std::vector<Vertex> vertices;
  };
  
  struct Box {
    double x, y, w, h;
    double x2, y2;
    int face;
    bool swap;
  };

private:
  std::string outPath;
  std::vector<Box> boxes;
  std::vector<Face> faces;
  unsigned char data[1024][1024][4];
  
  Texture *t;
};
