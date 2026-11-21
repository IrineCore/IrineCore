// =============================================================================
// IrineCore — EtMathPortable.h
// Cross-platform (Windows + Linux) pure C++ replacement for DirectX 9 D3DXMath.
// Preserves 100% binary layout and mathematical parity with d3dx9math.h.
// =============================================================================
#pragma once

#include <cmath>
#include <cstring>
#include <cstdint>
#include <algorithm>

#ifndef D3DX_PI
#define D3DX_PI (3.141592654f)
#endif

#ifndef D3DXToRadian
#define D3DXToRadian(degree) ((degree) * (D3DX_PI / 180.0f))
#endif

#ifndef D3DXToDegree
#define D3DXToDegree(radian) ((radian) * (180.0f / D3DX_PI))
#endif

#ifndef _D3DVIEWPORT9_DEFINED
#define _D3DVIEWPORT9_DEFINED
struct D3DVIEWPORT9
{
    uint32_t X;
    uint32_t Y;
    uint32_t Width;
    uint32_t Height;
    float    MinZ;
    float    MaxZ;
};
#endif

// =============================================================================
// 1. Core Math Types (Binary-compatible with d3dx9math.h)
// =============================================================================

struct D3DXVECTOR2
{
    float x, y;

    D3DXVECTOR2() : x(0.0f), y(0.0f) {}
    D3DXVECTOR2(const float* pf) : x(pf ? pf[0] : 0.0f), y(pf ? pf[1] : 0.0f) {}
    D3DXVECTOR2(float fx, float fy) : x(fx), y(fy) {}

    operator float*() { return &x; }
    operator const float*() const { return &x; }

    D3DXVECTOR2& operator+=(const D3DXVECTOR2& v) { x += v.x; y += v.y; return *this; }
    D3DXVECTOR2& operator-=(const D3DXVECTOR2& v) { x -= v.x; y -= v.y; return *this; }
    D3DXVECTOR2& operator*=(float f) { x *= f; y *= f; return *this; }
    D3DXVECTOR2& operator/=(float f) { float inv = 1.0f / f; x *= inv; y *= inv; return *this; }

    D3DXVECTOR2 operator+() const { return *this; }
    D3DXVECTOR2 operator-() const { return D3DXVECTOR2(-x, -y); }

    D3DXVECTOR2 operator+(const D3DXVECTOR2& v) const { return D3DXVECTOR2(x + v.x, y + v.y); }
    D3DXVECTOR2 operator-(const D3DXVECTOR2& v) const { return D3DXVECTOR2(x - v.x, y - v.y); }
    D3DXVECTOR2 operator*(float f) const { return D3DXVECTOR2(x * f, y * f); }
    D3DXVECTOR2 operator/(float f) const { float inv = 1.0f / f; return D3DXVECTOR2(x * inv, y * inv); }

    friend D3DXVECTOR2 operator*(float f, const D3DXVECTOR2& v) { return D3DXVECTOR2(f * v.x, f * v.y); }

    bool operator==(const D3DXVECTOR2& v) const { return x == v.x && y == v.y; }
    bool operator!=(const D3DXVECTOR2& v) const { return x != v.x || y != v.y; }
};

struct D3DXVECTOR3
{
    float x, y, z;

    D3DXVECTOR3() : x(0.0f), y(0.0f), z(0.0f) {}
    D3DXVECTOR3(const float* pf) : x(pf ? pf[0] : 0.0f), y(pf ? pf[1] : 0.0f), z(pf ? pf[2] : 0.0f) {}
    D3DXVECTOR3(float fx, float fy, float fz) : x(fx), y(fy), z(fz) {}

    operator float*() { return &x; }
    operator const float*() const { return &x; }

    D3DXVECTOR3& operator+=(const D3DXVECTOR3& v) { x += v.x; y += v.y; z += v.z; return *this; }
    D3DXVECTOR3& operator-=(const D3DXVECTOR3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
    D3DXVECTOR3& operator*=(float f) { x *= f; y *= f; z *= f; return *this; }
    D3DXVECTOR3& operator/=(float f) { float inv = 1.0f / f; x *= inv; y *= inv; z *= inv; return *this; }

    D3DXVECTOR3 operator+() const { return *this; }
    D3DXVECTOR3 operator-() const { return D3DXVECTOR3(-x, -y, -z); }

    D3DXVECTOR3 operator+(const D3DXVECTOR3& v) const { return D3DXVECTOR3(x + v.x, y + v.y, z + v.z); }
    D3DXVECTOR3 operator-(const D3DXVECTOR3& v) const { return D3DXVECTOR3(x - v.x, y - v.y, z - v.z); }
    D3DXVECTOR3 operator*(float f) const { return D3DXVECTOR3(x * f, y * f, z * f); }
    D3DXVECTOR3 operator/(float f) const { float inv = 1.0f / f; return D3DXVECTOR3(x * inv, y * inv, z * inv); }

    friend D3DXVECTOR3 operator*(float f, const D3DXVECTOR3& v) { return D3DXVECTOR3(f * v.x, f * v.y, f * v.z); }

    bool operator==(const D3DXVECTOR3& v) const { return x == v.x && y == v.y && z == v.z; }
    bool operator!=(const D3DXVECTOR3& v) const { return x != v.x || y != v.y || z != v.z; }
};

struct D3DXVECTOR4
{
    float x, y, z, w;

    D3DXVECTOR4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
    D3DXVECTOR4(const float* pf) : x(pf ? pf[0] : 0.0f), y(pf ? pf[1] : 0.0f), z(pf ? pf[2] : 0.0f), w(pf ? pf[3] : 0.0f) {}
    D3DXVECTOR4(const D3DXVECTOR3& v, float fw) : x(v.x), y(v.y), z(v.z), w(fw) {}
    D3DXVECTOR4(float fx, float fy, float fz, float fw) : x(fx), y(fy), z(fz), w(fw) {}

    operator float*() { return &x; }
    operator const float*() const { return &x; }

    D3DXVECTOR4& operator+=(const D3DXVECTOR4& v) { x += v.x; y += v.y; z += v.z; w += v.w; return *this; }
    D3DXVECTOR4& operator-=(const D3DXVECTOR4& v) { x -= v.x; y -= v.y; z -= v.z; w -= v.w; return *this; }
    D3DXVECTOR4& operator*=(float f) { x *= f; y *= f; z *= f; w *= f; return *this; }
    D3DXVECTOR4& operator/=(float f) { float inv = 1.0f / f; x *= inv; y *= inv; z *= inv; w *= inv; return *this; }

    D3DXVECTOR4 operator+() const { return *this; }
    D3DXVECTOR4 operator-() const { return D3DXVECTOR4(-x, -y, -z, -w); }

    D3DXVECTOR4 operator+(const D3DXVECTOR4& v) const { return D3DXVECTOR4(x + v.x, y + v.y, z + v.z, w + v.w); }
    D3DXVECTOR4 operator-(const D3DXVECTOR4& v) const { return D3DXVECTOR4(x - v.x, y - v.y, z - v.z, w - v.w); }
    D3DXVECTOR4 operator*(float f) const { return D3DXVECTOR4(x * f, y * f, z * f, w * f); }
    D3DXVECTOR4 operator/(float f) const { float inv = 1.0f / f; return D3DXVECTOR4(x * inv, y * inv, z * inv, w * inv); }

    friend D3DXVECTOR4 operator*(float f, const D3DXVECTOR4& v) { return D3DXVECTOR4(f * v.x, f * v.y, f * v.z, f * v.w); }

    bool operator==(const D3DXVECTOR4& v) const { return x == v.x && y == v.y && z == v.z && w == v.w; }
    bool operator!=(const D3DXVECTOR4& v) const { return !(*this == v); }
};

struct D3DXMATRIX
{
    union
    {
        struct
        {
            float _11, _12, _13, _14;
            float _21, _22, _23, _24;
            float _31, _32, _33, _34;
            float _41, _42, _43, _44;
        };
        float m[4][4];
    };

    D3DXMATRIX() { std::memset(m, 0, sizeof(m)); }
    D3DXMATRIX(const float* pf) { if (pf) std::memcpy(m, pf, sizeof(m)); else std::memset(m, 0, sizeof(m)); }
    D3DXMATRIX(
        float f11, float f12, float f13, float f14,
        float f21, float f22, float f23, float f24,
        float f31, float f32, float f33, float f34,
        float f41, float f42, float f43, float f44)
        : _11(f11), _12(f12), _13(f13), _14(f14),
          _21(f21), _22(f22), _23(f23), _24(f24),
          _31(f31), _32(f32), _33(f33), _34(f34),
          _41(f41), _42(f42), _43(f43), _44(f44) {}

    float& operator()(unsigned int row, unsigned int col) { return m[row][col]; }
    float  operator()(unsigned int row, unsigned int col) const { return m[row][col]; }

    operator float*() { return &_11; }
    operator const float*() const { return &_11; }

    D3DXMATRIX& operator*=(const D3DXMATRIX& rhs);
    D3DXMATRIX& operator+=(const D3DXMATRIX& rhs) { for (int i = 0; i < 16; ++i) (&_11)[i] += (&rhs._11)[i]; return *this; }
    D3DXMATRIX& operator-=(const D3DXMATRIX& rhs) { for (int i = 0; i < 16; ++i) (&_11)[i] -= (&rhs._11)[i]; return *this; }
    D3DXMATRIX& operator*=(float f) { for (int i = 0; i < 16; ++i) (&_11)[i] *= f; return *this; }
    D3DXMATRIX& operator/=(float f) { float inv = 1.0f / f; for (int i = 0; i < 16; ++i) (&_11)[i] *= inv; return *this; }

    D3DXMATRIX operator+() const { return *this; }
    D3DXMATRIX operator-() const { D3DXMATRIX r; for (int i = 0; i < 16; ++i) (&r._11)[i] = -(&_11)[i]; return r; }

    D3DXMATRIX operator*(const D3DXMATRIX& rhs) const;
    D3DXMATRIX operator+(const D3DXMATRIX& rhs) const { D3DXMATRIX r(*this); r += rhs; return r; }
    D3DXMATRIX operator-(const D3DXMATRIX& rhs) const { D3DXMATRIX r(*this); r -= rhs; return r; }
    D3DXMATRIX operator*(float f) const { D3DXMATRIX r(*this); r *= f; return r; }
    D3DXMATRIX operator/(float f) const { D3DXMATRIX r(*this); r /= f; return r; }

    friend D3DXMATRIX operator*(float f, const D3DXMATRIX& rhs) { return rhs * f; }

    bool operator==(const D3DXMATRIX& rhs) const { return std::memcmp(m, rhs.m, sizeof(m)) == 0; }
    bool operator!=(const D3DXMATRIX& rhs) const { return !(*this == rhs); }
};

struct D3DXQUATERNION
{
    float x, y, z, w;

