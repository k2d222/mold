#pragma once

#ifndef VEC_MATH_USE_SSE_H
#define VEC_MATH_USE_SSE_H 1
#endif

#ifdef VEC_MATH_USE_SSE_H
#include "md_simd.h"
#endif

#include "md_compiler.h"
#if MD_COMPILER_MSVC
#pragma warning( disable : 4201 ) // nameless structs
#endif

#include "md_common.h"
#include <stdint.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef union vec2_t {
    //union {
        struct {
            float x, y;
        };
        float elem[2];
    //};
#ifdef __cplusplus
    float& operator[](int64_t i)       { return elem[i]; }
    const float& operator[](int64_t i) const { return elem[i]; }
#endif
} vec2_t;

typedef union vec3_t {
    //union {
        struct {
            float x, y, z;  
        };
        float elem[3];
    //};
#ifdef __cplusplus
    float& operator[](int64_t i)       { return elem[i]; }
    const float& operator[](int64_t i) const { return elem[i]; }
#endif
} vec3_t;

typedef union vec4_t {
    //union {
        struct {
            float x, y, z, w;
        };
        float elem[4];
#if VEC_MATH_USE_SSE_H
        __m128 mm128;
#endif
    //};
#ifdef __cplusplus
    float& operator[](int64_t i)       { return elem[i]; }
    const float& operator[](int64_t i) const { return elem[i]; }
#endif
} vec4_t;

typedef union quat_t {
    //union {
        struct {
            float x, y, z, w;
        };
        float elem[4];
#if VEC_MATH_USE_SSE_H
        __m128 mm128;
#endif
    //};
#ifdef __cplusplus
        float& operator[](int64_t i)       { return elem[i]; }
        const float& operator[](int64_t i) const { return elem[i]; }
#endif
} quat_t;

typedef union mat2_t {
    //union {
        float elem[2][2];
        vec2_t col[2];
    //};
#ifdef __cplusplus
    vec2_t& operator[](int64_t i)       { return col[i]; }
    const vec2_t& operator[](int64_t i) const { return col[i]; }
#endif
} mat2_t;

typedef union mat3_t {
    //union {
        float elem[3][3];
        vec3_t col[3];
    //};
#ifdef __cplusplus
    vec3_t& operator[](int64_t i)       { return col[i]; }
    const vec3_t& operator[](int64_t i) const { return col[i]; }
#endif
} mat3_t;

typedef union mat4_t {
    //union {
        float elem[4][4];
        vec4_t col[4];
    //};
#ifdef __cplusplus
    vec4_t& operator[](int64_t i)       { return col[i]; }
    const vec4_t& operator[](int64_t i) const { return col[i]; }
#endif
} mat4_t;

static inline float stepf(float edge, float x) { return (float)((x - edge) > 0); }
static inline double step(double edge, double x) { return (double)((x - edge) > 0); }

static inline float fractf(float x) { return x - (int32_t)x; }
static inline double fract(double x) { return x - (int64_t)x; }

static inline float signf(float x)  { return (float)((x > 0) - (x < 0)); }
static inline double sign(double x) { return (double)((x > 0) - (x < 0)); }

static inline float deperiodizef(float val, float ref, float period) {
    float d = (val - ref) / period;
    d = fractf(d);
    d = d - signf(d) * (float)(fabsf(d) > 0.5f);
    return d * period + ref;
}

static inline double deperiodize(double val, double ref, double period) {
    double d = (val - ref) / period;
    d = fract(d);
    d = d - sign(d) * (double)(fabs(d) > 0.5);
    return d * period + ref;
}

static inline float lerpf(float a, float b, float t) {
    t = CLAMP(t, 0.0f, 1.0f);
    return a * (1.0f - t) + b * t;
}

// VEC2 OPERATIONS
static inline vec2_t vec2_from_vec3(vec3_t v) {
    vec2_t r = {v.x, v.y};
    return r;
}

static inline vec2_t vec2_from_vec4(vec4_t v) {
    vec2_t r = {v.x, v.y};
    return r;
}

static inline vec2_t vec2_add(vec2_t a, vec2_t b) {
    vec2_t res = {a.x + b.x, a.y + b.y};
    return res;
}

static inline vec2_t vec2_add_f(vec2_t a, float f) {
    vec2_t res = {a.x + f, a.y + f};
    return res;
}

static inline vec2_t vec2_sub(vec2_t a, vec2_t b) {
    vec2_t res = {a.x - b.x, a.y - b.y};
    return res;
}

static inline vec2_t vec2_sub_f(vec2_t a, float s) {
    vec2_t res = {a.x - s, a.y - s};
    return res;
}

static inline vec2_t vec2_mul(vec2_t a, vec2_t b) {
    vec2_t res = {a.x * b.x, a.y * b.y};
    return res;
}

static inline vec2_t vec2_mul_f(vec2_t a, float s) {
    vec2_t res = {a.x * s, a.y * s};
    return res;
}

static inline vec2_t vec2_madd(vec2_t a, vec2_t b, vec2_t c) {
    vec2_t res = {a.x * b.x + c.x, a.y * b.y + c.y};
    return res;
}

static inline vec2_t vec2_div(vec2_t a, vec2_t b) {
    vec2_t res = {a.x / b.x, a.y / b.y};
    return res;
}

static inline vec2_t vec2_div_f(vec2_t a, float s) {
    vec2_t res = {a.x / s, a.y / s};
    return res;
}

static inline float vec2_dot(vec2_t a, vec2_t b) {
    return a.x * b.x + a.y * b.y;
}

