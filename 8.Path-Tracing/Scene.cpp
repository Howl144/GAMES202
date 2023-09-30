//
// Created by Göksu Güvendiren on 2019-05-14.
//

#include "Scene.hpp"
#include "Sphere.hpp"
#include <cmath>

void Scene::buildBVH()
{
    std::string strMet;
    BVHAccel::SplitMethod method = BVHAccel::SplitMethod::NAIVE;
    if (method == BVHAccel::SplitMethod::NAIVE) strMet = "BVH";
    else if (method == BVHAccel::SplitMethod::SAH) strMet = "SAH";
    printf(" - Generating %s...\n\n", strMet.c_str());
    this->bvh = new BVHAccel(objects, 1, method);
}

Intersection Scene::intersect(const Ray &ray) const
{
    return this->bvh->Intersect(ray);
}

void Scene::sampleLight(Intersection &pos, double &pdf) const
{
    double emit_areaMultiEmit_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k)
    {
        if (objects[k]->hasEmit())
        {
            //面积和光的强度做比重
            emit_areaMultiEmit_sum += objects[k]->getEmitNorm() * objects[k]->getArea();
        }
    }
    //get_random_double()随机生成一个服从[0,1]的均匀分布的数
    double p = get_random_double() * emit_areaMultiEmit_sum;
    emit_areaMultiEmit_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k)
    {
        if (objects[k]->hasEmit())
        {
            emit_areaMultiEmit_sum += objects[k]->getEmitNorm() * objects[k]->getArea();
            //随机找到一个光源面，再在这个光源面中找到一个点
            if (p <= emit_areaMultiEmit_sum)
            {
                objects[k]->Sample(pos, pdf);
                break;
            }
        }
    }
}

bool Scene::trace(
    const Ray &ray,
    const std::vector<Object *> &objects,
    double &tNear, uint32_t &index, Object **hitObject)
{
    *hitObject = nullptr;
    for (uint32_t k = 0; k < objects.size(); ++k)
    {
        double tNearK = kInfinity;
        uint32_t indexK;
        Vector2f uvK;
        if (objects[k]->intersect(ray, tNearK, indexK) && tNearK < tNear)
        {
            *hitObject = objects[k];
            tNear = tNearK;
            index = indexK;
        }
    }

    return (*hitObject != nullptr);
}