    D3DXQUATERNION() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
    D3DXQUATERNION(const float* pf) : x(pf ? pf[0] : 0.0f), y(pf ? pf[1] : 0.0f), z(pf ? pf[2] : 0.0f), w(pf ? pf[3] : 0.0f) {}
    D3DXQUATERNION(float fx, float fy, float fz, float fw) : x(fx), y(fy), z(fz), w(fw) {}

    operator float*() { return &x; }
    operator const float*() const { return &x; }

    D3DXQUATERNION& operator+=(const D3DXQUATERNION& q) { x += q.x; y += q.y; z += q.z; w += q.w; return *this; }
    D3DXQUATERNION& operator-=(const D3DXQUATERNION& q) { x -= q.x; y -= q.y; z -= q.z; w -= q.w; return *this; }
    D3DXQUATERNION& operator*=(const D3DXQUATERNION& q);
    D3DXQUATERNION& operator*=(float f) { x *= f; y *= f; z *= f; w *= f; return *this; }
    D3DXQUATERNION& operator/=(float f) { float inv = 1.0f / f; x *= inv; y *= inv; z *= inv; w *= inv; return *this; }

    D3DXQUATERNION operator+() const { return *this; }
    D3DXQUATERNION operator-() const { return D3DXQUATERNION(-x, -y, -z, -w); }

    D3DXQUATERNION operator+(const D3DXQUATERNION& q) const { return D3DXQUATERNION(x + q.x, y + q.y, z + q.z, w + q.w); }
    D3DXQUATERNION operator-(const D3DXQUATERNION& q) const { return D3DXQUATERNION(x - q.x, y - q.y, z - q.z, w - q.w); }
    D3DXQUATERNION operator*(const D3DXQUATERNION& q) const;
    D3DXQUATERNION operator*(float f) const { return D3DXQUATERNION(x * f, y * f, z * f, w * f); }
    D3DXQUATERNION operator/(float f) const { float inv = 1.0f / f; return D3DXQUATERNION(x * inv, y * inv, z * inv, w * inv); }

    friend D3DXQUATERNION operator*(float f, const D3DXQUATERNION& q) { return q * f; }

    bool operator==(const D3DXQUATERNION& q) const { return x == q.x && y == q.y && z == q.z && w == q.w; }
    bool operator!=(const D3DXQUATERNION& q) const { return !(*this == q); }
};

struct D3DXPLANE
{
    float a, b, c, d;

    D3DXPLANE() : a(0.0f), b(0.0f), c(0.0f), d(0.0f) {}
    D3DXPLANE(const float* pf) : a(pf ? pf[0] : 0.0f), b(pf ? pf[1] : 0.0f), c(pf ? pf[2] : 0.0f), d(pf ? pf[3] : 0.0f) {}
    D3DXPLANE(float fa, float fb, float fc, float fd) : a(fa), b(fb), c(fc), d(fd) {}

    operator float*() { return &a; }
    operator const float*() const { return &a; }

    D3DXPLANE& operator*=(float f) { a *= f; b *= f; c *= f; d *= f; return *this; }
    D3DXPLANE& operator/=(float f) { float inv = 1.0f / f; a *= inv; b *= inv; c *= inv; d *= inv; return *this; }

    D3DXPLANE operator+() const { return *this; }
    D3DXPLANE operator-() const { return D3DXPLANE(-a, -b, -c, -d); }

    D3DXPLANE operator*(float f) const { return D3DXPLANE(a * f, b * f, c * f, d * f); }
    D3DXPLANE operator/(float f) const { float inv = 1.0f / f; return D3DXPLANE(a * inv, b * inv, c * inv, d * inv); }

    friend D3DXPLANE operator*(float f, const D3DXPLANE& p) { return p * f; }

    bool operator==(const D3DXPLANE& p) const { return a == p.a && b == p.b && c == p.c && d == p.d; }
    bool operator!=(const D3DXPLANE& p) const { return !(*this == p); }
};

struct D3DXCOLOR
{
    float r, g, b, a;

    D3DXCOLOR() : r(0.0f), g(0.0f), b(0.0f), a(0.0f) {}
    D3DXCOLOR(uint32_t dw)
    {
        const float f = 1.0f / 255.0f;
        r = f * static_cast<float>(static_cast<uint8_t>(dw >> 16));
        g = f * static_cast<float>(static_cast<uint8_t>(dw >>  8));
        b = f * static_cast<float>(static_cast<uint8_t>(dw >>  0));
        a = f * static_cast<float>(static_cast<uint8_t>(dw >> 24));
    }
    D3DXCOLOR(const float* pf) : r(pf ? pf[0] : 0.0f), g(pf ? pf[1] : 0.0f), b(pf ? pf[2] : 0.0f), a(pf ? pf[3] : 0.0f) {}
    D3DXCOLOR(float fr, float fg, float fb, float fa) : r(fr), g(fg), b(fb), a(fa) {}

    operator uint32_t() const
    {
        uint32_t dwR = r >= 1.0f ? 0xff : r <= 0.0f ? 0x00 : static_cast<uint32_t>(r * 255.0f + 0.5f);
        uint32_t dwG = g >= 1.0f ? 0xff : g <= 0.0f ? 0x00 : static_cast<uint32_t>(g * 255.0f + 0.5f);
        uint32_t dwB = b >= 1.0f ? 0xff : b <= 0.0f ? 0x00 : static_cast<uint32_t>(b * 255.0f + 0.5f);
        uint32_t dwA = a >= 1.0f ? 0xff : a <= 0.0f ? 0x00 : static_cast<uint32_t>(a * 255.0f + 0.5f);
        return (dwA << 24) | (dwR << 16) | (dwG << 8) | dwB;
    }
    operator float*() { return &r; }
    operator const float*() const { return &r; }

    D3DXCOLOR& operator+=(const D3DXCOLOR& c) { r += c.r; g += c.g; b += c.b; a += c.a; return *this; }
    D3DXCOLOR& operator-=(const D3DXCOLOR& c) { r -= c.r; g -= c.g; b -= c.b; a -= c.a; return *this; }
    D3DXCOLOR& operator*=(float f) { r *= f; g *= f; b *= f; a *= f; return *this; }
    D3DXCOLOR& operator/=(float f) { float inv = 1.0f / f; r *= inv; g *= inv; b *= inv; a *= inv; return *this; }

    D3DXCOLOR operator+() const { return *this; }
    D3DXCOLOR operator-() const { return D3DXCOLOR(-r, -g, -b, -a); }

    D3DXCOLOR operator+(const D3DXCOLOR& c) const { return D3DXCOLOR(r + c.r, g + c.g, b + c.b, a + c.a); }
    D3DXCOLOR operator-(const D3DXCOLOR& c) const { return D3DXCOLOR(r - c.r, g - c.g, b - c.b, a - c.a); }
    D3DXCOLOR operator*(float f) const { return D3DXCOLOR(r * f, g * f, b * f, a * f); }
    D3DXCOLOR operator/(float f) const { float inv = 1.0f / f; return D3DXCOLOR(r * inv, g * inv, b * inv, a * inv); }

    friend D3DXCOLOR operator*(float f, const D3DXCOLOR& c) { return c * f; }

