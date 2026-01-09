#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec3 rayDir;
uniform float uTime;
uniform vec3 uSunDirection;
uniform vec3 uAtmParams[10];
// uAtmParams[0-8] - Hosek-Wilkie
// uAtmParams[9] - sun brightness

#define PI 3.141592653589793

vec3 HosekWilkie(float cos_theta, float gamma, float cos_gamma) {
    vec3 A = uAtmParams[0];
    vec3 B = uAtmParams[1];
    vec3 C = uAtmParams[2];
    vec3 D = uAtmParams[3];
    vec3 E = uAtmParams[4];
    vec3 F = uAtmParams[5];
    vec3 G = uAtmParams[6];
    vec3 H = uAtmParams[7];
    vec3 I = uAtmParams[8];
    vec3 chi = (1 + cos_gamma * cos_gamma) / pow(1 + H * H - 2 * cos_gamma * H, vec3(1.5));
    return (1 + A * exp(B / (cos_theta + 0.01))) * (C + D * exp(E * gamma) + F * (cos_gamma * cos_gamma) + G * chi + I * sqrt(cos_theta));
}

void main(void) {
    vec3 V = normalize(rayDir);
    float cos_theta = clamp(V.y, 0, 1);
    float cos_gamma = dot(V, uSunDirection);
    float gamma = acos(cos_gamma);

    vec3 skyColor = uAtmParams[9] * 1 * HosekWilkie(cos_theta, gamma, cos_gamma);
    if (cos_gamma > 0) {
        skyColor = skyColor + pow(vec3(cos_gamma), vec3(512));
    }

    // imitates night
    float sunAlt = asin(uSunDirection.y);
    float dayFactor = clamp( sunAlt / 0.3, 0.0, 1.0);
    skyColor *= dayFactor;

    skyColor = clamp(skyColor, 0, 1); // temp

    FragColor = vec4(skyColor, 1.0);
    BrightColor = vec4(0, 0, 0, 1.0);
}