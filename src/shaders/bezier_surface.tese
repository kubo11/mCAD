#version 460 core

layout(isolines) in;

uniform mat4 projection_view;
uniform int line_count;
uniform bool flip_uv;

layout(location = 0) out vec2 patchUV;

vec3 bezier2(vec3 a, vec3 b, float t) { return mix(a, b, t); }

vec3 bezier3(vec3 a, vec3 b, vec3 c, float t) { return mix(bezier2(a, b, t), bezier2(b, c, t), t); }

vec3 bezier4(vec3 a, vec3 b, vec3 c, vec3 d, float t) { return mix(bezier3(a, b, c, t), bezier3(b, c, d, t), t); }

void main() {
  float u;
  float v;
  if (flip_uv) {
    u = (gl_TessCoord.y * float(line_count + 1)) / float(line_count);
    v = gl_TessCoord.x;
  } else {
    u = gl_TessCoord.x;
    v = (gl_TessCoord.y * float(line_count + 1)) / float(line_count);
  }

  vec3 bezierV[4];
  for (int i = 0; i < 4; ++i) {
    bezierV[i] = bezier4(gl_in[4 * i].gl_Position.xyz, gl_in[4 * i + 1].gl_Position.xyz, gl_in[4 * i + 2].gl_Position.xyz, gl_in[4 * i + 3].gl_Position.xyz, u);
  }
  gl_Position = projection_view * vec4(bezier4(bezierV[0], bezierV[1], bezierV[2], bezierV[3], v), 1);
  patchUV = vec2(u, v);
}