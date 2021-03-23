#ifndef VAO_H_GUARD
#define VAO_H_GUARD
#include <vector>
#include <cstdint>
#include <glad/glad.h>

class Vao
{
public:
	enum AttribType
	{
		F2F2,
		F2F2I1
	};

private:
	AttribType type;
	unsigned int usage;
	bool loaded;
	int stride;

	struct memPtr
	{
		uint8_t *ptr;
		size_t size;
		size_t location;
	};

	size_t totalSize;
	std::vector<memPtr> dataPointers;
	unsigned int vaoId;
	unsigned int vboId;


public:
	Vao(AttribType type, unsigned int usage);
	~Vao();

	//Returns index of object that can be drawn.
	int Prepare(size_t size, void *ptr);
	void Draw(int which, size_t count = 0, int mode = GL_TRIANGLES);
	void UpdateBuffer(int which, void *data, size_t count = 0);
	void Bind();
	void Load();
	
};

#endif /* VAO_H_GUARD */
