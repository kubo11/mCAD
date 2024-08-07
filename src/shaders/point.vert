#version 460 core
layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inModelPos;
layout(location = 2) in vec3 inColor;

uniform mat4 projection_view;
uniform vec2 window_size;

layout(location = 0) out vec3 outColor;

void main() {
  float size = 10.0;

  gl_Position = projection_view * vec4(inModelPos, 1.0);
  gl_Position /= gl_Position.w;
  gl_Position.x += size / window_size.x * inPos.x;
  gl_Position.y += size / window_size.y * inPos.y;
  outColor = inColor;
}