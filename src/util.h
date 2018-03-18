#pragma once

#define VEC_CROSS(a, b) a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]
#define VEC_SET(a, x, y, z) a[0] = x; a[1] = y; a[2] = z
#define VEC_COPY(a, b) b[0] = a[0]; b[1] = a[1]; b[2] = a[2]
#define VEC_SUB(a, b) a[0] -= b[0]; a[1] -= b[1]; a[2] -= b[2]
#define VEC_ADD(a, b) a[0] += b[0]; a[1] += b[1]; a[2] += b[2]
#define VEC_MULT(a, b) a[0] *= b, a[1] *= b, a[2] *= b
#define VEC_LENGTH(a) sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2])
#define VEC_CREATE(a, b, c) {a, b, c}
#define VEC_EXPAND(a) a[0], a[1], a[2]
#define VEC_NORMALIZE(a) do {double l = VEC_LENGTH(a); VEC_MULT(a, 1.0 / l);} while(0)
#define VEC_DOT(a, b) (a[0] * b[0] + a[1] * b[1] + a[2] * b[2])
