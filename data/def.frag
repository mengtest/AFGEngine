#version 120
uniform sampler2D tex0;
uniform int flat;

void main(void)
{
	gl_FragColor = texture2D(tex0, gl_TexCoord[0].xy)*(gl_Color);
}