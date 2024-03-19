#version 430 core

in vec3 aPos;

layout(location = 1) out vec3 nearPoint;
layout(location = 2) out vec3 farPoint;

uniform mat4 projection_view;

vec3 UnprojectPoint(float x, float y, float z, mat4 projection_view) {
  vec4 unprojectedPoint = inverse(projection_view) * vec4(x, y, z, 1.0);
  return unprojectedPoint.xyz / unprojectedPoint.w;
}

void main() {
  nearPoint = UnprojectPoint(aPos.x, aPos.y, -1.0, projection_view);
  farPoint = UnprojectPoint(aPos.x, aPos.y, 1.0, projection_view);
  gl_Position = vec4(aPos, 1.0);
}
