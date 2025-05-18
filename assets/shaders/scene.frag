#version 420 core
out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
    vec3 solidColor;
    bool useSolidColor;
};
struct DirLight {
    bool draw;

    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};// 80
struct PointLight {
    bool draw;

    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};// 96
struct SpotLight {
    bool draw;

    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};// 112

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

layout(std140, binding = 1) uniform Lights {
    DirLight dirLight;
    PointLight pointLight;
    SpotLight spotLight;
};

layout(std140, binding = 2) uniform Data {
    vec3 viewPos;
};

uniform sampler2D shadowMap;
uniform Material material;


vec4 simple() {
    // ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * dirLight.ambient;

    // diffuse
    vec3 norm = normalize(fs_in.Normal);
    vec3 lightDir = dirLight.direction;
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * dirLight.diffuse;

    // specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * dirLight.specular;

    vec3 result = (ambient + diffuse + specular) * vec3(texture(material.diffuse, fs_in.TexCoords));
    return vec4(result, 1.0);
}

float CalculateDirectionalShadow(vec4 fragPosLightSpace) {
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;

    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth > pcfDepth  ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if (projCoords.z > 1.0)
    shadow = 0.0;

    return shadow;
}

// todo solid color draw
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, fs_in.TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, fs_in.TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, fs_in.TexCoords));

    float shadow = CalculateDirectionalShadow(fs_in.FragPosLightSpace);

    return (ambient + (1.0 - shadow) * (diffuse + specular));
}
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, fs_in.TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, fs_in.TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, fs_in.TexCoords));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, fs_in.TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, fs_in.TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, fs_in.TexCoords));
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;

    float shadow = CalculateDirectionalShadow(fs_in.FragPosLightSpace);

    return (ambient + diffuse + specular);
}

void main() {
    // properties
    vec3 norm = normalize(fs_in.Normal);
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);

    vec3 result = vec3(0);

    if (dirLight.draw){
        result += CalcDirLight(dirLight, norm, viewDir);
    }

    if (pointLight.draw){
        result += CalcPointLight(pointLight, norm, fs_in.FragPos, viewDir);
    }

    if (spotLight.draw) {
        result += CalcSpotLight(spotLight, norm, fs_in.FragPos, viewDir);
    }

    FragColor = vec4(result, 1.0);
}
