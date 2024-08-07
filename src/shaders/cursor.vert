#version 460 core
layout(location = 0) in vec3 inPos;

uniform mat4 model;
uniform mat4 projection_view;

void main() { gl_Position = projection_view * model * vec4(inPos, 1.0); }