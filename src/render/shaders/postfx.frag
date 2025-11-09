#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

struct Effects {
    float gamma;
    bool quantization;
    int quantizationLevel;
    bool vignette;
    float vignetteIntensity;
    vec3 vignetteColor;
    float chromaticOffset;// ~0.002..0.01
};

uniform sampler2D screenTexture;
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

void main() {
    vec3 color = texture(screenTexture, TexCoords).rgb;
    vec2 screenDir = (TexCoords - 0.5) * 2.0;
    color = sampleChromatic(TexCoords, screenDir, effects.chromaticOffset * 0.2);
    // optional post effects
    if (effects.quantization) color = quantizeColor(color, effects.quantizationLevel);
    if (effects.vignette) color = applyVignette(color);

    // gamma
    FragColor = vec4(pow(color, vec3(1.0 / effects.gamma)), 1.0);
}
