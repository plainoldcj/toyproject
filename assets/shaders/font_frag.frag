#version 150

uniform sampler2D uDiffuseTex;

in vec2 vTexCoord;

void main()
{
	vec4 c = texture2D(uDiffuseTex, vTexCoord);
	gl_FragColor = c.w * vec4(1.0, 1.0, 1.0, 1.0);
}
