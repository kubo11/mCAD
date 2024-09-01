#version 460 core
out vec4 FragColor;

uniform vec3 color;
uniform int anaglyph_state;

void main() {
  if (anaglyph_state == 0)
    FragColor = vec4(color, 1.0);
  else if (anaglyph_state == 1)
    FragColor = vec4(1.0, 0.0, 0.0, 1.0);
  else
    FragColor = vec4(0.0, 1.0, 1.0, 1.0);
}