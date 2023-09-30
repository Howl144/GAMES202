//
// Created by LEI XU on 5/16/19.
//

#ifndef RAYTRACING_INTERSECTION_H
#define RAYTRACING_INTERSECTION_H
#include "Vector.hpp"
#include "Material.hpp"
#include <memory>
class Object;
class Sphere;

struct Intersection
{
    Intersection(){
        happened=false;
        coords=Vector3f();
        normal=Vector3f();
        distance= std::numeric_limits<double>::max();
        obj =nullptr;
        m=nullptr;
        isOutside = true;
    }
    bool happened;
    Vector3f coords;
    //存储光线的出发点
    Vector3f tcoords;
    Vector3f normal;
    Vector3f emit;
    double distance;
    Object* obj;
    std::shared_ptr<Material> m;
    bool isOutside;
};
#endif //RAYTRACING_INTERSECTION_H
