#version 330 core
layout (location = 0) in vec2 aPos; // the position variable has attribute position 0
layout (location = 1) in vec2 aTexCoord;

layout (std140) uniform Common
{
	mat4 transform;
};
out vec2 texCoord;

void main()
{
	gl_Position = transform * vec4(aPos, 0.0, 1.0);
	
	texCoord = aTexCoord;
}