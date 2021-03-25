#version 430 core
layout (location = 0) in vec2 aPos; // the position variable has attribute position 0
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in int aChannel;

uniform mat4 transform;
out vec2 texCoord;
out int channel;

void main()
{
	gl_Position = transform * vec4(aPos, 0.0, 1.0);
	texCoord = aTexCoord;

	channel = aChannel;
}