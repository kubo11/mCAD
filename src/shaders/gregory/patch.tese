#version 460 core

layout (isolines, equal_spacing, ccw) in;

uniform mat4 projection_view;
uniform int line_count;
uniform bool flip_uv;

vec4 bernstein(float t, int n) {
    mat4 base = mat4(0.0);
    base[0][0] = 1.0;
    float t1 = 1.0 - t;

    for (int j = 1; j <= n; j++) {
        base[j][0] = base[j - 1][0] * t1;

        for (int i = 1; i <= j; i++) {
            base[j][i] = base[j - 1][i] * t1 + base[j - 1][i - 1] * t;
        }
    }

    return vec4(base[n][0], base[n][1], base[n][2], base[n][3]);
}


vec3 deCasteljau2D(vec3[4][4] coefs, float u, float v) {
    vec4 uBernstein = bernstein(u, 3);
    vec4 vBernstein = bernstein(v, 3);

    vec3 res = vec3(0.0);

    for (int u = 0; u < 4; ++u) {
        vec3 tmp = vec3(0.0);

        tmp += coefs[u][0] * vBernstein[0];
        tmp += coefs[u][1] * vBernstein[1];
        tmp += coefs[u][2] * vBernstein[2];
        tmp += coefs[u][3] * vBernstein[3];

        res += tmp * uBernstein[u];
    }

    return res;
}

void main() {
    float u = flip_uv ? gl_TessCoord.x : gl_TessCoord.y * (line_count+1.f)/(line_count);
    float v = flip_uv ? gl_TessCoord.y * (line_count+1.f)/(line_count) : gl_TessCoord.x ;

    vec3 f12_19 = (v * gl_in[12].gl_Position.xyz + u * gl_in[19].gl_Position.xyz) / (u + v + 0.000001);
    vec3 f13_14 = ((1.0-v) * gl_in[13].gl_Position.xyz + u * gl_in[14].gl_Position.xyz) / (1.0 + u - v + 0.000001);
    vec3 f15_16 = (v * gl_in[16].gl_Position.xyz + (1.0-u) * gl_in[15].gl_Position.xyz) / (1.0 - u + v + 0.000001);
    vec3 f17_18 = ((1.0-v) * gl_in[17].gl_Position.xyz + (1.0-u) * gl_in[18].gl_Position.xyz) / (2.0 - u - v + 0.000001);

    vec3[4][4] coefficients = {
        vec3[4](gl_in[0].gl_Position.xyz, gl_in[1].gl_Position.xyz, gl_in[2].gl_Position.xyz, gl_in[3].gl_Position.xyz),
        vec3[4](gl_in[11].gl_Position.xyz, f12_19, f13_14, gl_in[4].gl_Position.xyz),
        vec3[4](gl_in[10].gl_Position.xyz, f17_18, f15_16, gl_in[5].gl_Position.xyz),
        vec3[4](gl_in[9].gl_Position.xyz, gl_in[8].gl_Position.xyz, gl_in[7].gl_Position.xyz, gl_in[6].gl_Position.xyz)
    };

    vec4 pos = vec4(deCasteljau2D(coefficients, u, v), 1.f);
    gl_Position = projection_view * pos;
}