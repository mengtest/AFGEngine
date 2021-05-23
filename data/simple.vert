#version 330 core
layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Color;

out vec4 Frag_Color;

layout (std140) uniform Common
{
	mat4 transform;
};
uniform float Alpha;


void main()
{
	Frag_Color = vec4(Color, Alpha);
	gl_Position = transform * vec4(Position, 1);
};
