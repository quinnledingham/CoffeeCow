#ifndef QLIB_SHADERS_H
#define QLIB_SHADERS_H

const char *basic_vs = "#version 330 core\n layout (location = 0) in vec3 position; layout (location = 1) in vec3 normal; layout (location = 2) in vec2 texture_coords; uniform mat4 model; uniform mat4 projection; uniform mat4 view; out vec2 uv; void main(void) { gl_Position = projection * view * model * vec4(position, 1.0f); uv = texture_coords;}";

const char *color_fs = "#version 330 core\n in vec2 uv; uniform vec4 user_color; out vec4 FragColor; void main() { FragColor  = vec4(user_color.x/255, user_color.y/255, user_color.z/255, user_color.w);}";

const char *tex_fs = "#version 330 core\n uniform sampler2D tex0; in vec2 uv; out vec4 FragColor; void main() { vec4 tex = texture(tex0, uv); FragColor = tex;}";

#endif //QLIB_SHADERS_H
