#include "./linear.h"
#include <stdio.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

float deg_to_rad(const float deg)
{
    return deg * 0.01745329252f;
}

void vec3f_add(vec3f out, const vec3f a, const vec3f b)
{
    out[0] = a[0] + b[0];
    out[1] = a[1] + b[1];
    out[2] = a[2] + b[2];
}

void vec3f_subtract(vec3f out, const vec3f a, const vec3f b)
{
    out[0] = a[0] - b[0];
    out[1] = a[1] - b[1];
    out[2] = a[2] - b[2];
}

void vec3f_scale(vec3f out, const vec3f a, const float x)
{
    out[0] = a[0] * x;
    out[1] = a[1] * x;
    out[2] = a[2] * x;
}

float vec3f_dot(const vec3f a, const vec3f b)
{
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

float vec3f_length(const vec3f a)
{
    return sqrtf(vec3f_dot(a, a));
}

void vec3f_normalize(vec3f out, const vec3f a)
{
    float x = 1.f / vec3f_length(a);
    vec3f_scale(out, a, x);
}

void vec3f_cross(vec3f out, const vec3f a, const vec3f b)
{
    out[0] = a[1] * b[2] - a[2] * b[1];
    out[1] = a[2] * b[0] - a[0] * b[2];
    out[2] = a[0] * b[1] - a[1] * b[0];
}

void vec3f_print(const char *str, const vec3f v)
{
    printf("%s", str);
    for (int i = 0; i < 3; i++) {
        printf("\t%.6f", v[i]);
    }
    printf("\n");
}

void mat4_identity(mat4 out)
{
    // x-col
    out[0] = 1.f;
    out[1] = 0.f;
    out[2] = 0.f;
    out[3] = 0.f;
    // y-col
    out[4] = 0.f;
    out[5] = 1.f;
    out[6] = 0.f;
    out[7] = 0.f;
    // z-col
    out[8] = 0.f;
    out[9] = 0.f;
    out[10] = 1.f;
    out[11] = 0.f;
    // w-col
    out[12] = 0.f;
    out[13] = 0.f;
    out[14] = 0.f;
    out[15] = 1.f;
}

void mat4_translate(mat4 out, const float x, const float y, const float z)
{
    mat4_identity(out);
    out[12] = x;
    out[13] = y;
    out[14] = z;
}

void mat4_translate_in_place(mat4 out, const float x, const float y, const float z)
{
    out[12] += out[0] * x + out[4] * y + out[8] * z;
    out[13] += out[1] * x + out[5] * y + out[9] * z;
    out[14] += out[2] * x + out[6] * y + out[10] * z;
    out[15] += out[3] * x + out[7] * y + out[11] * z;
}

void mat4_ortho(mat4 out, const float l, const float r, const float b, const float t,
        const float n, const float f)
{
    // x-col
    out[0] = 2.f / (r - l);
    out[1] = 0.f;
    out[2] = 0.f;
    out[3] = 0.f;
    // y-col
    out[4] = 0.f;
    out[5] = 2.f / (t - b);
    out[6] = 0.f;
    out[7] = 0.f;
    // z-col
    out[8] = 0.f;
    out[9] = 0.f;
    out[10] = -2.f / (f - n);
    out[11] = 0.f;
    // w-col
    out[12] = -(r + l) / (r - l);
    out[13] = -(t + b) / (t - b);
    out[14] = -(f + n) / (f - n);
    out[15] = 1.f;
}

void mat4_perspective(mat4 out, const float fov, const float aspect, const float n, const float f)
{
    const float s = 1.f / tanf(fov / 2.f);

    // x-col
    out[0] = s / aspect;
    out[1] = 0.f;
    out[2] = 0.f;
    out[3] = 0.f;
    // y-col
    out[4] = 0.f;
    out[5] = s;
    out[6] = 0.f;
    out[7] = 0.f;
    // z-col
    out[8] = 0.f;
    out[9] = 0.f;
    out[10] = -((f + n) / (f - n));
    out[11] = -1.f;
    // w-col
    out[12] = 0.f;
    out[13] = 0.f;
    out[14] = -((2.f * f * n) / (f - n));
    out[15] = 0.f;
}

void mat4_look_at(mat4 out, const vec3f eye, const vec3f center, const vec3f up)
{
    vec3f f;
    vec3f_subtract(f, center, eye);
    vec3f_normalize(f, f);

    vec3f s;
    vec3f_cross(s, f, up);
    vec3f_normalize(s, s);

    vec3f u;
    vec3f_cross(u, s, f);

    // x-col
    out[0] = s[0];
    out[1] = u[0];
    out[2] = -f[0];
    out[3] = 0.f;
    // y-col
    out[4] = s[1];
    out[5] = u[1];
    out[6] = -f[1];
    out[7] = 0.f;
    // z-col
    out[8] = s[2];
    out[9] = u[2];
    out[10] = -f[2];
    out[11] = 0.f;
    // w-col
    out[12] = 0.f;
    out[13] = 0.f;
    out[14] = 0.f;
    out[15] = 1.f;

    mat4_translate_in_place(out, -eye[0], -eye[1], -eye[2]);
}

void mat4_print(const char *str, const mat4 M)
{
    printf("%s:\n", str);
    int j = 0;
    for (int i = 0; i < 16; i++) {
        j++;
        printf("\t%.6f", M[i]);
        if (j == 4) {
            printf("\n");
            j = 0;
        }
    }
}
