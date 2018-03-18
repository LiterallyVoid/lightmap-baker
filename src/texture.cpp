
#include <X11/Xlib.h>

#define GLFW_INCLUDE_GLU

#include <GLFW/glfw3.h>
#include <unistd.h>
#include <stdlib.h>
#include <cstring>

#include <png.h>

#include "texture.h"

Texture::Texture(std::string filename) {
  png_image img;
  img.version = PNG_IMAGE_VERSION;
  img.opaque = NULL;
  png_image_begin_read_from_file(&img, filename.c_str());
  img.format = PNG_FORMAT_RGBA;

  pixels = new unsigned char[PNG_IMAGE_SIZE(img)];
  png_image_finish_read(&img, NULL, pixels, PNG_IMAGE_ROW_STRIDE(img), NULL);

  width = img.width;
  height = img.height;

  init(pixels);
  
  png_image_free(&img);
};

Texture::Texture(int width, int height, unsigned char *data) : width(width), height(height) {
  init(data);
  pixels = new unsigned char[width * height * 4];
  memcpy(pixels, data, width * height * 4);
};

Texture::~Texture() {
  glDeleteTextures(1, &gl_id);
  delete[] pixels;
};

void Texture::write(std::string filename) {
  png_image img;
  img.version = PNG_IMAGE_VERSION;
  img.opaque = NULL;
  img.width = width;
  img.height = height;
  img.format = PNG_FORMAT_RGBA;
  img.flags = 0;
  img.colormap_entries = 0;
  png_image_write_to_file(&img,
			  filename.c_str(),
			  0,
			  pixels,
			  0,
			  NULL);
};

void Texture::init(unsigned char *data) {
  glGenTextures(1, &gl_id);

  glBindTexture(GL_TEXTURE_2D, gl_id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
  //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

  glBindTexture(GL_TEXTURE_2D, gl_id);
};

void Texture::bind() {
  glBindTexture(GL_TEXTURE_2D, gl_id);
};

void Texture::getColor(int x, int y, unsigned char &r, unsigned char &g, unsigned char &b, unsigned char &a) {
  if(x < 0) {
    x = 0;
  }
  if(x >= width) {
    x = width - 1;
  }
  if(y < 0) {
    y = 0;
  }
  if(y >= height) {
    y = height - 1;
  }
  unsigned long pos = (y * width + x) * 4;
  r = pixels[pos];
  g = pixels[pos + 1];
  b = pixels[pos + 2];
  a = pixels[pos + 3];
};
