//
// Created by LEI XU on 5/16/19.
//

#ifndef RAYTRACING_MATERIAL_H
#define RAYTRACING_MATERIAL_H

#include "Vector.hpp"
#include "global.hpp"
#include <atomic>
#include <memory>

enum  MaterialType { DIFFUSE, MICROFACET, TRANSPARENT,MIRROR};

class Pdf {
public:
    Pdf():pdfValue(0), halfVec(Vector3f(0)), isReflect(true){}
    double pdfValue;
    Vector3f halfVec;
    bool isReflect;
};

class Material{
public:

    // Compute reflection direction
    Vector3f reflect(const Vector3f &I, const Vector3f &N) const
    {
        return I - 2 * dotProduct(I, N) * N;
    }

    // Compute refraction direction using Snell's law
    Vector3f refract(const Vector3f &I, const Vector3f &N,bool isOutside) const
    {
        double cosi = std::abs(dotProduct(I, N));
        double etai = 1, etat = ior;
        Vector3f n = N;
        if (isOutside == false) {
            std::swap(etai, etat);
        }
        double eta = etai / etat;
        double k = 1 - eta * eta * (1 - cosi * cosi);
        return k < 0 ? 
            0 : 
            eta * I + (eta * cosi - sqrtf(k)) * n;
    }

    Vector3f toWorld(const Vector3f &a, const Vector3f &N){
        Vector3f B, T;
        // |N.x| > |N.y| 会包括N在x轴上的情况
        if (std::fabs(N.x) > std::fabs(N.y)){
            //该分支不能算N在y轴上的情况
            double invLen = 1.0f / std::sqrt(N.x * N.x + N.z * N.z);
            //C向量是垂直于N向量的，因为N向量在xz平面上的投影为vec3(N.x, 0, N.z),则垂直于该向量的向量为vec3(N.z, 0, -N.x)或者vec3(-N.z, 0, N.x)。这样投影到xz平面上的向量围绕C向量旋转得到平面与C向量垂直，则N向量与C向量垂直。
            T = Vector3f(N.z * invLen, 0.0f, -N.x *invLen);
        }
        //反之，|N.x| <= |N.y| 会包括N在y轴上的情况
        else {
            //该分支不能算N在x轴上的情况
            double invLen = 1.0f / std::sqrt(N.y * N.y + N.z * N.z);
            T = Vector3f(0.0f, N.z * invLen, -N.y *invLen);
        }
        B = crossProduct(T, N);
        return a.x * T + a.y * B + a.z * N;
    }

    void mis_getpdf(MaterialType type, Vector3f N, Vector3f wo, Vector3f wi, Pdf& pdf){
        
        Vector3f H = pdf.halfVec;
        double NoH = dotProduct(N, H);
        double HoWi = dotProduct(H, wi);
        double NoWi = dotProduct(N, wi);

        switch (type)
        {
        case DIFFUSE: {
            if (NoWi > 0)
                pdf.pdfValue = NoWi / M_PI;
            else 
                pdf.pdfValue = 0;
            break;
        }
        case MICROFACET: {
            if (NoWi > 0) {
                double D = NDF(roughness, H, N);
                Vector3f F = Fresnel_Schlick(H, wi);
                pdf.pdfValue = F.x * (D * NoH) / (4.0 * HoWi) + (1 - F.x) * NoWi / M_PI;
            }
            else
                pdf.pdfValue = 0;
            break;
        }
        default:
            pdf.pdfValue = 0;
        }
        return;
    }

private:
    double G1G2(const double& Roughness, const Vector3f& wi, const Vector3f& wo, const Vector3f& N){
        double A_wi, A_wo;
        A_wi = (-1 + sqrt(1 + Roughness * Roughness * pow(tan(acos(dotProduct(wi, N))), 2))) / 2;
        A_wo = (-1 + sqrt(1 + Roughness * Roughness * pow(tan(acos(dotProduct(wo, N))), 2))) / 2;
        double divisor = (1 + A_wi + A_wo);
        if (divisor < EPSILON)
            return 1.0f / EPSILON;
        else
            return 1.0f / divisor;
    }

    double NDF(const double& Roughness, const Vector3f& h, const Vector3f& N){
        double cosTheta = dotProduct(h, N);
        double divisor = 1.0 + cosTheta * cosTheta * (Roughness * Roughness - 1);
        double divisor2 = M_PI * divisor * divisor;
        if (divisor2 < EPSILON)
            return (Roughness * Roughness) / EPSILON;
        else 
            return (Roughness * Roughness) / divisor2;
    }

    // Compute Fresnel equation
    Vector3f Fresnel(const Vector3f &I, const Vector3f &N,bool isOutside) const
    {
        double cosi = dotProduct(I, N);
        double etai = 1, etat = ior;
        if (isOutside == false) {  
            std::swap(etai, etat); 
        }
        // Compute sini using Snell's law
        double sint = etai / etat * sqrtf(std::fmax(0.f, 1 - cosi * cosi));
        // Total internal reflection
        if (sint >= 1) {
            return Vector3f(1.0f);
        }
        else {
            double cost = sqrtf(std::fmax(0.f, 1 - sint * sint));
            cosi = fabsf(cosi);
            double Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
            double Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
            return Vector3f((Rs * Rs + Rp * Rp) / 2);
        }
        // As a consequence of the conservation of energy, transmittance is given by:
        // kt = 1 - kr;
    }

