#version 460 core

in vec3 inTessA[];
in vec3 inTessB[];
in vec3 inTessC[];
in vec3 inTessD[];
in vec3 inTessNextPoint[];
in float inTessDt[];

uniform mat4 projection_view;
uniform vec2 window_size;

layout(vertices = 1) out;
out vec3 tessA[];
out vec3 tessB[];
out vec3 tessC[];
out vec3 tessD[];
out float tessDt[];
out float divisionY[];

vec2 screenPos(vec3 world_pos) {
  vec4 clip_pos = projection_view * vec4(world_pos, 1);
  clip_pos /= clip_pos.w;
  return (clip_pos.xy + 1) / 2 * window_size;
}

void main() {
  tessA[gl_InvocationID] = inTessA[gl_InvocationID];
  tessB[gl_InvocationID] = inTessB[gl_InvocationID];
  tessC[gl_InvocationID] = inTessC[gl_InvocationID];
  tessD[gl_InvocationID] = inTessD[gl_InvocationID];
  tessDt[gl_InvocationID] = inTessDt[gl_InvocationID];

  int poly_length = int(length(screenPos(inTessNextPoint[gl_InvocationID]) - screenPos(inTessA[gl_InvocationID])));
  int approximation_level = 1;
  int division = max(poly_length / approximation_level, 1);

  divisionY[gl_InvocationID] = min(division / 64 + 1, 64);
  gl_TessLevelOuter[0] = divisionY[0];
  gl_TessLevelOuter[1] = divisionY[0] == 1 ? division : 64;
}