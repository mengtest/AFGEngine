#version 330 core
layout (location = 0) in vec2 meshPos; // the position variable has attribute position 0
layout (location = 1) in vec2 meshTexCoord;

layout (location = 2) in vec2 partPos;
layout (location = 3) in vec2 partScale;

layout (std140) uniform Common
{
	mat4 transform;
};
out vec2 texCoord;

void main()
{
	
	gl_Position = transform * vec4((meshPos*partScale)+partPos, 0.0, 1.0);

	texCoord = meshTexCoord;
}