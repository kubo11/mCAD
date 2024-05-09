#version 430 core

#define EPS 1e-6

in vec3 nearPoint;
in vec3 farPoint;

uniform mat4 projection_view;

out vec4 outColor;

float depth(vec3 fragPos3D) {
	vec4 coord = projection_view * vec4(fragPos3D, 1);
	return coord.z / coord.w;
}

float computeLinearDepth(vec3 pos) {
    float clip_space_depth = gl_FragDepth;
    float linearDepth = (2.0 * gl_DepthRange.near * gl_DepthRange.far) / (gl_DepthRange.near + gl_DepthRange.far - clip_space_depth * gl_DepthRange.diff); // get linear value between 0.01 and 100
    return linearDepth / gl_DepthRange.far; // normalize
}

vec4 grid(vec3 fragPos3D, float scale) {
    vec2 coord = fragPos3D.xz / scale; // use the scale variable to set the distance between the lines
    vec2 derivative = fwidth(coord);
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;
    float line = min(grid.x, grid.y);
    float minimumz = min(derivative.y, 1);
    float minimumx = min(derivative.x, 1);
    vec4 color = vec4(0.2, 0.2, 0.2, 1.0 - min(line, 1.0));
    // z axis
    if(fragPos3D.x > -5 * minimumx && fragPos3D.x < 5 * minimumx)
        color.z = 1.0;
    // x axis
    if(fragPos3D.z > -5 * minimumz && fragPos3D.z < 5 * minimumz)
        color.x = 1.0;
    return color;
}

void main() {
    float t = -nearPoint.y / (farPoint.y - nearPoint.y);
    vec3 fragPos3D = nearPoint + t * (farPoint - nearPoint);
    gl_FragDepth = min((gl_DepthRange.diff * depth(fragPos3D) + gl_DepthRange.near + gl_DepthRange.far) / 2, gl_DepthRange.far - EPS);
    outColor = grid(fragPos3D, 10) * float(t > 0.0);
    float fading = max(0, gl_FragDepth / gl_DepthRange.diff);
    outColor.a *= fading;
}
