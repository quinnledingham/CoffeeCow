#version 330 core 
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texture_coords;
layout (location = 3) in mat4 model;
layout (location = 7) in float life;

out float alpha;
out vec2 uv; 

layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

void main(void) 
{ 
    gl_Position = projection * view * model * vec4(position, 1.0f); 
    alpha = life;
    uv = texture_coords;
}