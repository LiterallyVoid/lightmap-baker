
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

static std::string inputFile, outputFile = "out.png";

static void printHelp();

struct Option {
  char single;
  std::string string;
  std::string desc;
  std::string *value;
  void (*function)();
};

static Option options[] = {
  {'h', "help", "print this help", NULL, printHelp},
  {'i', "input", "set the input .raw filepath (must be a raw file, as exported from blender)", &inputFile},
  {'o', "output", "set the output image filepath (must be png)", &outputFile},
};
static int optionSize = sizeof(options) / sizeof(options[0]);

static int width, height;


static void printHelp() {
  printf("Usage: lightmap [OPTIONS]\n\nOptions:\n");
  for(int i = 0; i < optionSize; i++) {
    printf("    -%c,  --%-8s %s\n", options[i].single, options[i].string.c_str(), options[i].desc.c_str());
  }
};

int main(int argc, char **argv) {
  for(int i = 1; i < argc; i++) {
    if(argv[i][0] == '-') {
      if(argv[i][1] == '-') {
	std::string option(argv[i] + 2);
	std::string next;
	bool hasNext = false;
	if(i < argc - 1) {
	  next = std::string(argv[i + 1]);
	  hasNext = true;
	}
	int j;
	for(j = 0; j < optionSize; j++) {
	  if(options[j].string == option) {
	    if(options[j].value != NULL) {
	      if(hasNext) {
		*options[j].value = next;
	      } else {
		printf("Option --%s requires value\n", option.c_str());
	      }
	    } else {
	      options[j].function();
	    }
	    break;
	  }
	}
	if(j == optionSize) {
	  printf("Unknown option --%s\n", option.c_str());
	}
      } else {
	std::string option(argv[i] + 1);
	std::string next;
	bool hasNext = false;
	if(i < argc - 1) {
	  next = std::string(argv[i + 1]);
	  hasNext = true;
	}
	bool usedNext = false;
	for(int j = 0; j < optionSize; j++) {
	  if(option.find(options[j].single) != std::string::npos) {
	    if(options[j].value != NULL) {
	      if(hasNext && !usedNext) {
		*options[j].value = next;
		usedNext = true;
	      } else {
		printf("Option --%s requires value\n", option.c_str());
	      }
	    } else {
	      options[j].function();
	    }
	  }
	}
      }
    }
  }

  if(inputFile.size() == 0) {
    printHelp();
    printf("\ninput file is required to be specified\n");
    return 1;
  }
  
  if(!glfwInit()) {
    return 1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
  glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

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

  Map map(inputFile, outputFile);

  glfwShowWindow(window);
  
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
