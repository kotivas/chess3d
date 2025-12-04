#version 330 core
out vec3 rayDir;

uniform mat4 u_View;
uniform mat4 u_Projection;
uniform float time;

const vec2 quad[4] = vec2[](
vec2(-1.0,  1.0), vec2(-1.0, -1.0),
vec2( 1.0,  1.0), vec2( 1.0, -1.0)
);

void main() {
    gl_Position = vec4(quad[gl_VertexID], 0.0, 1.0);

    // Ray direction in world space
    vec4 rayClip = vec4(quad[gl_VertexID], 0.0, 1.0);
    vec4 rayEye = inverse(u_Projection) * rayClip;
    rayEye.z = -1.0; rayEye.w = 0.0;
    rayDir = normalize((inverse(u_View) * rayEye).xyz);
}