static inline float vec2_length(vec2_t v) {
    float l2 = vec2_dot(v,v);
    vec4_t r = {
        .mm128 = _mm_sqrt_ss(_mm_set1_ps(l2))
    };
    return r.x;
}

static inline float vec2_dist(vec2_t a, vec2_t b) {
    vec2_t d = vec2_sub(a, b);
    return vec2_length(d);
}

static inline vec2_t vec2_normalize(vec2_t v) {
    float len = vec2_length(v);
    if (len > 1.0e-5) {
        vec2_t res = {v.x / len, v.y / len};
        return res;
    }

    vec2_t res = {0, 0};
    return res;
}

static inline vec2_t vec2_lerp(vec2_t a, vec2_t b, float t) {
    ASSERT(0 <= t && t <= 1);
    return vec2_add(vec2_mul_f(a, 1.0f - t), vec2_mul_f(b, t));
}

static inline vec2_t vec2_fract(vec2_t v) {
    v.x = fractf(v.x);
    v.y = fractf(v.y);
    return v;
}

// VEC3 OPERATIONS

static inline vec3_t vec3_fract(vec3_t v) {
    v.x = fractf(v.x);
    v.y = fractf(v.y);
    v.z = fractf(v.z);
    return v;
}

static inline vec3_t vec3_abs(vec3_t v) {
    v.x = fabsf(v.x);
    v.y = fabsf(v.y);
    v.z = fabsf(v.z);
    return v;
}

static inline vec3_t vec3_clamp(vec3_t v, vec3_t min, vec3_t max) {
    v.x = CLAMP(v.x, min.x, max.x);
    v.y = CLAMP(v.y, min.y, max.y);
    v.z = CLAMP(v.z, min.z, max.z);
    return v;
}

static inline vec3_t vec3_clamp_f(vec3_t v, float min, float max) {
    v.x = CLAMP(v.x, min, max);
    v.y = CLAMP(v.y, min, max);
    v.z = CLAMP(v.z, min, max);
    return v;
}

static inline vec3_t vec3_from_vec2(vec2_t v, float z) {
    vec3_t r = {v.x, v.y, z};
    return r;
}

static inline vec3_t vec3_from_vec4(vec4_t v) {
    vec3_t r = {v.x, v.y, v.z};
    return r;
}

static inline vec3_t vec3_add(vec3_t a, vec3_t b) {
    vec3_t res = {a.x + b.x, a.y + b.y, a.z + b.z};
    return res;
}

static inline vec3_t vec3_add_f(vec3_t a, float f) {
    vec3_t res = {a.x + f, a.y + f, a.z + f};
    return res;
}

static inline vec3_t vec3_sub(vec3_t a, vec3_t b) {
    vec3_t res = {a.x - b.x, a.y - b.y, a.z - b.z};
    return res;
}

static inline vec3_t vec3_sub_f(vec3_t a, float f) {
    vec3_t res = {a.x - f, a.y - f, a.z - f};
    return res;
}

static inline vec3_t vec3_mul(vec3_t a, vec3_t b) {
    vec3_t res = {a.x * b.x, a.y * b.y, a.z * b.z};
    return res;
}

static inline vec3_t vec3_mul_f(vec3_t a, float f) {
    vec3_t res = {a.x * f, a.y * f, a.z * f};
    return res;
}

static inline vec3_t vec3_div(vec3_t a, vec3_t b) {
    vec3_t res = {a.x / b.x, a.y / b.y, a.z / b.z};
    return res;
}

static inline vec3_t vec3_div_f(vec3_t a, float f) {
    vec3_t res = {a.x / f, a.y / f, a.z / f};
    return res;
}

static inline vec3_t vec3_cross(vec3_t a, vec3_t b) {
    vec3_t res = {
        a.y * b.z - b.y * a.z,
        a.z * b.x - b.z * a.x,
        a.x * b.y - b.x * a.y};
    return res;
}

