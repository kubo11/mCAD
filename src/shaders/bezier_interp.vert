#version 460 core

layout(location = 0) in vec3 inA;
layout(location = 1) in vec3 inB;
layout(location = 2) in vec3 inC;
layout(location = 3) in vec3 inD;
layout(location = 4) in vec3 inNextPoint;
layout(location = 5) in float inDt;

out vec3 inTessA;
out vec3 inTessB;
out vec3 inTessC;
out vec3 inTessD;
out vec3 inTessNextPoint;
out float inTessDt;

void main() {
  inTessA = inA;
  inTessB = inB;
  inTessC = inC;
  inTessD = inD;
  inTessNextPoint = inNextPoint;
  inTessDt = inDt;
}