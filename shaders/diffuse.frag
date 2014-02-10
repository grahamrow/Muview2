#version 330

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
    vec4 baseColor = color;

    //calculate normal in world coordinates
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 normal = normalize(normalMatrix * vec3(fragNormal));

    //calculate the location of this fragment (pixel) in world coordinates
    vec3 fragPosition = mat3(model) * vec3(fragVertex);

    //calculate the vector from this pixels surface to the light source
    vec3 surfaceToLight = vec3(light.position) - vec3(fragPosition);

    //calculate the cosine of the angle of incidence
    float brightness = dot(normal, surfaceToLight) / (length(surfaceToLight) * length(normal));
    brightness = clamp(brightness, 0, 1);

    //calculate final color of the pixel, based on:
    // 1. The angle of incidence: brightness
    // 2. The color/intensities of the light: light.intensities
    // 3. The texture and texture coord: texture(tex, fragTexCoord)
    fragColor = (ambient + brightness * light.intensities) * baseColor;

}
