#version 330
#extension GL_ARB_gpu_shader5 : enable

smooth in vec4 col;
out vec4 fragColor;
in vec4 fragNormal;
in vec4 fragVertex;
in vec4 trans;

mat4 model;
// uniform mat4 model;
uniform float ambient;
uniform struct Light {
   vec4 position;
   vec4 intensities; //a.k.a the color of the light
} light;

void main( void )
{
    // model = mat4(1.0);
    // model[3][0] += translation.x;
    // model[3][1] += translation.y;
    // model[3][2] += translation.z;

    // // base color
    // vec4 baseColor = col;

    // //calculate normal in world coordinates
    // mat3 normalMatrix = transpose(inverse(mat3(model)));
    // vec3 normal = normalize(normalMatrix * vec3(fragNormal));

    // //calculate the location of this fragment (pixel) in world coordinates
    // vec3 fragPosition = mat3(model) * vec3(fragVertex);

    // //calculate the vector from this pixels surface to the light source
    // vec3 surfaceToLight = vec3(light.position) - vec3(fragPosition);

    // //calculate the cosine of the angle of incidence
    // float brightness = dot(normal, surfaceToLight) / (length(surfaceToLight) * length(normal));
    // brightness = clamp(brightness, 0, 1);

    //calculate final color of the pixel, based on:
    // 1. The angle of incidence: brightness
    // 2. The color/intensities of the light: light.intensities
    // 3. The texture and texture coord: texture(tex, fragTexCoord)
    // fragColor = (ambient + brightness * light.intensities) * baseColor;
    fragColor = col;

}
