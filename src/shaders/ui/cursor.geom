#version 460 core
layout(points) in;
layout(line_strip, max_vertices = 6) out;

uniform mat4 projection_view;
uniform float far_plane;

layout(location = 0) out vec3 outColor;

void emit_colored_line(vec4 origin, vec3 direction, float len, vec3 color) {
  gl_Position = projection_view * origin;
  outColor = color;
  EmitVertex();

  gl_Position = projection_view * (len * vec4(direction, 0.0) + origin);
  EmitVertex();

  EndPrimitive();
}

void main() {
  float len = 5.0;
  vec4 cs_pos = projection_view * gl_in[0].gl_Position;
  float scale = cs_pos.z / far_plane;

  emit_colored_line(gl_in[0].gl_Position, vec3(1.0, 0.0, 0.0), len * scale, vec3(1.0, 0.0, 0.0));
  emit_colored_line(gl_in[0].gl_Position, vec3(0.0, 1.0, 0.0), len * scale, vec3(0.0, 1.0, 0.0));
  emit_colored_line(gl_in[0].gl_Position, vec3(0.0, 0.0, 1.0), len * scale, vec3(0.0, 0.0, 1.0));
}