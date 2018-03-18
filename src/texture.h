/* -*- mode: c++ -*- */

#pragma once

#include <string>

class Texture {
public:
  /**
     Load a texture.

     @param filename The filename of the texture.
  **/
  Texture(std::string filename);
  Texture(int width, int height, unsigned char *data);
  ~Texture();

  void write(std::string filename);
  
  void init(unsigned char *data);
  
  /**
     Use this texture in subsequent draw calls.
  **/
  void bind();

  void getColor(int x, int y, unsigned char &r, unsigned char &g, unsigned char &b, unsigned char &a);

private:
  /**
     The number OpenGL uses to identify this texture.
  **/
  unsigned int gl_id;

  /**
     The width and height, respectively, of this texture.
  **/
  unsigned int width;
  unsigned int height;

  unsigned char *pixels;
};
