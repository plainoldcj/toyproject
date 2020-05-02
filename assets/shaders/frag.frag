#version 420

layout(binding=0) uniform sampler2D uDiffuseTex;

layout(location=1) in vec2 vTexCoord;

layout(location=0) out vec4 vFragColor;

void main()
{
	vFragColor = texture(uDiffuseTex, vTexCoord);
}
