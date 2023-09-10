#version 330 core 
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texture_coords;
layout (location = 3) in mat4 model;
layout (location = 7) in float life;

uniform mat4 projection; 
uniform mat4 view; 

out float alpha;
out vec2 uv; 

void main(void) 
{ 
    gl_Position = projection * view * model * vec4(position, 1.0f); 
    alpha = life;
    uv = texture_coords;
}
