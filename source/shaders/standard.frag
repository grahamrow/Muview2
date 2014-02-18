#version 140

out vec4 fragColor;

uniform vec4 color;
uniform float brightness;


void main( void )
{
    // base color
    fragColor = color * brightness;
}
