#version 330 core
in vec2 texCoord;
in int channel;

uniform sampler2D tex0;
uniform sampler2D palette;

out vec4 fragColor;

void main(void)
{
	fragColor = texture(tex0, texCoord);
}