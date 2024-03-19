#version 430 core

layout(location = 1) in vec3 nearPoint;
layout(location = 2) in vec3 farPoint;

out vec4 outColor;

uniform mat4 projection_view;

vec4 grid(vec3 fragPos3D, float scale) {
  vec2 coord = fragPos3D.xz * scale;
  vec2 derivative = fwidth(coord);
  vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;
  float line = min(grid.x, grid.y);
  float minimumz = min(derivative.y, 1.0);
  float minimumx = min(derivative.x, 1.0);
  if (1.0 - min(line, 1.0) < 0.5) {
    discard;
  }
  vec4 color = vec4(0.2, 0.2, 0.2, 1.0 - min(line, 1.0));
  // z axis
  if (fragPos3D.x > -0.5 * minimumx && fragPos3D.x < 0.5 * minimumx)
    color.z = 1.0;
  // x axis
  if (fragPos3D.z > -0.5 * minimumz && fragPos3D.z < 0.5 * minimumz)
    color.x = 1.0;
  return color;
}

float computeDepth(vec3 pos, mat4 projection_view) {
  vec4 clip_space_pos = projection_view * vec4(pos, 1.0);
  return (clip_space_pos.z / clip_space_pos.w);
}

void main() {
  float t = -nearPoint.y / (farPoint.y - nearPoint.y);
  vec3 fragPos3D = nearPoint + t * (farPoint - nearPoint);
  // gl_FragDepth = computeDepth(fragPos3D, projection_view);
  gl_FragDepth =
      ((gl_DepthRange.diff * computeDepth(fragPos3D, projection_view)) +
       gl_DepthRange.near + gl_DepthRange.far) /
      2.0;
  outColor = grid(fragPos3D, 1) * float(t > 0);

  // vec2 coord = fragPos3D.xz;

  // vec2 grid = abs(fract(coord - 0.5) - 0.5) / fwidth(coord);
  // float line = min(grid.x, grid.y);
  // float color = 1.0 - min(line, 1.0);

  // color = pow(color, 1.0 / 2.2);
  // outColor = vec4(color);
}
