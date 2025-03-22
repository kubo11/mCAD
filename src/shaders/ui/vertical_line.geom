#version 460 core

layout(points) in;
layout(line_strip, max_vertices = 2) out;

uniform mat4 projection_view;
uniform vec3 beg_pos;

layout(location = 0) out vec3 fragPos;

void main() {
  gl_Position = projection_view * vec4(beg_pos, 1.0);
  fragPos = beg_pos;
  EmitVertex();
  vec3 end_pos = vec3(beg_pos.x, 0.0, beg_pos.z);
  gl_Position = projection_view * vec4(end_pos, 1.0);
  fragPos = end_pos;
  EmitVertex();
  EndPrimitive();
}