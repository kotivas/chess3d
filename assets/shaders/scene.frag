#version 420 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec3 VertexLight;
} fs_in;

uniform sampler2D baseColor;

layout(std140, binding = 2) uniform Data {
    vec3 viewPos;
    float farPlane;
};

void main() {
    vec3 result = vec3(texture(baseColor, fs_in.TexCoords)) * fs_in.VertexLight;
    FragColor = vec4(result, 1.0);
}
