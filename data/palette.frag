#version 330 core
in vec2 texCoord;

uniform sampler2DRect tex0;
uniform sampler2D palette;
uniform int paletteSlot;

out vec4 fragColor;

vec4 indexedSample(vec2 texCoord)
{
	//return texture(tex0, texCoord);
	vec4 color = texture(palette, vec2(texture(tex0, texCoord).r, paletteSlot*0.5));
	//return color;
	return vec4(color.rgb, clamp((color.r+color.g+color.b)*256, 0, 1));
}

vec4 BilinearSmoothstepSample (vec2 P)
{
	const float sharpness = 0.3;
	vec2 pixel = P + 0.5;
	
	vec2 frac = fract(pixel);
	pixel = (floor(pixel)) - 0.5;

	vec4 C11 = indexedSample(pixel);
	vec4 C21 = indexedSample(pixel + vec2(1, 0.0));
	vec4 C12 = indexedSample(pixel + vec2(0.0, 1));
	vec4 C22 = indexedSample(pixel + vec2(1, 1));

	vec4 x1 = mix(C11, C21, smoothstep(sharpness, 1-sharpness, frac.x));
	vec4 x2 = mix(C12, C22, smoothstep(sharpness, 1-sharpness, frac.x));
	return mix(x1, x2, smoothstep(sharpness, 1-sharpness, frac.y));
}

void main(void)
{
	fragColor = BilinearSmoothstepSample(texCoord);
}