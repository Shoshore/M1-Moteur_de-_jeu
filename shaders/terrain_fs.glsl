#version 330 core

out vec4 FragColor;

flat in int whichTexture;
in vec2 TexCoords;

uniform sampler2D water;
uniform sampler2D sand;
uniform sampler2D grass;

void main()
{
    if (whichTexture == 0) { 
        FragColor = vec4(texture(water, TexCoords).rgb, 1.0);
    } else if (whichTexture == 1) {
        FragColor = vec4(texture(sand, TexCoords).rgb, 1.0);
    } else {
        FragColor = vec4(texture(grass, TexCoords).rgb, 1.0);
    }
}