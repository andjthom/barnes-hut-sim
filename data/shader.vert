#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aOffset;

uniform mat4 view;
uniform mat4 projection;

out vec3 color;

void main() {
    color = vec3(1.0);

    mat4 model = mat4(1.0) * 200.0;
    model[3] = vec4(aOffset, 1.0);
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
