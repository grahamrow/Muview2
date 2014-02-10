#version 330

in vec4 vertex;
in vec4 vertexNormal;

out vec4 fragVertex;
out vec4 fragNormal;

uniform mat4 model, view, projection;

void main( void )
{
    fragNormal = vertexNormal;
    fragVertex = vertex;

    gl_Position =  projection * view * model * vertex;
}
