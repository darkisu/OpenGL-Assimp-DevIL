#pragma once

#include <GLEW\glew.h>


#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#define SHADER_FROM_FILE 0
#define SHADER_FROM_STRING 1

class Shader
{
public:
	GLuint Program;
	Shader(const GLchar * vertexPath, const GLchar * fragmentPath,const GLuint &mode);

	~Shader();
	void Use()
	{
		glUseProgram(this->Program);
	}
};

