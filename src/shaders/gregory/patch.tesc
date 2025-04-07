#version 460 core

layout (vertices = 20) out;

uniform int line_count;
uniform bool filp_uv;

void main() {
    gl_TessLevelOuter[0] = line_count + 1;
    gl_TessLevelOuter[1] = 400;

    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}