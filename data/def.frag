#version 430 core
in vec2 texCoord;
//in int channel;

layout(binding = 0) uniform sampler2D tex0;
layout(binding = 1) uniform sampler2D palette;

out vec4 fragColor;

void main(void)
{
	vec4 color =  texture(palette, vec2(texture(tex0, texCoord).r, 0));
	if(color.r+color.g+color.b>0)
		fragColor = color;
	else
		discard;
}