    Vector3f Fresnel_Schlick(const Vector3f &h, const Vector3f &wo)
    {
        if (ior != 0) {
            F0 = (1 - ior) / (1 + ior);
            F0 = F0 * F0;
        }
        else {
            F0 = lerp(F0, albedo, metalness);
        }
        double subItem = 1.0 - dotProduct(h, wo);
        subItem = subItem * subItem * subItem * subItem * subItem;
        return F0 + (Vector3f(1.0f) - F0) * subItem;
    }


public:
    MaterialType m_type;
    Vector3f albedo;
    Vector3f m_emission;

    double ior;
    Vector3f F0;
    double metalness;
    double roughness;

    inline Material(MaterialType t=DIFFUSE, Vector3f e=Vector3f(0,0,0));
    inline MaterialType getType();
    //inline Vector3f getColor();
    inline Vector3f getColorAt(double u, double v);
    inline Vector3f getEmission();
    inline bool hasEmission();

    // sample a ray by Material properties
    inline Vector3f sample(const Vector3f &wo, const Vector3f &N, bool isOutside,Pdf& pdf);
    // given a ray, calculate the PdF of this ray
    //inline double pdf(const Vector3f &wi, const Vector3f &wo, const Vector3f &N);
    // given a ray, calculate the contribution of this ray
    inline Vector3f eval(const Vector3f &wo, const Vector3f &wi, const Vector3f &N, bool isOutside,const Pdf& pdf);

};

Material::Material(MaterialType t, Vector3f e):
    m_type(t), albedo(Vector3f(0.0f)),m_emission(e),ior(0),
    F0(Vector3f(0.0)), metalness(0), roughness(0){

}

MaterialType Material::getType(){return m_type;}

Vector3f Material::getEmission() {return m_emission;}
bool Material::hasEmission() {
    if (m_emission.norm() > EPSILON) return true;
    else return false;
}

Vector3f Material::getColorAt(double u, double v) {
    return Vector3f();
}


Vector3f Material::sample(const Vector3f &wo, const Vector3f &N, bool isOutside, Pdf& pdf){
    switch(m_type){
        case DIFFUSE:{
            // cosine sample on the hemisphere
            double x_1 = get_random_double(), x_2 = get_random_double();
            double z =  std::sqrt(1 - x_2);
            double sinTheta = std::sqrt(x_2), phi = 2 * M_PI * x_1;
            Vector3f localRay(sinTheta * std::cos(phi), sinTheta * std::sin(phi), z);
            //将局部坐标系转换为世界坐标系
            Vector3f ret = toWorld(localRay, N);
            pdf.pdfValue = dotProduct(ret, N) / M_PI;
            return ret; 
        }
        case MICROFACET:{
            //GGX重要性采样
            double x_1 = get_random_double(), x_2 = get_random_double();
            double a2 = std::pow(roughness, 4);
            double phi = 2.0 * M_PI * x_1;
            double cosTheta = sqrt((1.0 - x_2) / (1.0 + (a2 - 1.0) * x_2));
            double sinTheta = sqrt(1.0 - cosTheta * cosTheta);
            // from spherical coordinates to cartesian coordinates
            Vector3f h;
            h.x = cos(phi) * sinTheta;
            h.y = sin(phi) * sinTheta;
            h.z = cosTheta;
            // from tangent-space vector to world-space sample vector
            Vector3f up = abs(N.z) < 0.999 ? Vector3f(0.0, 0.0, 1.0) : Vector3f(1.0, 0.0, 0.0);
            Vector3f tangent = normalize(crossProduct(up, N));
            Vector3f bitangent = crossProduct(N, tangent);
            Vector3f H = normalize(tangent * h.x + bitangent * h.y + N * h.z);
            //Vector3f H = toWorld(h, N);
            //Vector3f H = reoriant(Vector3f(0,0,1),N,h);//Z和N反过来就是转到本地坐标
            Vector3f reflectDir = normalize(reflect(-wo, H));
            pdf.halfVec = H;

            double D = NDF(roughness, H, N);
            Vector3f F = Fresnel_Schlick(H, wo);
            pdf.pdfValue = (F.x * D * dotProduct(H,N) / (4.0 * dotProduct(H, reflectDir))) + (1 - F.x) * dotProduct(reflectDir, N) / M_PI;
            return reflectDir;
        }
        case TRANSPARENT:{
            //GGX重要性采样
            double x_1 = get_random_double(), x_2 = get_random_double();
            double a2 = std::pow(roughness,4);
            double phi = 2.0 * M_PI * x_1;
            double cosTheta = sqrt((1.0 - x_2) / (1.0 + (a2 - 1.0) * x_2));
            double sinTheta = sqrt(1.0 - cosTheta * cosTheta);
            // from spherical coordinates to cartesian coordinates
            Vector3f h;
            h.x = cos(phi) * sinTheta;
            h.y = sin(phi) * sinTheta;
            h.z = cosTheta;
            // from tangent-space vector to world-space sample vector
            Vector3f up        = abs(N.z) < 0.999 ? Vector3f(0.0, 0.0, 1.0) : Vector3f(1.0, 0.0, 0.0);
            Vector3f tangent   = normalize(crossProduct(up, N));
            Vector3f bitangent = crossProduct(N, tangent);
            Vector3f H = normalize(tangent * h.x + bitangent * h.y + N * h.z);

            pdf.halfVec = H;
            double D = NDF(roughness, H, N);
            double etai = 1, etao = ior;//空气默认为1
            if (isOutside == false)
                std::swap(etai, etao);
            Vector3f F = Fresnel(-wo, H,isOutside);

            //Compute reflection 
            Vector3f reflectDir = normalize(reflect(-wo, H));
            double sPdf = F.x * D * cosTheta / (4.0 * dotProduct(H, reflectDir));
            //Compute refraction 
            Vector3f refractDir = normalize(refract(-wo, H, isOutside));
            double divisor = etai * dotProduct(wo, H) + etao * dotProduct(refractDir, H);
            double divisor2 = divisor * divisor;
            double tPdf = (1 - F.x) * (D * cosTheta * etao * etao * std::abs(dotProduct(refractDir, H))) / divisor2;
            if (x_1 < F.x) {
                pdf.pdfValue = sPdf;
                pdf.isReflect = true;
                return reflectDir;
            }
            else {
                pdf.pdfValue = tPdf;
                pdf.isReflect = false;
                return refractDir;
            }
        }
        case MIRROR:{
            // reflect wo along N
            pdf.pdfValue = 1;
            Vector3f reflectDir = normalize(reflect(-wo, N));
            return reflectDir;
            
        }
    }
}

