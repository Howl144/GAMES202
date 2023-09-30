#pragma once
#include <iostream>
#include <cmath>
#include <random>
#include "Vector.hpp"
#include <eigen3/Core>
#include <eigen3/Dense>
#include <eigen3/src/Core/Matrix.h>

#undef M_PI
#define M_PI 3.141592653589793f

extern const double  EPSILON;
const double kInfinity = std::numeric_limits<double>::max();

inline double clamp(const double &lo, const double &hi, const double &v)
{ return std::max(lo, std::min(hi, v)); }

inline void clamp(const double &lo, const double &hi,Vector3f &v)
{ 
    v.x = std::max(lo, std::min(hi, v.x));
    v.y = std::max(lo, std::min(hi, v.y));
    v.z = std::max(lo, std::min(hi, v.z));
}

inline  bool solveQuadratic(const double &a, const double &b, const double &c, double &x0, double &x1)
{
    double discr = b * b - 4 * a * c;
    if (discr < 0) return false;
    else if (discr == 0) 
        x0 = x1 = - 0.5 * b / a;
    else {
        double q = (b > 0) ?
                  -0.5 * (b + sqrt(discr)) :
                  -0.5 * (b - sqrt(discr));
        x0 = q / a;
        x1 = c / q;
    }
    if (x0 > x1) std::swap(x0, x1);
    return true;
}

inline double get_random_double()
{
    //生成一个随机数，随机数服从均匀分布
    static std::random_device dev;
    static std::mt19937 rng(dev());
    static std::uniform_real_distribution<double> dist(0.f, 1.f); // distribution in range [0,1)

    return dist(rng);
}

inline void UpdateProgress(double progress)
{
    int barWidth = 70;

    std::cout << "[";
    int pos = barWidth * progress;
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << int(progress * 100.0) << " %\r";
    std::cout.flush();
};

inline double RadicalInverse_VdC(unsigned int bits) 
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return double(bits) * 2.3283064365386963e-10; // / 0x100000000
}

inline Vector2f Hammersley(unsigned int i, unsigned int N)
{
    return Vector2f(double(i)/double(N), RadicalInverse_VdC(i));
} 
//四元数乘法
inline Eigen::Vector4f qmul(Eigen::Vector4f r, Eigen::Vector4f s) {
    Eigen::Vector4f res;
    res[0] = (r[0] * s[0] - r[1] * s[1] - r[2] * s[2] - r[3] * s[3]);
    res[1] = (r[0] * s[1] + r[1] * s[0] - r[2] * s[3] + r[3] * s[2]);
    res[2] = (r[0] * s[2] + r[1] * s[3] + r[2] * s[0] - r[3] * s[1]);
    res[3] = (r[0] * s[3] - r[1] * s[2] + r[2] * s[1] + r[3] * s[0]);
    return res;
}

inline Vector3f rotate_vec(Vector3f v, Vector3f axis, double theta) {
    Eigen::Vector4f q = Eigen::Vector4f(cos(theta * 0.5), sin(theta * 0.5) * axis.x, sin(theta * 0.5) * axis.y, sin(theta * 0.5) * axis.z);
    Eigen::Vector4f q_inv = Eigen::Vector4f(q[0],-q[1],-q[2],-q[3]);
    Eigen::Vector4f p = Eigen::Vector4f(0, v.x,v.y,v.z);
    Eigen::Vector4f res = qmul(qmul(q_inv, p), q);
    return Vector3f(res[1], res[2], res[3]);
}
//世界坐标转本地坐标
inline Vector3f reoriant(Vector3f n1, Vector3f n2, Vector3f x) {
    double dotv = clamp(dotProduct(n1, n2), -1.0, 1.0);
    double dotv_abs = abs(dotv);
    if (dotv_abs == 1.0) return dotv * x;
    Vector3f axis = normalize(crossProduct(n1, n2));
    double theta = acos(dotv);
    return rotate_vec(x, axis, theta);
}
