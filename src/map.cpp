#include <GL/glew.h>
#define GL_GLEXT_PROTOTYPES
#include <GLFW/glfw3.h>

#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <cstring>

#include "util.h"
#include "compute.h"

#include "map.h"

#define LIGHTMAP_SCALE (1.0 / 64.0)
#define LIGHTMAP_PADDING 0.005

struct GLSLFace {
  float vertices[4][4]; // vec4 x4
  float normal[4]; // vec4
  float rect[4]; // vec4
  float oldPos[2]; // vec2
  unsigned int numVertices, side, swap, filler; // unsigned int x4
  float distance; // float
  char d[2];
};

bool sortFaces(const Map::Box& lhs, const Map::Box& rhs) {
  if((lhs.x2 - lhs.x) > (rhs.x2 - rhs.x)) {
    return true;
  }
  return false;
};

Map::Map(std::string path, std::string outPath) : outPath(outPath) {
  std::ifstream file;
  file.open(path);

  if(file.is_open()) {
    std::string line;
    while(std::getline(file, line)) {
      std::stringstream linestream(line);
      int num = 4;
      Face face;
      while(true) {
	Vertex vert;
	for(int j = 0; j < 3; j++) {
	  linestream >> vert.pos[j];
	}
	if(linestream.fail()) {
	  break;
	} else {
	  face.vertices.push_back(vert);
	}
      }
      faces.push_back(face);
    }
  }

  printf("Calculating normals...\n");
  calcNormals();
  printf("Calculating lightmap UVs...\n");
  calcLightmapUV();
  printf("Compiling lightmap...\n");
  compileLightmap();
};

Map::~Map() {
};

void Map::calcNormals() {
  for(int i = 0; i < faces.size(); i++) {
    Face *f = &faces[i];
    double a[3] = {VEC_EXPAND(f->vertices[2].pos)};
    VEC_SUB(a, f->vertices[0].pos);
    double b[3] = {VEC_EXPAND(f->vertices[1].pos)};
    VEC_SUB(b, f->vertices[0].pos);
    double normal[3] = {VEC_CROSS(a, b)};
    VEC_COPY(normal, f->normal);
    VEC_NORMALIZE(f->normal);
    f->distance = VEC_DOT(f->normal, f->vertices[0].pos);
    if(f->normal[1] * f->normal[1] < f->normal[0] * f->normal[0] &&
       f->normal[2] * f->normal[2] < f->normal[0] * f->normal[0]) {
      f->side = 0;
    } else if(f->normal[0] * f->normal[0] < f->normal[1] * f->normal[1] &&
	      f->normal[2] * f->normal[2] < f->normal[1] * f->normal[1]) {
      f->side = 1;
    } else {
      f->side = 2;
    }
  }
};

void Map::calcLightmapUV() {
  for(int i = 0; i < faces.size(); i++) {
    Box box;
    box.face = i;
    box.swap = false;
    for(int j = 0; j < faces[i].vertices.size(); j++) {
      Vertex *v = &faces[i].vertices[j];
      v->lightmapUV[0] = v->pos[faces[i].side == 0 ? 1 : 0] * LIGHTMAP_SCALE;
      v->lightmapUV[1] = v->pos[faces[i].side == 2 ? 1 : 2] * LIGHTMAP_SCALE;
      if(j == 0) {
	box.x = v->lightmapUV[0];
	box.x2 = v->lightmapUV[0];
	box.y = v->lightmapUV[1];
	box.y2 = v->lightmapUV[1];
      } else {
	box.x = std::min(box.x, v->lightmapUV[0]);
	box.x2 = std::max(box.x2, v->lightmapUV[0]);
	box.y = std::min(box.y, v->lightmapUV[1]);
	box.y2 = std::max(box.y2, v->lightmapUV[1]);
      }
    }
    if((box.x2 - box.x) > (box.y2 - box.y)) {
      for(int j = 0; j < faces[i].vertices.size(); j++) {
	Vertex *v = &faces[i].vertices[j];
	double t = v->lightmapUV[0];
	v->lightmapUV[0] = v->lightmapUV[1];
	v->lightmapUV[1] = t;
      }
      double t = box.x;
      box.x = box.y;
      box.y = t;
      t = box.x2;
      box.x2 = box.y2;
      box.y2 = t;
      box.swap = true;
    }
    box.w = box.x2 - box.x;
    box.h = box.y2 - box.y;
    boxes.push_back(box);
  }
  std::sort(boxes.begin(), boxes.end(), &sortFaces);
  double padding = LIGHTMAP_PADDING;
  double x = padding, y = padding;
  double rowSize = 0;
  double rowHeight = 1;
  for(int i = 0; i < boxes.size(); i++) {
    if(y + boxes[i].h > rowHeight - padding) {
      y = padding;
      x += rowSize + padding;
      rowSize = 0;
    }
    rowSize = std::max(rowSize, boxes[i].w);
    double offset[2] = {x - boxes[i].x, y - boxes[i].y};
    for(int j = 0; j < faces[boxes[i].face].vertices.size(); j++) {
      Vertex *v = &faces[boxes[i].face].vertices[j];
      v->lightmapUV[0] += offset[0];
      v->lightmapUV[1] += offset[1];
    }
    boxes[i].x2 = boxes[i].x;
    boxes[i].y2 = boxes[i].y;
    boxes[i].x = x;
    boxes[i].y = y;
    y += boxes[i].h + padding;
  }
};

