//
// Created by LEI XU on 5/13/19.
//

#ifndef RAYTRACING_SPHERE_H
#define RAYTRACING_SPHERE_H

#include "Object.hpp"
#include "Vector.hpp"
#include "Bounds3.hpp"
#include "Material.hpp"
#include <memory>

class Sphere : public Object{
public:
    Vector3f center;
    double radius, radius2;
    std::shared_ptr<Material> m;
    double area;
    Sphere(const Vector3f &c, const double &r, std::shared_ptr<Material> mt = std::make_shared<Material>()) : center(c), radius(r), radius2(r * r), m(mt), area(4 * M_PI *r *r) {}
    bool intersect(const Ray& ray) {
        // analytic solution
        Vector3f L = ray.origin - center;
        double a = dotProduct(ray.direction, ray.direction);
        double b = 2 * dotProduct(ray.direction, L);
        double c = dotProduct(L, L) - radius2;
        double t0, t1;
        double area = 4 * M_PI * radius2;
        if (!solveQuadratic(a, b, c, t0, t1)) return false;
        if (t0 < 0) t0 = t1;
        if (t0 < 0) return false;
        return true;
    }
    bool intersect(const Ray& ray, double &tnear, uint32_t &index) const
    {
        // analytic solution
        Vector3f L = ray.origin - center;
        double a = dotProduct(ray.direction, ray.direction);
        double b = 2 * dotProduct(ray.direction, L);
        double c = dotProduct(L, L) - radius2;
        double t0, t1;
        if (!solveQuadratic(a, b, c, t0, t1)) return false;
        if (t0 < 0) t0 = t1;
        if (t0 < 0) return false;
        tnear = t0;

        return true;
    }
    Intersection getIntersection(Ray ray){
        Intersection result;
        result.happened = false;
        Vector3f L = ray.origin - center;
        double a = dotProduct(ray.direction, ray.direction);
        double b = 2 * dotProduct(ray.direction, L);
        double c = dotProduct(L, L) - radius2;
        double t0, t1;
        if (!solveQuadratic(a, b, c, t0, t1)) return result;
        if (t0 < 0.001) t0 = t1;
        if (t0 < 0.001) return result;
        
        result.coords = Vector3f(ray.origin + ray.direction * t0);
        result.normal = normalize(Vector3f(result.coords - center));

        if (dotProduct(result.normal, ray.direction) > 0 && m->getType() == TRANSPARENT) {
            result.normal = -result.normal;
            result.isOutside = false;
        }

        result.happened = true;
        result.m = this->m;
        result.obj = this;
        result.distance = t0;
        return result;
    }
    void getSurfaceProperties(const Vector3f &P, const Vector3f &I, const uint32_t &index, const Vector2f &uv, Vector3f &N, Vector2f &st) const
    { N = normalize(P - center); }

    Vector3f evalDiffuseColor(const Vector2f &st)const {
        return Vector3f(0.0f);
    }
    Bounds3 getBounds(){
        return Bounds3(Vector3f(center.x-radius, center.y-radius, center.z-radius),
                       Vector3f(center.x+radius, center.y+radius, center.z+radius));
    }
    void Sample(Intersection &pos, double &pdf){
        //对球进行采样
        double cos_theta_max = sqrt(1- radius2/(center - pos.tcoords).norm());

        double phi = 2.0 * M_PI * get_random_double();

        double z = 1 + get_random_double() * (cos_theta_max - 1);

        Vector3f dir(sqrt(1 - z * z) * std::cos(phi), sqrt(1 - z * z) * std::sin(phi), z);
        dir = toWorld(dir, normalize(pos.tcoords - center));
        pos.coords = center + radius * dir;
        pos.normal = dir;
        pos.emit = m->getEmission();

        double solid_angle = 2 * M_PI * (1 - cos_theta_max);
        pdf = 1.0 / solid_angle;
    }

    Vector3f toWorld(const Vector3f& a, const Vector3f& N) {
        Vector3f B, T;
        // |N.x| > |N.y| 会包括N在x轴上的情况
        if (std::fabs(N.x) > std::fabs(N.y)) {
            //该分支不能算N在y轴上的情况
            double invLen = 1.0f / std::sqrt(N.x * N.x + N.z * N.z);
            //C向量是垂直于N向量的，因为N向量在xz平面上的投影为vec3(N.x, 0, N.z),则垂直于该向量的向量为vec3(N.z, 0, -N.x)或者vec3(-N.z, 0, N.x)。这样投影到xz平面上的向量围绕C向量旋转得到平面与C向量垂直，则N向量与C向量垂直。
            T = Vector3f(N.z * invLen, 0.0f, -N.x * invLen);
        }
        //反之，|N.x| <= |N.y| 会包括N在y轴上的情况
        else {
            //该分支不能算N在x轴上的情况
            double invLen = 1.0f / std::sqrt(N.y * N.y + N.z * N.z);
            T = Vector3f(0.0f, N.z * invLen, -N.y * invLen);
        }
        B = crossProduct(T, N);
        return a.x * T + a.y * B + a.z * N;
    }

    double getArea(){
        return area;
    }
    bool hasEmit(){
        return m->hasEmission();
    }
    double getEmitNorm() {
        return m->m_emission.norm();
    }
};




#endif //RAYTRACING_SPHERE_H
