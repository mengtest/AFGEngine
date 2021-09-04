#version 330 core
in vec2 texCoord;
uniform sampler2D tex0;
uniform vec4 mulColor;

out vec4 fragColor;

void main(void)
{
	vec4 color = texture(tex0, texCoord/textureSize(tex0,0))*mulColor;
	fragColor = color;
}