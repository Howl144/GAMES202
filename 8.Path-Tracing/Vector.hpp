//
// Created by LEI XU on 5/13/19.
//
#pragma once
#ifndef RAYTRACING_VECTOR_H
#define RAYTRACING_VECTOR_H

#include <iostream>
#include <cmath>
#include <algorithm>

class Vector3f {
public:
    double x, y, z;
    Vector3f() : x(0), y(0), z(0) {}
    Vector3f(double xx) : x(xx), y(xx), z(xx) {}
    Vector3f(double xx, double yy, double zz) : x(xx), y(yy), z(zz) {}
    Vector3f operator * (const double &r) const { return Vector3f(x * r, y * r, z * r); }
    Vector3f operator / (const double &r) const { return Vector3f(x / r, y / r, z / r); }

    double norm() {return std::sqrt(x * x + y * y + z * z);}
    Vector3f normalized() {
        double n = std::sqrt(x * x + y * y + z * z);
        return Vector3f(x / n, y / n, z / n);
    }

    Vector3f operator * (const Vector3f &v) const { return Vector3f(x * v.x, y * v.y, z * v.z); }
    Vector3f operator - (const Vector3f &v) const { return Vector3f(x - v.x, y - v.y, z - v.z); }
    Vector3f operator + (const Vector3f &v) const { return Vector3f(x + v.x, y + v.y, z + v.z); }
    Vector3f operator - () const { return Vector3f(-x, -y, -z); }
    Vector3f& operator += (const Vector3f &v) { x += v.x, y += v.y, z += v.z; return *this; }
    friend Vector3f operator * (const double &r, const Vector3f &v)
    { return Vector3f(v.x * r, v.y * r, v.z * r); }
    friend std::ostream & operator << (std::ostream &os, const Vector3f &v)
    { return os << v.x << ", " << v.y << ", " << v.z; }
    double       operator[](int index) const;
    double&      operator[](int index);

    void rotateAxis(Vector3f axis, double angle) {
        double n1 = axis.x;
        double n2 = axis.y;
        double n3 = axis.z;

        double cosAngle = std::cos(angle / 180.0 * acos(-1));
        double sinAngle = std::sin(angle / 180.0 * acos(-1));

        Vector3f tmpvec1(n1 * n1 * (1 - cosAngle) + cosAngle, n1 * n2 * (1 - cosAngle) - n3 * sinAngle, n1 * n3 * (1 - cosAngle) + n2 * sinAngle);
        Vector3f tmpvec2(n1 * n2 * (1 - cosAngle) + n3 * sinAngle, n2 * n2 * (1 - cosAngle) + cosAngle, n2 * n3 * (1 - cosAngle) - n1 * sinAngle);
        Vector3f tmpvec3(n1 * n3 * (1 - cosAngle) - n2 * sinAngle, n2 * n3 * (1 - cosAngle) + n1 * sinAngle, n3 * n3 * (1 - cosAngle) + cosAngle);
        x = x * tmpvec1.x + y * tmpvec2.x + z * tmpvec3.x;
        y = x * tmpvec1.y + y * tmpvec2.y + z * tmpvec3.y;
        z = x * tmpvec1.z + y * tmpvec2.z + z * tmpvec3.z;
    }


    static Vector3f Min(const Vector3f &p1, const Vector3f &p2) {
        return Vector3f(std::min(p1.x, p2.x), std::min(p1.y, p2.y),
                       std::min(p1.z, p2.z));
    }

    static Vector3f Max(const Vector3f &p1, const Vector3f &p2) {
        return Vector3f(std::max(p1.x, p2.x), std::max(p1.y, p2.y),
                       std::max(p1.z, p2.z));
    }
    
};
inline double Vector3f::operator[](int index) const {
    return (&x)[index];
}


class Vector2f
{
public:
    Vector2f() : x(0), y(0) {}
    Vector2f(double xx) : x(xx), y(xx) {}
    Vector2f(double xx, double yy) : x(xx), y(yy) {}
    Vector2f operator * (const double &r) const { return Vector2f(x * r, y * r); }
    Vector2f operator + (const Vector2f &v) const { return Vector2f(x + v.x, y + v.y); }
    double x, y;
};

inline Vector3f lerp(const Vector3f &a, const Vector3f& b, const double &t)
{ return a * (1 - t) + b * t; }

inline Vector3f normalize(const Vector3f &v)
{
    double mag2 = v.x * v.x + v.y * v.y + v.z * v.z;
    if (mag2 > 0) {
        double invMag = 1 / sqrtf(mag2);
        return Vector3f(v.x * invMag, v.y * invMag, v.z * invMag);
    }

    return v;
}

inline double dotProduct(const Vector3f &a, const Vector3f &b)
{ return a.x * b.x + a.y * b.y + a.z * b.z; }

inline Vector3f crossProduct(const Vector3f &a, const Vector3f &b)
{
    return Vector3f(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
    );
}



#endif //RAYTRACING_VECTOR_H
