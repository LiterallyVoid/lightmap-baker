#include <GL/glew.h>
#define GL_GLEXT_PROTOTYPES
#include <GLFW/glfw3.h>

#include <fstream>
#include <sstream>
#include <string>

#include "compute.h"

Compute::Compute() {};

Compute::Compute(std::string path) {
  std::ifstream file(path);

  std::string string;

  file.seekg(0, std::ios::end);   
  string.reserve(file.tellg());
  file.seekg(0, std::ios::beg);
  
  string.assign((std::istreambuf_iterator<char>(file)),
		std::istreambuf_iterator<char>());
  
  GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
  const char *str = string.c_str();
  glShaderSource(shader, 1, &str, 0);
  glCompileShader(shader);

  GLint success = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if(success == GL_FALSE) {
    GLint maxLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

    std::vector<GLchar> errorLog(maxLength);
    glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);

    printf("%s\n", &errorLog[0]);
	
    glDeleteShader(shader);
    return;
  }

  program = glCreateProgram();

  glAttachShader(program, shader);
  glLinkProgram(program);

  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if(success == GL_FALSE) {
    GLint maxLength = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

    std::vector<GLchar> errorLog(maxLength);
    glGetProgramInfoLog(program, maxLength, &maxLength, &errorLog[0]);

    printf("%s\n", &errorLog[0]);
  }

  glDetachShader(program, shader);
  glGenBuffers(1, &ssboIn1);
  glGenBuffers(1, &ssboIn2);

  glGenTextures(1, &texOut);

  glBindTexture(GL_TEXTURE_2D, texOut);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, 1024, 1024, 0, GL_RED, GL_FLOAT, NULL);
};

Compute::~Compute() {
  glDeleteProgram(program);
  glDeleteBuffers(1, &ssboIn1);
  glDeleteBuffers(1, &ssboIn2);
  glDeleteTextures(1, &texOut);
};

void Compute::setData(int num, unsigned long size, void *data) {
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, num, num ? ssboIn2 : ssboIn1);
  glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_DYNAMIC_DRAW);
};

std::vector<float> Compute::getData() {
  glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
  std::vector<float> data(1024 * 1024);
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, &data[0]);
  return data;
};

void Compute::execute(int sizeX, int sizeY, int sizeZ) {
  glUseProgram(program);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboIn1);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboIn2);
  glBindTexture(GL_TEXTURE_2D, texOut);
  glBindImageTexture(3, texOut, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
  glDispatchCompute(sizeX, sizeY, sizeZ);
};
