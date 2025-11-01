#version 420 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec3 VertexLight;
} vs_out;

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;

	float constant;
    float linear;
    float quadratic;
};

layout(std140, binding = 0) uniform Matrices {
    mat4 u_Projection;
    mat4 u_View;
};

layout(std140, binding = 1) uniform Lights {
    Light light;
};


uniform mat4 u_Model;

vec3 calcLightning(){
    // per-vertex lightning
    vec3 lightDir = normalize(light.position - vs_out.FragPos);
    float diff = max(dot(vs_out.Normal, -lightDir), 0.0);

    float distance    = length(light.position - vs_out.FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance +
    light.quadratic * (distance * distance));

    vec3 diffuse = (light.diffuse * diff) * attenuation;
    vec3 ambient = (light.ambient) * attenuation;

    return ambient + diffuse;
}

void main() {
    vs_out.FragPos = vec3(u_Model * vec4(aPos, 1.0));
    vs_out.Normal = normalize(mat3(u_Model) * aNormal);
    vs_out.TexCoords = aTexCoords;
    vs_out.VertexLight = calcLightning();

    gl_Position = u_Projection * u_View * u_Model * vec4(aPos, 1.0);
}