bool Map::rayFaceCollision(int face, double start[3], double dir[3], double &time) {
  Face *f = &faces[face];
  double startVal = start[0] * f->normal[0] + start[1] * f->normal[1] + start[2] * f->normal[2] - f->distance;
  double normal =
    dir[0] * f->normal[0] +
    dir[1] * f->normal[1] +
    dir[2] * f->normal[2];
  if(normal < 0.0001 && normal > -0.0001) {
    return false;
  }
  time = startVal / -normal;
  if(normal > 0) {
    time = -time;
  }
  double collision[3] = {
    start[0] + dir[0] * time,
    start[1] + dir[1] * time,
    start[2] + dir[2] * time,
  };
  int p1 = f->side == 0 ? 1 : 0;
  int p2 = f->side == 2 ? 1 : 2;

  for(int i = 0; i < f->vertices.size(); i++) {
    int j = (i == f->vertices.size() - 1 ? 0 : i + 1);
    double a[2] = {
      f->vertices[i].pos[p1],
      f->vertices[i].pos[p2],
    };
    double b[2] = {
      f->vertices[j].pos[p1],
      f->vertices[j].pos[p2],
    };
    double n[2] = {
      a[1] - b[1],
      b[0] - a[0]
    };
    double dst = n[0] * a[0] + n[1] * a[1];
    double pt = (collision[p1] * n[0] + collision[p2] * n[1]);
    if(pt < dst) {
      return false;
    }
  }
  return true;
};

bool Map::castRay(double start[3], double end[3], double &minTime, int ignore) {
  minTime = -1;
  double dir[3] = {
    end[0] - start[0],
    end[1] - start[1],
    end[2] - start[2]
  };
  for(int i = 0; i < faces.size(); i++) {
    if(i == ignore) {
      continue;
    }
    double time;
    if(rayFaceCollision(i, start, dir, time)) {
      if(time > 0 && (time < minTime || minTime < 0)) {
	minTime = time;
      }
    }
  }
  return minTime > -0.5;
};

