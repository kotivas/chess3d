#version 420 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec3 Tangent;
    vec4 DirFragPosLightSpace;
    vec4 SpotFragPosLightSpace;
} vs_out;

layout(std140, binding = 0) uniform Matrices {
    mat4 uProjection;
    mat4 uView;
    mat4 uDirLightSpaceMatrix;
    mat4 uSpotLightSpaceMatrix;
};

uniform mat4 uModel;

void main() {
    vs_out.FragPos = vec3(uModel * vec4(aPos, 1.0));
    vs_out.Normal = transpose(inverse(mat3(uModel))) * aNormal;
    vs_out.Tangent = transpose(inverse(mat3(uModel))) * aTangent;
    vs_out.TexCoords = aTexCoords;

    vs_out.DirFragPosLightSpace = uDirLightSpaceMatrix * vec4(vs_out.FragPos, 1.0);
    vs_out.SpotFragPosLightSpace = uSpotLightSpaceMatrix * vec4(vs_out.FragPos, 1.0);

    gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
}