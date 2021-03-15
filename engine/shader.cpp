#include <glad/glad.h>

#include <iostream>

#include "util.h"
#include "shader.h"

int globalShaderProgram;

int LoadShader(const char *vertex_path, const char *fragment_path)
{
	char log[2048];

	std::string vertShaderStr = ReadFile(vertex_path);
	std::string fragShaderStr = ReadFile(fragment_path);
	const char *vertShaderSrc = vertShaderStr.c_str();
	const char *fragShaderSrc = fragShaderStr.c_str();

	int myVertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(myVertexShader, 1, &vertShaderSrc, 0);
	glCompileShader(myVertexShader);
	glGetShaderInfoLog(myVertexShader, 2048, 0, log);
	if (log[0])
		std::cout << "Vertex Shader: " << log << "\n";

	int myFragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(myFragShader, 1, &fragShaderSrc, 0);
	glCompileShader(myFragShader);
	glGetShaderInfoLog(myFragShader, 2048, 0, log);
	if (log[0])
		std::cout << "Fragment Shader: " << log << "\n";

	int myProgram = glCreateProgram();
	if (myProgram == 0)
		std::cout << "Shader program creation failed/n";

	glAttachShader(myProgram, myVertexShader);
	glAttachShader(myProgram, myFragShader);
	glLinkProgram(myProgram);
	glGetProgramInfoLog(myProgram, 2048, 0, log);
	if (log[0])
		std::cout << "Shader linking stage: " << log << "\n";

	return myProgram;
}