    bool operator==(const D3DXCOLOR& c) const { return r == c.r && g == c.g && b == c.b && a == c.a; }
    bool operator!=(const D3DXCOLOR& c) const { return !(*this == c); }
};

// =============================================================================
// 2. D3DXVec2 Functions
// =============================================================================

inline float D3DXVec2Length(const D3DXVECTOR2* pV) { return std::sqrt(pV->x * pV->x + pV->y * pV->y); }
inline float D3DXVec2LengthSq(const D3DXVECTOR2* pV) { return pV->x * pV->x + pV->y * pV->y; }
inline float D3DXVec2Dot(const D3DXVECTOR2* pV1, const D3DXVECTOR2* pV2) { return pV1->x * pV2->x + pV1->y * pV2->y; }
inline float D3DXVec2CCW(const D3DXVECTOR2* pV1, const D3DXVECTOR2* pV2) { return pV1->x * pV2->y - pV1->y * pV2->x; }
inline D3DXVECTOR2* D3DXVec2Add(D3DXVECTOR2* pOut, const D3DXVECTOR2* pV1, const D3DXVECTOR2* pV2) { pOut->x = pV1->x + pV2->x; pOut->y = pV1->y + pV2->y; return pOut; }
inline D3DXVECTOR2* D3DXVec2Subtract(D3DXVECTOR2* pOut, const D3DXVECTOR2* pV1, const D3DXVECTOR2* pV2) { pOut->x = pV1->x - pV2->x; pOut->y = pV1->y - pV2->y; return pOut; }
inline D3DXVECTOR2* D3DXVec2Minimize(D3DXVECTOR2* pOut, const D3DXVECTOR2* pV1, const D3DXVECTOR2* pV2) { pOut->x = pV1->x < pV2->x ? pV1->x : pV2->x; pOut->y = pV1->y < pV2->y ? pV1->y : pV2->y; return pOut; }
inline D3DXVECTOR2* D3DXVec2Maximize(D3DXVECTOR2* pOut, const D3DXVECTOR2* pV1, const D3DXVECTOR2* pV2) { pOut->x = pV1->x > pV2->x ? pV1->x : pV2->x; pOut->y = pV1->y > pV2->y ? pV1->y : pV2->y; return pOut; }
inline D3DXVECTOR2* D3DXVec2Scale(D3DXVECTOR2* pOut, const D3DXVECTOR2* pV, float s) { pOut->x = pV->x * s; pOut->y = pV->y * s; return pOut; }
inline D3DXVECTOR2* D3DXVec2Lerp(D3DXVECTOR2* pOut, const D3DXVECTOR2* pV1, const D3DXVECTOR2* pV2, float s) { pOut->x = pV1->x + s * (pV2->x - pV1->x); pOut->y = pV1->y + s * (pV2->y - pV1->y); return pOut; }
inline D3DXVECTOR2* D3DXVec2Normalize(D3DXVECTOR2* pOut, const D3DXVECTOR2* pV)
{
    float len = D3DXVec2Length(pV);
    if (len > 1e-8f) { float inv = 1.0f / len; pOut->x = pV->x * inv; pOut->y = pV->y * inv; }
    else { pOut->x = 0.0f; pOut->y = 0.0f; }
    return pOut;
}
inline D3DXVECTOR2* D3DXVec2Hermite(D3DXVECTOR2* pOut, const D3DXVECTOR2* pV1, const D3DXVECTOR2* pT1, const D3DXVECTOR2* pV2, const D3DXVECTOR2* pT2, float s)
{
    float s2 = s * s, s3 = s2 * s;
    float h1 = 2.0f * s3 - 3.0f * s2 + 1.0f;
    float h2 = -2.0f * s3 + 3.0f * s2;
    float h3 = s3 - 2.0f * s2 + s;
    float h4 = s3 - s2;
    *pOut = (*pV1) * h1 + (*pV2) * h2 + (*pT1) * h3 + (*pT2) * h4;
    return pOut;
}
inline D3DXVECTOR2* D3DXVec2CatmullRom(D3DXVECTOR2* pOut, const D3DXVECTOR2* pV0, const D3DXVECTOR2* pV1, const D3DXVECTOR2* pV2, const D3DXVECTOR2* pV3, float s)
{
    float s2 = s * s, s3 = s2 * s;
    *pOut = 0.5f * ((2.0f * (*pV1)) + (-(*pV0) + (*pV2)) * s + (2.0f * (*pV0) - 5.0f * (*pV1) + 4.0f * (*pV2) - (*pV3)) * s2 + (-(*pV0) + 3.0f * (*pV1) - 3.0f * (*pV2) + (*pV3)) * s3);
    return pOut;
}
inline D3DXVECTOR2* D3DXVec2BaryCentric(D3DXVECTOR2* pOut, const D3DXVECTOR2* pV1, const D3DXVECTOR2* pV2, const D3DXVECTOR2* pV3, float f, float g)
{
    *pOut = *pV1 + f * (*pV2 - *pV1) + g * (*pV3 - *pV1);
    return pOut;
}
inline D3DXVECTOR4* D3DXVec2Transform(D3DXVECTOR4* pOut, const D3DXVECTOR2* pV, const D3DXMATRIX* pM)
{
    float x = pV->x, y = pV->y;
    pOut->x = x * pM->_11 + y * pM->_21 + pM->_41;
    pOut->y = x * pM->_12 + y * pM->_22 + pM->_42;
    pOut->z = x * pM->_13 + y * pM->_23 + pM->_43;
    pOut->w = x * pM->_14 + y * pM->_24 + pM->_44;
    return pOut;
}
inline D3DXVECTOR2* D3DXVec2TransformCoord(D3DXVECTOR2* pOut, const D3DXVECTOR2* pV, const D3DXMATRIX* pM)
{
    float x = pV->x, y = pV->y;
    float w = x * pM->_14 + y * pM->_24 + pM->_44;
    float invW = (std::fabs(w) > 1e-8f) ? (1.0f / w) : 1.0f;
    pOut->x = (x * pM->_11 + y * pM->_21 + pM->_41) * invW;
    pOut->y = (x * pM->_12 + y * pM->_22 + pM->_42) * invW;
    return pOut;
}
inline D3DXVECTOR2* D3DXVec2TransformNormal(D3DXVECTOR2* pOut, const D3DXVECTOR2* pV, const D3DXMATRIX* pM)
{
    float x = pV->x, y = pV->y;
    pOut->x = x * pM->_11 + y * pM->_21;
    pOut->y = x * pM->_12 + y * pM->_22;
    return pOut;
}
inline D3DXVECTOR4* D3DXVec2TransformArray(D3DXVECTOR4* pOut, unsigned int OutStride, const D3DXVECTOR2* pV, unsigned int VStride, const D3DXMATRIX* pM, unsigned int n)
{
    for (unsigned int i = 0; i < n; ++i)
        D3DXVec2Transform(reinterpret_cast<D3DXVECTOR4*>(reinterpret_cast<uint8_t*>(pOut) + i * OutStride),
                          reinterpret_cast<const D3DXVECTOR2*>(reinterpret_cast<const uint8_t*>(pV) + i * VStride), pM);
    return pOut;
}
inline D3DXVECTOR2* D3DXVec2TransformCoordArray(D3DXVECTOR2* pOut, unsigned int OutStride, const D3DXVECTOR2* pV, unsigned int VStride, const D3DXMATRIX* pM, unsigned int n)
{
    for (unsigned int i = 0; i < n; ++i)
        D3DXVec2TransformCoord(reinterpret_cast<D3DXVECTOR2*>(reinterpret_cast<uint8_t*>(pOut) + i * OutStride),
                               reinterpret_cast<const D3DXVECTOR2*>(reinterpret_cast<const uint8_t*>(pV) + i * VStride), pM);
    return pOut;
}
inline D3DXVECTOR2* D3DXVec2TransformNormalArray(D3DXVECTOR2* pOut, unsigned int OutStride, const D3DXVECTOR2* pV, unsigned int VStride, const D3DXMATRIX* pM, unsigned int n)
{
    for (unsigned int i = 0; i < n; ++i)
        D3DXVec2TransformNormal(reinterpret_cast<D3DXVECTOR2*>(reinterpret_cast<uint8_t*>(pOut) + i * OutStride),
                                reinterpret_cast<const D3DXVECTOR2*>(reinterpret_cast<const uint8_t*>(pV) + i * VStride), pM);
    return pOut;
}

// =============================================================================
// 3. D3DXVec3 Functions
// =============================================================================

inline float D3DXVec3Length(const D3DXVECTOR3* pV) { return std::sqrt(pV->x * pV->x + pV->y * pV->y + pV->z * pV->z); }
inline float D3DXVec3LengthSq(const D3DXVECTOR3* pV) { return pV->x * pV->x + pV->y * pV->y + pV->z * pV->z; }
inline float D3DXVec3Dot(const D3DXVECTOR3* pV1, const D3DXVECTOR3* pV2) { return pV1->x * pV2->x + pV1->y * pV2->y + pV1->z * pV2->z; }
inline D3DXVECTOR3* D3DXVec3Cross(D3DXVECTOR3* pOut, const D3DXVECTOR3* pV1, const D3DXVECTOR3* pV2)
{
    D3DXVECTOR3 v(
        pV1->y * pV2->z - pV1->z * pV2->y,
        pV1->z * pV2->x - pV1->x * pV2->z,
        pV1->x * pV2->y - pV1->y * pV2->x);
    *pOut = v;
    return pOut;
}
inline D3DXVECTOR3* D3DXVec3Add(D3DXVECTOR3* pOut, const D3DXVECTOR3* pV1, const D3DXVECTOR3* pV2) { pOut->x = pV1->x + pV2->x; pOut->y = pV1->y + pV2->y; pOut->z = pV1->z + pV2->z; return pOut; }
inline D3DXVECTOR3* D3DXVec3Subtract(D3DXVECTOR3* pOut, const D3DXVECTOR3* pV1, const D3DXVECTOR3* pV2) { pOut->x = pV1->x - pV2->x; pOut->y = pV1->y - pV2->y; pOut->z = pV1->z - pV2->z; return pOut; }
inline D3DXVECTOR3* D3DXVec3Minimize(D3DXVECTOR3* pOut, const D3DXVECTOR3* pV1, const D3DXVECTOR3* pV2) { pOut->x = pV1->x < pV2->x ? pV1->x : pV2->x; pOut->y = pV1->y < pV2->y ? pV1->y : pV2->y; pOut->z = pV1->z < pV2->z ? pV1->z : pV2->z; return pOut; }
inline D3DXVECTOR3* D3DXVec3Maximize(D3DXVECTOR3* pOut, const D3DXVECTOR3* pV1, const D3DXVECTOR3* pV2) { pOut->x = pV1->x > pV2->x ? pV1->x : pV2->x; pOut->y = pV1->y > pV2->y ? pV1->y : pV2->y; pOut->z = pV1->z > pV2->z ? pV1->z : pV2->z; return pOut; }
inline D3DXVECTOR3* D3DXVec3Scale(D3DXVECTOR3* pOut, const D3DXVECTOR3* pV, float s) { pOut->x = pV->x * s; pOut->y = pV->y * s; pOut->z = pV->z * s; return pOut; }
inline D3DXVECTOR3* D3DXVec3Lerp(D3DXVECTOR3* pOut, const D3DXVECTOR3* pV1, const D3DXVECTOR3* pV2, float s) { pOut->x = pV1->x + s * (pV2->x - pV1->x); pOut->y = pV1->y + s * (pV2->y - pV1->y); pOut->z = pV1->z + s * (pV2->z - pV1->z); return pOut; }
inline D3DXVECTOR3* D3DXVec3Normalize(D3DXVECTOR3* pOut, const D3DXVECTOR3* pV)
{
    float len = D3DXVec3Length(pV);
    if (len > 1e-8f) { float inv = 1.0f / len; pOut->x = pV->x * inv; pOut->y = pV->y * inv; pOut->z = pV->z * inv; }
    else { pOut->x = 0.0f; pOut->y = 0.0f; pOut->z = 0.0f; }
    return pOut;
}
inline D3DXVECTOR3* D3DXVec3Hermite(D3DXVECTOR3* pOut, const D3DXVECTOR3* pV1, const D3DXVECTOR3* pT1, const D3DXVECTOR3* pV2, const D3DXVECTOR3* pT2, float s)
{
    float s2 = s * s, s3 = s2 * s;
    float h1 = 2.0f * s3 - 3.0f * s2 + 1.0f;
    float h2 = -2.0f * s3 + 3.0f * s2;
    float h3 = s3 - 2.0f * s2 + s;
    float h4 = s3 - s2;
    *pOut = (*pV1) * h1 + (*pV2) * h2 + (*pT1) * h3 + (*pT2) * h4;
    return pOut;
}
inline D3DXVECTOR3* D3DXVec3CatmullRom(D3DXVECTOR3* pOut, const D3DXVECTOR3* pV0, const D3DXVECTOR3* pV1, const D3DXVECTOR3* pV2, const D3DXVECTOR3* pV3, float s)
{
    float s2 = s * s, s3 = s2 * s;
    *pOut = 0.5f * ((2.0f * (*pV1)) + (-(*pV0) + (*pV2)) * s + (2.0f * (*pV0) - 5.0f * (*pV1) + 4.0f * (*pV2) - (*pV3)) * s2 + (-(*pV0) + 3.0f * (*pV1) - 3.0f * (*pV2) + (*pV3)) * s3);
    return pOut;
}
inline D3DXVECTOR3* D3DXVec3BaryCentric(D3DXVECTOR3* pOut, const D3DXVECTOR3* pV1, const D3DXVECTOR3* pV2, const D3DXVECTOR3* pV3, float f, float g)
{
    *pOut = *pV1 + f * (*pV2 - *pV1) + g * (*pV3 - *pV1);
    return pOut;
}
inline D3DXVECTOR4* D3DXVec3Transform(D3DXVECTOR4* pOut, const D3DXVECTOR3* pV, const D3DXMATRIX* pM)
{
    float x = pV->x, y = pV->y, z = pV->z;
    pOut->x = x * pM->_11 + y * pM->_21 + z * pM->_31 + pM->_41;
    pOut->y = x * pM->_12 + y * pM->_22 + z * pM->_32 + pM->_42;
    pOut->z = x * pM->_13 + y * pM->_23 + z * pM->_33 + pM->_43;
    pOut->w = x * pM->_14 + y * pM->_24 + z * pM->_34 + pM->_44;
    return pOut;
}
inline D3DXVECTOR3* D3DXVec3TransformCoord(D3DXVECTOR3* pOut, const D3DXVECTOR3* pV, const D3DXMATRIX* pM)
{
    float x = pV->x, y = pV->y, z = pV->z;
    float w = x * pM->_14 + y * pM->_24 + z * pM->_34 + pM->_44;
    float invW = (std::fabs(w) > 1e-8f) ? (1.0f / w) : 1.0f;
    pOut->x = (x * pM->_11 + y * pM->_21 + z * pM->_31 + pM->_41) * invW;
    pOut->y = (x * pM->_12 + y * pM->_22 + z * pM->_32 + pM->_42) * invW;
    pOut->z = (x * pM->_13 + y * pM->_23 + z * pM->_33 + pM->_43) * invW;
    return pOut;
}
inline D3DXVECTOR3* D3DXVec3TransformNormal(D3DXVECTOR3* pOut, const D3DXVECTOR3* pV, const D3DXMATRIX* pM)
{
    float x = pV->x, y = pV->y, z = pV->z;
    pOut->x = x * pM->_11 + y * pM->_21 + z * pM->_31;
    pOut->y = x * pM->_12 + y * pM->_22 + z * pM->_32;
    pOut->z = x * pM->_13 + y * pM->_23 + z * pM->_33;
    return pOut;
}
inline D3DXVECTOR4* D3DXVec3TransformArray(D3DXVECTOR4* pOut, unsigned int OutStride, const D3DXVECTOR3* pV, unsigned int VStride, const D3DXMATRIX* pM, unsigned int n)
{
    for (unsigned int i = 0; i < n; ++i)
        D3DXVec3Transform(reinterpret_cast<D3DXVECTOR4*>(reinterpret_cast<uint8_t*>(pOut) + i * OutStride),
                          reinterpret_cast<const D3DXVECTOR3*>(reinterpret_cast<const uint8_t*>(pV) + i * VStride), pM);
    return pOut;
}
inline D3DXVECTOR3* D3DXVec3TransformCoordArray(D3DXVECTOR3* pOut, unsigned int OutStride, const D3DXVECTOR3* pV, unsigned int VStride, const D3DXMATRIX* pM, unsigned int n)
{
    for (unsigned int i = 0; i < n; ++i)
        D3DXVec3TransformCoord(reinterpret_cast<D3DXVECTOR3*>(reinterpret_cast<uint8_t*>(pOut) + i * OutStride),
                               reinterpret_cast<const D3DXVECTOR3*>(reinterpret_cast<const uint8_t*>(pV) + i * VStride), pM);
    return pOut;
}
inline D3DXVECTOR3* D3DXVec3TransformNormalArray(D3DXVECTOR3* pOut, unsigned int OutStride, const D3DXVECTOR3* pV, unsigned int VStride, const D3DXMATRIX* pM, unsigned int n)
{
    for (unsigned int i = 0; i < n; ++i)
        D3DXVec3TransformNormal(reinterpret_cast<D3DXVECTOR3*>(reinterpret_cast<uint8_t*>(pOut) + i * OutStride),
                                reinterpret_cast<const D3DXVECTOR3*>(reinterpret_cast<const uint8_t*>(pV) + i * VStride), pM);
    return pOut;
}

// =============================================================================
// 4. D3DXVec4 Functions
// =============================================================================

inline float D3DXVec4Length(const D3DXVECTOR4* pV) { return std::sqrt(pV->x * pV->x + pV->y * pV->y + pV->z * pV->z + pV->w * pV->w); }
inline float D3DXVec4LengthSq(const D3DXVECTOR4* pV) { return pV->x * pV->x + pV->y * pV->y + pV->z * pV->z + pV->w * pV->w; }
inline float D3DXVec4Dot(const D3DXVECTOR4* pV1, const D3DXVECTOR4* pV2) { return pV1->x * pV2->x + pV1->y * pV2->y + pV1->z * pV2->z + pV1->w * pV2->w; }
inline D3DXVECTOR4* D3DXVec4Add(D3DXVECTOR4* pOut, const D3DXVECTOR4* pV1, const D3DXVECTOR4* pV2) { *pOut = *pV1 + *pV2; return pOut; }
inline D3DXVECTOR4* D3DXVec4Subtract(D3DXVECTOR4* pOut, const D3DXVECTOR4* pV1, const D3DXVECTOR4* pV2) { *pOut = *pV1 - *pV2; return pOut; }
inline D3DXVECTOR4* D3DXVec4Minimize(D3DXVECTOR4* pOut, const D3DXVECTOR4* pV1, const D3DXVECTOR4* pV2)
{
    pOut->x = pV1->x < pV2->x ? pV1->x : pV2->x;
    pOut->y = pV1->y < pV2->y ? pV1->y : pV2->y;
    pOut->z = pV1->z < pV2->z ? pV1->z : pV2->z;
    pOut->w = pV1->w < pV2->w ? pV1->w : pV2->w;
    return pOut;
}
inline D3DXVECTOR4* D3DXVec4Maximize(D3DXVECTOR4* pOut, const D3DXVECTOR4* pV1, const D3DXVECTOR4* pV2)
{
    pOut->x = pV1->x > pV2->x ? pV1->x : pV2->x;
    pOut->y = pV1->y > pV2->y ? pV1->y : pV2->y;
    pOut->z = pV1->z > pV2->z ? pV1->z : pV2->z;
    pOut->w = pV1->w > pV2->w ? pV1->w : pV2->w;
    return pOut;
}
inline D3DXVECTOR4* D3DXVec4Scale(D3DXVECTOR4* pOut, const D3DXVECTOR4* pV, float s) { *pOut = (*pV) * s; return pOut; }
inline D3DXVECTOR4* D3DXVec4Lerp(D3DXVECTOR4* pOut, const D3DXVECTOR4* pV1, const D3DXVECTOR4* pV2, float s) { *pOut = *pV1 + (*pV2 - *pV1) * s; return pOut; }
inline D3DXVECTOR4* D3DXVec4Cross(D3DXVECTOR4* pOut, const D3DXVECTOR4* pV1, const D3DXVECTOR4* pV2, const D3DXVECTOR4* pV3)
{
    D3DXVECTOR4 v;
    v.x = pV1->y * (pV2->z * pV3->w - pV3->z * pV2->w) - pV1->z * (pV2->y * pV3->w - pV3->y * pV2->w) + pV1->w * (pV2->y * pV3->z - pV2->z * pV3->y);
    v.y = -(pV1->x * (pV2->z * pV3->w - pV3->z * pV2->w) - pV1->z * (pV2->x * pV3->w - pV3->x * pV2->w) + pV1->w * (pV2->x * pV3->z - pV3->x * pV2->z));
    v.z = pV1->x * (pV2->y * pV3->w - pV3->y * pV2->w) - pV1->y * (pV2->x * pV3->w - pV3->x * pV2->w) + pV1->w * (pV2->x * pV3->y - pV3->x * pV2->y);
    v.w = -(pV1->x * (pV2->y * pV3->z - pV3->y * pV2->z) - pV1->y * (pV2->x * pV3->z - pV3->x * pV2->z) + pV1->z * (pV2->x * pV3->y - pV3->x * pV2->y));
    *pOut = v;
    return pOut;
}
inline D3DXVECTOR4* D3DXVec4Normalize(D3DXVECTOR4* pOut, const D3DXVECTOR4* pV)
{
    float len = D3DXVec4Length(pV);
    if (len > 1e-8f) *pOut = (*pV) / len;
    else *pOut = D3DXVECTOR4(0, 0, 0, 0);
    return pOut;
}
inline D3DXVECTOR4* D3DXVec4Hermite(D3DXVECTOR4* pOut, const D3DXVECTOR4* pV1, const D3DXVECTOR4* pT1, const D3DXVECTOR4* pV2, const D3DXVECTOR4* pT2, float s)
{
    float s2 = s * s, s3 = s2 * s;
    float h1 = 2.0f * s3 - 3.0f * s2 + 1.0f;
    float h2 = -2.0f * s3 + 3.0f * s2;
    float h3 = s3 - 2.0f * s2 + s;
    float h4 = s3 - s2;
    *pOut = (*pV1) * h1 + (*pV2) * h2 + (*pT1) * h3 + (*pT2) * h4;
    return pOut;
}
inline D3DXVECTOR4* D3DXVec4CatmullRom(D3DXVECTOR4* pOut, const D3DXVECTOR4* pV0, const D3DXVECTOR4* pV1, const D3DXVECTOR4* pV2, const D3DXVECTOR4* pV3, float s)
{
    float s2 = s * s, s3 = s2 * s;
    *pOut = 0.5f * ((2.0f * (*pV1)) + (-(*pV0) + (*pV2)) * s + (2.0f * (*pV0) - 5.0f * (*pV1) + 4.0f * (*pV2) - (*pV3)) * s2 + (-(*pV0) + 3.0f * (*pV1) - 3.0f * (*pV2) + (*pV3)) * s3);
    return pOut;
}
inline D3DXVECTOR4* D3DXVec4BaryCentric(D3DXVECTOR4* pOut, const D3DXVECTOR4* pV1, const D3DXVECTOR4* pV2, const D3DXVECTOR4* pV3, float f, float g)
{
    *pOut = *pV1 + f * (*pV2 - *pV1) + g * (*pV3 - *pV1);
    return pOut;
}
inline D3DXVECTOR4* D3DXVec4Transform(D3DXVECTOR4* pOut, const D3DXVECTOR4* pV, const D3DXMATRIX* pM)
{
    D3DXVECTOR4 v(
        pV->x * pM->_11 + pV->y * pM->_21 + pV->z * pM->_31 + pV->w * pM->_41,
        pV->x * pM->_12 + pV->y * pM->_22 + pV->z * pM->_32 + pV->w * pM->_42,
        pV->x * pM->_13 + pV->y * pM->_23 + pV->z * pM->_33 + pV->w * pM->_43,
        pV->x * pM->_14 + pV->y * pM->_24 + pV->z * pM->_34 + pV->w * pM->_44);
    *pOut = v;
    return pOut;
}
inline D3DXVECTOR4* D3DXVec4TransformArray(D3DXVECTOR4* pOut, unsigned int OutStride, const D3DXVECTOR4* pV, unsigned int VStride, const D3DXMATRIX* pM, unsigned int n)
{
    for (unsigned int i = 0; i < n; ++i)
        D3DXVec4Transform(reinterpret_cast<D3DXVECTOR4*>(reinterpret_cast<uint8_t*>(pOut) + i * OutStride),
                          reinterpret_cast<const D3DXVECTOR4*>(reinterpret_cast<const uint8_t*>(pV) + i * VStride), pM);
    return pOut;
}

// =============================================================================
// 5. D3DXMatrix & D3DXQuaternion Functions
// =============================================================================

inline D3DXMATRIX* D3DXMatrixIdentity(D3DXMATRIX* pOut)
{
    std::memset(pOut->m, 0, sizeof(pOut->m));
    pOut->_11 = pOut->_22 = pOut->_33 = pOut->_44 = 1.0f;
    return pOut;
}
inline int D3DXMatrixIsIdentity(const D3DXMATRIX* pM)
{
    return pM->_11 == 1.0f && pM->_12 == 0.0f && pM->_13 == 0.0f && pM->_14 == 0.0f &&
           pM->_21 == 0.0f && pM->_22 == 1.0f && pM->_23 == 0.0f && pM->_24 == 0.0f &&
           pM->_31 == 0.0f && pM->_32 == 0.0f && pM->_33 == 1.0f && pM->_34 == 0.0f &&
           pM->_41 == 0.0f && pM->_42 == 0.0f && pM->_43 == 0.0f && pM->_44 == 1.0f;
}
inline D3DXMATRIX* D3DXMatrixTranspose(D3DXMATRIX* pOut, const D3DXMATRIX* pM)
{
    D3DXMATRIX r;
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            r.m[i][j] = pM->m[j][i];
    *pOut = r;
    return pOut;
}
inline D3DXMATRIX* D3DXMatrixMultiply(D3DXMATRIX* pOut, const D3DXMATRIX* pM1, const D3DXMATRIX* pM2)
{
    D3DXMATRIX r;
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            r.m[i][j] = pM1->m[i][0] * pM2->m[0][j] + pM1->m[i][1] * pM2->m[1][j] +
                        pM1->m[i][2] * pM2->m[2][j] + pM1->m[i][3] * pM2->m[3][j];
    *pOut = r;
    return pOut;
}
inline D3DXMATRIX& D3DXMATRIX::operator*=(const D3DXMATRIX& rhs) { D3DXMatrixMultiply(this, this, &rhs); return *this; }
inline D3DXMATRIX D3DXMATRIX::operator*(const D3DXMATRIX& rhs) const { D3DXMATRIX r; D3DXMatrixMultiply(&r, this, &rhs); return r; }

inline D3DXMATRIX* D3DXMatrixMultiplyTranspose(D3DXMATRIX* pOut, const D3DXMATRIX* pM1, const D3DXMATRIX* pM2)
{
    D3DXMatrixMultiply(pOut, pM1, pM2);
    return D3DXMatrixTranspose(pOut, pOut);
}
inline float D3DXMatrixDeterminant(const D3DXMATRIX* pM)
{
    const float* m = &pM->_11;
    float a0 = m[0] * m[5] - m[1] * m[4];
    float a1 = m[0] * m[6] - m[2] * m[4];
    float a2 = m[0] * m[7] - m[3] * m[4];
    float a3 = m[1] * m[6] - m[2] * m[5];
    float a4 = m[1] * m[7] - m[3] * m[5];
    float a5 = m[2] * m[7] - m[3] * m[6];
    float b0 = m[8] * m[13] - m[9] * m[12];
    float b1 = m[8] * m[14] - m[10] * m[12];
    float b2 = m[8] * m[15] - m[11] * m[12];
    float b3 = m[9] * m[14] - m[10] * m[13];
    float b4 = m[9] * m[15] - m[11] * m[13];
    float b5 = m[10] * m[15] - m[11] * m[14];
    return a0 * b5 - a1 * b4 + a2 * b3 + a3 * b2 - a4 * b1 + a5 * b0;
}
inline D3DXMATRIX* D3DXMatrixInverse(D3DXMATRIX* pOut, float* pDeterminant, const D3DXMATRIX* pM)
{
    const float* m = &pM->_11;
    float a0 = m[0] * m[5] - m[1] * m[4];
    float a1 = m[0] * m[6] - m[2] * m[4];
    float a2 = m[0] * m[7] - m[3] * m[4];
    float a3 = m[1] * m[6] - m[2] * m[5];
    float a4 = m[1] * m[7] - m[3] * m[5];
    float a5 = m[2] * m[7] - m[3] * m[6];
    float b0 = m[8] * m[13] - m[9] * m[12];
    float b1 = m[8] * m[14] - m[10] * m[12];
    float b2 = m[8] * m[15] - m[11] * m[12];
    float b3 = m[9] * m[14] - m[10] * m[13];
    float b4 = m[9] * m[15] - m[11] * m[13];
    float b5 = m[10] * m[15] - m[11] * m[14];
    float det = a0 * b5 - a1 * b4 + a2 * b3 + a3 * b2 - a4 * b1 + a5 * b0;
    if (pDeterminant) *pDeterminant = det;
    if (std::fabs(det) <= 1e-12f) return nullptr;
    float invDet = 1.0f / det;
    D3DXMATRIX inv;
    inv._11 = ( m[ 5] * b5 - m[ 6] * b4 + m[ 7] * b3) * invDet;
    inv._12 = (-m[ 1] * b5 + m[ 2] * b4 - m[ 3] * b3) * invDet;
    inv._13 = ( m[13] * a5 - m[14] * a4 + m[15] * a3) * invDet;
    inv._14 = (-m[ 9] * a5 + m[10] * a4 - m[11] * a3) * invDet;
    inv._21 = (-m[ 4] * b5 + m[ 6] * b2 - m[ 7] * b1) * invDet;
    inv._22 = ( m[ 0] * b5 - m[ 2] * b2 + m[ 3] * b1) * invDet;
    inv._23 = (-m[12] * a5 + m[14] * a2 - m[15] * a1) * invDet;
    inv._24 = ( m[ 8] * a5 - m[10] * a2 + m[11] * a1) * invDet;
    inv._31 = ( m[ 4] * b4 - m[ 5] * b2 + m[ 7] * b0) * invDet;
    inv._32 = (-m[ 0] * b4 + m[ 1] * b2 - m[ 3] * b0) * invDet;
    inv._33 = ( m[12] * a4 - m[13] * a2 + m[15] * a0) * invDet;
    inv._34 = (-m[ 8] * a4 + m[ 9] * a2 - m[11] * a0) * invDet;
    inv._41 = (-m[ 4] * b3 + m[ 5] * b1 - m[ 6] * b0) * invDet;
    inv._42 = ( m[ 0] * b3 - m[ 1] * b1 + m[ 2] * b0) * invDet;
    inv._43 = (-m[12] * a3 + m[13] * a1 - m[14] * a0) * invDet;
    inv._44 = ( m[ 8] * a3 - m[ 9] * a1 + m[10] * a0) * invDet;
    *pOut = inv;
    return pOut;
}
inline D3DXMATRIX* D3DXMatrixScaling(D3DXMATRIX* pOut, float sx, float sy, float sz)
{
    D3DXMatrixIdentity(pOut);
    pOut->_11 = sx; pOut->_22 = sy; pOut->_33 = sz;
    return pOut;
}
inline D3DXMATRIX* D3DXMatrixTranslation(D3DXMATRIX* pOut, float x, float y, float z)
{
    D3DXMatrixIdentity(pOut);
    pOut->_41 = x; pOut->_42 = y; pOut->_43 = z;
    return pOut;
}
inline D3DXMATRIX* D3DXMatrixRotationX(D3DXMATRIX* pOut, float angle)
{
    D3DXMatrixIdentity(pOut);
    float s = std::sin(angle), c = std::cos(angle);
    pOut->_22 = c;  pOut->_23 = s;
    pOut->_32 = -s; pOut->_33 = c;
    return pOut;
}
inline D3DXMATRIX* D3DXMatrixRotationY(D3DXMATRIX* pOut, float angle)
{
    D3DXMatrixIdentity(pOut);
    float s = std::sin(angle), c = std::cos(angle);
    pOut->_11 = c; pOut->_13 = -s;
    pOut->_31 = s; pOut->_33 = c;
    return pOut;
}
inline D3DXMATRIX* D3DXMatrixRotationZ(D3DXMATRIX* pOut, float angle)
{
    D3DXMatrixIdentity(pOut);
    float s = std::sin(angle), c = std::cos(angle);
    pOut->_11 = c;  pOut->_12 = s;
    pOut->_21 = -s; pOut->_22 = c;
    return pOut;
}
inline D3DXMATRIX* D3DXMatrixRotationAxis(D3DXMATRIX* pOut, const D3DXVECTOR3* pV, float angle)
{
    D3DXVECTOR3 v;
    D3DXVec3Normalize(&v, pV);
    float s = std::sin(angle), c = std::cos(angle), t = 1.0f - c;
    D3DXMatrixIdentity(pOut);
    pOut->_11 = t * v.x * v.x + c;       pOut->_12 = t * v.x * v.y + s * v.z; pOut->_13 = t * v.x * v.z - s * v.y;
    pOut->_21 = t * v.x * v.y - s * v.z; pOut->_22 = t * v.y * v.y + c;       pOut->_23 = t * v.y * v.z + s * v.x;
    pOut->_31 = t * v.x * v.z + s * v.y; pOut->_32 = t * v.y * v.z - s * v.x; pOut->_33 = t * v.z * v.z + c;
    return pOut;
}
inline D3DXMATRIX* D3DXMatrixRotationQuaternion(D3DXMATRIX* pOut, const D3DXQUATERNION* pQ)
{
    D3DXMatrixIdentity(pOut);
    float xx = pQ->x * pQ->x, yy = pQ->y * pQ->y, zz = pQ->z * pQ->z;
    float xy = pQ->x * pQ->y, xz = pQ->x * pQ->z, yz = pQ->y * pQ->z;
    float wx = pQ->w * pQ->x, wy = pQ->w * pQ->y, wz = pQ->w * pQ->z;
    pOut->_11 = 1.0f - 2.0f * (yy + zz); pOut->_12 = 2.0f * (xy + wz);        pOut->_13 = 2.0f * (xz - wy);
    pOut->_21 = 2.0f * (xy - wz);        pOut->_22 = 1.0f - 2.0f * (xx + zz); pOut->_23 = 2.0f * (yz + wx);
    pOut->_31 = 2.0f * (xz + wy);        pOut->_32 = 2.0f * (yz - wx);        pOut->_33 = 1.0f - 2.0f * (xx + yy);
    return pOut;
}
inline D3DXQUATERNION* D3DXQuaternionRotationYawPitchRoll(D3DXQUATERNION* pOut, float Yaw, float Pitch, float Roll)
{
    float sy = std::sin(Yaw * 0.5f),   cy = std::cos(Yaw * 0.5f);
    float sp = std::sin(Pitch * 0.5f), cp = std::cos(Pitch * 0.5f);
    float sr = std::sin(Roll * 0.5f),  cr = std::cos(Roll * 0.5f);
    pOut->x = cy * sp * cr + sy * cp * sr;
    pOut->y = sy * cp * cr - cy * sp * sr;
    pOut->z = cy * cp * sr - sy * sp * cr;
    pOut->w = cy * cp * cr + sy * sp * sr;
    return pOut;
}
inline D3DXMATRIX* D3DXMatrixRotationYawPitchRoll(D3DXMATRIX* pOut, float Yaw, float Pitch, float Roll)
{
    D3DXQUATERNION q;
    D3DXQuaternionRotationYawPitchRoll(&q, Yaw, Pitch, Roll);
    return D3DXMatrixRotationQuaternion(pOut, &q);
}
inline D3DXQUATERNION* D3DXQuaternionRotationMatrix(D3DXQUATERNION* pOut, const D3DXMATRIX* pM)
{
    float trace = pM->_11 + pM->_22 + pM->_33;
    if (trace > 0.0f)
    {
        float s = std::sqrt(trace + 1.0f) * 2.0f;
        pOut->w = 0.25f * s;
        pOut->x = (pM->_23 - pM->_32) / s;
        pOut->y = (pM->_31 - pM->_13) / s;
        pOut->z = (pM->_12 - pM->_21) / s;
    }
    else if (pM->_11 > pM->_22 && pM->_11 > pM->_33)
    {
        float s = std::sqrt(1.0f + pM->_11 - pM->_22 - pM->_33) * 2.0f;
        pOut->w = (pM->_23 - pM->_32) / s;
        pOut->x = 0.25f * s;
        pOut->y = (pM->_12 + pM->_21) / s;
        pOut->z = (pM->_13 + pM->_31) / s;
    }
    else if (pM->_22 > pM->_33)
    {
        float s = std::sqrt(1.0f + pM->_22 - pM->_11 - pM->_33) * 2.0f;
        pOut->w = (pM->_31 - pM->_13) / s;
        pOut->x = (pM->_12 + pM->_21) / s;
        pOut->y = 0.25f * s;
        pOut->z = (pM->_23 + pM->_32) / s;
    }
    else
    {
        float s = std::sqrt(1.0f + pM->_33 - pM->_11 - pM->_22) * 2.0f;
        pOut->w = (pM->_12 - pM->_21) / s;
        pOut->x = (pM->_13 + pM->_31) / s;
        pOut->y = (pM->_23 + pM->_32) / s;
        pOut->z = 0.25f * s;
    }
    return pOut;
}
inline int D3DXMatrixDecompose(D3DXVECTOR3* pOutScale, D3DXQUATERNION* pOutRotation, D3DXVECTOR3* pOutTranslation, const D3DXMATRIX* pM)
{
    if (pOutTranslation) *pOutTranslation = D3DXVECTOR3(pM->_41, pM->_42, pM->_43);
    D3DXVECTOR3 r0(pM->_11, pM->_12, pM->_13);
    D3DXVECTOR3 r1(pM->_21, pM->_22, pM->_23);
    D3DXVECTOR3 r2(pM->_31, pM->_32, pM->_33);
    float sx = D3DXVec3Length(&r0), sy = D3DXVec3Length(&r1), sz = D3DXVec3Length(&r2);
    if (pOutScale) *pOutScale = D3DXVECTOR3(sx, sy, sz);
    if (pOutRotation)
    {
        D3DXMATRIX rot = *pM;
        rot._41 = rot._42 = rot._43 = 0.0f; rot._44 = 1.0f;
        if (sx > 1e-8f) { rot._11 /= sx; rot._12 /= sx; rot._13 /= sx; }
        if (sy > 1e-8f) { rot._21 /= sy; rot._22 /= sy; rot._23 /= sy; }
        if (sz > 1e-8f) { rot._31 /= sz; rot._32 /= sz; rot._33 /= sz; }
        D3DXQuaternionRotationMatrix(pOutRotation, &rot);
    }
    return 0;
}
inline D3DXMATRIX* D3DXMatrixTransformation(D3DXMATRIX* pOut, const D3DXVECTOR3* pScalingCenter, const D3DXQUATERNION* pScalingRotation, const D3DXVECTOR3* pScaling, const D3DXVECTOR3* pRotationCenter, const D3DXQUATERNION* pRotation, const D3DXVECTOR3* pTranslation)
{
    D3DXMATRIX m;
    D3DXMatrixIdentity(&m);
    if (pScaling)
    {
        D3DXMATRIX ms;
        D3DXMatrixScaling(&ms, pScaling->x, pScaling->y, pScaling->z);
        m = m * ms;
    }
    if (pRotation)
    {
        D3DXMATRIX mr;
        D3DXMatrixRotationQuaternion(&mr, pRotation);
        if (pRotationCenter)
        {
            D3DXMATRIX mt1, mt2;
            D3DXMatrixTranslation(&mt1, -pRotationCenter->x, -pRotationCenter->y, -pRotationCenter->z);
            D3DXMatrixTranslation(&mt2, pRotationCenter->x, pRotationCenter->y, pRotationCenter->z);
            m = m * mt1 * mr * mt2;
        }
        else m = m * mr;
    }
    if (pTranslation)
    {
        m._41 += pTranslation->x; m._42 += pTranslation->y; m._43 += pTranslation->z;
    }
    *pOut = m;
    return pOut;
}
inline D3DXMATRIX* D3DXMatrixTransformation2D(D3DXMATRIX* pOut, const D3DXVECTOR2* pScalingCenter, float ScalingRotation, const D3DXVECTOR2* pScaling, const D3DXVECTOR2* pRotationCenter, float Rotation, const D3DXVECTOR2* pTranslation)
{
    D3DXVECTOR3 sc(pScalingCenter ? pScalingCenter->x : 0.f, pScalingCenter ? pScalingCenter->y : 0.f, 0.f);
    D3DXVECTOR3 s(pScaling ? pScaling->x : 1.f, pScaling ? pScaling->y : 1.f, 1.f);
    D3DXVECTOR3 rc(pRotationCenter ? pRotationCenter->x : 0.f, pRotationCenter ? pRotationCenter->y : 0.f, 0.f);
    D3DXVECTOR3 t(pTranslation ? pTranslation->x : 0.f, pTranslation ? pTranslation->y : 0.f, 0.f);
    D3DXQUATERNION qr(0.f, 0.f, std::sin(Rotation * 0.5f), std::cos(Rotation * 0.5f));
    return D3DXMatrixTransformation(pOut, &sc, nullptr, &s, &rc, &qr, &t);
}
inline D3DXMATRIX* D3DXMatrixAffineTransformation(D3DXMATRIX* pOut, float Scaling, const D3DXVECTOR3* pRotationCenter, const D3DXQUATERNION* pRotation, const D3DXVECTOR3* pTranslation)
{
    D3DXVECTOR3 s(Scaling, Scaling, Scaling);
    return D3DXMatrixTransformation(pOut, nullptr, nullptr, &s, pRotationCenter, pRotation, pTranslation);
}
inline D3DXMATRIX* D3DXMatrixAffineTransformation2D(D3DXMATRIX* pOut, float Scaling, const D3DXVECTOR2* pRotationCenter, float Rotation, const D3DXVECTOR2* pTranslation)
{
    D3DXVECTOR2 s(Scaling, Scaling);
    return D3DXMatrixTransformation2D(pOut, nullptr, 0.0f, &s, pRotationCenter, Rotation, pTranslation);
}
inline D3DXMATRIX* D3DXMatrixLookAtLH(D3DXMATRIX* pOut, const D3DXVECTOR3* pEye, const D3DXVECTOR3* pAt, const D3DXVECTOR3* pUp)
{
    D3DXVECTOR3 zaxis, xaxis, yaxis, diff = *pAt - *pEye;
    D3DXVec3Normalize(&zaxis, &diff);
    D3DXVec3Cross(&xaxis, pUp, &zaxis);
    D3DXVec3Normalize(&xaxis, &xaxis);
    D3DXVec3Cross(&yaxis, &zaxis, &xaxis);
    *pOut = D3DXMATRIX(
        xaxis.x, yaxis.x, zaxis.x, 0.0f,
        xaxis.y, yaxis.y, zaxis.y, 0.0f,
        xaxis.z, yaxis.z, zaxis.z, 0.0f,
        -D3DXVec3Dot(&xaxis, pEye), -D3DXVec3Dot(&yaxis, pEye), -D3DXVec3Dot(&zaxis, pEye), 1.0f);
    return pOut;
}
inline D3DXMATRIX* D3DXMatrixLookAtRH(D3DXMATRIX* pOut, const D3DXVECTOR3* pEye, const D3DXVECTOR3* pAt, const D3DXVECTOR3* pUp)
{
    D3DXVECTOR3 zaxis, xaxis, yaxis, diff = *pEye - *pAt;
    D3DXVec3Normalize(&zaxis, &diff);
    D3DXVec3Cross(&xaxis, pUp, &zaxis);
    D3DXVec3Normalize(&xaxis, &xaxis);
    D3DXVec3Cross(&yaxis, &zaxis, &xaxis);
    *pOut = D3DXMATRIX(
        xaxis.x, yaxis.x, zaxis.x, 0.0f,
        xaxis.y, yaxis.y, zaxis.y, 0.0f,
        xaxis.z, yaxis.z, zaxis.z, 0.0f,
        -D3DXVec3Dot(&xaxis, pEye), -D3DXVec3Dot(&yaxis, pEye), -D3DXVec3Dot(&zaxis, pEye), 1.0f);
    return pOut;
}
inline D3DXMATRIX* D3DXMatrixPerspectiveLH(D3DXMATRIX* pOut, float w, float h, float zn, float zf)
{
    std::memset(pOut->m, 0, sizeof(pOut->m));
    pOut->_11 = 2.0f * zn / w;
    pOut->_22 = 2.0f * zn / h;
    pOut->_33 = zf / (zf - zn);
    pOut->_34 = 1.0f;
    pOut->_43 = zn * zf / (zn - zf);
    return pOut;
}
inline D3DXMATRIX* D3DXMatrixPerspectiveRH(D3DXMATRIX* pOut, float w, float h, float zn, float zf)
{
    std::memset(pOut->m, 0, sizeof(pOut->m));
    pOut->_11 = 2.0f * zn / w;
    pOut->_22 = 2.0f * zn / h;
    pOut->_33 = zf / (zn - zf);
    pOut->_34 = -1.0f;
    pOut->_43 = zn * zf / (zn - zf);
    return pOut;
}
inline D3DXMATRIX* D3DXMatrixPerspectiveFovLH(D3DXMATRIX* pOut, float fovy, float Aspect, float zn, float zf)
{
    float yScale = 1.0f / std::tan(fovy * 0.5f);
    float xScale = yScale / Aspect;
    std::memset(pOut->m, 0, sizeof(pOut->m));
    pOut->_11 = xScale;
    pOut->_22 = yScale;
    pOut->_33 = zf / (zf - zn);
    pOut->_34 = 1.0f;
    pOut->_43 = -zn * zf / (zf - zn);
    return pOut;
}
inline D3DXMATRIX* D3DXMatrixPerspectiveFovRH(D3DXMATRIX* pOut, float fovy, float Aspect, float zn, float zf)
{
    float yScale = 1.0f / std::tan(fovy * 0.5f);
    float xScale = yScale / Aspect;
    std::memset(pOut->m, 0, sizeof(pOut->m));
    pOut->_11 = xScale;
    pOut->_22 = yScale;
    pOut->_33 = zf / (zn - zf);
    pOut->_34 = -1.0f;
    pOut->_43 = zn * zf / (zn - zf);
    return pOut;
}
inline D3DXMATRIX* D3DXMatrixPerspectiveOffCenterLH(D3DXMATRIX* pOut, float l, float r, float b, float t, float zn, float zf)
{
    std::memset(pOut->m, 0, sizeof(pOut->m));
    pOut->_11 = 2.0f * zn / (r - l);
    pOut->_22 = 2.0f * zn / (t - b);
    pOut->_31 = (l + r) / (l - r);
    pOut->_32 = (t + b) / (b - t);
    pOut->_33 = zf / (zf - zn);
    pOut->_34 = 1.0f;
    pOut->_43 = zn * zf / (zn - zf);
    return pOut;
}
inline D3DXMATRIX* D3DXMatrixPerspectiveOffCenterRH(D3DXMATRIX* pOut, float l, float r, float b, float t, float zn, float zf)
{
    std::memset(pOut->m, 0, sizeof(pOut->m));
    pOut->_11 = 2.0f * zn / (r - l);
    pOut->_22 = 2.0f * zn / (t - b);
    pOut->_31 = (l + r) / (r - l);
    pOut->_32 = (t + b) / (t - b);
    pOut->_33 = zf / (zn - zf);
    pOut->_34 = -1.0f;
    pOut->_43 = zn * zf / (zn - zf);
    return pOut;
}
inline D3DXMATRIX* D3DXMatrixOrthoLH(D3DXMATRIX* pOut, float w, float h, float zn, float zf)
{
    D3DXMatrixIdentity(pOut);
    pOut->_11 = 2.0f / w;
    pOut->_22 = 2.0f / h;
    pOut->_33 = 1.0f / (zf - zn);
    pOut->_43 = -zn / (zf - zn);
    return pOut;
}
inline D3DXMATRIX* D3DXMatrixOrthoRH(D3DXMATRIX* pOut, float w, float h, float zn, float zf)
{
    D3DXMatrixIdentity(pOut);
    pOut->_11 = 2.0f / w;
    pOut->_22 = 2.0f / h;
    pOut->_33 = 1.0f / (zn - zf);
    pOut->_43 = zn / (zn - zf);
    return pOut;
}
inline D3DXMATRIX* D3DXMatrixOrthoOffCenterLH(D3DXMATRIX* pOut, float l, float r, float b, float t, float zn, float zf)
{
    D3DXMatrixIdentity(pOut);
    pOut->_11 = 2.0f / (r - l);
    pOut->_22 = 2.0f / (t - b);
    pOut->_33 = 1.0f / (zf - zn);
    pOut->_41 = (l + r) / (l - r);
    pOut->_42 = (t + b) / (b - t);
    pOut->_43 = zn / (zn - zf);
    return pOut;
}
inline D3DXMATRIX* D3DXMatrixOrthoOffCenterRH(D3DXMATRIX* pOut, float l, float r, float b, float t, float zn, float zf)
{
    D3DXMatrixIdentity(pOut);
    pOut->_11 = 2.0f / (r - l);
    pOut->_22 = 2.0f / (t - b);
    pOut->_33 = 1.0f / (zn - zf);
    pOut->_41 = (l + r) / (l - r);
    pOut->_42 = (t + b) / (b - t);
    pOut->_43 = zn / (zn - zf);
    return pOut;
}

inline D3DXPLANE* D3DXPlaneNormalize(D3DXPLANE* pOut, const D3DXPLANE* pP);

inline D3DXMATRIX* D3DXMatrixShadow(D3DXMATRIX* pOut, const D3DXVECTOR4* pLight, const D3DXPLANE* pPlane)
{
    D3DXPLANE P;
    D3DXPlaneNormalize(&P, pPlane);
    float dot = P.a * pLight->x + P.b * pLight->y + P.c * pLight->z + P.d * pLight->w;
    *pOut = D3DXMATRIX(
        dot - P.a * pLight->x, -P.a * pLight->y,      -P.a * pLight->z,      -P.a * pLight->w,
        -P.b * pLight->x,      dot - P.b * pLight->y, -P.b * pLight->z,      -P.b * pLight->w,
        -P.c * pLight->x,      -P.c * pLight->y,      dot - P.c * pLight->z, -P.c * pLight->w,
        -P.d * pLight->x,      -P.d * pLight->y,      -P.d * pLight->z,      dot - P.d * pLight->w);
    return pOut;
}
inline D3DXMATRIX* D3DXMatrixReflect(D3DXMATRIX* pOut, const D3DXPLANE* pPlane)
{
    D3DXPLANE P;
    D3DXPlaneNormalize(&P, pPlane);
    *pOut = D3DXMATRIX(
        1.0f - 2.0f * P.a * P.a, -2.0f * P.a * P.b,       -2.0f * P.a * P.c,       0.0f,
        -2.0f * P.b * P.a,       1.0f - 2.0f * P.b * P.b, -2.0f * P.b * P.c,       0.0f,
        -2.0f * P.c * P.a,       -2.0f * P.c * P.b,       1.0f - 2.0f * P.c * P.c, 0.0f,
        -2.0f * P.d * P.a,       -2.0f * P.d * P.b,       -2.0f * P.d * P.c,       1.0f);
    return pOut;
}

inline D3DXVECTOR3* D3DXVec3Project(D3DXVECTOR3* pOut, const D3DXVECTOR3* pV, const D3DVIEWPORT9* pViewport, const D3DXMATRIX* pProjection, const D3DXMATRIX* pView, const D3DXMATRIX* pWorld)
{
    D3DXMATRIX m = (*pWorld) * (*pView) * (*pProjection);
    D3DXVECTOR3 v;
    D3DXVec3TransformCoord(&v, pV, &m);
    pOut->x = pViewport->X + (1.0f + v.x) * pViewport->Width * 0.5f;
    pOut->y = pViewport->Y + (1.0f - v.y) * pViewport->Height * 0.5f;
    pOut->z = pViewport->MinZ + v.z * (pViewport->MaxZ - pViewport->MinZ);
    return pOut;
}
inline D3DXVECTOR3* D3DXVec3Unproject(D3DXVECTOR3* pOut, const D3DXVECTOR3* pV, const D3DVIEWPORT9* pViewport, const D3DXMATRIX* pProjection, const D3DXMATRIX* pView, const D3DXMATRIX* pWorld)
{
    D3DXMATRIX m = (*pWorld) * (*pView) * (*pProjection);
    D3DXMatrixInverse(&m, nullptr, &m);
    D3DXVECTOR3 v(
        2.0f * (pV->x - pViewport->X) / pViewport->Width - 1.0f,
        1.0f - 2.0f * (pV->y - pViewport->Y) / pViewport->Height,
        (pV->z - pViewport->MinZ) / (pViewport->MaxZ - pViewport->MinZ));
    return D3DXVec3TransformCoord(pOut, &v, &m);
}
inline D3DXVECTOR3* D3DXVec3ProjectArray(D3DXVECTOR3* pOut, unsigned int OutStride, const D3DXVECTOR3* pV, unsigned int VStride, const D3DVIEWPORT9* pViewport, const D3DXMATRIX* pProjection, const D3DXMATRIX* pView, const D3DXMATRIX* pWorld, unsigned int n)
{
    for (unsigned int i = 0; i < n; ++i)
        D3DXVec3Project(reinterpret_cast<D3DXVECTOR3*>(reinterpret_cast<uint8_t*>(pOut) + i * OutStride),
                        reinterpret_cast<const D3DXVECTOR3*>(reinterpret_cast<const uint8_t*>(pV) + i * VStride), pViewport, pProjection, pView, pWorld);
    return pOut;
}
inline D3DXVECTOR3* D3DXVec3UnprojectArray(D3DXVECTOR3* pOut, unsigned int OutStride, const D3DXVECTOR3* pV, unsigned int VStride, const D3DVIEWPORT9* pViewport, const D3DXMATRIX* pProjection, const D3DXMATRIX* pView, const D3DXMATRIX* pWorld, unsigned int n)
{
    for (unsigned int i = 0; i < n; ++i)
        D3DXVec3Unproject(reinterpret_cast<D3DXVECTOR3*>(reinterpret_cast<uint8_t*>(pOut) + i * OutStride),
                          reinterpret_cast<const D3DXVECTOR3*>(reinterpret_cast<const uint8_t*>(pV) + i * VStride), pViewport, pProjection, pView, pWorld);
    return pOut;
}

// =============================================================================
// 6. D3DXQuaternion Functions
// =============================================================================

inline float D3DXQuaternionLength(const D3DXQUATERNION* pQ) { return std::sqrt(pQ->x * pQ->x + pQ->y * pQ->y + pQ->z * pQ->z + pQ->w * pQ->w); }
inline float D3DXQuaternionLengthSq(const D3DXQUATERNION* pQ) { return pQ->x * pQ->x + pQ->y * pQ->y + pQ->z * pQ->z + pQ->w * pQ->w; }
inline float D3DXQuaternionDot(const D3DXQUATERNION* pQ1, const D3DXQUATERNION* pQ2) { return pQ1->x * pQ2->x + pQ1->y * pQ2->y + pQ1->z * pQ2->z + pQ1->w * pQ2->w; }
inline D3DXQUATERNION* D3DXQuaternionIdentity(D3DXQUATERNION* pOut) { pOut->x = pOut->y = pOut->z = 0.0f; pOut->w = 1.0f; return pOut; }
inline int D3DXQuaternionIsIdentity(const D3DXQUATERNION* pQ) { return pQ->x == 0.0f && pQ->y == 0.0f && pQ->z == 0.0f && pQ->w == 1.0f; }
inline D3DXQUATERNION* D3DXQuaternionConjugate(D3DXQUATERNION* pOut, const D3DXQUATERNION* pQ) { pOut->x = -pQ->x; pOut->y = -pQ->y; pOut->z = -pQ->z; pOut->w = pQ->w; return pOut; }
inline void D3DXQuaternionToAxisAngle(const D3DXQUATERNION* pQ, D3DXVECTOR3* pAxis, float* pAngle)
{
    if (pAxis) *pAxis = D3DXVECTOR3(pQ->x, pQ->y, pQ->z);
    if (pAngle) *pAngle = 2.0f * std::acos((std::max)(-1.0f, (std::min)(1.0f, pQ->w)));
}
inline D3DXQUATERNION* D3DXQuaternionRotationAxis(D3DXQUATERNION* pOut, const D3DXVECTOR3* pV, float Angle)
{
    D3DXVECTOR3 v;
    D3DXVec3Normalize(&v, pV);
    float s = std::sin(Angle * 0.5f);
    pOut->x = v.x * s; pOut->y = v.y * s; pOut->z = v.z * s; pOut->w = std::cos(Angle * 0.5f);
    return pOut;
}
inline D3DXQUATERNION* D3DXQuaternionMultiply(D3DXQUATERNION* pOut, const D3DXQUATERNION* pQ1, const D3DXQUATERNION* pQ2)
{
    D3DXQUATERNION r(
        pQ2->w * pQ1->x + pQ2->x * pQ1->w + pQ2->y * pQ1->z - pQ2->z * pQ1->y,
        pQ2->w * pQ1->y - pQ2->x * pQ1->z + pQ2->y * pQ1->w + pQ2->z * pQ1->x,
        pQ2->w * pQ1->z + pQ2->x * pQ1->y - pQ2->y * pQ1->x + pQ2->z * pQ1->w,
        pQ2->w * pQ1->w - pQ2->x * pQ1->x - pQ2->y * pQ1->y - pQ2->z * pQ1->z);
    *pOut = r;
    return pOut;
}
inline D3DXQUATERNION& D3DXQUATERNION::operator*=(const D3DXQUATERNION& q) { D3DXQuaternionMultiply(this, this, &q); return *this; }
inline D3DXQUATERNION D3DXQUATERNION::operator*(const D3DXQUATERNION& q) const { D3DXQUATERNION r; D3DXQuaternionMultiply(&r, this, &q); return r; }

inline D3DXQUATERNION* D3DXQuaternionNormalize(D3DXQUATERNION* pOut, const D3DXQUATERNION* pQ)
{
    float len = D3DXQuaternionLength(pQ);
    if (len > 1e-8f) *pOut = (*pQ) / len;
    else D3DXQuaternionIdentity(pOut);
    return pOut;
}
inline D3DXQUATERNION* D3DXQuaternionInverse(D3DXQUATERNION* pOut, const D3DXQUATERNION* pQ)
{
    float lenSq = D3DXQuaternionLengthSq(pQ);
    if (lenSq > 1e-8f) { D3DXQuaternionConjugate(pOut, pQ); *pOut /= lenSq; }
    else D3DXQuaternionIdentity(pOut);
    return pOut;
}
inline D3DXQUATERNION* D3DXQuaternionLn(D3DXQUATERNION* pOut, const D3DXQUATERNION* pQ)
{
    float a = std::acos((std::max)(-1.0f, (std::min)(1.0f, pQ->w)));
    float sina = std::sin(a);
    pOut->w = 0.0f;
    if (std::fabs(sina) > 1e-6f) { float k = a / sina; pOut->x = pQ->x * k; pOut->y = pQ->y * k; pOut->z = pQ->z * k; }
    else { pOut->x = pQ->x; pOut->y = pQ->y; pOut->z = pQ->z; }
    return pOut;
}
inline D3DXQUATERNION* D3DXQuaternionExp(D3DXQUATERNION* pOut, const D3DXQUATERNION* pQ)
{
    float a = std::sqrt(pQ->x * pQ->x + pQ->y * pQ->y + pQ->z * pQ->z);
    pOut->w = std::cos(a);
    if (a > 1e-6f) { float k = std::sin(a) / a; pOut->x = pQ->x * k; pOut->y = pQ->y * k; pOut->z = pQ->z * k; }
    else { pOut->x = pQ->x; pOut->y = pQ->y; pOut->z = pQ->z; }
    return pOut;
}
inline D3DXQUATERNION* D3DXQuaternionSlerp(D3DXQUATERNION* pOut, const D3DXQUATERNION* pQ1, const D3DXQUATERNION* pQ2, float t)
{
    float cosOmega = D3DXQuaternionDot(pQ1, pQ2);
    D3DXQUATERNION q2 = *pQ2;
    if (cosOmega < 0.0f) { cosOmega = -cosOmega; q2 = -q2; }
    float k0, k1;
    if (cosOmega > 0.9995f) { k0 = 1.0f - t; k1 = t; }
    else
    {
        float sinOmega = std::sqrt(1.0f - cosOmega * cosOmega);
        float omega = std::atan2(sinOmega, cosOmega);
        float oneOverSinOmega = 1.0f / sinOmega;
        k0 = std::sin((1.0f - t) * omega) * oneOverSinOmega;
        k1 = std::sin(t * omega) * oneOverSinOmega;
    }
    *pOut = (*pQ1) * k0 + q2 * k1;
    return pOut;
}
inline D3DXQUATERNION* D3DXQuaternionSquad(D3DXQUATERNION* pOut, const D3DXQUATERNION* pQ1, const D3DXQUATERNION* pA, const D3DXQUATERNION* pB, const D3DXQUATERNION* pC, float t)
{
    D3DXQUATERNION q1, q2;
    D3DXQuaternionSlerp(&q1, pQ1, pC, t);
    D3DXQuaternionSlerp(&q2, pA, pB, t);
    return D3DXQuaternionSlerp(pOut, &q1, &q2, 2.0f * t * (1.0f - t));
}
inline D3DXQUATERNION* D3DXQuaternionBaryCentric(D3DXQUATERNION* pOut, const D3DXQUATERNION* pQ1, const D3DXQUATERNION* pQ2, const D3DXQUATERNION* pQ3, float f, float g)
{
    float s = f + g;
    if (std::fabs(s) < 1e-8f) { *pOut = *pQ1; return pOut; }
    D3DXQUATERNION q1, q2;
    D3DXQuaternionSlerp(&q1, pQ1, pQ2, s);
    D3DXQuaternionSlerp(&q2, pQ1, pQ3, s);
    return D3DXQuaternionSlerp(pOut, &q1, &q2, g / s);
}

// =============================================================================
// 7. D3DXPlane & D3DXColor Functions
// =============================================================================

inline float D3DXPlaneDot(const D3DXPLANE* pP, const D3DXVECTOR4* pV) { return pP->a * pV->x + pP->b * pV->y + pP->c * pV->z + pP->d * pV->w; }
inline float D3DXPlaneDotCoord(const D3DXPLANE* pP, const D3DXVECTOR3* pV) { return pP->a * pV->x + pP->b * pV->y + pP->c * pV->z + pP->d; }
inline float D3DXPlaneDotNormal(const D3DXPLANE* pP, const D3DXVECTOR3* pV) { return pP->a * pV->x + pP->b * pV->y + pP->c * pV->z; }
inline D3DXPLANE* D3DXPlaneScale(D3DXPLANE* pOut, const D3DXPLANE* pP, float s) { *pOut = (*pP) * s; return pOut; }
inline D3DXPLANE* D3DXPlaneNormalize(D3DXPLANE* pOut, const D3DXPLANE* pP)
{
    float norm = std::sqrt(pP->a * pP->a + pP->b * pP->b + pP->c * pP->c);
    if (norm > 1e-8f) *pOut = (*pP) / norm;
    else *pOut = D3DXPLANE(0, 0, 0, 0);
    return pOut;
}
inline D3DXVECTOR3* D3DXPlaneIntersectLine(D3DXVECTOR3* pOut, const D3DXPLANE* pP, const D3DXVECTOR3* pV1, const D3DXVECTOR3* pV2)
{
    D3DXVECTOR3 dir = *pV2 - *pV1;
    float denom = D3DXPlaneDotNormal(pP, &dir);
    if (std::fabs(denom) <= 1e-8f) return nullptr;
    float t = -D3DXPlaneDotCoord(pP, pV1) / denom;
    *pOut = *pV1 + dir * t;
    return pOut;
}
inline D3DXPLANE* D3DXPlaneFromPointNormal(D3DXPLANE* pOut, const D3DXVECTOR3* pPoint, const D3DXVECTOR3* pNormal)
{
    pOut->a = pNormal->x; pOut->b = pNormal->y; pOut->c = pNormal->z;
    pOut->d = -D3DXVec3Dot(pPoint, pNormal);
    return pOut;
}
inline D3DXPLANE* D3DXPlaneFromPoints(D3DXPLANE* pOut, const D3DXVECTOR3* pV1, const D3DXVECTOR3* pV2, const D3DXVECTOR3* pV3)
{
    D3DXVECTOR3 e1 = *pV2 - *pV1, e2 = *pV3 - *pV1, n;
    D3DXVec3Cross(&n, &e1, &e2);
    D3DXVec3Normalize(&n, &n);
    return D3DXPlaneFromPointNormal(pOut, pV1, &n);
}
inline D3DXPLANE* D3DXPlaneTransform(D3DXPLANE* pOut, const D3DXPLANE* pP, const D3DXMATRIX* pM)
{
    D3DXVECTOR4 v(pP->a, pP->b, pP->c, pP->d), out;
    D3DXVec4Transform(&out, &v, pM);
    *pOut = D3DXPLANE(out.x, out.y, out.z, out.w);
    return pOut;
}
inline D3DXPLANE* D3DXPlaneTransformArray(D3DXPLANE* pOut, unsigned int OutStride, const D3DXPLANE* pP, unsigned int PStride, const D3DXMATRIX* pM, unsigned int n)
{
    for (unsigned int i = 0; i < n; ++i)
        D3DXPlaneTransform(reinterpret_cast<D3DXPLANE*>(reinterpret_cast<uint8_t*>(pOut) + i * OutStride),
                           reinterpret_cast<const D3DXPLANE*>(reinterpret_cast<const uint8_t*>(pP) + i * PStride), pM);
    return pOut;
}

inline D3DXCOLOR* D3DXColorNegative(D3DXCOLOR* pOut, const D3DXCOLOR* pC) { *pOut = D3DXCOLOR(1.0f - pC->r, 1.0f - pC->g, 1.0f - pC->b, pC->a); return pOut; }
inline D3DXCOLOR* D3DXColorAdd(D3DXCOLOR* pOut, const D3DXCOLOR* pC1, const D3DXCOLOR* pC2) { *pOut = *pC1 + *pC2; return pOut; }
inline D3DXCOLOR* D3DXColorSubtract(D3DXCOLOR* pOut, const D3DXCOLOR* pC1, const D3DXCOLOR* pC2) { *pOut = *pC1 - *pC2; return pOut; }
inline D3DXCOLOR* D3DXColorScale(D3DXCOLOR* pOut, const D3DXCOLOR* pC, float s) { *pOut = (*pC) * s; return pOut; }
inline D3DXCOLOR* D3DXColorModulate(D3DXCOLOR* pOut, const D3DXCOLOR* pC1, const D3DXCOLOR* pC2) { *pOut = D3DXCOLOR(pC1->r * pC2->r, pC1->g * pC2->g, pC1->b * pC2->b, pC1->a * pC2->a); return pOut; }
inline D3DXCOLOR* D3DXColorLerp(D3DXCOLOR* pOut, const D3DXCOLOR* pC1, const D3DXCOLOR* pC2, float s) { *pOut = *pC1 + (*pC2 - *pC1) * s; return pOut; }
inline D3DXCOLOR* D3DXColorAdjustSaturation(D3DXCOLOR* pOut, const D3DXCOLOR* pC, float s)
{
    float grey = pC->r * 0.2125f + pC->g * 0.7154f + pC->b * 0.0721f;
    *pOut = D3DXCOLOR(grey + s * (pC->r - grey), grey + s * (pC->g - grey), grey + s * (pC->b - grey), pC->a);
    return pOut;
}
inline D3DXCOLOR* D3DXColorAdjustContrast(D3DXCOLOR* pOut, const D3DXCOLOR* pC, float c)
{
    *pOut = D3DXCOLOR(0.5f + c * (pC->r - 0.5f), 0.5f + c * (pC->g - 0.5f), 0.5f + c * (pC->b - 0.5f), pC->a);
    return pOut;
}
