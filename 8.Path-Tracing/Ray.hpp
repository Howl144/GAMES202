//
// Created by LEI XU on 5/16/19.
//

#ifndef RAYTRACING_RAY_H
#define RAYTRACING_RAY_H
#include "Vector.hpp"
#include "Material.hpp" 
struct Ray{
    //Destination = origin + t*direction
    Vector3f origin;
    Vector3f direction, direction_inv;
    double t;//transportation time,
    double t_min, t_max;


    Ray(const Vector3f& ori = Vector3f(0.0f), const Vector3f& dir = Vector3f(0.0f), const double _t = 0.0): origin(ori), direction(dir),t(_t){
        direction_inv = Vector3f(1./direction.x, 1./direction.y, 1./direction.z);
        t_min = 0.0;
        t_max = std::numeric_limits<double>::max();
    }

    Vector3f operator()(double t) const{return origin+direction*t;}

    friend std::ostream &operator<<(std::ostream& os, const Ray& r){
        os<<"[origin:="<<r.origin<<", direction="<<r.direction<<", time="<< r.t<<"]\n";
        return os;
    }
};
#endif //RAYTRACING_RAY_H
