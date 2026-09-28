#version 330 core

layout (location = 0) in vec3 vertices_position_modelspace;
layout (location = 1) in vec3 vertices_normals_modelspace;
layout (location = 2) in vec2 vertices_texcoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection; 

out vec2 TexCoords;
out vec3 WorldPos;
out vec3 Normal;

void main()
{
    vec4 worldPos = model * vec4(vertices_position_modelspace, 1.0);
    WorldPos = worldPos.xyz;

    mat3 normalMatrix = transpose(inverse(mat3(model)));
    Normal = normalize(normalMatrix * vertices_normals_modelspace);

    gl_Position = projection * view * worldPos;

    TexCoords = vertices_texcoords;
}

