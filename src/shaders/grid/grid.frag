#version 460 core

#define EPS 1e-6

in vec3 nearPoint;
in vec3 farPoint;

uniform mat4 projection_view;
uniform float near_plane;
uniform float far_plane;
uniform int anaglyph_state;

out vec4 outColor;

float depth(vec3 pos) {
  vec4 coord = projection_view * vec4(pos, 1.0);
  return coord.z / coord.w;
}

float linear_depth(vec3 pos) {
  vec4 clip_space_pos = projection_view * vec4(pos, 1.0);
  float clip_space_depth = (clip_space_pos.z / clip_space_pos.w) * 2.0 - 1.0;
  float linearDepth =
      (2.0 * near_plane * far_plane) / (far_plane + near_plane - clip_space_depth * (far_plane - near_plane));
  return max(linearDepth / far_plane, 0.001);
}

vec4 grid(vec3 pos, float scale) {
  vec2 coord = pos.xz / scale;
  vec2 derivative = fwidth(coord);
  vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;
  float line = min(grid.x, grid.y);
  float minimumz = min(derivative.y, 1);
  float minimumx = min(derivative.x, 1);
  vec4 color = vec4(0.2, 0.2, 0.2, 1.0 - min(line, 1.0));

  // z axis
  if (pos.x > -scale * minimumx && pos.x < scale * minimumx) color.z = 1.0;
  // x axis
  if (pos.z > -scale * minimumz && pos.z < scale * minimumz) color.x = 1.0;

  if (anaglyph_state == 1)
    color.xyz = vec3(1.0, 0.0, 0.0);
  else if (anaglyph_state == 2)
    color.xyz = vec3(0.0, 1.0, 1.0);

  return color;
}

void main() {
  float t = -nearPoint.y / (farPoint.y - nearPoint.y);
  vec3 pos = nearPoint + t * (farPoint - nearPoint);
  gl_FragDepth =
      min((gl_DepthRange.diff * depth(pos) + gl_DepthRange.near + gl_DepthRange.far) / 2, gl_DepthRange.far - EPS);
  vec4 primary_grid = grid(pos, 1.0);
  vec4 secondary_grid = grid(pos, 0.1);
  secondary_grid.a *= 0.5;
  outColor = max(primary_grid, secondary_grid) * float(t > 0.0);
  float ld = linear_depth(pos);
  if (!(ld > 0.001 && ld < 2.0)) ld = 2.0;
  float fading = max(0, (0.5 - ld));
  outColor.a *= fading;
}
