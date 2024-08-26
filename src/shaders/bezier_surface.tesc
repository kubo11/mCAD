#version 460 core

#define controlVerticesCount 16

in vec3 inTessPos[];

uniform int line_count;

layout(vertices = controlVerticesCount) out;
out vec3 tessPos[];

void main() {
  tessPos[gl_InvocationID] = inTessPos[gl_InvocationID];

  if (gl_InvocationID == 0) {
    gl_TessLevelInner[0] = line_count - 1;
    gl_TessLevelInner[1] = line_count;

    gl_TessLevelOuter[0] = line_count;
    gl_TessLevelOuter[1] = line_count - 1;
    gl_TessLevelOuter[2] = line_count;
    gl_TessLevelOuter[3] = line_count - 1;
  }
}