#include <fstream>
#include "Vector.hpp"
#include "Renderer.hpp"
#include "Scene.hpp"
#include <optional>

inline float deg2rad(const float &deg)
{
    return deg * M_PI / 180.0;
}

// Compute reflection direction
Vector3f reflect(const Vector3f &I, const Vector3f &N)
{
    return I - 2 * dotProduct(I, N) * N;
}

// [comment]
// url : https://www.scratchapixel.com/lessons/3d-basic-rendering/introduction-to-shading/reflection-refraction-fresnel.html
// Compute refraction direction using Snell's law ： n1 * sin(i) = n2 * sin(t)，其中n1为入射介质的折射率，n2为折射介质的折射率，theta1为入射角，theta2为折射角。
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
// [/comment]
Vector3f refract(const Vector3f &I, const Vector3f &N, const float &ior)
{
    float cosi = clamp(-1, 1, dotProduct(I, N));//确保cosi在-1到1之间
    float etai = 1, etat = ior;
    Vector3f n = N;
    //注意incident view ray是从空气进入物体的，所以cosi是负数，取反后才是夹角
    if (cosi < 0)
    {
        cosi = -cosi;
    }
    //inside the object，互换折射率，取反法线
    else
    {
        std::swap(etai, etat);
        n = -N;
    }
    float eta = etai / etat;
    //cost = sqrtf(1 - eta * eta * (1 - cosi * cosi));
    float k = 1 - eta * eta * (1 - cosi * cosi);
    //T = A + B, A = M * sint, B = -N * cost,其中T为折射光线方向，A为折射光线在水平方向上的分量，B为折射光线在垂直方向上的分量，M为水平单位向量，N为法线单位向量。
    //M = (I + N * cosi) / sini,其中I为入射单位向量
    //T = (I + N * cosi) * sint / sini - N * cost
    //T = (I + N * cosi) * eta - N * cost;
    //T = eta * I + (eta * cosi - cost) * n;
    return k < 0 ? 0 : eta * I + (eta * cosi - sqrtf(k)) * n;
}

// [comment]
// url ：https://www.scratchapixel.com/lessons/3d-basic-rendering/introduction-to-shading/reflection-refraction-fresnel.html
// Compute Fresnel equation
//
// \param I is the incident view direction
//
// \param N is the normal at the intersection point
//
// \param ior is the material refractive index
// [/comment]
float fresnel(const Vector3f &I, const Vector3f &N, const float &ior)
{
    float cosi = clamp(-1, 1, dotProduct(I, N));
    float etai = 1, etat = ior;
    //inside the object，互换折射率
    if (cosi > 0)
    {
        std::swap(etai, etat);
    }
    // Compute sini using Snell's law
    float sint = etai / etat * sqrtf(std::max(0.f, 1 - cosi * cosi));
    // Total internal reflection，只有折射率大的介质向折射率小的介质发射光线时才有可能发生全反射。（未考虑Fresnel的情况下）
    if (sint >= 1)
    {
        return 1;
    }
    else
    {
        float cost = sqrtf(std::max(0.f, 1 - sint * sint));
        cosi = fabsf(cosi);
        //计算反射光的水平和垂直偏正光，然后求平均值，即反射率
        float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
        float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
        return (Rs * Rs + Rp * Rp) / 2;
    }
    // As a consequence of the conservation of energy, transmittance is given by:
    // kt = 1 - kr;
}

