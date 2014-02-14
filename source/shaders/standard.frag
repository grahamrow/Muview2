#version 140

out vec4 fragColor;
in vec4 fragNormal;
in vec4 fragVertex;

uniform mat4 model;
uniform vec4 color;
uniform float ambient;

uniform struct Light {
   vec4 position;
   vec4 intensities; //a.k.a the color of the light
} light;

void main( void )
{
    // base color
    fragColor = color * light.intensities;

}
