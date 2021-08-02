#include "ubo.h"
#include <glad/glad.h>
#include <stdexcept>

Ubo::Ubo(const char* _blockName, unsigned int _bindingPoint):
blockName(_blockName),
bindingPoint(_bindingPoint),
buffer(0),
size(0)
{}

void Ubo::Init(int _size)
{
	
	GLint maxSize;
	glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxSize);
	if(_size > maxSize)
		throw std::length_error("Uniform buffer size exceeds GL_MAX_UNIFORM_BLOCK_SIZE.");

	size = _size;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, buffer);
	glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_STREAM_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, buffer);
}

Ubo::~Ubo()
{
	glDeleteBuffers(1, &buffer);
}

void Ubo::Bind(unsigned int program)
{
	int index = glGetUniformBlockIndex(program, blockName);
	glUniformBlockBinding(program, index, bindingPoint);
}

void Ubo::SetData(const void* data)
{
	//glBindBuffer(GL_UNIFORM_BUFFER, buffer);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, size, data);
}