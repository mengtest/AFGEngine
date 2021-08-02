#version 330 core
in vec2 texCoord;
uniform sampler2DRect tex0;

out vec4 fragColor;

void main(void)
{

	vec4 color = texture(tex0, texCoord);
	/* float alpha = color.a;
	color.g += color.r*color.a*0.7;
	color.b += 2*color.r*pow(alpha,20);
	color.a *=2;
	color.r = 1; */

	fragColor = color;
}