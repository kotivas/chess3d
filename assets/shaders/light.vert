#version 420 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

layout(std140, binding = 0) uniform Matrices {
    mat4 u_Projection;
    mat4 u_View;
    mat4 u_LightSpaceMatrix;
};

uniform mat4 u_Model;

void main() {
    gl_Position = u_Projection * u_View * u_Model * vec4(aPos, 1.0);
}