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
    // mat4 model = mat4(1.0);
    // model[3][0] = trans.x;
    // model[3][1] = trans.y;
    // model[3][2] = trans.z;



    // mat4 mv = view * model; 
    // // // base color
    // // vec4 baseColor = color;

    //calculate normal in world coordinates
    // mat3 normalMatrix = transpose(inverse(mat3(mv)));
    // vec3 normal = normalize(normalMatrix * vec3(fragNormal));
    // vec3 normal = nrm;
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

    // fragColor = vec4(nrm,0.0);
}
