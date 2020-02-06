#version 150

uniform sampler2D uDiffuseTex;

in vec2 vTexCoord;

void main()
{
	gl_FragColor.xyz = vec3(1.0, 1.0, 1.0) * texture2D(uDiffuseTex, vTexCoord).w;
}
