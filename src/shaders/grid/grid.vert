#version 430 core

in vec3 aPos;

uniform mat4 projection_view;

out vec3 nearPoint;
out vec3 farPoint;

vec3 UnprojectPoint(float x, float y, float z) {
    mat4 projection_view_inv = inverse(projection_view);
    vec4 unprojectedPoint =  projection_view_inv * vec4(x, y, z, 1.0);
    return unprojectedPoint.xyz / unprojectedPoint.w;
}

void main() {
    nearPoint = UnprojectPoint(aPos.x, aPos.y, -1.0).xyz;
    farPoint = UnprojectPoint(aPos.x, aPos.y, 1.0).xyz;
    gl_Position = vec4(aPos, 1.0);
}