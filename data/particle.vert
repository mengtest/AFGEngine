#version 330 core
layout (location = 0) in vec2 meshPos; // the position variable has attribute position 0
layout (location = 1) in vec2 meshTexCoord;

layout (location = 2) in vec4 posScale;
layout (location = 3) in vec2 sincos;

layout (std140) uniform Common
{
	mat4 transform;
};
out vec2 texCoord;

void main()
{
	vec2 scaled = posScale.zw*meshPos;
	vec2 rotated = vec2(
		sincos.y*scaled.x - sincos.x*scaled.y,
		sincos.x*scaled.x + sincos.y*scaled.y
	);
	gl_Position = transform * vec4(rotated+posScale.xy, 0.0, 1.0);

	texCoord = meshTexCoord;
}