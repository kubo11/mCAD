#version 460 core

layout(location = 0) in vec3 inPos;

out vec3 inTessPos;

void main() { inTessPos = inPos; }