#version 460 core

layout(location = 0) in vec2 fragUV;

out vec4 FragColor;

uniform vec3 color;
uniform int anaglyph_state;
uniform int useTexture;
uniform sampler2D textureSampler;

void main() {
  if (useTexture == 1) {
    vec3 texColor = texture(textureSampler, fragUV).rgb;
    if (texColor != vec3(0.0, 0.0, 0.0) && texColor != vec3(1.0, 1.0, 1.0)) {
      discard;
    }
  }
  if (anaglyph_state == 0)
    FragColor = vec4(color, 1.0);
  else if (anaglyph_state == 1)
    FragColor = vec4(1.0, 0.0, 0.0, 1.0);
  else
    FragColor = vec4(0.0, 1.0, 1.0, 1.0);
}