void Map::compileLightmap() {  
  /*
  for(int x = 0; x < 1024; x++) {
    double tx = (double) x / 1024.0;
    for(int y = 0; y < 1024; y++) {
      double ty = (double) y / 1024.0;
      double brightness = 0;
      for(int i = 0; i < boxes.size(); i++) {
	if(tx > boxes[i].x - padding && tx < boxes[i].x + boxes[i].w + padding &&
	   ty > boxes[i].y - padding && ty < boxes[i].y + boxes[i].h + padding) {
	  Face *f = &faces[boxes[i].face];
	  double pos[3];
	  int p1 = f->side == 0 ? 1 : 0;
	  int p2 = f->side == 2 ? 1 : 2;
	  pos[p1] = (tx + (boxes[i].x2 - boxes[i].x)) / LIGHTMAP_SCALE;
	  pos[p2] = (ty + (boxes[i].y2 - boxes[i].y)) / LIGHTMAP_SCALE;
	  if(boxes[i].swap) {
	    double t = pos[p1];
	    pos[p1] = pos[p2];
	    pos[p2] = t;
	  }
	  pos[f->side] = (f->distance - (f->normal[p1] * pos[p1] + f->normal[p2] * pos[p2])) / f->normal[f->side];
	  brightness = 16;
	  for(double x = -0.1; x <= 0.1; x += 0.05) {
	    for(double y = -0.1; y <= 0.1; y += 0.05) {
	      for(double z = -0.1; z <= 0.1; z += 0.05) {
		double sunDiff[3] = {
		  0.408248 + x * 3,
		  0.408248 + y * 3,
		  0.816496 + z * 3
		};
		double light = -VEC_DOT(sunDiff, faces[boxes[i].face].normal);
		if(light > 0) {
		  double sunPos[3] = {
		    pos[0] + sunDiff[0] * 50.0,
		    pos[1] + sunDiff[1] * 50.0,
		    pos[2] + sunDiff[2] * 50.0
		  };
		  double minTime;
		  if(!castRay(pos, sunPos, minTime, boxes[i].face)) {
		    brightness += light * 0.8;
		  }
		}
	      }
	    }
	  }
	  for(double x = -1.0; x <= 1.0; x += 0.2) {
	    for(double y = -1.0; y <= 1.0; y += 0.2) {
	      for(double z = -1.0; z <= 1.0; z += 0.2) {
		double sunDiff[3] = {x, y, z};
		double light = -VEC_DOT(sunDiff, f->normal);
		if(light > 0) {
		  double sunPos[3] = {
		    pos[0] + sunDiff[0] * 1.2,
		    pos[1] + sunDiff[1] * 1.2,
		    pos[2] + sunDiff[2] * 1.2
		  };
		  double minTime = -1.0;
		  if(!castRay(pos, sunPos, minTime, boxes[i].face)) {
		    minTime = 1.0;
		  }
		  if(minTime > 1.0 || minTime < 0) {
		    minTime = 1.0;
		  }
		  brightness += light * minTime * 0.3;
		}
	      }
	    }
	  }
	  if(brightness < 0) {
	    brightness = 0;
	  }
	  if(brightness > 255) {
	    brightness = 255;
	  }
	  break;
	}
      }
      data[y][x][0] = brightness;
      data[y][x][1] = brightness;
      data[y][x][2] = brightness;
      data[y][x][3] = 255;
    }
    }*/
  double padding = LIGHTMAP_PADDING * 0.5;
  std::string path("shaders/lightmap.comp");
  Compute compute(path);
  struct DataType {
    unsigned int numFaces, a, b, c;
  };

  DataType *in1 = (DataType*) malloc(sizeof(DataType) + sizeof(GLSLFace) * boxes.size());

  GLSLFace *gfaces = (GLSLFace*) (in1 + 1);
  
  in1->numFaces = boxes.size();
  for(int i = 0; i < boxes.size(); i++) {
    Face *f = &faces[boxes[i].face];
    GLSLFace gf;
    gf.normal[0] = f->normal[0];
    gf.normal[1] = f->normal[1];
    gf.normal[2] = f->normal[2];
    gf.normal[3] = 0.0;

    for(int i = 0; i < f->vertices.size(); i++) {
      gf.vertices[i][0] = f->vertices[i].pos[0];
      gf.vertices[i][1] = f->vertices[i].pos[1];
      gf.vertices[i][2] = f->vertices[i].pos[2];
      gf.vertices[i][3] = 1.0;
    }
    gf.numVertices = f->vertices.size();
    gf.side = f->side;
    gf.swap = boxes[i].swap;

    gf.rect[0] = boxes[i].x;
    gf.rect[1] = boxes[i].y;
    gf.rect[2] = boxes[i].w;
    gf.rect[3] = boxes[i].h;
    gf.oldPos[0] = boxes[i].x2;
    gf.oldPos[1] = boxes[i].y2;

    gf.distance = f->distance;

    gfaces[i] = gf;
  }
  compute.setData(0, in1->numFaces * sizeof(GLSLFace) + sizeof(DataType) * 4, (void*) in1);
  compute.execute(1024, 1024, 1);
  std::vector<float> data = compute.getData();

  unsigned char *tex = new unsigned char[1024 * 1024 * 4];
  for(int i = 0; i < 1024; i++) {
    for(int j = 0; j < 1024; j++) {
      int k = i * 1024 + j;
      tex[k * 4 + 0] = data[k] * 255.0;
      tex[k * 4 + 1] = data[k] * 255.0;
      tex[k * 4 + 2] = data[k] * 255.0;
      tex[k * 4 + 3] = 255;
    }
  }
  
  t = new Texture(1024, 1024, (unsigned char*) tex);
  t->bind();
  t->write(outPath);
};

void Map::draw() {
  for(int i = 0; i < faces.size(); i++) {
    glBegin(GL_POLYGON);
    for(int j = 0; j < faces[i].vertices.size(); j++) {
      glTexCoord2dv(faces[i].vertices[j].lightmapUV);
      glVertex3dv(faces[i].vertices[j].pos);
    }
    glEnd();
  }
};
