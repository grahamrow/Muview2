#version 330

smooth in vec4 col;
out vec4 fragColor;

//uniform vec4 color;
//uniform float brightness;


void main( void )
{
    fragColor = col; // * brightness;
}
