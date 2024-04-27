#version 430 core

layout (isolines) in;
in vec3 tessPos[];
in float divisionY[];

uniform mat4 projection_view;

vec3 bezier2(vec3 a, vec3 b, float t);
vec3 bezier3(vec3 a, vec3 b, vec3 c, float t);
vec3 bezier4(vec3 a, vec3 b, vec3 c, vec3 d, float t);

void main()
{
	float t = gl_TessCoord.x / divisionY[0] + gl_TessCoord.y;
	if (!isnan(tessPos[3].x)) {
	    gl_Position = projection_view * vec4(bezier4(tessPos[0], tessPos[1], tessPos[2], tessPos[3], t), 1);
	}
	else if (!isnan(tessPos[2].x)) {
	    gl_Position = projection_view * vec4(bezier3(tessPos[0], tessPos[1], tessPos[2], t), 1);
	}
	else {
	    gl_Position = projection_view * vec4(bezier2(tessPos[0], tessPos[1], t), 1);
	}
}

vec3 bezier2(vec3 a, vec3 b, float t)
{
	return mix(a, b, t);
}

vec3 bezier3(vec3 a, vec3 b, vec3 c, float t)
{
	return mix(bezier2(a, b, t), bezier2(b, c, t), t);
}

vec3 bezier4(vec3 a, vec3 b, vec3 c, vec3 d, float t)
{
	return mix(bezier3(a, b, c, t), bezier3(b, c, d, t), t);
}