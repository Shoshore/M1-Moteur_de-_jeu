#version 330 core
layout(location = 0) in vec2 inPos;   
layout(location = 1) in vec2 inUV;    
layout(location = 2) in vec4 inColor; 

out vec2 UV;
out vec4 Color;

void main() {
    UV = inUV;
    Color = inColor;
    gl_Position = vec4(inPos, 0.0, 1.0);
}