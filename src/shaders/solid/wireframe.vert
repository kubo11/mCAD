#version 430 core
layout(location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 projection_view;

void main() { gl_Position = projection_view * model * vec4(aPos, 1.0); }