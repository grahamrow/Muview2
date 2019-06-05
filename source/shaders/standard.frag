#version 330

smooth in vec4 fragNormal;
smooth in vec4 fragVertex;
in vec4 col;
in vec4 trans;
in mat4 mv;
smooth in vec3 nrm;
out vec4 fragColor;

mat4 model;

uniform mat4 view;
uniform float ambient;
uniform struct Light {
   vec4 position;
   vec4 intensities; //a.k.a the color of the light
} light;

void main( void )
{
    // Drop thresholded or clipped values as dictated by vertex shader
    if (col.w > 0.5)
        discard;

    //calculate the location of this fragment (pixel) in world coordinates
    vec3 fragPosition = mat3(mv) * vec3(fragVertex);

    //calculate the vector from this pixels surface to the light source
    vec3 surfaceToLight = vec3(light.position) - vec3(fragPosition);

    //calculate the cosine of the angle of incidence
    float brightness = dot(nrm, surfaceToLight) / (length(surfaceToLight) * length(nrm));
    brightness = clamp(brightness, 0, 1);

    // calculate final color of the pixel, based on:
    // 1. The angle of incidence: brightness
    // 2. The color/intensities of the light: light.intensities
    // 3. The texture and texture coord: texture(tex, fragTexCoord)
    fragColor = (ambient + brightness * light.intensities) * col;
}
