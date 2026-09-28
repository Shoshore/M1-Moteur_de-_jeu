#version 330 core
in vec2 UV;
in vec4 Color;

uniform sampler2D spriteTex;

out vec4 fragColor;

void main() {
    vec4 tex = texture(spriteTex, UV);
    fragColor = tex * Color; 
}