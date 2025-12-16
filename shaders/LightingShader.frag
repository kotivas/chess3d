#version 420 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

struct Material {
    sampler2D diffuse;
    bool useDiffuse;
    sampler2D specular;
    bool useSpecular;
    sampler2D normal;
    bool useNormal;
    float shininess;
    vec3 solidColor;
};
struct DirLight {
    int draw;

    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};// 80
struct PointLight {
    int draw;

    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};// 96
struct SpotLight {
    int draw;

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
    vec3 Tangent;
    vec4 DirFragPosLightSpace;
    vec4 SpotFragPosLightSpace;
} fs_in;

layout(std140, binding = 1) uniform Lights {
    DirLight dirLight;
    PointLight pointLight;
    SpotLight spotLight;
};

layout(std140, binding = 2) uniform Data {
    vec3 viewPos;
    float farPlane;
};

uniform sampler2D spotShadowMap;
uniform sampler2D dirShadowMap;
uniform samplerCube omniShadowMap;
uniform Material material;

// array of offset direction for sampling
vec3 gridSamplingDisk[20] = vec3[] (
vec3(1, 1, 1), vec3(1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1),
vec3(1, 1, -1), vec3(1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
vec3(1, 1, 0), vec3(1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
vec3(1, 0, 1), vec3(-1, 0, 1), vec3(1, 0, -1), vec3(-1, 0, -1),
vec3(0, 1, 1), vec3(0, -1, 1), vec3(0, -1, -1), vec3(0, 1, -1)
);

float PCF_Shadow(vec4 fragPosLightSpace, sampler2D shadowMap) {
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
    float bias = 0.001;
    int range = 3;
    for (int x = -range; x <= range; ++x) {
        for (int y = -range; y <= range; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= float((range * 2 + 1) * (range * 2 + 1));

    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if (projCoords.z > 1.0)
    shadow = 0.0;

    return shadow;
}
float PCF_Shadow(vec3 fragPos, vec3 lightPos, samplerCube shadowCubemap) {
    vec3 fragToLight = fragPos - lightPos;
    float currentDepth = length(fragToLight);
    float shadow = 0.0;
    float bias = 0.15;
    int samples = 10;
    float viewDistance = length(viewPos - fragPos);
    float diskRadius = (1.0 + (viewDistance / farPlane)) / 25.0;
    for (int i = 0; i < samples; ++i) {
        float closestDepth = texture(shadowCubemap, fragToLight + gridSamplingDisk[i] * diskRadius).r;
        closestDepth *= farPlane;// undo mapping [0;1]
        if (currentDepth - bias > closestDepth)
        shadow += 1.0;
    }

    shadow /= float(samples);

    return shadow;
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 diffuse_color) {
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // combine results
    vec3 ambient = light.ambient * diffuse_color;
    vec3 diffuse = light.diffuse * diff * diffuse_color;
    vec3 specular = light.specular * spec * vec3(texture(material.specular, fs_in.TexCoords));

    float shadow = PCF_Shadow(fs_in.DirFragPosLightSpace, dirShadowMap);

    return ambient + (1.0 - shadow) * (diffuse + specular);
}
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 diffuse_color) {
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
    vec3 ambient = light.ambient * diffuse_color;
    vec3 diffuse = light.diffuse * diff * diffuse_color;
    vec3 specular = light.specular * spec * vec3(texture(material.specular, fs_in.TexCoords));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    float shadow = PCF_Shadow(fs_in.FragPos, light.position, omniShadowMap);

    return ambient + (1.0 - shadow) * (diffuse + specular);
}
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 diffuse_color) {
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
    vec3 ambient = light.ambient * diffuse_color;
    vec3 diffuse = light.diffuse * diff * diffuse_color;
    vec3 specular = light.specular * spec * vec3(texture(material.specular, fs_in.TexCoords));
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;

    float shadow = PCF_Shadow(fs_in.SpotFragPosLightSpace, spotShadowMap);

    return ambient + (1.0 - shadow) * (diffuse + specular);
}

vec3 CalcBumpedNormal() {
    vec3 Normal = normalize(fs_in.Normal);
    vec3 Tangent = normalize(fs_in.Tangent);
    Tangent = normalize(Tangent - dot(Tangent, Normal) * Normal);
    vec3 Bitangent = cross(Tangent, Normal);
    vec3 BumpMapNormal = texture(material.normal, fs_in.TexCoords).xyz;
    BumpMapNormal = 2.0 * BumpMapNormal - vec3(1.0, 1.0, 1.0);
    vec3 NewNormal;
    mat3 TBN = mat3(Tangent, Bitangent, Normal);
    NewNormal = TBN * BumpMapNormal;
    NewNormal = normalize(NewNormal);
    return NewNormal;
}

void main() {
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 result = vec3(0);

    vec3 diffuse = material.solidColor;
    vec3 normal = normalize(fs_in.Normal);

    if (material.useDiffuse) diffuse = texture(material.diffuse, fs_in.TexCoords).rgb;
    if (material.useNormal) normal = CalcBumpedNormal();

    if (dirLight.draw == 1)   result += CalcDirLight(dirLight, normal, viewDir, diffuse);
    if (pointLight.draw == 1) result += CalcPointLight(pointLight, normal, fs_in.FragPos, viewDir, diffuse);
    if (spotLight.draw == 1)  result += CalcSpotLight(spotLight, normal, fs_in.FragPos, viewDir, diffuse);

    FragColor = vec4(result, 1.0);

    // save brightest colors for bloom
    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if (brightness > 2.0) BrightColor = vec4(FragColor.rgb, 1.0);
    else BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}
