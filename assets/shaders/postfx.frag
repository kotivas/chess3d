#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

struct Effects {
    bool quantization;
    int quantizationLevel;

    bool vignette;
    float vignetteIntensity;
    vec3 vignetteColor;
};

uniform sampler2D screenTexture;
uniform Effects effects;    
uniform vec2 resolution;

vec3 quantizeColor(vec3 color, int levels) {
    return floor(color * levels) / levels;
}
vec3 applyVignette(vec3 color){
    vec2 coord = (TexCoords - 0.5) * (resolution.x/resolution.y) * 2.0;
    float rf = 1.0 + dot(coord, coord) * effects.vignetteIntensity*effects.vignetteIntensity;
    float vignette = 1.0 / (rf * rf);
    
    return mix(color * vignette,
               color * effects.vignetteColor,
               (1.0 - vignette) * 0.5);
}

void main() {
    vec3 color = texture(screenTexture, TexCoords).rgb;

    if (effects.quantization) color = quantizeColor(color, effects.quantizationLevel);
    if (effects.vignette) color = applyVignette(color);



    FragColor = vec4(color, 1.0);
}