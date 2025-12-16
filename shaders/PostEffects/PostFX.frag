#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform sampler2D bloomBlur;

struct Effects {
    bool bloom;
    float gamma;
    bool quantization;
    int quantizationLevel;
    bool vignette;
    float vignetteIntensity;
    vec3 vignetteColor;
    float chromaticOffset;
    float saturation;
    float exposure;
};

uniform Effects effects;
uniform vec2 resolution;
uniform float time;

// Chromatic-aware sampling: sample three channels with small offsets along dir
vec3 sampleChromatic(vec2 uv, vec2 dir, float chromaScale) {
    vec2 d = normalize(dir + vec2(1e-6));
    // scale chroma with distance to center is handled by caller via chromaScale
    vec3 c;
    c.r = texture(screenTexture, uv + d * (chromaScale)).r;
    c.g = texture(screenTexture, uv).g;
    c.b = texture(screenTexture, uv - d * (chromaScale)).b;
    return c;
}

vec3 applyVignette(vec3 color){
    vec2 coord = (TexCoords - 0.5) * (resolution.x/resolution.y) * 2.0;
    float rf = 1.0 + dot(coord, coord) * effects.vignetteIntensity*effects.vignetteIntensity;
    float vignette = 1.0 / (rf * rf);
    return mix(color * vignette, color * effects.vignetteColor, (1.0 - vignette) * 0.5);
}

vec3 quantizeColor(vec3 color, int levels) {
    return floor(color * float(levels)) / float(max(levels, 1));
}

vec3 ACESFilm(vec3 x) {
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return clamp((x*(a*x+b))/(x*(c*x+d)+e), 0.0, 1.0);
}

void main() {
    vec3 color = texture(screenTexture, TexCoords).rgb;
    vec2 screenDir = (TexCoords - 0.5) * 2.0;

    if (effects.chromaticOffset > 0) color = sampleChromatic(TexCoords, screenDir, effects.chromaticOffset * 0.2);
    if (effects.quantization) color = quantizeColor(color, effects.quantizationLevel);
    if (effects.vignette) color = applyVignette(color);
    if (effects.bloom) color += texture(bloomBlur, TexCoords).rgb; // bloom blur blending

    color *= effects.exposure;
    color = ACESFilm(color); // tone mapping (ACES)
    // color grading
    float intensity = dot(color, vec3(0.2126, 0.7152, 0.0722));
    vec3 grayscale = vec3(intensity);
    color = mix(grayscale, color, effects.saturation); // saturation

    FragColor = vec4(pow(color, vec3(1.0 / effects.gamma)), 1.0); // w gamma correction
}