// [comment]
// Returns true if the ray intersects an object, false otherwise.
//
// \param orig is the ray origin
// \param dir is the ray direction
// \param objects is the list of objects the scene contains
// \param[out] tNear contains the distance to the cloesest intersected object.
// \param[out] index stores the index of the intersect triangle if the interesected object is a mesh.
// \param[out] uv stores the u and v barycentric coordinates of the intersected point
// \param[out] *hitObject stores the pointer to the intersected object (used to retrieve material information, etc.)
// \param isShadowRay is it a shadow ray. We can return from the function sooner as soon as we have found a hit.
// [/comment]
std::optional<hit_payload> trace(
    const Vector3f &orig, const Vector3f &dir,
    const std::vector<std::unique_ptr<Object>> &objects)
{
    float tNear = kInfinity;
    std::optional<hit_payload> payload;
    //primary ray分别与场景中的每个物体求交，取最近的交点
    for (const auto &object : objects)
    {
        float tNearK = kInfinity;
        uint32_t indexK;
        Vector2f uvK;
        if (object->intersect(orig, dir, tNearK, indexK, uvK) && tNearK < tNear)
        {
            payload.emplace();
            payload->hit_obj = object.get();
            payload->tNear = tNearK;
            payload->index = indexK;
            payload->uv = uvK;
            //更新深度
            tNear = tNearK;
        }
    }

    return payload;
}

// [comment]
// Implementation of the Whitted-style light transport algorithm (E [S*] (D|G) L)
//
// This function is the function that compute the color at the intersection point
// of a ray defined by a position and a direction. Note that thus function is recursive (it calls itself).
//
// If the material of the intersected object is either reflective or reflective and refractive,
// then we compute the reflection/refraction direction and cast two new rays into the scene
// by calling the castRay() function recursively. When the surface is transparent, we mix
// the reflection and refraction color using the result of the fresnel equations (it computes
// the amount of reflection and refraction depending on the surface normal, incident view direction
// and surface refractive index).
//
// If the surface is diffuse/glossy we use the Phong illumation model to compute the color
// at the intersection point.
// [/comment]
Vector3f castRay(
    const Vector3f &orig, const Vector3f &dir, const Scene &scene,
    int depth)
{
    if (depth > scene.maxDepth)
    {
        return Vector3f(0.0, 0.0, 0.0);
    }

    Vector3f hitColor = scene.backgroundColor;
    if (auto payload = trace(orig, dir, scene.get_objects()); payload)
    {
        Vector3f hitPoint = orig + dir * payload->tNear;
        Vector3f N;  // normal
        Vector2f st; // st coordinates
        payload->hit_obj->getSurfaceProperties(hitPoint, dir, payload->index, payload->uv, N, st);
        switch (payload->hit_obj->materialType)
        {
        case REFLECTION_AND_REFRACTION:
        {
            Vector3f reflectionDirection = normalize(reflect(dir, N));
            Vector3f refractionDirection = normalize(refract(dir, N, payload->hit_obj->ior));
            //epsilon是为了区分反射光线和折射光线的起点在物体内部还是外部，如果不加这epsilon，由于隐式表面求交会得到两个解析解，当发射光线在物体表面时，会出现求交始终得到t趋近于0的情况，这样就会一直递归下去，导致该像素为黑色。加上epsilon后，如果是在物体表面，就会得到一个t大于0和t小于0的解析解，从而避免了这种情况。
            Vector3f reflectionRayOrig = (dotProduct(reflectionDirection, N) < 0) ? hitPoint - N * scene.epsilon : hitPoint + N * scene.epsilon;
            Vector3f refractionRayOrig = (dotProduct(refractionDirection, N) < 0) ? hitPoint - N * scene.epsilon : hitPoint + N * scene.epsilon;

            Vector3f reflectionColor = castRay(reflectionRayOrig, reflectionDirection, scene, depth + 1);
            Vector3f refractionColor = castRay(refractionRayOrig, refractionDirection, scene, depth + 1);
            float kr = fresnel(dir, N, payload->hit_obj->ior);
            hitColor = reflectionColor * kr + refractionColor * (1 - kr);
            break;
        }
        case REFLECTION:
        {
            float kr = fresnel(dir, N, payload->hit_obj->ior);
            Vector3f reflectionDirection = reflect(dir, N);
            Vector3f reflectionRayOrig = (dotProduct(reflectionDirection, N) < 0) ? hitPoint - N * scene.epsilon : hitPoint + N * scene.epsilon;
            hitColor = castRay(reflectionRayOrig, reflectionDirection, scene, depth + 1) * kr;
            break;
        }
        default:
        {
            // [comment]
            // We use the Phong illumation model int the default case. The phong model
            // is composed of a diffuse and a specular reflection component.
            // [/comment]
            Vector3f lightAmt = 0, specularColor = 0;
            Vector3f shadowPointOrig = (dotProduct(dir, N) < 0) ? hitPoint + N * scene.epsilon : hitPoint - N * scene.epsilon;
            // [comment]
            // Loop over all lights in the scene and sum their contribution up
            // We also apply the lambert cosine law
            // [/comment]
            for (auto &light : scene.get_lights())
            {
                Vector3f lightDir = light->position - hitPoint;
                // square of the distance between hitPoint and the light
                float lightDistance2 = dotProduct(lightDir, lightDir);
                lightDir = normalize(lightDir);
                float LdotN = std::max(0.f, dotProduct(lightDir, N));
                // is the point in shadow, and is the nearest occluding object closer to the object than the light itself?
                auto shadow_res = trace(shadowPointOrig, lightDir, scene.get_objects());
                //payload->tNear的平方小于光源到交点的距离的平方
                bool inShadow = shadow_res && (shadow_res->tNear * shadow_res->tNear < lightDistance2);

                //ambient
                lightAmt += inShadow ? 0.05 : light->intensity * LdotN;
                Vector3f reflectionDirection = reflect(-lightDir, N);

                //specular
                // lightDir是hitpoint指向光源的方向，reflectionDirection是hitpoint指向orig发射点的位置，与dir的方向相反。
                specularColor += powf(std::max(0.f, -dotProduct(reflectionDirection, dir)),
                                      payload->hit_obj->specularExponent) *
                                 light->intensity ;
            }

            hitColor = lightAmt * payload->hit_obj->evalDiffuseColor(st) * payload->hit_obj->Kd + specularColor * payload->hit_obj->Ks;
            break;
        }
        }
    }

    return hitColor;
}

