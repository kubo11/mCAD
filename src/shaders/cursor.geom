#version 460 core
layout(points) in;
layout(line_strip, max_vertices = 6) out;

uniform mat4 projection_view;
uniform vec2 window_size;

layout(location = 0) out vec3 outColor;

void emit_colored_line(vec4 origin, vec3 direction, float length, vec3 color) {
  gl_Position = origin;
  outColor = color;
  EmitVertex();

  vec3 corrected_direction = length * normalize((projection_view * vec4(direction, 0.0)).xyz);
  gl_Position /= gl_Position.w;
  gl_Position.xyz += corrected_direction;
  EmitVertex();

  EndPrimitive();
}

void main() {
  float length = 50.0;

  emit_colored_line(gl_in[0].gl_Position, vec3(1.0, 0.0, 0.0), length / window_size.x, vec3(1.0, 0.0, 0.0));
  emit_colored_line(gl_in[0].gl_Position, vec3(0.0, 1.0, 0.0), length / window_size.y, vec3(0.0, 1.0, 0.0));
  emit_colored_line(gl_in[0].gl_Position, vec3(0.0, 0.0, 1.0), length / window_size.x, vec3(0.0, 0.0, 1.0));
}