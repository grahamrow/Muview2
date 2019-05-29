#version 330

layout(location = 0) in vec4 vertex;
layout(location = 1) in vec4 vertexNormal;
layout(location = 2) in vec4 color;
layout(location = 3) in vec4 translation;

out vec4 fragVertex;
out vec4 fragNormal;
smooth out vec4 col;

uniform mat4 model, view, projection;

void main( void )
{
	col = color;

    fragNormal = vertexNormal;
    fragVertex = vertex;

    gl_Position =  projection * view * model * (translation + vertex);
}
