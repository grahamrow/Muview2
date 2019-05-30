#version 330

const float PI = 3.1415926535897932384626433832795;

layout(location = 0) in vec4 vertex;
layout(location = 1) in vec4 vertexNormal;
layout(location = 2) in vec4 magnetization;
layout(location = 3) in vec4 translation;

out vec4 fragVertex;
out vec4 fragNormal;
smooth out vec4 col;

float mag, relmag, hue, phi, theta;

uniform mat4 model, view, projection;
uniform float maxmag, thresholdLow, thresholdHigh;

float atan2(in float y, in float x)
{
    bool s = (abs(x) > abs(y));
    return mix(PI/2.0 - atan(x,y), atan(y,x), s);
}

float hue2rgb(float f1, float f2, float hue) {
    if (hue < 0.0)
        hue += 1.0;
    else if (hue > 1.0)
        hue -= 1.0;
    float res;
    if ((6.0 * hue) < 1.0)
        res = f1 + (f2 - f1) * 6.0 * hue;
    else if ((2.0 * hue) < 1.0)
        res = f2;
    else if ((3.0 * hue) < 2.0)
        res = f1 + (f2 - f1) * ((2.0 / 3.0) - hue) * 6.0;
    else
        res = f1;
    return res;
}

vec3 hsl2rgb(vec3 hsl) {
    vec3 rgb;
    
    if (hsl.y == 0.0) {
        rgb = vec3(hsl.z); // Luminance
    } else {
        float f2;
        
        if (hsl.z < 0.5)
            f2 = hsl.z * (1.0 + hsl.y);
        else
            f2 = hsl.z + hsl.y - hsl.y * hsl.z;
            
        float f1 = 2.0 * hsl.z - f2;
        
        rgb.r = hue2rgb(f1, f2, hsl.x + (1.0/3.0));
        rgb.g = hue2rgb(f1, f2, hsl.x);
        rgb.b = hue2rgb(f1, f2, hsl.x - (1.0/3.0));
    }   
    return rgb;
}

void main( void )
{
	
    fragNormal = vertexNormal;
    fragVertex = vertex;

    mag    = length(magnetization);
    relmag = mag/maxmag;
    col    = vec4(magnetization.x/mag, 0.0,0.0,0.0);
    theta  = acos(magnetization.z/mag);
    phi    = atan2(magnetization.y, magnetization.x);
    
    hue   = phi/(2.0*PI);
    if (hue < 0.0)
    	hue += 1.0;
// 
    // col = vec4(magnetization.x/maxmag,0.0,0.0,0.0);


    col = vec4(hsl2rgb(vec3(hue, 1.0, 0.5)), 0.0);
    if (relmag < thresholdLow - 0.01)
        col.w = 1.0;
    if (relmag > thresholdHigh + 0.01)
        col.w = 1.0;
    
    gl_Position =  projection * view * model * (translation + vertex);
}
