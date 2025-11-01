#version 420
out vec4 FragColor;
uniform sampler2D baseColor;

in VS_OUT {
    vec2 TexCoords;
} fs_in;

void main() {
    vec4 color = texture(baseColor, fs_in.TexCoords);
//    if (color.a < 0.1) discard;  // чтобы избавиться от артефактов
    FragColor = color;
}