//101 FinalProject: NEE MIS BRDF BSDF
Vector3f Scene::traceSpecular(Vector3f wo, Vector3f wi, Intersection intersection, int depth, bool isGI) const
{
    //ShadowRay
    Ray shadowRay = Ray(intersection.coords, wi);
    //位移防止重复在同一点进行递归。
    float sign = std::signbit(dotProduct(intersection.normal, wi)) ? -1.0 : 1.0;
    shadowRay.origin = shadowRay.origin + intersection.normal * sign * EPSILON;
    Intersection shadowRay_inter = intersect(shadowRay);
    //与场景没有交点则返回
    if (!shadowRay_inter.happened)
        return {};
    //打到光源就返回光源的颜色
    if (shadowRay_inter.happened && shadowRay_inter.m->hasEmission())
        return shadowRay_inter.m->getEmission();
    //打到Diffuse物体，则直接返回shadowRay_inter处的全局光或直接光。
    if (isGI == true)
        return  mis_castRay(shadowRay, depth + 1);
    Intersection L_dir_Inter;
    double pdf_light = 0;
    L_dir_Inter.tcoords = shadowRay_inter.coords;
    sampleLight(L_dir_Inter, pdf_light);
    return mis_directLight(shadowRay_inter, -wi, L_dir_Inter, pdf_light, depth + 1);
}
Vector3f Scene::traceTransparent(Vector3f wo,Vector3f wi, Intersection intersection ,int depth,bool isGI) const
{
    //ShadowRay
    Ray shadowRay = Ray(intersection.coords, wi);
    //位移防止重复在同一点进行递归。
    float sign = std::signbit(dotProduct(intersection.normal, wi)) ? -1.0 : 1.0;
    shadowRay.origin = shadowRay.origin + intersection.normal * sign * EPSILON;
    Intersection shadowRay_inter = intersect(shadowRay);
    //与场景没有交点则返回
    if (!shadowRay_inter.happened)
        return {};
    //打到光源就返回光源的颜色
    if (shadowRay_inter.happened && shadowRay_inter.m->hasEmission())
        return shadowRay_inter.m->getEmission();
    //打到Diffuse物体，则直接返回shadowRay_inter处的全局光或直接光。
    if (shadowRay_inter.happened && shadowRay_inter.m->getType() == DIFFUSE) {     
        if (isGI == true) {
            return  mis_castRay(shadowRay, depth + 1);
        }
        else {
            Intersection L_dir_Inter;
            double pdf_light = 0;
            L_dir_Inter.tcoords = shadowRay_inter.coords;
            sampleLight(L_dir_Inter, pdf_light);
            return mis_directLight(shadowRay_inter,-wi, L_dir_Inter, pdf_light, depth + 1);
        }
    }
    //以上都不是，则生成新的光线进行递归.
    if (get_random_double() < 0.95) {
        Pdf pdf;
        Vector3f new_wi = (shadowRay_inter.m->sample(-wi, shadowRay_inter.normal, shadowRay_inter.isOutside, pdf)).normalized();
        Vector3f bxdf = shadowRay_inter.m->eval(-wi, new_wi, shadowRay_inter.normal, shadowRay_inter.isOutside, pdf);
        if (pdf.pdfValue < EPSILON)
            return {};
        else {
            Vector3f castRay = traceTransparent(-wi, new_wi, shadowRay_inter, depth,isGI);
            return castRay * bxdf * std::abs(dotProduct(new_wi, shadowRay_inter.normal)) / pdf.pdfValue / 0.95;
        }
    }
    return {};
}
Vector3f Scene::mis_directLight(Intersection intersection,Vector3f wo,Intersection L_dir_Inter,double pdf_light, int depth) const
{
    Vector3f Le(0.0);
    MaterialType interType = intersection.m->getType();

    //1. 直接对光源采样
    //起始点
    Vector3f p = intersection.coords;
    //光源位置点
    Vector3f x = L_dir_Inter.coords;
    //光源方向，入射方向
    Vector3f w_Li = (x - p).normalized();
    Ray p_2_light_ray(p, w_Li);
    float sign = std::signbit(dotProduct(intersection.normal, w_Li)) ? -1.0 : 1.0;
    p_2_light_ray.origin = p_2_light_ray.origin + intersection.normal * sign * EPSILON;
    Intersection p_2_light_inter = intersect(p_2_light_ray);
    //击中的是光源 且 起始点的材质不是镜面或者透明。
    if (p_2_light_inter.happened && p_2_light_inter.m->hasEmission() && interType != MIRROR && interType != TRANSPARENT)
    {
        Pdf pdf;
        pdf.halfVec = normalize(wo + w_Li);
        double light_solidAngle_pdf = pdf_light;
        intersection.m->mis_getpdf(interType, intersection.normal,wo, w_Li,pdf);
        double brdf_solidAngle_pdf = pdf.pdfValue;
        Vector3f f_r = intersection.m->eval(wo, w_Li, intersection.normal, intersection.isOutside, pdf);
        double weight = PowerHeuristic(1.0, light_solidAngle_pdf, 1.0, brdf_solidAngle_pdf);
        if (light_solidAngle_pdf < EPSILON)
            Le = Vector3f(0.0);
        else {
            Le = L_dir_Inter.emit * f_r * std::fmax(dotProduct(w_Li, intersection.normal),0.0) / light_solidAngle_pdf * weight;
            //Le += L_dir_Inter.emit * f_r * std::fmax(dotProduct(w_Li, intersection.normal),0.0) / light_solidAngle_pdf;
        }
    }


    //2.brdf对光源采样
    Pdf pdf;
    Vector3f wi = (intersection.m->sample(wo, intersection.normal, intersection.isOutside, pdf)).normalized();
    if (pdf.pdfValue < EPSILON) return Le;
    Vector3f bxdf = intersection.m->eval(wo, wi, intersection.normal, intersection.isOutside, pdf);
    Ray L_brdf_Ray = Ray(p, wi);
    sign = std::signbit(dotProduct(intersection.normal, wi)) ? -1.0 : 1.0;
    L_brdf_Ray.origin = L_brdf_Ray.origin + intersection.normal * sign * EPSILON;
    Intersection L_brdf_Inter = intersect(L_brdf_Ray);
    //击中的是光源 且 起始点的材质不是镜面或者透明。
    if (L_brdf_Inter.happened && L_brdf_Inter.m->hasEmission() && interType != MIRROR && interType != TRANSPARENT)
    {
        //起始点
        Vector3f newP = intersection.coords;
        //光源位置点
        Vector3f newX = L_brdf_Inter.coords;
        double lightDistance = (newX - newP).norm();
        double lightDistance2 = lightDistance * lightDistance;
        double newPdf_light = 0.0;
        //面光源
        if (dynamic_cast<Sphere*>(L_brdf_Inter.obj) == nullptr) {
            //面光源是两个三角形组成 * 2
            newPdf_light = lightDistance2 / L_brdf_Inter.obj->getArea() * 2 * std::fmax(dotProduct(-wi, L_brdf_Inter.normal), 0.0);
        }
        //球光源
        else {
            Intersection tmp;
            tmp.tcoords = intersection.coords;
            //只用拿到pdf就行
            L_brdf_Inter.obj->Sample(tmp, newPdf_light);
        }
        double light_solidAngle_pdf = newPdf_light;
        double brdf_solidAngle_pdf = pdf.pdfValue;
        
        double weight = PowerHeuristic(1.0, brdf_solidAngle_pdf, 1.0, light_solidAngle_pdf);
        Le += L_brdf_Inter.m->getEmission() * bxdf * std::fmax(dotProduct(wi, intersection.normal), 0.0) / brdf_solidAngle_pdf * weight;
        //Le += L_brdf_Inter.m->getEmission() * bxdf * std::fmax(dotProduct(wi, intersection.normal),0.0) / brdf_solidAngle_pdf;
    }
    //镜面和透明物质单独处理，这是直接光。
    if (interType == TRANSPARENT && depth == 0) {
        Vector3f castRay = traceTransparent(wo, wi, intersection, depth, true);
        Le = castRay * bxdf * std::abs(dotProduct(wi, intersection.normal)) / pdf.pdfValue;
    }
    if (interType == MIRROR && depth == 0) {
        Le = traceSpecular(wo, wi, intersection, depth, true) * bxdf * std::fmax((dotProduct(wi, intersection.normal)),0.0) / pdf.pdfValue;
    }
    return Le;
}
Vector3f Scene::mis_castRay(const Ray& ray, int depth) const
{
    Intersection intersection = intersect(ray);

    if (!intersection.happened)
        return Vector3f(0.0);

    //NEE
    if (intersection.m->hasEmission() && depth == 0) {
        return intersection.m->getEmission();
    }

    //直接光
    Vector3f L_dir = Vector3f(0, 0, 0);

    //相机方向，出射方向
    Vector3f wo = -ray.direction;
    //起始点
    Vector3f p = intersection.coords;
    
    //直接光的交点
    Intersection L_dir_Inter;
    double pdf_light = 0;
    L_dir_Inter.tcoords = intersection.coords;
    //在场景的所有 光源上按面积 uniform 地 sample 一个点，并计算该 sample 的概率密度
    sampleLight(L_dir_Inter, pdf_light);
    //直接光以及BSDF
    L_dir = mis_directLight(intersection, wo, L_dir_Inter, pdf_light,depth);

    //深度大于3之后再用RR，降低噪声
    if (depth > 3 && get_random_double() > RussianRoulette)
        return L_dir;

    //间接光
    Vector3f L_indir = Vector3f(0, 0, 0);
    Pdf pdf;
    Vector3f wi = (intersection.m->sample(wo, intersection.normal, intersection.isOutside, pdf)).normalized();
    if (pdf.pdfValue < EPSILON) return L_dir;
    Vector3f bxdf = intersection.m->eval(wo, wi, intersection.normal, intersection.isOutside, pdf);
    Ray L_indir_Ray = Ray(p, wi);
    //位移防止重复在同一点进行递归。
    float sign = std::signbit(dotProduct(intersection.normal, wi)) ? -1.0 : 1.0;
    L_indir_Ray.origin = L_indir_Ray.origin + intersection.normal * sign * EPSILON;    
    Intersection L_indir_Inter = intersect(L_indir_Ray);
    MaterialType interType = intersection.m->getType();
    if (L_indir_Inter.happened && !L_indir_Inter.m->hasEmission() && interType != MIRROR && interType != TRANSPARENT) {
        L_indir = mis_castRay(L_indir_Ray, depth + 1) * bxdf * std::fmax(dotProduct(wi, intersection.normal), 0.0) / pdf.pdfValue;
    }

    //depth大于0时，起始点的材质是镜面或者透明 则递归寻找Diffuse材质（包含光源）。
    if (interType == TRANSPARENT && depth > 0 ) {
        Vector3f castRay = traceTransparent(wo, wi, intersection, depth,false);
        L_indir = castRay * bxdf * std::abs(dotProduct(wi, intersection.normal)) / pdf.pdfValue;
    }
    if (interType == MIRROR && depth == 1) {
        Vector3f castRay = traceSpecular(wo, wi, intersection, depth, false);
        L_indir = castRay * bxdf * std::abs(dotProduct(wi, intersection.normal)) / pdf.pdfValue;
    }

    if (depth > 3)
        L_indir = L_indir / RussianRoulette;

    return L_dir + L_indir;
}