#version 330

const float PI = 3.1415926535897932384626433832795;

layout(location = 0) in vec4 vertex;
layout(location = 1) in vec4 vertexNormal;
layout(location = 2) in vec4 magnetization;
layout(location = 3) in vec4 translation;

smooth out vec4 fragVertex;
smooth out vec4 fragNormal;
smooth out vec4 col;
out vec4 trans;
out mat4 mv;
smooth out vec3 nrm;

float mag, relmag, phi, theta;

// Which quantity to use for coloration
// 1 = Full Orientation, 2 = In-Plane Angle, 3 = X-component,
// 4 = Y-Component, 5 = Z-Component
uniform int display_type;
uniform int rotate_glyphs;

uniform mat4 view, projection;
uniform vec3 com; // Center of mass
uniform float maxmag, thresholdLow, thresholdHigh;
uniform float xSliceLow, xSliceHigh, ySliceLow, ySliceHigh, zSliceLow, zSliceHigh;

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
    trans = translation;

	// mod_model = model;
    mat4 model = mat4(1.0);
    model[3][0] += 2.0*(translation.x-com.x);
    model[3][1] += 2.0*(translation.y-com.y);
    model[3][2] += 2.0*(translation.z-com.z);

    fragNormal = vertexNormal;
    fragVertex = vertex; // + translation;

    mag    = length(magnetization);
    relmag = mag/maxmag;
    col    = vec4(magnetization.x/mag, 0.0,0.0,0.0);
    theta  = acos(magnetization.z/mag);
    phi    = atan2(magnetization.y, magnetization.x);
    
    mat4 rot_theta = mat4(1.0);
    rot_theta[1][1] = rot_theta[2][2] = cos(theta);
    rot_theta[1][2] = rot_theta[2][1] = sin(theta);
    rot_theta[1][2] *= -1;

    mat4 rot_phi = mat4(1.0);
    rot_phi[0][0] = rot_phi[1][1] = cos(phi+0.5*PI);
    rot_phi[0][1] = rot_phi[1][0] = sin(phi+0.5*PI);
    rot_phi[0][1] *= -1;

    model = model * rot_phi * rot_theta;
    mv = view * model;

    mat3 normalMatrix = transpose(inverse(mat3(mv)));
    nrm = normalize(normalMatrix * vec3(fragNormal));

    // In-plane angle coloring
    float hue = phi/(2.0*PI);
    float lum = 0.5;
    if (hue < 0.0)
    	hue += 1.0;
    if (hue < 0.0)
        hue = 0.0;
    if (display_type == 1)
        lum += 0.5*magnetization.z/mag;
    if (display_type >= 3) // by component
        hue += 0.5*magnetization[display_type-3]/mag;

    col = vec4(hsl2rgb(vec3(hue, 1.0, lum)), 0.0);

    // Discarded because of thresholding
    if (relmag < thresholdLow - 0.01)
        col.w = 1.0;
    if (relmag > thresholdHigh + 0.01)
        col.w = 1.0;

    // Discarded because of clipping
    if (translation.x < xSliceLow)
        col.w = 1.0;
    if (translation.x > xSliceHigh)
        col.w = 1.0;
    if (translation.y < ySliceLow)
        col.w = 1.0;
    if (translation.y > ySliceHigh)
        col.w = 1.0;
    if (translation.z < zSliceLow)
        col.w = 1.0;
    if (translation.z > zSliceHigh)
        col.w = 1.0;
    
    gl_Position =  projection * view * model * vertex;
}
