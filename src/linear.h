#ifndef LINEAR_H_
#define LINEAR_H_

typedef float vec3f[3];
typedef float mat4[16];

float deg_to_rad(const float deg);

void vec3f_add(vec3f out, const vec3f a, const vec3f b);
void vec3f_subtract(vec3f out, const vec3f a, const vec3f b);
void vec3f_scale(vec3f out, const vec3f a, const float x);
float vec3f_dot(const vec3f a, const vec3f b);
float vec3f_length(const vec3f a);
void vec3f_normalize(vec3f out, const vec3f a);
void vec3f_cross(vec3f out, const vec3f a, const vec3f b);
void vec3f_print(const char *str, const vec3f v);

void mat4_identity(mat4 out);
void mat4_translate(mat4 out, const float x, const float y, const float z);
void mat4_translate_in_place(mat4 out, const float x, const float y, const float z);
void mat4_ortho(mat4 out, const float left, const float right, const float bottom, const float top, const float near, const float far);
void mat4_perspective(mat4 out, const float fov, const float aspect, const float near, const float far);
void mat4_look_at(mat4 out, const vec3f eye, const vec3f center, const vec3f up);
void mat4_print(const char *str, const mat4 M);

#endif