Vector3f Material::eval(const Vector3f &wo, const Vector3f &wi, const Vector3f &N, bool isOutside,const Pdf& pdf){
    switch(m_type){
        case DIFFUSE:
        {
            if (dotProduct(wo, N) > 0) {
                //Lambertian漫反射模型,除以π是因为 ∫Ω cosθ dω = π，抵消这部分进行的归一化
                Vector3f diffuse = albedo / M_PI;
                return diffuse;
            }
            else {
                return Vector3f(0.0);
            }
        }
        case MICROFACET:
        {
            if (dotProduct(wo, N) > 0) {
                // calculate the contribution of Microfacet model
                double G, D;
                Vector3f F;
                Vector3f h = pdf.halfVec;
                F = Fresnel_Schlick(h, wo);
                //Trowbridge–Reitz GGx的几何项 
                G = G1G2(roughness, wi, wo, N);
                //Trowbridge–Reitz GGx的法线分布项
                D = NDF(roughness, h, N);
                // BRDF
                Vector3f Ks = F;
                Vector3f diffuse = (Vector3f(1.0f) - Ks) * (1 - metalness) * albedo / M_PI;
                Vector3f specular;
                double divisor = 4 * (dotProduct(N, wi)) * (dotProduct(N, wo));
                if (divisor < EPSILON) return Vector3f(0.0);
                specular = F * G * D / divisor;
                return diffuse + specular;
            }
            else {
                return Vector3f(0.0);
            }
        }
        case TRANSPARENT:
        {
            double G, D;
            Vector3f F;
            Vector3f h = pdf.halfVec;
            double etai = 1, etao = ior;//空气默认为1
            if (isOutside == false)
                std::swap(etai, etao);
            F = Fresnel(-wo, h, isOutside);
            //Trowbridge–Reitz GGx的几何项 
            G = G1G2(roughness, wo, wi, h);
            //Trowbridge–Reitz GGx的法线分布项
            D = NDF(roughness, h, N);
            if (pdf.isReflect == true) {
                // brdf
                double divisor = 4 * std::abs(dotProduct(N, wo)) * std::abs(dotProduct(N, wi));
                if (divisor < EPSILON) return {};
                return F * G * D / divisor;
            }
            else {
                // btdf
                double prefixEntry = std::abs(dotProduct(wo, h)) * std::abs(dotProduct(wi, h)) / (std::abs(dotProduct(wo, N)) * std::abs(dotProduct(wi, N)));
                double divisor1 = etai * dotProduct(wo, h) + etao * dotProduct(wi, h);
                double divisor2 = divisor1 * divisor1;
                if (divisor2 < EPSILON) return {};
                return (prefixEntry * etao * etao * (Vector3f(1.0f) - F) * G * D) / divisor2;
            }
            return {};
        }
        case MIRROR:
        {
            if (dotProduct(wo, N) > 0) {
                //镜面反射的brdf
                double cosalpha = dotProduct(N, wi);
                Vector3f F = Vector3f(1.0);
                return F * 1 / cosalpha;
            }
            else {
                return Vector3f(0.0);   
            }
        }
    }
}
#endif //RAYTRACING_MATERIAL_H
