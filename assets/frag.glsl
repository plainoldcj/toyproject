#version 150

uniform sampler2D uDiffuseTex;

in vec2 vTexCoord;

void main()
{
	gl_FragColor = texture2D(uDiffuseTex, vTexCoord);
}