static inline float vec3_dot(vec3_t a, vec3_t b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline float vec3_length(vec3_t v) {
    float l2 = vec3_dot(v,v);
    vec4_t r = {
        .mm128 = _mm_sqrt_ss(_mm_set1_ps(l2))
    };
    return r.x;
}

static inline float vec3_distance(vec3_t a, vec3_t b) {
    vec3_t d = vec3_sub(a, b);
    return vec3_length(d);
}

static inline float vec3_distance_squared(vec3_t a, vec3_t b) {
    vec3_t d = vec3_sub(a, b);
    return vec3_dot(d, d);
}

static inline vec3_t vec3_normalize(vec3_t v) {
    float len = vec3_length(v);
    if (len > 1.0e-5) {
        vec3_t res = {v.x / len, v.y / len, v.z / len};
        return res;
    }

    vec3_t res = {0, 0, 0};
    return res;
}

static inline vec3_t vec3_lerp(vec3_t a, vec3_t b, float t) {
    ASSERT(0 <= t && t <= 1);
    return vec3_add(vec3_mul_f(a, 1.0f - t), vec3_mul_f(b, t));
}

// VEC4 OPERATIONS
static inline vec4_t vec4_zero() {
    vec4_t res;
#if VEC_MATH_USE_SSE_H
    res.mm128 = _mm_setzero_ps();
#else
    res.x = 0;
    res.y = 0;
    res.z = 0;
    res.w = 0;
#endif
    return res;
}

static inline vec4_t vec4_from_float(float v) {
    vec4_t res;
#if VEC_MATH_USE_SSE_H
    res.mm128 = _mm_set1_ps(v);
#else
    res.x = v;
    res.y = v;
    res.z = v;
    res.w = v;
#endif
    return res;
}

static inline vec4_t vec4_from_vec3(vec3_t v, float w) {
    vec4_t r = {v.x, v.y, v.z, w};
    return r;
}

static inline vec4_t vec4_mul(vec4_t a, vec4_t b) {
    vec4_t c = {0,0,0,0};
#if VEC_MATH_USE_SSE_H
    c.mm128 = _mm_mul_ps(a.mm128, b.mm128);
#else
    c.x = a.x * b.x;
    c.y = a.y * b.y;
    c.z = a.z * b.z;
    c.w = a.w * b.w;
#endif
    return c;
}

static inline vec4_t vec4_mul_f(vec4_t a, float s) {
    vec4_t c = {0,0,0,0};
#if VEC_MATH_USE_SSE_H
    c.mm128 = _mm_mul_ps(a.mm128, _mm_set1_ps(s));
#else
    c.x = a.x * s;
    c.y = a.y * s;
    c.z = a.z * s;
    c.w = a.w * s;
#endif
    return c;
}

static inline vec4_t vec4_div(vec4_t a, vec4_t b) {
    vec4_t c = {0,0,0,0};
#if VEC_MATH_USE_SSE_H
    c.mm128 = _mm_div_ps(a.mm128, b.mm128);
#else
    c.x = a.x / b.x;
    c.y = a.y / b.y;
    c.z = a.z / b.z;
    c.w = a.w / b.w;
#endif
    return c;
}

static inline vec4_t vec4_div_f(vec4_t a, float s) {
    vec4_t c = {0,0,0,0};
#if VEC_MATH_USE_SSE_H
    c.mm128 = _mm_div_ps(a.mm128, _mm_set1_ps(s));
#else
    c.x = a.x / s;
    c.y = a.y / s;
    c.z = a.z / s;
    c.w = a.w / s;
#endif
    return c;
}

static inline vec4_t vec4_add(vec4_t a, vec4_t b) {
    vec4_t c = {0,0,0,0};
#if VEC_MATH_USE_SSE_H
    c.mm128 = _mm_add_ps(a.mm128, b.mm128);
#else
    c.x = a.x + b.x;
    c.y = a.y + b.y;
    c.z = a.z + b.z;
    c.w = a.w + b.w;
#endif
    return c;
}

static inline vec4_t vec4_add_f(vec4_t a, float s) {
    vec4_t c = {0,0,0,0};
#if VEC_MATH_USE_SSE_H
    c.mm128 = _mm_add_ps(a.mm128, _mm_set1_ps(s));
#else
    c.x = a.x + s;
    c.y = a.y + s;
    c.z = a.z + s;
    c.w = a.w + s;
#endif
    return c;
}

static inline vec4_t vec4_sub(vec4_t a, vec4_t b) {
    vec4_t c = {0,0,0,0};
#if VEC_MATH_USE_SSE_H
    c.mm128 = _mm_sub_ps(a.mm128, b.mm128);
#else
    c.x = a.x - b.x;
    c.y = a.y - b.y;
    c.z = a.z - b.z;
    c.w = a.w - b.w;
#endif
    return c;
}

static inline vec4_t vec4_sub_f(vec4_t a, float s) {
    vec4_t c;
#if VEC_MATH_USE_SSE_H
    c.mm128 = _mm_sub_ps(a.mm128, _mm_set1_ps(s));
#else
    c.x = a.x - s;
    c.y = a.y - s;
    c.z = a.z - s;
    c.w = a.w - s;
#endif
    return c;
}

static inline float vec4_dot(vec4_t a, vec4_t b) {
    vec4_t res = vec4_mul(a, b);
    return res.x + res.y + res.z + res.w;
}

static inline float vec4_distance_squared(vec4_t a, vec4_t b) {
    vec4_t d = vec4_sub(a, b);
    return vec4_dot(d, d);
}

static inline vec4_t vec4_lerp(vec4_t a, vec4_t b, float t) {
    t = CLAMP(t, 0.0f, 1.0f);
    vec4_t r = vec4_add(vec4_mul_f(a, 1.0f - t), vec4_mul_f(b, t));
    return r;
}

static inline vec4_t vec4_min(vec4_t a, vec4_t b) {
    vec4_t c;
#if VEC_MATH_USE_SSE_H
    c.mm128 = _mm_min_ps(a.mm128, b.mm128);
#else
    c.x = MIN(a.x, b.x);
    c.y = MIN(a.y, b.y);
    c.z = MIN(a.z, b.z);
    c.w = MIN(a.w, b.w);
#endif
    return c;
}

static inline vec4_t vec4_max(vec4_t a, vec4_t b) {
    vec4_t c;
#if VEC_MATH_USE_SSE_H
    c.mm128 = _mm_max_ps(a.mm128, b.mm128);
#else
    c.x = MAX(a.x, b.x);
    c.y = MAX(a.y, b.y);
    c.z = MAX(a.z, b.z);
    c.w = MAX(a.w, b.w);
#endif
    return c;
}

static inline vec4_t vec4_abs(vec4_t v) {
    vec4_t r;
#if VEC_MATH_USE_SSE_H
    r.mm128 = md_simd_abs_f128(v.mm128);
#else
    c.x = ABS(a.x, b.x);
    c.y = ABS(a.y, b.y);
    c.z = ABS(a.z, b.z);
    c.w = ABS(a.w, b.w);
#endif
    return r;
}

static inline vec4_t vec4_from_u32(uint32_t rgba) {
    vec4_t v = {(float)((rgba >> 0) & 0xFF), (float)((rgba >> 8) & 0xFF), (float)((rgba >> 16) & 0xFF), (float)((rgba >> 24) & 0xFF)};
    return vec4_mul_f(v, 1.0f / 255.0f);
}

static inline uint32_t u32_from_vec4(vec4_t v) {
    uint32_t out;
    out  = ((uint32_t)(CLAMP(v.x, 0.0f, 1.0f) * 255.0f + 0.5f)) << 0;
    out |= ((uint32_t)(CLAMP(v.y, 0.0f, 1.0f) * 255.0f + 0.5f)) << 8;
    out |= ((uint32_t)(CLAMP(v.z, 0.0f, 1.0f) * 255.0f + 0.5f)) << 16;
    out |= ((uint32_t)(CLAMP(v.w, 0.0f, 1.0f) * 255.0f + 0.5f)) << 24;
    return out;
}

// quat
quat_t quat_angle_axis(float angle, vec3_t axis);

static inline quat_t quat_mul(quat_t a, quat_t b) {
    quat_t c = {
        a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
        a.w * b.y + a.y * b.w + a.z * b.x - a.x * b.z,
        a.w * b.z + a.z * b.w + a.x * b.y - a.y * b.x,
        a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z
    };
    return c;
}

static inline quat_t quat_mul_f(quat_t q, float s) {
    quat_t c = {
        q.x * s,
        q.y * s,
        q.z * s,
        q.w * s
    };
    return c;
}

static inline vec3_t quat_mul_vec3(quat_t q, vec3_t v) {
    vec3_t u = {q.x, q.y, q.z};
    vec3_t t = vec3_mul_f(vec3_cross(u, v), 2.0f);
    vec3_t r = vec3_add(v, vec3_mul_f(t, q.w));
    return vec3_add(r, vec3_cross(u, t));
}

static inline quat_t quat_conj(quat_t q) {
    quat_t r = {
        -q.x,
        -q.y,
        -q.z,
         q.w
    };

    return r;
}

quat_t quat_normalize(quat_t q);

quat_t quat_from_mat3(mat3_t M);
quat_t quat_from_mat4(mat4_t M);

#if VEC_MATH_USE_SSE_H
static inline __m128 linear_combine_sse(__m128 a, mat4_t B) {
    __m128 res;
    res = _mm_mul_ps(_mm_shuffle_ps(a, a, 0x00), B.col[0].mm128);
    res = _mm_add_ps(res, _mm_mul_ps(_mm_shuffle_ps(a, a, 0x55), B.col[1].mm128));
    res = _mm_add_ps(res, _mm_mul_ps(_mm_shuffle_ps(a, a, 0xaa), B.col[2].mm128));
    res = _mm_add_ps(res, _mm_mul_ps(_mm_shuffle_ps(a, a, 0xff), B.col[3].mm128));
    return res;
}
#endif

// MAT2
static inline mat2_t mat2_ident() {
    mat2_t M = {
        .col = {
            {1,0},
            {0,1}
        }
    };
    return M;
}

// MAT3
static inline mat3_t mat3_ident() {
    mat3_t M = {
        .col = {
            {1,0,0},
            {0,1,0},
            {0,0,1}
        }
    };
    return M;
}

static inline mat3_t mat3_from_mat4(mat4_t M) {
    mat3_t R = {
        .col = {
            {M.elem[0][0], M.elem[0][1], M.elem[0][2]},
            {M.elem[1][0], M.elem[1][1], M.elem[1][2]},
            {M.elem[2][0], M.elem[2][1], M.elem[2][2]},
        }
    };
    return R;
}

static inline mat3_t mat3_from_quat(quat_t q) {
    float yy2 = 2.0f * q.elem[1] * q.elem[1];
    float xy2 = 2.0f * q.elem[0] * q.elem[1];
    float xz2 = 2.0f * q.elem[0] * q.elem[2];
    float yz2 = 2.0f * q.elem[1] * q.elem[2];
    float zz2 = 2.0f * q.elem[2] * q.elem[2];
    float wz2 = 2.0f * q.elem[3] * q.elem[2];
    float wy2 = 2.0f * q.elem[3] * q.elem[1];
    float wx2 = 2.0f * q.elem[3] * q.elem[0];
    float xx2 = 2.0f * q.elem[0] * q.elem[0];

    mat3_t M;
    M.elem[0][0] = - yy2 - zz2 + 1.0f;
    M.elem[0][1] = xy2 + wz2;
    M.elem[0][2] = xz2 - wy2;
    M.elem[1][0] = xy2 - wz2;
    M.elem[1][1] = - xx2 - zz2 + 1.0f;
    M.elem[1][2] = yz2 + wx2;
    M.elem[2][0] = xz2 + wy2;
    M.elem[2][1] = yz2 - wx2;
    M.elem[2][2] = - xx2 - yy2 + 1.0f;
    return M;
}

// Create scale matrix from scalar components x, y, z which dictates the scale on each corresponding axis
static inline mat3_t mat3_scale(float x, float y, float z) {
    mat3_t M = {
        .col = {
            {x,0,0},
            {0,y,0},
            {0,0,z}
        }
    };
    return M;
}

static inline mat3_t mat3_add(mat3_t A, mat3_t B) {
    mat3_t M = {
        .col = {
            vec3_add(A.col[0], B.col[0]),
            vec3_add(A.col[1], B.col[1]),
            vec3_add(A.col[2], B.col[2]),
        }
    };
    return M;
}

static inline mat3_t mat3_sub(mat3_t A, mat3_t B) {
    mat3_t M = {
        .col = {
            vec3_sub(A.col[0], B.col[0]),
            vec3_sub(A.col[1], B.col[1]),
            vec3_sub(A.col[2], B.col[2]),
    }
    };
    return M;
}

static inline vec3_t mat3_mul_vec3(mat3_t M, vec3_t v) {
    vec3_t res;
    res.x = M.elem[0][0] * v.x + M.elem[1][0] * v.y + M.elem[2][0] * v.z;
    res.y = M.elem[0][1] * v.x + M.elem[1][1] * v.y + M.elem[2][1] * v.z;
    res.z = M.elem[0][2] * v.x + M.elem[1][2] * v.y + M.elem[2][2] * v.z;
    return res;
}

static inline mat3_t mat3_mul(mat3_t A, mat3_t B) {
    mat3_t C;
#define MULT(col, row) A.elem[0][row] * B.elem[col][0] + A.elem[1][row] * B.elem[col][1] + A.elem[2][row] * B.elem[col][2]
    C.elem[0][0] = MULT(0, 0);
    C.elem[0][1] = MULT(0, 1);
    C.elem[0][2] = MULT(0, 2);

    C.elem[1][0] = MULT(1, 0);
    C.elem[1][1] = MULT(1, 1);
    C.elem[1][2] = MULT(1, 2);

    C.elem[2][0] = MULT(2, 0);
    C.elem[2][1] = MULT(2, 1);
    C.elem[2][2] = MULT(2, 2);
#undef MULT
    return C;
}

static inline mat3_t mat3_mul_f(mat3_t M, float s) {
    mat3_t R = {
        .col = {
            vec3_mul_f(M.col[0], s),
            vec3_mul_f(M.col[1], s),
            vec3_mul_f(M.col[2], s),
        }
    };
    return R;
}

static inline mat3_t mat3_transpose(mat3_t M) {
    mat3_t T = {
        .col = {
            {M.elem[0][0], M.elem[1][0], M.elem[2][0]},
            {M.elem[0][1], M.elem[1][1], M.elem[2][1]},
            {M.elem[0][2], M.elem[1][2], M.elem[2][2]}
        }
    };
    return T;
}

static inline float mat3_determinant(mat3_t M) {
    float d = M.elem[0][0] * (M.elem[1][1] * M.elem[2][2] - M.elem[2][1] * M.elem[1][2])
            - M.elem[1][0] * (M.elem[0][1] * M.elem[2][2] - M.elem[2][1] * M.elem[0][2])
            + M.elem[2][0] * (M.elem[0][1] * M.elem[1][2] - M.elem[1][1] * M.elem[0][2]);
    return d;
}

void   mat3_eigen(const mat3_t M, vec3_t vectors[3], float values[3]);
void   mat3_svd(const mat3_t M, mat3_t* U, mat3_t* S, mat3_t* V);
mat3_t mat3_extract_rotation(const mat3_t M);

mat3_t mat3_covariance_matrix(
    const float* x, const float* y, const float* z,
    vec3_t com,
    int64_t count);

mat3_t mat3_covariance_matrix_vec3(const vec3_t* xyz, vec3_t com, int64_t count);

mat3_t mat3_cross_covariance_matrix(
    const float* x0, const float* y0, const float* z0,
    const float* x1, const float* y1, const float* z1,
    vec3_t com0,
    vec3_t com1,
    int64_t count);

mat3_t mat3_weighted_covariance_matrix(
    const float* x, const float* y, const float* z, const float* weight,
    vec3_t com,
    int64_t count);

mat3_t mat3_weighted_cross_covariance_matrix(
    const float* x0, const float* y0, const float* z0,
    const float* x1, const float* y1, const float* z1,
    const float* weight,
    vec3_t com0,
    vec3_t com1,
    int64_t count);

// MAT4
static inline mat4_t mat4_ident() {
    mat4_t M = {
        .col = {
            {1,0,0,0},
            {0,1,0,0},
            {0,0,1,0},
            {0,0,0,1}
        }
    };
    return M;
}

static inline mat4_t mat4_from_mat3(const mat3_t M) {
    mat4_t R = {
        .col = {
            {M.col[0].x, M.col[0].y, M.col[0].z, 0},
            {M.col[1].x, M.col[1].y, M.col[1].z, 0},
            {M.col[2].x, M.col[2].y, M.col[2].z, 0},
            {0, 0, 0, 1},
        }
    };
    return R;
}

static inline mat4_t mat4_from_quat(quat_t q) {
    float yy2 = 2.0f * q.elem[1] * q.elem[1];
    float xy2 = 2.0f * q.elem[0] * q.elem[1];
    float xz2 = 2.0f * q.elem[0] * q.elem[2];
    float yz2 = 2.0f * q.elem[1] * q.elem[2];
    float zz2 = 2.0f * q.elem[2] * q.elem[2];
    float wz2 = 2.0f * q.elem[3] * q.elem[2];
    float wy2 = 2.0f * q.elem[3] * q.elem[1];
    float wx2 = 2.0f * q.elem[3] * q.elem[0];
    float xx2 = 2.0f * q.elem[0] * q.elem[0];

    mat4_t M;
    M.elem[0][0] = - yy2 - zz2 + 1.0f;
    M.elem[0][1] = xy2 + wz2;
    M.elem[0][2] = xz2 - wy2;
    M.elem[0][3] = 0;
    M.elem[1][0] = xy2 - wz2;
    M.elem[1][1] = - xx2 - zz2 + 1.0f;
    M.elem[1][2] = yz2 + wx2;
    M.elem[1][3] = 0;
    M.elem[2][0] = xz2 + wy2;
    M.elem[2][1] = yz2 - wx2;
    M.elem[2][2] = - xx2 - yy2 + 1.0f;
    M.elem[2][3] = 0;
    M.elem[3][0] = M.elem[3][1] = M.elem[3][2] = 0;
    M.elem[3][3] = 1;
    return M;
}

// Create mat4 scaling matrix from scalars x, y, z which dictates the corresponding scaling factor for each axis
static inline mat4_t mat4_scale(float x, float y, float z) {
    mat4_t M = {
        .col = {
            {x,0,0,0},
            {0,y,0,0},
            {0,0,z,0},
            {0,0,0,1}
        }
    };
    return M;
}

// Create mat4 translation matrix from scalars x, y, z which dictates the corresponding translation for each axis
static inline mat4_t mat4_translate(float x, float y, float z) {
    mat4_t M = {
        .col = {
            {1,0,0,0},
            {0,1,0,0},
            {0,0,1,0},
            {x,y,z,1}
        }
    };
    return M;
}

static inline mat4_t mat4_add(mat4_t A, mat4_t B) {
    mat4_t M = {
        .col = {
            vec4_add(A.col[0], B.col[0]),
            vec4_add(A.col[1], B.col[1]),
            vec4_add(A.col[2], B.col[2]),
            vec4_add(A.col[3], B.col[3]),
    }
    };
    return M;
}

static inline mat4_t mat4_sub(mat4_t A, mat4_t B) {
    mat4_t M = {
        .col = {
            vec4_sub(A.col[0], B.col[0]),
            vec4_sub(A.col[1], B.col[1]),
            vec4_sub(A.col[2], B.col[2]),
            vec4_sub(A.col[3], B.col[3]),
    }
    };
    return M;
}

static inline mat4_t mat4_mul(mat4_t A, mat4_t B) {
    mat4_t C = {0};
#if VEC_MATH_USE_SSE_H
    C.col[0].mm128 = linear_combine_sse(B.col[0].mm128, A);
    C.col[1].mm128 = linear_combine_sse(B.col[1].mm128, A);
    C.col[2].mm128 = linear_combine_sse(B.col[2].mm128, A);
    C.col[3].mm128 = linear_combine_sse(B.col[3].mm128, A);
#else
#define MULT(col, row) \
    A.elem[0][row] * B.elem[col][0] + A.elem[1][row] * B.elem[col][1] + A.elem[2][row] * B.elem[col][2] + A.elem[3][row] * B.elem[col][3]
    C.elem[0][0] = MULT(0, 0);
    C.elem[0][1] = MULT(0, 1);
    C.elem[0][2] = MULT(0, 2);
    C.elem[0][3] = MULT(0, 3);

    C.elem[1][0] = MULT(1, 0);
    C.elem[1][1] = MULT(1, 1);
    C.elem[1][2] = MULT(1, 2);
    C.elem[1][3] = MULT(1, 3);

    C.elem[2][0] = MULT(2, 0);
    C.elem[2][1] = MULT(2, 1);
    C.elem[2][2] = MULT(2, 2);
    C.elem[2][3] = MULT(2, 3);

    C.elem[3][0] = MULT(3, 0);
    C.elem[3][1] = MULT(3, 1);
    C.elem[3][2] = MULT(3, 2);
    C.elem[3][3] = MULT(3, 3);
#undef MULT
#endif
    return C;
}

static inline vec4_t mat4_mul_vec4(mat4_t M, vec4_t v) {
    vec4_t r = {0,0,0,0};
#if VEC_MATH_USE_SSE_H
    r.mm128 = linear_combine_sse(v.mm128, M);
#else
    ASSERT(false);
#endif
    return r;
}

static inline vec3_t mat4_mul_vec3(mat4_t M, vec3_t v, float w) {
    vec4_t r = {v.x, v.y, v.z, w};
    r = mat4_mul_vec4(M, r);
    return vec3_from_vec4(r);
}

static inline mat4_t mat4_mul_f(mat4_t M, float s) {
    mat4_t C = {0};
#if VEC_MATH_USE_SSE_H
    C.col[0] = vec4_mul_f(M.col[0], s);
    C.col[1] = vec4_mul_f(M.col[1], s);
    C.col[2] = vec4_mul_f(M.col[2], s);
    C.col[3] = vec4_mul_f(M.col[3], s);
#else
    C.elem[0][0] = M.elem[0][0] * s;
    C.elem[0][1] = M.elem[0][1] * s;
    C.elem[0][2] = M.elem[0][2] * s;
    C.elem[0][3] = M.elem[0][3] * s;

    C.elem[1][0] = M.elem[1][0] * s;
    C.elem[1][1] = M.elem[1][1] * s;
    C.elem[1][2] = M.elem[1][2] * s;
    C.elem[1][3] = M.elem[1][3] * s;

    C.elem[2][0] = M.elem[2][0] * s;
    C.elem[2][1] = M.elem[2][1] * s;
    C.elem[2][2] = M.elem[2][2] * s;
    C.elem[2][3] = M.elem[2][3] * s;

    C.elem[3][0] = M.elem[3][0] * s;
    C.elem[3][1] = M.elem[3][1] * s;
    C.elem[3][2] = M.elem[3][2] * s;
    C.elem[3][3] = M.elem[3][3] * s;
#endif
    return C;
}

mat4_t mat4_inverse(mat4_t M);

static inline mat4_t mat4_transpose(mat4_t M) {
    mat4_t T;

#if VEC_MATH_USE_SSE_H
    T = M;
    _MM_TRANSPOSE4_PS(T.col[0].mm128, T.col[1].mm128, T.col[2].mm128, T.col[3].mm128);
#else
    T.elem[0][0] = M.elem[0][0];
    T.elem[0][1] = M.elem[1][0];
    T.elem[0][2] = M.elem[2][0];
    T.elem[0][3] = M.elem[3][0];

    T.elem[1][0] = M.elem[0][1];
    T.elem[1][1] = M.elem[1][1];
    T.elem[1][2] = M.elem[2][1];
    T.elem[1][3] = M.elem[3][1];

    T.elem[2][0] = M.elem[0][2];
    T.elem[2][1] = M.elem[1][2];
    T.elem[2][2] = M.elem[2][2];
    T.elem[2][3] = M.elem[3][2];

    T.elem[3][0] = M.elem[0][3];
    T.elem[3][1] = M.elem[1][3];
    T.elem[3][2] = M.elem[2][3];
    T.elem[3][3] = M.elem[3][3];
#endif
    return T;
}

static inline vec3_t mat4_unproject(vec3_t window_coords, mat4_t inv_view_proj_mat, vec4_t viewport) {
    vec4_t tmp = vec4_from_vec3(window_coords, 1.f);
    tmp.x = (tmp.x - viewport.elem[0]) / viewport.elem[2];
    tmp.y = (tmp.y - viewport.elem[1]) / viewport.elem[3];
    tmp = vec4_sub_f(vec4_mul_f(tmp, 2.f), 1.f);

    vec4_t obj = mat4_mul_vec4(inv_view_proj_mat, tmp);
    obj = vec4_div_f(obj, obj.w);

    return vec3_from_vec4(obj);
}

// These are routines for performing the same operation on many items
void vec3_batch_translate(float* RESTRICT x, float* RESTRICT y, float* RESTRICT z, int64_t count, vec3_t translation);
void vec3_batch_translate2(float* out_x, float* out_y, float* out_z, const float* in_x, const float* in_y, const float* in_z, int64_t count, vec3_t translation);

void mat4_batch_transform(float* RESTRICT x, float* RESTRICT y, float* RESTRICT z, float w_comp, int64_t count, mat4_t transform);
void mat4_batch_transform2(float* out_x, float* out_y, float* out_z, const float* in_x, const float* in_y, const float* in_z, float w_comp, int64_t count, mat4_t transform);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
static inline bool operator == (vec2_t a, vec2_t b){
    return a.x == b.x && a.y == b.y;
}

static inline bool operator != (vec2_t a, vec2_t b){
    return !(a == b);
}

static inline vec2_t operator + (vec2_t a, vec2_t b) {
    vec2_t c = {a.x + b.x, a.y + b.y};
    return c;
}

static inline vec2_t operator + (vec2_t a, float s) {
    vec2_t c = {a.x + s, a.y + s};
    return c;
}

static inline vec2_t operator + (float s, vec2_t a) {
    vec2_t c = {a.x + s, a.y + s};
    return c;
}

static inline vec2_t operator - (vec2_t a, vec2_t b) {
    vec2_t c = {a.x - b.x, a.y - b.y};
    return c;
}

static inline vec2_t operator - (vec2_t a, float s) {
    vec2_t c = {a.x - s, a.y - s};
    return c;
}

static inline vec2_t operator - (float s, vec2_t a) {
    vec2_t c = {s - a.x, s - a.y};
    return c;
}

static inline vec2_t operator * (vec2_t a, vec2_t b) {
    vec2_t c = {a.x * b.x, a.y * b.y};
    return c;
}

static inline vec2_t operator * (vec2_t a, float s) {
    vec2_t c = {a.x * s, a.y * s};
    return c;
}

static inline vec2_t operator / (vec2_t a, vec2_t b) {
    vec2_t c = {a.x / b.x, a.y / b.y};
    return c;
}

static inline vec2_t operator / (vec2_t a, float s) {
    vec2_t c = {a.x / s, a.y / s};
    return c;
}

static inline vec2_t operator / (float s, vec2_t a) {
    vec2_t c = {s / a.x, s / a.y};
    return c;
}

// vec3
static inline bool operator == (vec3_t a, vec3_t b){
    return a.x == b.x && a.y == b.y && a.z == b.z;
}

static inline bool operator != (vec3_t a, vec3_t b){
    return !(a == b);
}

static inline vec3_t operator + (vec3_t a, vec3_t b) {
    vec3_t c = {a.x + b.x, a.y + b.y, a.z + b.z};
    return c;
}

static inline vec3_t operator + (vec3_t a, float s) {
    vec3_t c = {a.x + s, a.y + s, a.z + s};
    return c;
}

static inline vec3_t operator + (float s, vec3_t a) {
    vec3_t c = {a.x + s, a.y + s, a.z + s};
    return c;
}

static inline vec3_t operator - (vec3_t a, vec3_t b) {
    vec3_t c = {a.x - b.x, a.y - b.y, a.z - b.z};
    return c;
}

static inline vec3_t operator - (vec3_t a, float s) {
    vec3_t c = {a.x - s, a.y - s, a.z - s};
    return c;
}

static inline vec3_t operator - (float s, vec3_t a) {
    vec3_t c = {s - a.x, s - a.y, s - a.z};
    return c;
}

static inline vec3_t operator * (vec3_t a, vec3_t b) {
    vec3_t c = {a.x * b.x, a.y * b.y, a.z * b.z};
    return c;
}

static inline vec3_t operator * (vec3_t a, float s) {
    vec3_t c = {a.x * s, a.y * s, a.z * s};
    return c;
}

static inline vec3_t operator * (float s, vec3_t a) {
    vec3_t c = {a.x * s, a.y * s, a.z * s};
    return c;
}

static inline vec3_t operator / (vec3_t a, vec3_t b) {
    vec3_t c = {a.x / b.x, a.y / b.y, a.z / b.z};
    return c;
}

static inline vec3_t operator / (vec3_t a, float s) {
    vec3_t c = {a.x / s, a.y / s, a.z / s};
    return c;
}

static inline vec3_t operator / (float s, vec3_t a) {
    vec3_t c = {s / a.x, s / a.y, s / a.z};
    return c;
}

// vec4_t
static inline bool operator == (vec4_t a, vec4_t b){
    return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

static inline bool operator != (vec4_t a, vec4_t b){
    return !(a == b);
}

static inline vec4_t operator + (vec4_t a, vec4_t b) {
    vec4_t c = {a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
    return c;
}

static inline vec4_t operator + (vec4_t a, float s) {
    vec4_t c = {a.x + s, a.y + s, a.z + s, a.w + s};
    return c;
}

static inline vec4_t operator + (float s, vec4_t a) {
    vec4_t c = {a.x + s, a.y + s, a.z + s, a.w + s};
    return c;
}

static inline vec4_t operator - (vec4_t a, vec4_t b) {
    vec4_t c = {a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
    return c;
}

static inline vec4_t operator - (vec4_t a, float s) {
    vec4_t c = {a.x - s, a.y - s, a.z - s, a.w - s};
    return c;
}

static inline vec4_t operator - (float s, vec4_t a) {
    vec4_t c = {s - a.x, s - a.y, s - a.z, s - a.w};
    return c;
}

static inline vec4_t operator * (vec4_t a, vec4_t b) {
    vec4_t c = {a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w};
    return c;
}

static inline vec4_t operator * (vec4_t a, float s) {
    vec4_t c = {a.x * s, a.y * s, a.z * s, a.w * s};
    return c;
}

static inline vec4_t operator * (float s, vec4_t a) {
    vec4_t c = {a.x * s, a.y * s, a.z * s, a.w * s};
    return c;
}

static inline vec4_t operator / (vec4_t a, vec4_t b) {
    vec4_t c = {a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w};
    return c;
}

static inline vec4_t operator / (vec4_t a, float s) {
    vec4_t c = {a.x / s, a.y / s, a.z / s, a.w / s};
    return c;
}

static inline vec4_t operator / (float s, vec4_t a) {
    vec4_t c = {s / a.x, s / a.y, s / a.z, s / a.w};
    return c;
}

// quat

static inline quat_t operator * (quat_t a, quat_t b) {
    return quat_mul(a, b);
}

static inline vec3_t operator * (quat_t q, vec3_t v) {
    return quat_mul_vec3(q, v);
}

// mat

static inline mat3_t operator * (mat3_t A, mat3_t B) {
    return mat3_mul(A, B);
}

static inline mat3_t operator * (mat3_t A, float s) {
    return mat3_mul_f(A, s);
}

static inline mat3_t operator * (float s, mat3_t A) {
    return mat3_mul_f(A, s);
}

static inline mat3_t operator + (mat3_t A, mat3_t B) {
    return mat3_add(A, B);
}

static inline mat3_t operator - (mat3_t A, mat3_t B) {
    return mat3_sub(A, B);
}

static inline bool operator == (mat3_t A, mat3_t B) {
    return A[0] == B[0] && A[1] == B[1] && A[2] == B[2];
}

static inline bool operator != (mat3_t A, mat3_t B) {
    return !(A == B);
}

static inline vec3_t operator * (mat3_t M, vec3_t v) {
    return mat3_mul_vec3(M, v);
}

static inline mat4_t operator * (mat4_t A, mat4_t B) {
    return mat4_mul(A, B);
}

static inline mat4_t operator + (mat4_t A, mat4_t B) {
    return mat4_add(A, B);
}

static inline mat4_t operator - (mat4_t A, mat4_t B) {
    return mat4_sub(A, B);
}

static inline vec4_t operator * (mat4_t M, vec4_t v) {
    return mat4_mul_vec4(M, v);
}

static inline bool operator == (mat4_t A, mat4_t B) {
    return A[0] == B[0] && A[1] == B[1] && A[2] == B[2] && A[3] == B[3];
}

static inline bool operator != (mat4_t A, mat4_t B) {
    return !(A == B);
}

template<typename T, typename V>
T lerp (T a, T b, V t) {
    return (1 - t) * a + t * b;
}

template <typename T, typename V>
T cubic_spline(T p0, T p1, T p2, T p3, V s, V tension = (V)0.5) {
    T v0 = (p2 - p0) * tension;
    T v1 = (p3 - p1) * tension;
    V s2 = s * s;
    V s3 = s * s2;

    return ((V)2.0 * p1 - (V)2.0 * p2 + v0 + v1) * s3 + (-(V)3.0 * p1 + (V)3.0 * p2 - (V)2.0 * v0 - v1) * s2 + v0 * s + p1;
}

#endif