#ifndef UBO_H_GUARD
#define UBO_H_GUARD

class Ubo
{
private:
	const char* blockName;
	unsigned int bindingPoint;
	unsigned int buffer;
	int size;

public:
	Ubo(const char* blockName, unsigned int bindingPoint);
	~Ubo();
	
	void Init(int size);
	void Bind(unsigned int program);
	void SetData(const void* data);
};

#endif /* UBO_H_GUARD */
