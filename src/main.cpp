
#include <X11/Xlib.h>

#include <GL/glew.h>
#define GLFW_INCLUDE_GLU
#define GL_GLEXT_PROTOTYPES
#include <GLFW/glfw3.h>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>

#include "map.h"
#include "texture.h"

static int width, height;

int main(int argc, char **argv) {
  if(!glfwInit()) {
    return 1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

  GLFWwindow* window = glfwCreateWindow(640, 480, "Lightmap", nullptr, nullptr);
  if(window == NULL) {
    glfwTerminate();
    return 1;
  }

  glfwMakeContextCurrent(window);

  int err = glewInit();
  if(err != GLEW_OK) {
    fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
    return 1;
  }

  glfwSwapInterval(1);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_TEXTURE_2D);
  //glEnable(GL_CULL_FACE);

  Map map("test.raw");
  
 while(!glfwWindowShouldClose(window)) {
   glfwGetFramebufferSize(window, &width, &height);
   

    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(90.0, width / (float) height, 0.1, 500.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(sin(glfwGetTime()) * 12, cos(glfwGetTime()) * 12, 15, 0, 0, 0, 0, 0, 1);
    
    map.draw();
    
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
};
