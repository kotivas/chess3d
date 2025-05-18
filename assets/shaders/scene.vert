#version 420 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} vs_out;

layout(std140, binding = 0) uniform Matrices {
    mat4 u_Projection;
    mat4 u_View;
    mat4 u_LightSpaceMatrix;
};

uniform mat4 u_Model;

void main() {
    vs_out.FragPos = vec3(u_Model * vec4(aPos, 1.0));
    vs_out.Normal = transpose(inverse(mat3(u_Model))) * aNormal;
    vs_out.TexCoords = aTexCoords;
    vs_out.FragPosLightSpace = u_LightSpaceMatrix * vec4(vs_out.FragPos, 1.0);
    gl_Position = u_Projection * u_View * u_Model * vec4(aPos, 1.0);
}