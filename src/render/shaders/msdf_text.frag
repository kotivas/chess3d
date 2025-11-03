#version 330 core
in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uAtlas;
uniform vec4 uColor;
uniform float uPxRange; // масштаб SDF -> пиксели

float median(float r, float g, float b) {
    return max(min(r,g), min(max(r,g), b));
}

void main() {
    vec3 msd = texture(uAtlas, vUV).rgb;
    float sd = median(msd.r, msd.g, msd.b) - 0.5; // signed distance normalized
    float dist = sd * uPxRange;                    // distance в пикселях

    // ширина перехода в пиксельном пространстве
    float w = fwidth(dist);
    // плавный переход вокруг нуля
    float alpha = smoothstep(-w, w, dist);

    if(alpha < 0.01) discard;
    // можно вернуть непреумножённый цвет (как у вас)
    FragColor = vec4(uColor.rgb, alpha * uColor.a);
}
