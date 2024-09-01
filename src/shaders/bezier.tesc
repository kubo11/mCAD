#version 460 core

#define controlVerticesCount 4

in vec3 inTessPos[];

uniform mat4 projection_view;
uniform vec2 window_size;

layout(vertices = controlVerticesCount) out;
out vec3 tessPos[];
out float divisionY[];

int count_valid_vertices() {
  if (!isnan(inTessPos[3].x)) {
    return 4;
  } else if (!isnan(inTessPos[2].x)) {
    return 3;
  } else {
    return 2;
  }
}

void main() {
  tessPos[gl_InvocationID] = inTessPos[gl_InvocationID];
  int validVerticesCount = count_valid_vertices();
  if (gl_InvocationID % 4 == 0) {
    vec2 tessPosScreen[controlVerticesCount];
    for (int i = 0; i < validVerticesCount; ++i) {
      vec4 clipPos = projection_view * vec4(inTessPos[i], 1);
      clipPos /= clipPos.w;
      tessPosScreen[i] = (clipPos.xy + 1) / 2 * window_size;
    }
    int polylineLengthScreen = 0;
    for (int i = 1; i < validVerticesCount; ++i) {
      polylineLengthScreen += int(length(tessPosScreen[i] - tessPosScreen[i - 1]));
    }
    int approximationLevel = 1;
    int division = max(polylineLengthScreen / approximationLevel, 1);

    divisionY[gl_InvocationID] = min(division / 64 + 1, 64);
    gl_TessLevelOuter[0] = divisionY[gl_InvocationID];
    gl_TessLevelOuter[1] = divisionY[gl_InvocationID] == 1 ? division : 64;
  }
}