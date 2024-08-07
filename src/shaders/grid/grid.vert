#version 460 core

in vec2 inPos;

uniform mat4 projection_view;

out vec3 nearPoint;
out vec3 farPoint;

vec3 unproject_point(vec3 point) {
  mat4 projection_view_inverse = inverse(projection_view);
  vec4 unprojected_point = projection_view_inverse * vec4(point, 1.0);
  return unprojected_point.xyz / unprojected_point.w;
}

void main() {
  nearPoint = unproject_point(vec3(inPos, -1.0));
  farPoint = unproject_point(vec3(inPos, 1.0));
  gl_Position = vec4(inPos, 0.0, 1.0);
}