// [comment]
// The main render function. This where we iterate over all pixels in the image, generate
// primary rays and cast these rays into the scene. The content of the framebuffer is
// saved to a file.
// [/comment]
void Renderer::Render(const Scene &scene)
{
    std::vector<Vector3f> framebuffer(scene.width * scene.height);

    float scale = std::tan(deg2rad(scene.fov * 0.5f));
    float imageAspectRatio = scene.width / (float)scene.height;

    // Use this variable as the eye position to start your rays.
    Vector3f eye_pos(0);
    int m = 0;
    for (int j = 0; j < scene.height; ++j)
    {
        for (int i = 0; i < scene.width; ++i)
        {
            // generate primary ray direction
            // TODO: Find the x and y positions of the current pixel to get the direction
            // vector that passes through it.
            // Also, don't forget to multiply both of them with the variable *scale*, and
            // x (horizontal) variable with the *imageAspectRatio*

            // +0.5是取像素中间的光线
            // 把坐标系原点平移到中心，也就是转换到以屏幕中心为原点，[-1，1]的坐标系。
            //屏幕坐标是从左上角开始计数的，而之前使用的是左下角。使用前者，在[0, 1]^2映射到[-1, 1]^2这一步需要把y取负。
            float x = (2 * ((float)i + 0.5) / (scene.width - 1) - 1) * scale * imageAspectRatio;
            float y = (2 * ((float)j + 0.5) / (scene.height - 1) - 1) * -1 * scale;
            //z = -1的平面上
            Vector3f dir = normalize(Vector3f(x, y, -1)); 
            framebuffer[m++] = castRay(eye_pos, dir, scene, 0);
        }
        UpdateProgress(j / (float)scene.height);
    }

    // save framebuffer to file
    FILE *fp = fopen("binary.ppm", "wb");
    (void)fprintf(fp, "P6\n%d %d\n255\n", scene.width, scene.height);
    for (auto i = 0; i < scene.height * scene.width; ++i)
    {
        static unsigned char color[3];
        color[0] = (char)(255 * clamp(0, 1, framebuffer[i].x));
        color[1] = (char)(255 * clamp(0, 1, framebuffer[i].y));
        color[2] = (char)(255 * clamp(0, 1, framebuffer[i].z));
        fwrite(color, 1, 3, fp);
    }
    fclose(fp);
}
