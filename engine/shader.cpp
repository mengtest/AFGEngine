#include "shader.h"
#include <glad/glad.h>
#include <iostream>
#include <fstream>
#include <string>

unsigned int globalShaderProgram;

const char *vertex = R"()";
const char *fragment = R"()";

std::string ReadFile(const char *filePath)
{
    std::string content;
    std::ifstream fileStream(filePath, std::ios::in);

    if(!fileStream.is_open()) {
        return "";
    }

    std::string line = "";
    while(!fileStream.eof()) {
        std::getline(fileStream, line);
        content.append(line + "\n");
    }

    fileStream.close();
    return content;
}

GLuint CreateShader(const char **src, GLenum type)
{
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, src, nullptr);
	glCompileShader(shader);
	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if(!success)
	{
		char log[2048];
		glGetShaderInfoLog(shader, 2048, 0, log);
		switch(type)
		{
			case GL_VERTEX_SHADER:
				std::cerr << "Vertex ";
				break;
			case GL_FRAGMENT_SHADER:
				std::cerr << "Frament ";
				break;
			default:
				std::cerr << "Other ";
		}
		std::cerr << "Shader error:\n" << log;
	}
	return shader;
}


unsigned int LoadShader(const char *vertex_path, const char *fragment_path)
{
	const char *vertShaderSrc = vertex;
	const char *fragShaderSrc = fragment;
	std::string vertShaderStr, fragShaderStr;
	if(vertex_path != nullptr && fragment_path != nullptr)
	{
		vertShaderStr = ReadFile(vertex_path);
		fragShaderStr = ReadFile(fragment_path);
		vertShaderSrc = vertShaderStr.c_str();
		fragShaderSrc = fragShaderStr.c_str();
	}

	GLuint myVertexShader = CreateShader(&vertShaderSrc, GL_VERTEX_SHADER);
	GLuint myFragShader = CreateShader(&fragShaderSrc, GL_FRAGMENT_SHADER);

	unsigned int myProgram = glCreateProgram();
	if (myProgram == 0)
		std::cout << "Shader program creation failed\n";

	glAttachShader(myProgram, myVertexShader);
	glAttachShader(myProgram, myFragShader);
	glLinkProgram(myProgram);
	GLint success;
	glGetProgramiv(myProgram, GL_LINK_STATUS, &success);
	if(!success)
	{
		char log[2048];
		glGetProgramInfoLog(myProgram, 2048, 0, log);
		std::cerr << "Shader linking error:\n" << log;
	}

	glDeleteShader(myVertexShader);
	glDeleteShader(myFragShader);
	return myProgram;
}