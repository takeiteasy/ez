/* ezmath.h -- Common math functions + types
 https://github.com/takeiteasy/
 
 Some functionality relies on Clang + GCC extensions.
 When building `-fenable-matrix` must be passed.
 
 The MIT License (MIT)

 Copyright (c) 2022 George Watson

 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge,
 publish, distribute, sublicense, and/or sell copies of the Software,
 and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:

 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#if !defined(__clang__) && !(defined(__GNUC__) || defined(__GNUG__))
#error Unsupported compiler! This library relies on Clang/GCC extensions
#endif

#ifndef EZMATH_HEADER
#define EZMATH_HEADER
#if defined(__cplusplus)
extern "C" {
#endif

#include <math.h>

#ifndef PI
#define PI 3.14159265358979323846f
#endif
#ifndef TWO_PI
#define TWO_PI 6.28318530717958647692f
#endif
#ifndef TAU
#define TAU TWO_PI
#endif
#ifndef HALF_PI
#define HALF_PI 0.636619772367581343075f
#endif
#ifndef PHI
#define PHI 1.61803398874989484820f
#endif
#ifndef INV_PHI
#define INV_PHI 0.61803398874989484820f
#endif
#ifndef EULER
#define EULER 2.71828182845904523536f
#endif
#ifndef EPSILON
#define EPSILON 0.000001f
#endif

#define Bytes(n) (n)
#define Kilobytes(n) (n << 10)
#define Megabytes(n) (n << 20)
#define Gigabytes(n) (((uint64_t)n) << 30)
#define Terabytes(n) (((uint64_t)n) << 40)

#define Thousand(n) ((n)*1000)
#define Million(n) ((n)*1000000)
#define Billion(n) ((n)*1000000000LL)

#define U8Max ((uint8_t)0xFF)
#define U8Min ((uint8_t)0)
#define U16Max ((uint16_t)0xFFFF)
#define U16Min ((uint16_t)0)
#define U32Max ((uint32_t)0xFFFFFFFF)
#define U32Min ((uint32_t)0)
#if !defined(U64Max)
#define U64Max ((uint64_t)0xFFFFFFFFFFFFFFFF)
#endif
#define U64Min ((uint64_t)0)
#define S8Max ((int8_t)0x7F)
#define S8Min ((int8_t)-1 - 0x7F)
#define S16Max ((int16_t)0x7FFF)
#define S16Min ((int16_t)-1 - 0x7FFF)
#define S32Max ((int132_t)0x7FFFFFFF)
#define S32Min ((int32_t)-1 - 0x7FFFFFFF)
#if !defined(S64Max)
#define S64Max ((int64_t)0x7FFFFFFFFFFFFFFF)
#endif
#if !defined(S64Min)
#define S64Min ((int64_t)-1 - 0x7FFFFFFFFFFFFFFF)
#endif

#define MATRIX_TYPES \
    X(2, 2)          \
    X(3, 3)          \
    X(4, 4)

#define __MATRIX_T(W, H) Mat##W##H
#define __MATRIX_D(W, H, F) Mat##W##H##F
#define X(W, H)                                                        \
    typedef float __MATRIX_T(W, H) __attribute__((matrix_type(W, H))); \
    __MATRIX_T(W, H) __MATRIX_D(W, H, Identity)(void);                 \
    float __MATRIX_D(W, H, Trace)(__MATRIX_T(W, H));                   \
    __MATRIX_T(W, H) __MATRIX_D(W, H, Zero)(void);
MATRIX_TYPES
#undef X

#define VECTOR_TYPES \
    X(2)             \
    X(3)             \
    X(4)

#define __VEC_T(L) Vec##L##f
#define __VEC_D(L, F) Vec##L##F
#define X(L)                                                                    \
    typedef float __VEC_T(L) __attribute__((ext_vector_type(L)));               \
    typedef __VEC_T(L) Vector##L;                                               \
    typedef int Vec##L##i __attribute__((ext_vector_type(L)));                  \
    __VEC_T(L) __VEC_D(L, Zero)(void);                                          \
    __VEC_T(L) __VEC_D(L, New)(float x, ...);                                   \
    void __VEC_D(L, Print)(__VEC_T(L) v);                                       \
    float __VEC_D(L, Sum)(__VEC_T(L) v);                                        \
    bool __VEC_D(L, Equals)(__VEC_T(L) a, __VEC_T(L) b);                        \
    float __VEC_D(L, LengthSqr)(__VEC_T(L) v);                                  \
    float __VEC_D(L, Length)(__VEC_T(L) v);                                     \
    float __VEC_D(L, Dot)(__VEC_T(L) v1, __VEC_T(L) v2);                        \
    __VEC_T(L) __VEC_D(L, Normalize)(__VEC_T(L) v);                             \
    float __VEC_D(L, DistSqr)(__VEC_T(L) v1, __VEC_T(L) v2);                    \
    float __VEC_D(L, Dist)(__VEC_T(L) v1, __VEC_T(L) v2);                       \
    __VEC_T(L) __VEC_D(L, Clamp)(__VEC_T(L) v, __VEC_T(L) min, __VEC_T(L) max); \
    __VEC_T(L) __VEC_D(L, Min)(__VEC_T(L) v, __VEC_T(L) min);                   \
    __VEC_T(L) __VEC_D(L, Max)(__VEC_T(L) v, __VEC_T(L) max);                   \
    __VEC_T(L) __VEC_D(L, Lerp)(__VEC_T(L) v1, __VEC_T(L) v2, float n);
VECTOR_TYPES
#undef X

typedef Vec4f Quaternion;
typedef Vec2i Position;
typedef Mat44 Matrix;

float Vec2Angle(Vec2f v1, Vec2f v2);
Vec2f Vec2Rotate(Vec2f v, float angle);
Vec2f Vec2MoveTowards(Vec2f v, Vec2f target, float maxDistance);
Vec2f Vec2Reflect(Vec2f v, Vec2f normal);

Vec3f Vec3Reflect(Vec3f v, Vec3f normal);
Vec3f Vec3Cross(Vec3f v1, Vec3f v2);
Vec3f Vec3Perpendicular(Vec3f v);
float Vec3Angle(Vec3f v1, Vec3f v2);
Vec3f Vec3RotateByQuaternion(Vec3f v, Quaternion q);
Vec3f Vec3RotateByAxisAngle(Vec3f v, Vec3f axis, float angle);
Vec3f Vec3Refract(Vec3f v, Vec3f n, float r);
Vec3f Vec3Transform(Vec3f v, Matrix mat);
Vec3f Vec3Barycentre(Vec3f p, Vec3f a, Vec3f b, Vec3f c);
Vec3f Vec3Unproject(Vec3f source, Matrix projection, Matrix view);

#define QuaternionZero(...) Vec4Zero(__VA_ARGS__)
#define QuaternionNew(...) Vec4New(__VA_ARGS__)
#define QuaternionPrint(...) Vec4Print(__VA_ARGS__)
#define QuaternionSum(...) Vec4Sum(__VA_ARGS__)
#define QuaternionLengthSqr(...) Vec4LengthSqr(__VA_ARGS__)
#define QuaternionLength(...) Vec4Length(__VA_ARGS__)
#define QuaternionDot(...) Vec4Dot(__VA_ARGS__)
#define QuaternionNormalize(...) Vec4Normalize(__VA_ARGS__)
#define QuaternionDistSqr(...) Vec4DistSqr(__VA_ARGS__)
#define QuaternionDist(...) Vec4Dist(__VA_ARGS__)
#define QuaternionClamp(...) Vec4Clamp(__VA_ARGS__)
#define QuaternionMin(...) Vec4Min(__VA_ARGS__)
#define QuaternionMax(...) Vec4Max(__VA_ARGS__)
#define QuaternionLerp(...) Vec4Lerp(__VA_ARGS__)

Quaternion QuaternionIdentity(void);
Quaternion QuaternionMultiply(Quaternion q1, Quaternion q2);
Quaternion QuaternionInvert(Quaternion q);
Quaternion QuaternionFromVec3ToVec3(Vec3f from, Vec3f to);
Quaternion QuaternionFromMatrix(Matrix mat);
Matrix QuaternionToMatrix(Quaternion q);
Quaternion QuaternionFromAxisAngle(Vector3 axis, float angle);
void QuaternionToAxisAngle(Quaternion q, Vector3 *outAxis, float *outAngle);
Quaternion QuaternionFromEuler(float pitch, float yaw, float roll);
Vector3 QuaternionToEuler(Quaternion q);
Quaternion QuaternionTransform(Quaternion q, Matrix mat);
bool QuaternionEquals(Quaternion p, Quaternion q);

float MatrixDetermint(Matrix mat);
Matrix MatrixInvert(Matrix mat);
Matrix MatrixTranslation(Vec3f v);
Matrix MatrixRotation(Vec3f axis, float angle);
Matrix MatrixScaling(Vec3f scale);
Matrix MatrixFrustum(float left, float right, float bottom, float top, float near, float far);
Matrix MatrixPerspective(float fovY, float aspect, float nearPlane, float farPlane);
Matrix MatrixOrtho(float left, float right, float bottom, float top, float nearPlane, float farPlane);

bool FloatCmp(float a, float b);
int Min(int a, int b);
int Max(int a, int b);
int Clamp(int n, int min, int max);
float Degrees(float radians);
float Radians(float degrees);

// Taken from `raylib` -- https://github.com/raysan5/raylib/blob/master/examples/others/reasings.h
// Linear Easing functions
float EaseLinearNone(float t, float b, float c, float d);
float EaseLinearIn(float t, float b, float c, float d);
float EaseLinearOut(float t, float b, float c, float d);
float EaseLinearInOut(float t, float b, float c, float d);
// Sine Easing functions
float EaseSineIn(float t, float b, float c, float d);
float EaseSineOut(float t, float b, float c, float d);
float EaseSineInOut(float t, float b, float c, float d);
// Circular Easing functions
float EaseCircIn(float t, float b, float c, float d);
float EaseCircOut(float t, float b, float c, float d);
float EaseCircInOut(float t, float b, float c, float d);
// Cubic Easing functions
float EaseCubicIn(float t, float b, float c, float d);
float EaseCubicOut(float t, float b, float c, float d);
float EaseCubicInOut(float t, float b, float c, float d);
// Quadratic Easing functions
float EaseQuadIn(float t, float b, float c, float d);
float EaseQuadOut(float t, float b, float c, float d);
float EaseQuadInOut(float t, float b, float c, float d);
// Exponential Easing functions
float EaseExpoIn(float t, float b, float c, float d);
float EaseExpoOut(float t, float b, float c, float d);
float EaseExpoInOut(float t, float b, float c, float d);
// Back Easing functions
float EaseBackIn(float t, float b, float c, float d);
float EaseBackOut(float t, float b, float c, float d);
float EaseBackInOut(float t, float b, float c, float d);
// Bounce Easing functions
float EaseBounceOut(float t, float b, float c, float d);
float EaseBounceIn(float t, float b, float c, float d);
float EaseBounceInOut(float t, float b, float c, float d);
// Elastic Easing functions
float EaseElasticIn(float t, float b, float c, float d);
float EaseElasticOut(float t, float b, float c, float d);
float EaseElasticInOut(float t, float b, float c, float d);

#if defined(__cplusplus)
}
#endif
#endif // EZMATH_HEADER

#if defined(EZMATH_IMPLEMENTATION) || defined(EZ_IMPLEMENTATION)
#define X(W, H)                                                      \
    __MATRIX_T(W, H) __MATRIX_D(W, H, Identity)(void)                \
    {                                                                \
        __MATRIX_T(W, H) m;                                          \
        memset(&m, 0, sizeof(float) * W * H);                        \
        if (W == H)                                                  \
            for (int i = 0; i < W; i++)                              \
                m[i][i] = 1.f;                                       \
        return m;                                                    \
    }                                                                \
    float __MATRIX_D(W, H, TRACE)(__MATRIX_T(W, H) m)                \
    {                                                                \
        float r = 0.f;                                               \
        if (W == H)                                                  \
            for (int i = 0; i < W; i++)                              \
                r += m[i][i];                                        \
        return r;                                                    \
    }                                                                \
    __MATRIX_T(W, H) __MATRIX_D(W, H, Transpose)(__MATRIX_T(W, H) m) \
    {                                                                \
        __MATRIX_T(W, H) result = __MATRIX_D(W, H, Zero)();          \
        for (int x = 0; x < W; x++)                                  \
            for (int y = 0; y < H; y++)                              \
                result[x][y] = m[y][x];                              \
        return result;                                               \
    }                                                                \
    __MATRIX_T(W, H) __MATRIX_D(W, H, Zero)(void)                    \
    {                                                                \
        __MATRIX_T(W, H) m;                                          \
        memset(&m, 0, sizeof(float) * W * H);                        \
        return m;                                                    \
    }
MATRIX_TYPES
#undef X

#define X(L)                                                                   \
    __VEC_T(L) __VEC_D(L, Zero)(void)                                          \
    {                                                                          \
        __VEC_T(L) v;                                                          \
        memset(&v, 0, sizeof(float) * L);                                      \
        return v;                                                              \
    }                                                                          \
    __VEC_T(L) __VEC_D(L, New)(float x, ...)                                   \
    {                                                                          \
        va_list args;                                                          \
        va_start(args, L - 1);                                                 \
        __VEC_T(L) v;                                                          \
        v[0] = x;                                                              \
        for (int i = 0; i < L - 1; i++)                                        \
            v[i + 1] = (float)va_arg(args, double);                            \
        va_end(args);                                                          \
        return v;                                                              \
    }                                                                          \
    void __VEC_D(L, Print)(__VEC_T(L) v)                                       \
    {                                                                          \
        puts("{ ");                                                            \
        for (int i = 0; i < L; i++)                                            \
            printf("%f ", v[i]);                                               \
        puts("}");                                                             \
    }                                                                          \
    float __VEC_D(L, Sum)(__VEC_T(L) v)                                        \
    {                                                                          \
        float r = 0.f;                                                         \
        for (int i = 0; i < L; i++)                                            \
            r += v[i];                                                         \
        return r;                                                              \
    }                                                                          \
    bool __VEC_D(L, Equals)(__VEC_T(L) a, __VEC_T(L) b)                        \
    {                                                                          \
        bool r = true;                                                         \
        for (int i = 0; i < L; i++)                                            \
            if (!FloatCmp(a[i], b[i]))                                         \
                return false;                                                  \
        return r;                                                              \
    }                                                                          \
    float __VEC_D(L, LengthSqr)(__VEC_T(L) v)                                  \
    {                                                                          \
        return __VEC_D(L, Sum)(v * v);                                         \
    }                                                                          \
    float __VEC_D(L, Length)(__VEC_T(L) v)                                     \
    {                                                                          \
        return sqrtf(__VEC_D(L, LengthSqr)(v));                                \
    }                                                                          \
    float __VEC_D(L, Dot)(__VEC_T(L) v1, __VEC_T(L) v2)                        \
    {                                                                          \
        return __VEC_D(L, Sum)(v1 * v2);                                       \
    }                                                                          \
    __VEC_T(L) __VEC_D(L, Normalize) (__VEC_T(L) v)                            \
    {                                                                          \
        __VEC_T(L) r = {0};                                                    \
        float l = __VEC_D(L, Length)(v);                                       \
        for (int i = 0; i < L; i++)                                            \
            r[i] = v[i] * (1.f / l);                                           \
        return r;                                                              \
    }                                                                          \
    float __VEC_D(L, DistSqr)(__VEC_T(L) v1, __VEC_T(L) v2)                    \
    {                                                                          \
        __VEC_T(L) v = v2 - v1;                                                \
        return __VEC_D(L, Sum)(v * v);                                         \
    }                                                                          \
    float __VEC_D(L, Dist)(__VEC_T(L) v1, __VEC_T(L) v2)                       \
    {                                                                          \
        return sqrtf(__VEC_D(L, DistSqr)(v1, v2));                             \
    }                                                                          \
    __VEC_T(L) __VEC_D(L, Clamp)(__VEC_T(L) v, __VEC_T(L) min, __VEC_T(L) max) \
    {                                                                          \
    __VEC_T(L) r = {0};                                                        \
        for (int i = 0; i < L; i++)                                            \
            r[i] = Clamp(v[i], min[i], max[i]);                                \
        return r;                                                              \
    }                                                                          \
    __VEC_T(L) __VEC_D(L, Min)(__VEC_T(L) v, __VEC_T(L) min)                   \
    {                                                                          \
    __VEC_T(L) r = {0};                                                        \
        for (int i = 0; i < L; i++)                                            \
            r[i] = Min(v[i], min[i]);                                          \
        return r;                                                              \
    }                                                                          \
    __VEC_T(L) __VEC_D(L, Max)(__VEC_T(L) v, __VEC_T(L) max)                   \
    {                                                                          \
    __VEC_T(L) r = {0};                                                        \
        for (int i = 0; i < L; i++)                                            \
            r[i] = Max(v[i], max[i]);                                          \
        return r;                                                              \
    }                                                                          \
    __VEC_T(L) __VEC_D(L, Lerp)(__VEC_T(L) v1, __VEC_T(L) v2, float n)         \
    {                                                                          \
        return v1 + n * (v2 - v1);                                             \
    }
VECTOR_TYPES
#undef X

float Vec2Angle(Vec2f v1, Vec2f v2) {
    return atan2f(v2.y, v2.x) - atan2f(v1.y, v1.x);
}

Vec2f Vec2Rotate(Vec2f v, float angle) {
    float c = cosf(angle);
    float s = sinf(angle);
    return (Vec2f) { v.x*c - v.y*s,  v.x*s + v.y*c };
}

Vec2f Vec2MoveTowards(Vec2f v, Vec2f target, float maxDistance) {
    Vec2f delta = target - v;
    float dist = Vec2LengthSqr(delta);
    if (!dist || (maxDistance >= 0 && dist <= maxDistance * maxDistance))
        return target;
    else
        return v + (delta / (sqrtf(dist) * maxDistance));
}

Vec2f Vec2Reflect(Vec2f v, Vec2f normal) {
    return v - (2.f * normal) * Vec2Dot(v, normal);
}

Vec3f Vec3Reflect(Vec3f v, Vec3f normal) {
    return v - (2.f * normal) * Vec3Dot(v, normal);
}

Vec3f Vec3Cross(Vec3f v1, Vec3f v2) {
    return (Vec3f) { v1.y*v2.z - v1.z*v2.y, v1.z*v2.x - v1.x*v2.z, v1.x*v2.y - v1.y*v2.x };
}

Vec3f Vec3Perpendicular(Vec3f v) {
    float min = (float) fabs(v.x);
    Vec3f a = {1.0f, 0.0f, 0.0f};
    if (fabsf(v.y) < min) {
        min = (float) fabs(v.y);
        a = (Vec3f){0.0f, 1.0f, 0.0f};
    }
    if (fabsf(v.z) < min)
        a = (Vec3f){0.0f, 0.0f, 1.0f};
    return (Vec3f) { v.y*a.z - v.z*a.y,
                    v.z*a.x - v.x*a.z,
                    v.x*a.y - v.y*a.x };
}

float Vec3Angle(Vec3f v1, Vec3f v2) {
    return atan2f(Vec3Length(Vec3Cross(v1, v2)), Vec3Dot(v1, v2));
}

Vec3f Vec3RotateByQuaternion(Vec3f v, Quaternion q) {
    return (Vec3f) {
        v.x*(q.x*q.x + q.w*q.w - q.y*q.y - q.z*q.z) + v.y*(2*q.x*q.y - 2*q.w*q.z) + v.z*(2*q.x*q.z + 2*q.w*q.y),
        v.x*(2*q.w*q.z + 2*q.x*q.y) + v.y*(q.w*q.w - q.x*q.x + q.y*q.y - q.z*q.z) + v.z*(-2*q.w*q.x + 2*q.y*q.z),
        v.x*(-2*q.w*q.y + 2*q.x*q.z) + v.y*(2*q.w*q.x + 2*q.y*q.z)+ v.z*(q.w*q.w - q.x*q.x - q.y*q.y + q.z*q.z)
    };
}

Vec3f Vec3RotateByAxisAngle(Vec3f v, Vec3f axis, float angle) {
    // Using Euler-Rodrigues Formula
    // Ref.: https://en.wikipedia.org/w/index.php?title=Euler%E2%80%93Rodrigues_formula

    axis *= (1.0f / (Clamp(Vec3Length(axis), 0.f, 1.f)));

    angle /= 2.0f;
    Vec3f w = axis * sinf(angle);
    Vec3f wv = Vec3Cross(w, v);
    return v + (wv * (cosf(angle) * 2.f)) + (Vec3Cross(w, wv) * 2.f);
}

Vec3f Vec3Refract(Vec3f v, Vec3f n, float r) {
    float dot = Vec3Dot(v, n);
    float d = 1.0f - r*r*(1.0f - dot*dot);
    return d < 0 ? Vec3Zero() : r * v - (r * dot + sqrtf(d)) * n;
}

Vec3f Vec3Transform(Vec3f v, Matrix mat) {
    return (Vec3f) {
        mat[0][0]*v.x + mat[0][1]*v.y + mat[0][2]*v.z + mat[0][3],
        mat[1][0]*v.x + mat[1][1]*v.y + mat[1][2]*v.z + mat[1][3],
        mat[2][0]*v.x + mat[2][1]*v.y + mat[2][2]*v.z + mat[2][3]
    };
}

Vec3f Vec3Barycentre(Vec3f p, Vec3f a, Vec3f b, Vec3f c) {
    Vector3 v0 = b - a;
    Vector3 v1 = c - a;
    Vector3 v2 = p - a;
    float d00 = Vec3Dot(v0, v1);
    float d01 = Vec3Dot(v1, v1);
    float d11 = Vec3Dot(v1, v1);
    float d20 = Vec3Dot(v2, v0);
    float d21 = Vec3Dot(v2, v1);
    float denom = d00*d11 - d01*d01;
    float y = (d11*d20 - d01*d21)/denom;
    float z = (d00*d21 - d01*d20)/denom;
    return (Vec3f) {1.0f - (z + y), y, z};
}

Vector3 Vec3Unproject(Vector3 source, Matrix projection, Matrix view) {
    Quaternion p = QuaternionTransform((Quaternion){source.x, source.y, source.z, 1.f }, MatrixInvert(view * projection));
    return (Vec3f) {
        p.x / p.w,
        p.y / p.w,
        p.z / p.w
    };
}

Quaternion QuaternionIdentity(void) {
    return (Quaternion){0.f, 0.f, 0.f, 1.f};
}

Quaternion QuaternionMultiply(Quaternion q1, Quaternion q2) {
    float qax = q1.x, qay = q1.y, qaz = q1.z, qaw = q1.w;
    float qbx = q2.x, qby = q2.y, qbz = q2.z, qbw = q2.w;
    return (Quaternion) {
        qax*qbw + qaw*qbx + qay*qbz - qaz*qby,
        qay*qbw + qaw*qby + qaz*qbx - qax*qbz,
        qaz*qbw + qaw*qbz + qax*qby - qay*qbx,
        qaw*qbw - qax*qbx - qay*qby - qaz*qbz
    };
}

Quaternion QuaternionInvert(Quaternion q) {
    float lsqr = QuaternionLengthSqr(q);
    Quaternion result = q;
    if (lsqr != 0.f) {
        float inv = 1.f / lsqr;
        result *= (Vec4f){-inv, -inv, -inv, inv};
    }
    return result;
}

Quaternion QuaternionFromVec3ToVec3(Vec3f from, Vec3f to) {
    Vec3f cross = Vec3Cross(from, to);
    return QuaternionNormalize(QuaternionNew(cross.x, cross.y, cross.z, 1.f + Vec3Dot(from, to)));
}

Quaternion QuaternionFromMatrix(Matrix mat) {
    float fourWSquaredMinus1 = mat[0][0] + mat[1][1] + mat[2][2];
    float fourXSquaredMinus1 = mat[0][0] - mat[1][1] - mat[2][2];
    float fourYSquaredMinus1 = mat[1][1] - mat[0][0] - mat[2][2];
    float fourZSquaredMinus1 = mat[2][2] - mat[0][0] - mat[1][1];

    int biggestIndex = 0;
    float fourBiggestSquaredMinus1 = fourWSquaredMinus1;
    if (fourXSquaredMinus1 > fourBiggestSquaredMinus1) {
        fourBiggestSquaredMinus1 = fourXSquaredMinus1;
        biggestIndex = 1;
    }
    if (fourYSquaredMinus1 > fourBiggestSquaredMinus1) {
        fourBiggestSquaredMinus1 = fourYSquaredMinus1;
        biggestIndex = 2;
    }
    if (fourZSquaredMinus1 > fourBiggestSquaredMinus1) {
        fourBiggestSquaredMinus1 = fourZSquaredMinus1;
        biggestIndex = 3;
    }

    float biggestVal = sqrtf(fourBiggestSquaredMinus1 + 1.f) * .5f;
    float mult = .25f / biggestVal;
    switch (biggestIndex)
    {
        case 0:
            return (Quaternion) {
                (mat[2][1] - mat[1][2]) * mult,
                (mat[0][2] - mat[2][0]) * mult,
                (mat[1][0] - mat[0][1]) * mult,
                biggestVal
            };
            break;
        case 1:
            return (Quaternion) {
                biggestVal,
                (mat[1][0] + mat[0][1]) * mult,
                (mat[0][2] + mat[2][0]) * mult,
                (mat[2][1] - mat[1][2]) * mult
            };
        case 2:
            return (Quaternion) {
                biggestVal,
                (mat[1][0] + mat[0][1]) * mult,
                (mat[0][2] + mat[2][0]) * mult,
                (mat[2][1] - mat[1][2]) * mult
            };
        case 3:
            return (Quaternion) {
                biggestVal,
                (mat[1][0] + mat[0][1]) * mult,
                (mat[0][2] + mat[2][0]) * mult,
                (mat[2][1] - mat[1][2]) * mult
            };
        default:
            return QuaternionZero();
    }
}

Matrix QuaternionToMatrix(Quaternion q) {
    float a2 = q.x*q.x;
    float b2 = q.y*q.y;
    float c2 = q.z*q.z;
    float ac = q.x*q.z;
    float ab = q.x*q.y;
    float bc = q.y*q.z;
    float ad = q.w*q.x;
    float bd = q.w*q.y;
    float cd = q.w*q.z;
    
    Matrix result = Mat44Identity();
    result[0][0] = 1 - 2*(b2 + c2);
    result[1][0] = 2*(ab + cd);
    result[2][0] = 2*(ac - bd);
    
    result[0][1] = 2*(ab - cd);
    result[1][1] = 1 - 2*(a2 + c2);
    result[2][1] = 2*(bc + ad);
    
    result[0][2] = 2*(ac + bd);
    result[1][2] = 2*(bc - ad);
    result[2][2] = 1 - 2*(a2 + b2);
    return result;
}

Quaternion QuaternionFromAxisAngle(Vector3 axis, float angle) {
    float axisLength = Vec3Length(axis);
    if (axisLength == 0.f)
        return QuaternionIdentity();
    
    axis = Vec3Normalize(axis);
    angle *= .5f;
    float sinres = sinf(angle);
    float cosres = cosf(angle);
    return (Quaternion) {
        axis.x*sinres,
        axis.y*sinres,
        axis.z*sinres,
        cosres
    };
}

void QuaternionToAxisAngle(Quaternion q, Vector3 *outAxis, float *outAngle) {
    if (fabsf(q.w) > 1.0f)
        q = QuaternionNormalize(q);
    float resAngle = 2.0f*acosf(q.w);
    float den = sqrtf(1.0f - q.w*q.w);
    Vec3f qxyz = (Vec3f){q.x, q.y, q.z};
    Vec3f resAxis = den > EPSILON ? qxyz / den : (Vec3f){1.f, 0.f, 0.f};
    *outAxis = resAxis;
    *outAngle = resAngle;
}

Quaternion QuaternionFromEuler(float pitch, float yaw, float roll) {
    float x0 = cosf(pitch*0.5f);
    float x1 = sinf(pitch*0.5f);
    float y0 = cosf(yaw*0.5f);
    float y1 = sinf(yaw*0.5f);
    float z0 = cosf(roll*0.5f);
    float z1 = sinf(roll*0.5f);
    
    return (Quaternion) {
        x1*y0*z0 - x0*y1*z1,
        x0*y1*z0 + x1*y0*z1,
        x0*y0*z1 - x1*y1*z0,
        x0*y0*z0 + x1*y1*z1
    };
}

Vector3 QuaternionToEuler(Quaternion q) {
    // Roll (x-axis rotation)
    float x0 = 2.0f*(q.w*q.x + q.y*q.z);
    float x1 = 1.0f - 2.0f*(q.x*q.x + q.y*q.y);
    // Pitch (y-axis rotation)
    float y0 = 2.0f*(q.w*q.y - q.z*q.x);
    y0 = y0 > 1.0f ? 1.0f : y0;
    y0 = y0 < -1.0f ? -1.0f : y0;
    // Yaw (z-axis rotation)
    float z0 = 2.0f*(q.w*q.z + q.x*q.y);
    float z1 = 1.0f - 2.0f*(q.y*q.y + q.z*q.z);
    return (Vec3f) {
        atan2f(x0, x1),
        asinf(y0),
        atan2f(z0, z1)
    };
}

Quaternion QuaternionTransform(Quaternion q, Matrix mat) {
    return (Quaternion) {
        mat[0][0]*q.x + mat[0][1]*q.y + mat[0][2]*q.z + mat[0][3]*q.w,
        mat[1][0]*q.x + mat[1][1]*q.y + mat[1][2]*q.z + mat[1][3]*q.w,
        mat[2][0]*q.x + mat[2][1]*q.y + mat[2][2]*q.z + mat[2][3]*q.w,
        mat[3][0]*q.x + mat[3][1]*q.y + mat[3][2]*q.z + mat[3][3]*q.w
    };
}

bool QuaternionEquals(Quaternion p, Quaternion q) {
    return (((fabsf(p.x - q.x)) <= (EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.x), fabsf(q.x))))) && ((fabsf(p.y - q.y)) <= (EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.y), fabsf(q.y))))) && ((fabsf(p.z - q.z)) <= (EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.z), fabsf(q.z))))) && ((fabsf(p.w - q.w)) <= (EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.w), fabsf(q.w)))))) || (((fabsf(p.x + q.x)) <= (EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.x), fabsf(q.x))))) && ((fabsf(p.y + q.y)) <= (EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.y), fabsf(q.y))))) && ((fabsf(p.z + q.z)) <= (EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.z), fabsf(q.z))))) && ((fabsf(p.w + q.w)) <= (EPSILON*fmaxf(1.0f, fmaxf(fabsf(p.w), fabsf(q.w))))));
}

float MatrixDetermint(Matrix mat) {
    return mat[0][3]*mat[1][2]*mat[2][1]*mat[3][0] - mat[0][2]*mat[1][3]*mat[2][1]*mat[3][0] - mat[0][3]*mat[1][1]*mat[2][2]*mat[3][0] + mat[0][1]*mat[1][3]*mat[2][2]*mat[3][0] + mat[0][2]*mat[1][1]*mat[2][3]*mat[3][0] - mat[0][1]*mat[1][2]*mat[2][3]*mat[3][0] - mat[0][3]*mat[1][2]*mat[2][0]*mat[3][1] + mat[0][2]*mat[1][3]*mat[2][0]*mat[3][1] + mat[0][3]*mat[1][0]*mat[2][2]*mat[3][1] - mat[0][0]*mat[1][3]*mat[2][2]*mat[3][1] - mat[0][2]*mat[1][0]*mat[2][3]*mat[3][1] + mat[0][0]*mat[1][2]*mat[2][3]*mat[3][1] + mat[0][3]*mat[1][1]*mat[2][0]*mat[3][2] - mat[0][1]*mat[1][3]*mat[2][0]*mat[3][2] - mat[0][3]*mat[1][0]*mat[2][1]*mat[3][2] + mat[0][0]*mat[1][3]*mat[2][1]*mat[3][2] + mat[0][1]*mat[1][0]*mat[2][3]*mat[3][2] - mat[0][0]*mat[1][1]*mat[2][3]*mat[3][2] - mat[0][2]*mat[1][1]*mat[2][0]*mat[3][3] + mat[0][1]*mat[1][2]*mat[2][0]*mat[3][3] + mat[0][2]*mat[1][0]*mat[2][1]*mat[3][3] - mat[0][0]*mat[1][2]*mat[2][1]*mat[3][3] - mat[0][1]*mat[1][0]*mat[2][2]*mat[3][3] + mat[0][0]*mat[1][1]*mat[2][2]*mat[3][3];
}

Matrix MatrixInvert(Matrix mat) {
    float b00 = mat[0][0]*mat[1][1] - mat[1][0]*mat[0][1];
    float b01 = mat[0][0]*mat[2][1] - mat[2][0]*mat[0][1];
    float b02 = mat[0][0]*mat[3][1] - mat[3][0]*mat[0][1];
    float b03 = mat[1][0]*mat[2][1] - mat[2][0]*mat[1][1];
    float b04 = mat[1][0]*mat[3][1] - mat[3][0]*mat[1][1];
    float b05 = mat[2][0]*mat[3][1] - mat[3][0]*mat[2][1];
    float b06 = mat[0][2]*mat[1][3] - mat[1][2]*mat[0][3];
    float b07 = mat[0][2]*mat[2][3] - mat[2][2]*mat[0][3];
    float b08 = mat[0][2]*mat[3][3] - mat[3][2]*mat[0][3];
    float b09 = mat[1][2]*mat[2][3] - mat[2][2]*mat[1][3];
    float b10 = mat[1][2]*mat[3][3] - mat[3][2]*mat[1][3];
    float b11 = mat[2][2]*mat[3][3] - mat[3][2]*mat[2][3];
    float invDet = 1.0f/(b00*b11 - b01*b10 + b02*b09 + b03*b08 - b04*b07 + b05*b06);
    
    Matrix result = Mat44Zero();
    result[0][0] = (mat[1][1]*b11 - mat[2][1]*b10 + mat[3][1]*b09)*invDet;
    result[1][0] = (-mat[1][0]*b11 + mat[2][0]*b10 - mat[3][0]*b09)*invDet;
    result[2][0] = (mat[1][3]*b05 - mat[2][3]*b04 + mat[3][3]*b03)*invDet;
    result[3][0] = (-mat[1][2]*b05 + mat[2][2]*b04 - mat[3][2]*b03)*invDet;
    result[0][1] = (-mat[0][1]*b11 + mat[2][1]*b08 - mat[3][1]*b07)*invDet;
    result[1][1] = (mat[0][0]*b11 - mat[2][0]*b08 + mat[3][0]*b07)*invDet;
    result[2][1] = (-mat[0][3]*b05 + mat[2][3]*b02 - mat[3][3]*b01)*invDet;
    result[3][1] = (mat[0][2]*b05 - mat[2][2]*b02 + mat[3][2]*b01)*invDet;
    result[0][2] = (mat[0][1]*b10 - mat[1][1]*b08 + mat[3][1]*b06)*invDet;
    result[1][2] = (-mat[0][0]*b10 + mat[1][0]*b08 - mat[3][0]*b06)*invDet;
    result[2][2] = (mat[0][3]*b04 - mat[1][3]*b02 + mat[3][3]*b00)*invDet;
    result[3][2] = (-mat[0][2]*b04 + mat[1][2]*b02 - mat[3][2]*b00)*invDet;
    result[0][3] = (-mat[0][1]*b09 + mat[1][1]*b07 - mat[2][1]*b06)*invDet;
    result[1][3] = (mat[0][0]*b09 - mat[1][0]*b07 + mat[2][0]*b06)*invDet;
    result[2][3] = (-mat[0][3]*b03 + mat[1][3]*b01 - mat[2][3]*b00)*invDet;
    result[3][3] = (mat[0][2]*b03 - mat[1][2]*b01 + mat[2][2]*b00)*invDet;
    return result;
}

Matrix MatrixTranslation(Vec3f v) {
    Matrix result = Mat44Identity();
    result[0][3] = v.x;
    result[1][3] = v.y;
    result[2][3] = v.z;
    return result;
}

Matrix MatrixRotation(Vec3f axis, float angle) {
    Vec3f a = Vec3LengthSqr(axis);
    float s = sinf(angle);
    float c = cosf(angle);
    float t = 1.f - c;
    
    Matrix result = Mat44Identity();
    result[0][0] = a.x*a.x*t + c;
    result[1][0] = a.y*a.x*t + a.z*s;
    result[2][0] = a.z*a.x*t - a.y*s;
    result[0][1] = a.x*a.y*t - a.z*s;
    result[1][1] = a.y*a.y*t + c;
    result[2][1] = a.z*a.y*t + a.x*s;
    result[0][2] = a.x*a.z*t + a.y*s;
    result[1][2] = a.y*a.z*t - a.x*s;
    result[2][2] = a.z*a.z*t + c;
    return result;
}

Matrix MatrixScaling(Vec3f scale) {
    Matrix result = Mat44Zero();
    result[0][0] = scale.x;
    result[1][1] = scale.y;
    result[2][2] = scale.z;
    result[3][3] = 1.f;
    return result;
}

Matrix MatrixFrustum(float left, float right, float bottom, float top, float near, float far) {
    float rl = right - left;
    float tb = top - bottom;
    float fn = far - near;
    
    Matrix result = Mat44Zero();
    result[0][0] = (near*2.0f)/rl;
    result[1][1] = (near*2.0f)/tb;
    result[0][2] = (right + left)/rl;
    result[1][2] = (top + bottom)/tb;
    result[2][2] = -(far + near)/fn;
    result[3][2] = -1.0f;
    result[2][3] = -(far*near*2.0f)/fn;
    return result;
}

Matrix MatrixPerspective(float fovY, float aspect, float nearPlane, float farPlane) {
    float top = nearPlane*tan(fovY*0.5);
    float bottom = -top;
    float right = top*aspect;
    float left = -right;
    float rl = right - left;
    float tb = top - bottom;
    float fn = farPlane - nearPlane;
    
    Matrix result = Mat44Zero();
    result[0][0] = (nearPlane*2.0f)/rl;
    result[1][1] = (nearPlane*2.0f)/tb;
    result[0][2] = (right + left)/rl;
    result[1][2] = (top + bottom)/tb;
    result[2][2] = -(farPlane + nearPlane)/fn;
    result[3][2] = -1.0f;
    result[2][3] = -(farPlane*nearPlane*2.0f)/fn;
    return result;
}

Matrix MatrixOrtho(float left, float right, float bottom, float top, float nearPlane, float farPlane) {
    float rl = right - left;
    float tb = top - bottom;
    float fn = farPlane - nearPlane;

    Matrix result = Mat44Zero();
    result[0][0] = 2.0f/rl;
    result[1][1] = 2.0f/tb;
    result[2][2] = -2.0f/fn;
    result[0][3] = -(left + right)/rl;
    result[1][3] = -(top + bottom)/tb;
    result[2][3] = -(farPlane + nearPlane)/fn;
    result[3][3] = 1.0f;
    return result;
}

Matrix MatrixLookAt(Vec3f eye, Vec3f target, Vec3f up) {
    Vec3f vz = Vec3Normalize(eye - target);
    Vec3f vx = Vec3Normalize(Vec3Cross(up, vz));
    Vec3f vy = Vec3Cross(vz, vx);
    
    Matrix result = Mat44Zero();
    result[0][0] = vx.x;
    result[1][0] = vy.x;
    result[2][0] = vz.x;
    result[0][1] = vx.y;
    result[1][1] = vy.y;
    result[2][1] = vz.y;
    result[0][2] = vx.z;
    result[1][2] = vy.z;
    result[2][2] = vz.z;
    result[0][3] = -Vec3Dot(vx, eye);
    result[1][3] = -Vec3Dot(vy, eye);
    result[2][3] = -Vec3Dot(vz, eye);
    result[3][3] = 1.0f;
    return result;
}

bool FloatCmp(float a, float b) {
    return (fabsf(a - b)) <= (EPSILON * fmaxf(1.0f, fmaxf(fabsf(a), fabsf(b))));
}

int Min(int a, int b) {
    return a < b ? a : b;
}

int Max(int a, int b) {
    return a > b ? a : b;
}

int Clamp(int n, int min, int max) {
    return Min(Max(n, min), max);
}

float Degrees(float radians) {
    return radians * (180.0f / PI);
}

float Radians(float degrees) {
    return degrees * (PI / 180.0f);
}

float EaseLinearNone(float t, float b, float c, float d) {
    return (c * t / d + b);
}

float EaseLinearIn(float t, float b, float c, float d) {
    return (c * t / d + b);
}

float EaseLinearOut(float t, float b, float c, float d) {
    return (c * t / d + b);
}

float EaseLinearInOut(float t, float b, float c, float d) {
    return (c * t / d + b);
}

float EaseSineIn(float t, float b, float c, float d) {
    return (-c * cosf(t / d * (PI / 2.0f)) + c + b);
}

float EaseSineOut(float t, float b, float c, float d) {
    return (c * sinf(t / d * (PI / 2.0f)) + b);
}

float EaseSineInOut(float t, float b, float c, float d) {
    return (-c / 2.0f * (cosf(PI * t / d) - 1.0f) + b);
}

float EaseCircIn(float t, float b, float c, float d) {
    t /= d;
    return (-c * (sqrtf(1.0f - t * t) - 1.0f) + b);
}

float EaseCircOut(float t, float b, float c, float d) {
    t = t / d - 1.0f;
    return (c * sqrtf(1.0f - t * t) + b);
}

float EaseCircInOut(float t, float b, float c, float d) {
    if ((t /= d / 2.0f) < 1.0f)
        return (-c / 2.0f * (sqrtf(1.0f - t * t) - 1.0f) + b);
    t -= 2.0f;
    return (c / 2.0f * (sqrtf(1.0f - t * t) + 1.0f) + b);
}

float EaseCubicIn(float t, float b, float c, float d) {
    t /= d;
    return (c * t * t * t + b);
}

float EaseCubicOut(float t, float b, float c, float d) {
    t = t / d - 1.0f;
    return (c * (t * t * t + 1.0f) + b);
}

float EaseCubicInOut(float t, float b, float c, float d) {
    if ((t /= d / 2.0f) < 1.0f)
        return (c / 2.0f * t * t * t + b);
    t -= 2.0f;
    return (c / 2.0f * (t * t * t + 2.0f) + b);
}

float EaseQuadIn(float t, float b, float c, float d) {
    t /= d;
    return (c * t * t + b);
}

float EaseQuadOut(float t, float b, float c, float d) {
    t /= d;
    return (-c * t * (t - 2.0f) + b);
}

float EaseQuadInOut(float t, float b, float c, float d) {
    if ((t /= d / 2) < 1)
        return (((c / 2) * (t * t)) + b);
    return (-c / 2.0f * (((t - 1.0f) * (t - 3.0f)) - 1.0f) + b);
}

float EaseExpoIn(float t, float b, float c, float d) {
    return (t == 0.0f) ? b : (c * powf(2.0f, 10.0f * (t / d - 1.0f)) + b);
}

float EaseExpoOut(float t, float b, float c, float d) {
    return (t == d) ? (b + c) : (c * (-powf(2.0f, -10.0f * t / d) + 1.0f) + b);
}

float EaseExpoInOut(float t, float b, float c, float d) {
    if (t == 0.0f)
        return b;
    if (t == d)
        return (b + c);
    if ((t /= d / 2.0f) < 1.0f)
        return (c / 2.0f * powf(2.0f, 10.0f * (t - 1.0f)) + b);

    return (c / 2.0f * (-powf(2.0f, -10.0f * (t - 1.0f)) + 2.0f) + b);
}

float EaseBackIn(float t, float b, float c, float d) {
    float s = 1.70158f;
    float postFix = t /= d;
    return (c * (postFix)*t * ((s + 1.0f) * t - s) + b);
}

float EaseBackOut(float t, float b, float c, float d) {
    float s = 1.70158f;
    t = t / d - 1.0f;
    return (c * (t * t * ((s + 1.0f) * t + s) + 1.0f) + b);
}

float EaseBackInOut(float t, float b, float c, float d) {
    float s = 1.70158f;
    if ((t /= d / 2.0f) < 1.0f) {
        s *= 1.525f;
        return (c / 2.0f * (t * t * ((s + 1.0f) * t - s)) + b);
    }

    float postFix = t -= 2.0f;
    s *= 1.525f;
    return (c / 2.0f * ((postFix)*t * ((s + 1.0f) * t + s) + 2.0f) + b);
}

float EaseBounceOut(float t, float b, float c, float d) {
    if ((t /= d) < (1.0f / 2.75f)) {
        return (c * (7.5625f * t * t) + b);
    } else if (t < (2.0f / 2.75f)) {
        float postFix = t -= (1.5f / 2.75f);
        return (c * (7.5625f * (postFix)*t + 0.75f) + b);
    } else if (t < (2.5 / 2.75)) {
        float postFix = t -= (2.25f / 2.75f);
        return (c * (7.5625f * (postFix)*t + 0.9375f) + b);
    } else {
        float postFix = t -= (2.625f / 2.75f);
        return (c * (7.5625f * (postFix)*t + 0.984375f) + b);
    }
}

float EaseBounceIn(float t, float b, float c, float d) {
    return (c - EaseBounceOut(d - t, 0.0f, c, d) + b);
    
}

float EaseBounceInOut(float t, float b, float c, float d) {
    if (t < d / 2.0f)
        return (EaseBounceIn(t * 2.0f, 0.0f, c, d) * 0.5f + b);
    else
        return (EaseBounceOut(t * 2.0f - d, 0.0f, c, d) * 0.5f + c * 0.5f + b);
}

float EaseElasticIn(float t, float b, float c, float d) {
    if (t == 0.0f)
        return b;
    if ((t /= d) == 1.0f)
        return (b + c);

    float p = d * 0.3f;
    float a = c;
    float s = p / 4.0f;
    float postFix = a * powf(2.0f, 10.0f * (t -= 1.0f));

    return (-(postFix * sinf((t * d - s) * (2.0f * PI) / p)) + b);
}

float EaseElasticOut(float t, float b, float c, float d) {
    if (t == 0.0f)
        return b;
    if ((t /= d) == 1.0f)
        return (b + c);

    float p = d * 0.3f;
    float a = c;
    float s = p / 4.0f;

    return (a * powf(2.0f, -10.0f * t) * sinf((t * d - s) * (2.0f * PI) / p) + c + b);
}

float EaseElasticInOut(float t, float b, float c, float d) {
    if (t == 0.0f)
        return b;
    if ((t /= d / 2.0f) == 2.0f)
        return (b + c);

    float p = d * (0.3f * 1.5f);
    float a = c;
    float s = p / 4.0f;

    if (t < 1.0f) {
        float postFix = a * powf(2.0f, 10.0f * (t -= 1.0f));
        return -0.5f * (postFix * sinf((t * d - s) * (2.0f * PI) / p)) + b;
    }

    float postFix = a * powf(2.0f, -10.0f * (t -= 1.0f));

    return (postFix * sinf((t * d - s) * (2.0f * PI) / p) * 0.5f + c + b);
}
#endif
