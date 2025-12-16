#version 330 core
in vec3 rayDir;
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

uniform float uTime;
uniform vec3 uSunDirection;
uniform vec3 uSunColor;
uniform float uCirrus;
uniform float uCumulus;
uniform vec2 uWindDirection;
uniform sampler3D tNoise;

// Simple hash / noise
float hash(float n){ return fract(sin(n)*43758.5453123); }
float noise(vec3 x){
    vec3 f = fract(x); float n = dot(floor(x), vec3(1.0, 157.0, 113.0));
    return mix(mix(mix(hash(n+0.0), hash(n+1.0), f.x),
    mix(hash(n+157.0), hash(n+158.0), f.x), f.y),
    mix(mix(hash(n+113.0), hash(n+114.0), f.x),
    mix(hash(n+270.0), hash(n+271.0), f.x), f.y), f.z);
}

const mat3 m = mat3(0.0, 1.6, 1.2, -1.6, 0.72, -0.96, -1.2, -0.96, 1.28);
float fbm(vec3 p){
    float f=0.0;
    f+=noise(p)/2.0; p=m*p*1.1;
    f+=noise(p)/4.0; p=m*p*1.2;
    f+=noise(p)/6.0; p=m*p*1.3;
    f+=noise(p)/12.0; p=m*p*1.4;
    f+=noise(p)/24.0;
    return f;
}

void main() {
    // Sky gradient
    vec3 skyColor = mix(vec3(0.6, 0.7, 0.9), vec3(0.1, 0.3, 0.6), rayDir.y);

    // Sun disk
    float sunAmount = smoothstep(0.997, 1.0, dot(normalize(rayDir), uSunDirection));
    skyColor += uSunColor * sunAmount * 100;

    // Cirrus clouds
    vec3 cirrusSeed = normalize(rayDir) * 10.0 + vec3(uTime * 0.05, 0.0, 0.0);
    float cirrusDensity = smoothstep(1.0 - uCirrus, 1.0, noise(cirrusSeed)) * 0.3;
    cirrusDensity = clamp(cirrusDensity, 0, 1);
    skyColor = mix(skyColor, vec3(1.0), cirrusDensity * rayDir.y);

    //    for (int i=0;i<3;i++){
    //        vec3 cumulusSeed = normalize(rayDir) * 8.0 + vec3(i * 0.01 + uTime * 0.1, 0.0, 0.0);
    //        skyColor = mix(skyColor, vec3(1.0), cumulusDensity * rayDir.y);
    //    }

    // Tiny dithering
    skyColor += noise(rayDir*1000.0)*0.01;

    FragColor = vec4(skyColor, 1.0);
    BrightColor = vec4(0, 0, 0, 1.0);
}
