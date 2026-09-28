#version 330 core

layout (location = 0) in vec3 vertices_position_modelspace;
layout (location = 1) in vec3 vertices_normals_modelspace;
layout (location = 2) in vec2 vertices_texcoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoords;
flat out int whichTexture;

void main()
{
    TexCoords = vertices_texcoords;

    vec4 worldPos = model * vec4(vertices_position_modelspace, 1.0);
    
    gl_Position = projection * view * worldPos;

    if (worldPos.y <= 0.0) {
        whichTexture = 0;
    } else if (worldPos.y < 0.5) {
        whichTexture = 1;
    } else {
        whichTexture = 2;
    }
}