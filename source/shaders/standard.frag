#version 330

smooth in vec4 col;
out vec4 fragColor;

void main( void )
{
	if (col.w > 0.5)
		discard;
		
    fragColor = col; // * brightness;
}
