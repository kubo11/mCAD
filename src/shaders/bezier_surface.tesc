#version 460 core

#define controlVerticesCount 16

in vec3 inTessPos[];

uniform int line_count;

layout(vertices = controlVerticesCount) out;
out vec3 tessPos[];

void main() {
  gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

  gl_TessLevelOuter[0] = line_count + 1;
  gl_TessLevelOuter[1] = 400;
}