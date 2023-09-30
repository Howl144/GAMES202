#pragma once

#include "Object.hpp"
#include "Vector.hpp"

class Sphere : public Object
{
public:
    Sphere(const Vector3f& c, const float& r)
        : center(c)
        , radius(r)
        , radius2(r * r)
    {}

    bool intersect(const Vector3f& orig, const Vector3f& dir, float& tnear, uint32_t&, Vector2f&) const override
    {
        // analytic solution
        // 光线与球体的交点：
        // (P - C)² - R² = 0 (P: 光线与球体的交点, C: 球心, R: 球半径)
        // (O + tD - C)² - R² = 0 (O: 光线原点, D: 光线方向(单位向量), t: 光线与球体的交点距离光线原点的距离)
        // D²t² + 2(O - C)D·t + (O - C)² - R² = 0
        Vector3f L = orig - center;
        float a = dotProduct(dir, dir);
        float b = 2 * dotProduct(dir, L);
        float c = dotProduct(L, L) - radius2;
        float t0, t1;
        if (!solveQuadratic(a, b, c, t0, t1))
            return false;
        //两个都为负数，说明光线反射后与球发生了求交计算，而且得到的两个值都为负数。
        if (t0 < 0 && t1 < 0)
            return false;
        //在物体表面发生一次折射,发射点偏移了epsilon，折射的光线与物体表面的交点可能会出现t0 < 0 && t1 > 0的情况,此时t0是一个非常小的数字:比如-1.15148005e-05，但是我们定义光线是直线传播而且不能为负，所以我们要取t1为交点。
        if (t0 < 0 && t1 > 0)
            t0 = t1;
        tnear = t0;

        return true;
    }

    void getSurfaceProperties(const Vector3f& P, const Vector3f&, const uint32_t&, const Vector2f&,
                              Vector3f& N, Vector2f&) const override
    {
        N = normalize(P - center);
    }

    Vector3f center;
    float radius, radius2;
};
