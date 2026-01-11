#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec3 rayDir;
uniform float uTime;
uniform vec3 uSunDirection;
uniform vec3 uAtmParams[10];
const float sunRadius = 0.1;

#define PI 3.14159265359
#define TAU 1.57079632679

// SIMPLEX NOISE AND CLOUDS

vec3 mod289(vec3 x) {
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x) {
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x) {
    return mod289(((x*34.0)+10.0)*x);
}

vec4 taylorInvSqrt(vec4 r)
{
    return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(vec3 v) {
    const vec2  C = vec2(1.0/6.0, 1.0/3.0);
    const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

    // First corner
    vec3 i  = floor(v + dot(v, C.yyy));
    vec3 x0 =   v - i + dot(i, C.xxx);

    // Other corners
    vec3 g = step(x0.yzx, x0.xyz);
    vec3 l = 1.0 - g;
    vec3 i1 = min(g.xyz, l.zxy);
    vec3 i2 = max(g.xyz, l.zxy);

    //   x0 = x0 - 0.0 + 0.0 * C.xxx;
    //   x1 = x0 - i1  + 1.0 * C.xxx;
    //   x2 = x0 - i2  + 2.0 * C.xxx;
    //   x3 = x0 - 1.0 + 3.0 * C.xxx;
    vec3 x1 = x0 - i1 + C.xxx;
    vec3 x2 = x0 - i2 + C.yyy;// 2.0*C.x = 1/3 = C.y
    vec3 x3 = x0 - D.yyy;// -1.0+3.0*C.x = -0.5 = -D.y

    // Permutations
    i = mod289(i);
    vec4 p = permute(permute(permute(
    i.z + vec4(0.0, i1.z, i2.z, 1.0))
    + i.y + vec4(0.0, i1.y, i2.y, 1.0))
    + i.x + vec4(0.0, i1.x, i2.x, 1.0));

    // Gradients: 7x7 points over a square, mapped onto an octahedron.
    // The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
    float n_ = 0.142857142857;// 1.0/7.0
    vec3  ns = n_ * D.wyz - D.xzx;

    vec4 j = p - 49.0 * floor(p * ns.z * ns.z);//  mod(p,7*7)

    vec4 x_ = floor(j * ns.z);
    vec4 y_ = floor(j - 7.0 * x_);// mod(j,N)

    vec4 x = x_ *ns.x + ns.yyyy;
    vec4 y = y_ *ns.x + ns.yyyy;
    vec4 h = 1.0 - abs(x) - abs(y);

    vec4 b0 = vec4(x.xy, y.xy);
    vec4 b1 = vec4(x.zw, y.zw);

    //vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
    //vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
    vec4 s0 = floor(b0)*2.0 + 1.0;
    vec4 s1 = floor(b1)*2.0 + 1.0;
    vec4 sh = -step(h, vec4(0.0));

    vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy;
    vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww;

    vec3 p0 = vec3(a0.xy, h.x);
    vec3 p1 = vec3(a0.zw, h.y);
    vec3 p2 = vec3(a1.xy, h.z);
    vec3 p3 = vec3(a1.zw, h.w);

    //Normalise gradients
    vec4 norm = taylorInvSqrt(vec4(dot(p0, p0), dot(p1, p1), dot(p2, p2), dot(p3, p3)));
    p0 *= norm.x;
    p1 *= norm.y;
    p2 *= norm.z;
    p3 *= norm.w;

    // Mix final noise value
    vec4 m = max(0.5 - vec4(dot(x0, x0), dot(x1, x1), dot(x2, x2), dot(x3, x3)), 0.0);
    m = m * m;
    return 105.0 * dot(m*m, vec4(dot(p0, x0), dot(p1, x1),
    dot(p2, x2), dot(p3, x3)));
}
float fbm(vec3 p) {
    float value = 0.0;
    float amplitude = 1.0;
    float frequency = 1.1;

    for (int i=0; i<5; i++) {
        value += amplitude * snoise(p * frequency);
        frequency *= 2.0;
        amplitude *= 0.5;
    }
    return value;
}

float Cloud(vec3 V){
    vec3 wind = vec3(0.001, 0, 0.000);

    float cloud = 0;

    cloud += fbm(V*3.0 + wind*uTime);


    return smoothstep(-0.5, 1.0, cloud) * smoothstep(0.1, 0.5, V.y);// density
}

// SKY COLORS

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

//float hgPhase(float cosTheta, float g) {
//    float g2 = g * g;
//    return (1.0 - g2) / pow(1.0 + g2 - 2.0 * g * cosTheta, 1.5);
//}

void main(void) {
    vec3 V = normalize(rayDir);
    float cos_theta = clamp(V.y, 0, 1);
    float cos_gamma = dot(V, uSunDirection);
    float gamma = acos(cos_gamma);

    // Day sky
    vec3 day = uAtmParams[9] * HosekWilkie(cos_theta, gamma, cos_gamma);
    float sun = smoothstep(cos(sunRadius), 1.0, cos_gamma);
    day += sun * uAtmParams[9];

    // Night sky
    vec3 night = vec3(0.2);

    // day and night mix
    float t = smoothstep(-0.1, 0.0, uSunDirection.y);
    vec3 sky = mix(vec3(0.2), day, t);

    vec3 cloud = Cloud(V) * uAtmParams[9];
    vec3 finalSky = mix(sky, vec3(1.0), cloud);

    float luminance = dot(finalSky, vec3(0.2126, 0.7152, 0.0722));
    FragColor = vec4(clamp(finalSky, 0, 1), 1.0);
    BrightColor = vec4(finalSky * step(1.0, luminance), 1.0);
}