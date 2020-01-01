#version 150

uniform mat4 uProjection;
uniform mat4 uModelView;

in vec4 aPos;
in vec2 aTexCoord;

out vec2 vTexCoord;

void main()
{
	vTexCoord = aTexCoord;
	gl_Position = uProjection * uModelView * aPos;
}
