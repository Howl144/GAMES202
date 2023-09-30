#include "Renderer.hpp"
#include "Scene.hpp"
#include "Triangle.hpp"
#include "Sphere.hpp"
#include "Vector.hpp"
#include "global.hpp"
#include <chrono>

// In the main function of the program, we create the scene (create objects and
// lights) as well as set the options for the render (image width and height,
// maximum recursion depth, field-of-view, etc.). We then call the render
// function().
int main(int argc, char** argv)
{
    // Change the definition here to change resolution
    Scene scene(784,784);
    std::shared_ptr<Material> red = std::make_shared<Material>(DIFFUSE, Vector3f(0.0f));
    red->albedo = Vector3f(0.63f, 0.065f, 0.05f);
    std::shared_ptr<Material> green = std::make_shared<Material>(DIFFUSE, Vector3f(0.0f));
    green->albedo = Vector3f(0.14f, 0.45f, 0.091f);
    std::shared_ptr<Material> white = std::make_shared<Material>(DIFFUSE, Vector3f(0.0f));
    white->albedo = Vector3f(0.725f, 0.71f, 0.68f);
    std::shared_ptr<Material> light = std::make_shared<Material>(DIFFUSE, (8.0f * Vector3f(0.747f+0.058f, 0.747f+0.258f, 0.747f) + 15.6f * Vector3f(0.740f+0.287f,0.740f+0.160f,0.740f) + 18.4f * Vector3f(0.737f+0.642f,0.737f+0.159f,0.737f)));
    light->albedo = Vector3f(0.65f);

    std::shared_ptr<Material> gold = std::make_shared<Material>(MICROFACET, Vector3f(0.0f));
    gold->albedo = Vector3f(0.7216, 0.451, 0.2);
    gold->roughness = 0.1f;
    gold->F0 = Vector3f(1.00, 0.71, 0.29);
    gold->metalness = 1.0f;
    std::shared_ptr<Material> sliver = std::make_shared<Material>(MICROFACET, Vector3f(0.0f));
    sliver->albedo = Vector3f(0.95, 0.93, 0.88);
    sliver->roughness = 0.4f;
    sliver->F0 = Vector3f(0.95, 0.93, 0.88);
    sliver->metalness = 1.0f;

    std::shared_ptr<Material> glass_specu = std::make_shared<Material>(TRANSPARENT, Vector3f(0.0f));
    glass_specu->roughness = 0.08f;
    glass_specu->ior = 1.5f;

    std::shared_ptr<Material> glass_rough = std::make_shared<Material>(TRANSPARENT, Vector3f(0.0f));
    glass_rough->roughness = 0.28f;
    glass_rough->ior = 1.5f;

    std::shared_ptr<Material> specu_mirror = std::make_shared<Material>(MIRROR, Vector3f(0.0f));
    //全反射
    specu_mirror->ior = 0.1f;
    

    //cornellbox
    MeshTriangle bunny_box("../models/bunny/stanford-bunny.obj", glass_rough,
                       Vector3f(150,-60,180), Vector3f(1500,1500,1500), Vector3f(0,1,0),180);
    Sphere sphere_box(Vector3f(150, 100, 150), 100, glass_specu);
    Sphere sphere1_box(Vector3f(400, 100, 350), 100, specu_mirror);
    Sphere sphere2_box(Vector3f(400, 60, 150), 60, glass_specu);
    MeshTriangle floor_box("../models/cornellbox/floor.obj", white);
    MeshTriangle shortbox_box("../models/cornellbox/shortbox.obj", white);
    MeshTriangle tallbox_box("../models/cornellbox/tallbox.obj", white);
    MeshTriangle left_box("../models/cornellbox/left.obj", red);
    MeshTriangle right_box("../models/cornellbox/right.obj", green);
    MeshTriangle light_box("../models/cornellbox/light.obj", light);

    //scene.Add(&bunny_box);
    //scene.Add(&sphere_box);
    //scene.Add(&sphere1_box);
    //scene.Add(&sphere2_box);
    scene.Add(&floor_box);
    scene.Add(&shortbox_box);
    scene.Add(&tallbox_box);
    scene.Add(&left_box);
    scene.Add(&right_box);
    scene.Add(&light_box);


    ////mis_test
    //std::shared_ptr<Material> plate1 = std::make_shared<Material>(MICROFACET, Vector3f(0.0f));
    //plate1->albedo = Vector3f((0.56, 0.57, 0.58));
    //plate1->roughness = 0.001f;
    //plate1->F0 = Vector3f((0.56, 0.57, 0.58));
    //plate1->metalness = 1.0f;
    //std::shared_ptr<Material> plate2 = std::make_shared<Material>(MICROFACET, Vector3f(0.0f));
    //plate2->albedo = Vector3f((0.56, 0.57, 0.58));
    //plate2->roughness = 0.05f;
    //plate2->F0 = Vector3f((0.56, 0.57, 0.58));
    //plate2->metalness = 1.0f;
    //std::shared_ptr<Material> plate3 = std::make_shared<Material>(MICROFACET, Vector3f(0.0f));
    //plate3->albedo = Vector3f((0.56, 0.57, 0.58));
    //plate3->roughness = 0.1f;
    //plate3->F0 = Vector3f((0.56, 0.57, 0.58));
    //plate3->metalness = 1.0f;
    //std::shared_ptr<Material> plate4 = std::make_shared<Material>(MICROFACET, Vector3f(0.0f));
    //plate4->albedo = Vector3f((0.56, 0.57, 0.58));
    //plate4->roughness = 0.2f;
    //plate4->F0 = Vector3f((0.56, 0.57, 0.58));
    //plate4->metalness = 1.0f;

    //std::shared_ptr<Material> light_mis1 = std::make_shared<Material>(DIFFUSE, Vector3f(2.0));
    //light_mis1->albedo = Vector3f(1.0f);
    //std::shared_ptr<Material> light_mis2 = std::make_shared<Material>(DIFFUSE, Vector3f(0.0,0.0,10.0));
    //light_mis2->albedo = Vector3f(1.0f);
    //std::shared_ptr<Material> light_mis3 = std::make_shared<Material>(DIFFUSE, Vector3f(0.0,50.0,0.0));
    //light_mis3->albedo = Vector3f(1.0f);
    //std::shared_ptr<Material> light_mis4 = std::make_shared<Material>(DIFFUSE, Vector3f(400.0,0.0,0.0));
    //light_mis4->albedo = Vector3f(1.0f);

    //MeshTriangle floor("../models/meshes/floor.obj", white);
    //MeshTriangle plate1_obj("../models/meshes/plate1.obj", plate1);
    //MeshTriangle plate2_obj("../models/meshes/plate2.obj", plate2);
    //MeshTriangle plate3_obj("../models/meshes/plate3.obj", plate3);
    //MeshTriangle plate4_obj("../models/meshes/plate4.obj", plate4);

    //Sphere sphere1(Vector3f(3.5, 0, 0),     1.0, light_mis1);
    //Sphere sphere2(Vector3f(1, 0, 0),       0.6, light_mis2);
    //Sphere sphere3(Vector3f(-1.7, 0, 0),    0.2, light_mis3);
    //Sphere sphere4(Vector3f(-4, 0, 0),      0.05, light_mis4);

    //scene.Add(&floor);
    //scene.Add(&plate1_obj);
    //scene.Add(&plate2_obj);
    //scene.Add(&plate3_obj);
    //scene.Add(&plate4_obj);
    //scene.Add(&sphere1);
    //scene.Add(&sphere2);
    //scene.Add(&sphere3);
    //scene.Add(&sphere4);

    scene.buildBVH();

    Renderer r;

    auto start = std::chrono::system_clock::now();
    r.Render(scene);
    auto stop = std::chrono::system_clock::now();

    std::cout << "Render complete: \n";
    std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::hours>(stop - start).count() << " hours\n";
    std::cout << "          : " << std::chrono::duration_cast<std::chrono::minutes>(stop - start).count() << " minutes\n";
    std::cout << "          : " << std::chrono::duration_cast<std::chrono::seconds>(stop - start).count() << " seconds\n";

    return 0;
}