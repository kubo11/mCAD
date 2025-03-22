#version 460 core
layout(location = 0) in vec3 fragPos;

uniform int anaglyph_state;
uniform vec3 beg_pos;

out vec4 FragColor;

const float segment_length = 0.01;

void main() {
  vec3 color = vec3(0.8, 0.8, 0.8);
  if (anaglyph_state == 1)
    color = vec3(1.0, 0.0, 0.0);
  else if (anaglyph_state == 2)
    color = vec3(0.0, 1.0, 1.0);

  FragColor = vec4(color, int(length(fragPos - beg_pos) / segment_length) % 2);
}