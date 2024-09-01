#version 460 core
layout(location = 0) in vec3 inColor;

out vec4 FragColor;
uniform int anaglyph_state;

void main() {
  if (anaglyph_state == 0)
    FragColor = vec4(inColor, 1.0);
  else if (anaglyph_state == 1)
    FragColor = vec4(1.0, 0.0, 0.0, 1.0);
  else
    FragColor = vec4(0.0, 1.0, 1.0, 1.0);
}