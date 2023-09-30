//
// Created by Göksu Güvendiren on 2019-05-14.
//

#pragma once

#include <vector>
#include "Vector.hpp"
#include "Object.hpp"
#include "Light.hpp"
#include "AreaLight.hpp"
#include "BVH.hpp"
#include "Ray.hpp"


class Scene
{
public:
    // setting up options
    int width =  960;
    int height = 960;
    double fov = 40;//cornellbox
    //double fov = 45;//mis_test
    Vector3f backgroundColor = Vector3f(0.235294, 0.67451, 0.843137);
    int maxDepth = 1;
    double RussianRoulette = 0.8;

    Scene(int w, int h) : width(w), height(h)
    {}

    void Add(Object *object) { objects.push_back(object); }
    void Add(std::unique_ptr<Light> light) { lights.push_back(std::move(light)); }

    const std::vector<Object*>& get_objects() const { return objects; }
    const std::vector<std::unique_ptr<Light> >&  get_lights() const { return lights; }
    Intersection intersect(const Ray& ray) const;
    BVHAccel *bvh;
    void buildBVH();
    Vector3f castRay(const Ray &ray, int depth) const;
    Vector3f mis_castRay(const Ray& ray, int depth) const;
    Vector3f mis_directLight(Intersection intersection, Vector3f wo, Intersection L_dir_Inter, double pdf_light,int depth) const;
    void sampleLight(Intersection &pos, double &pdf) const;
    bool trace(const Ray &ray, const std::vector<Object*> &objects, double &tNear, uint32_t &index, Object **hitObject);
    Vector3f traceTransparent(Vector3f wo,Vector3f wi, Intersection intersection, int depth,bool isGI) const;
    Vector3f traceSpecular(Vector3f wo, Vector3f wi, Intersection intersection, int depth, bool isGI) const;
    // creating the scene (adding objects and lights)
    std::vector<Object* > objects;
    std::vector<std::unique_ptr<Light> > lights;

    // Compute reflection direction
    Vector3f reflect(const Vector3f &I, const Vector3f &N) const
    {
        return I - 2 * dotProduct(I, N) * N;
    }


    double PowerHeuristic(double nf, double fPdf, double ng, double gPdf) const {
        double f = nf * fPdf, g = ng * gPdf;
        return (f * f) / (f * f + g * g);
    }

// Compute refraction direction using Snell's law
//
// We need to handle with care the two possible situations:
//
//    - When the ray is inside the object
//
//    - When the ray is outside.
//
// If the ray is outside, you need to make cosi positive cosi = -N.I
//
// If the ray is inside, you need to invert the refractive indices and negate the normal N
    Vector3f refract(const Vector3f &I, const Vector3f &N, const double &ior) const
    {
        double cosi = clamp(-1, 1, dotProduct(I, N));
        double etai = 1, etat = ior;
        Vector3f n = N;
        if (cosi < 0) { cosi = -cosi; } else { std::swap(etai, etat); n= -N; }
        double eta = etai / etat;
        double k = 1 - eta * eta * (1 - cosi * cosi);
        return k < 0 ? 0 : eta * I + (eta * cosi - sqrtf(k)) * n;
    }



    // Compute Fresnel equation
//
// \param I is the incident view direction
//
// \param N is the normal at the intersection point
//
// \param ior is the material refractive index
//
// \param[out] kr is the amount of light reflected
    void fresnel(const Vector3f &I, const Vector3f &N, const double &ior, double &kr) const
    {
        double cosi = clamp(-1, 1, dotProduct(I, N));
        double etai = 1, etat = ior;
        if (cosi > 0) {  std::swap(etai, etat); }
        // Compute sini using Snell's law
        double sint = etai / etat * sqrtf(std::fmax(0.f, 1 - cosi * cosi));
        // Total internal reflection
        if (sint >= 1) {
            kr = 1;
        }
        else {
            double cost = sqrtf(std::fmax(0.f, 1 - sint * sint));
            cosi = fabsf(cosi);
            double Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
            double Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
            kr = (Rs * Rs + Rp * Rp) / 2;
        }
        // As a consequence of the conservation of energy, transmittance is given by:
        // kt = 1 - kr;
    }
};