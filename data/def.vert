#version 120
uniform mat4 transform;

void main()
{
	gl_Position = transform * gl_Vertex;

	gl_FrontColor = gl_Color;
	gl_TexCoord[0] = gl_MultiTexCoord0;
}