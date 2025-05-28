#version 430 core
layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec2 fragUV;

uniform mat4 model;
uniform mat4 projection_view;

void main() { 
  gl_Position = projection_view * model * vec4(inPos, 1.0);
  fragUV = inUV;
}