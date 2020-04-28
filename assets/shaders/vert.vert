#version 420

layout(std140, binding=0) uniform UniformsHot
{
	uniform mat4 uProjection;
	uniform mat4 uModelView;
};

layout(location=0) in vec4 aPos;
layout(location=1) in vec2 aTexCoord;

layout(location=0) out vec2 vTexCoord;

void main()
{
	vTexCoord = aTexCoord;
	gl_Position = uProjection * uModelView * aPos;
}
