//
// Created by goksu on 2/25/20.
//

#include <fstream>
#include <thread>
#include <mutex>
#include <atomic>
#include "Scene.hpp"
#include "Renderer.hpp"


inline double deg2rad(const double& deg) { return deg * M_PI / 180.0; }


//const double EPSILON = 1e-16;//mis test for specular reflection
const double EPSILON = 1e-8;//cornell Box
// The main render function. This where we iterate over all pixels in the image,
// generate primary rays and cast these rays into the scene. The content of the
// framebuffer is saved to a file.
void Renderer::Render(const Scene& scene)
{
    std::vector<Vector3f> framebuffer(scene.width * scene.height);

    double scale = tan(deg2rad(scene.fov * 0.5));
    double imageAspectRatio = scene.width / (double)scene.height;
    // change the spp value to change sample ammount
    std::mutex ProgressBarMutex;
    std::atomic<int> progressBar = 0;
 
    constexpr int spp = 1;
    Vector3f eye_pos(278, 273, -800);//cornellbox
    std::cout << "SPP: " << spp << "\n";
    auto castRayMultiThreading = [&](int rowStart,int rowEnd,int colStart,int colEnd){
        for (int j = rowStart; j < rowEnd; ++j) {
            int m = j * scene.width + colStart;
            for (int i = colStart; i < colEnd; ++i) {
                for(int k = 0;k < spp;++k){
                    double x = (2 * (i + get_random_double()) / (double)scene.width - 1) *
                            imageAspectRatio * scale;
                    double y = (2 * (j + get_random_double()) / (double)scene.height - 1) * scale;
                    Vector3f dir = normalize(Vector3f(-x, -y, 1));
                    Vector3f result = scene.mis_castRay(Ray(eye_pos, dir), 0);
                    if (result.x != result.x || result.x < 0) result.x = 0;
                    if (result.y != result.y || result.y < 0) result.y = 0;
                    if (result.z != result.z || result.z < 0) result.z = 0;
                    framebuffer[m] += result;
                }
                ++m;
                ++progressBar;
            }    
            std::lock_guard<std::mutex> mtx_lock(ProgressBarMutex);
            UpdateProgress((double)progressBar / scene.height / scene.width);
        }
    };

    //constexpr int spp = 16;
    //Vector3f eye_pos(0, 0, 11);//mis_test
    //std::cout << "SPP: " << spp << "\n";
    //auto castRayMultiThreading = [&](int rowStart, int rowEnd, int colStart, int colEnd) {
    //    for (int j = rowStart; j < rowEnd; ++j) {
    //        int m = j * scene.width + colStart;
    //        for (int i = colStart; i < colEnd; ++i) {
    //            for (int k = 0; k < spp; ++k) {
    //                double x = (2 * (i + get_random_double()) / (double)scene.width - 1) *
    //                    imageAspectRatio * scale;
    //                double y = (1 - 2 * (j + get_random_double()) / (double)scene.height) * scale;
    //                Vector3f dir = normalize(Vector3f(x, y, -1));
    //                dir.rotateAxis(Vector3f(1, 0, 0), 10);
    //                Vector3f result = scene.mis_castRay(Ray(eye_pos, dir), 0);
    //                if (result.x != result.x || result.x < 0) result.x = 0;
    //                if (result.y != result.y || result.y < 0) result.y = 0;
    //                if (result.z != result.z || result.z < 0) result.z = 0;
    //                framebuffer[m] += result;
    //            }
    //            ++m;
    //            ++progressBar;
    //        }
    //        std::lock_guard<std::mutex> mtx_lock(ProgressBarMutex);
    //        UpdateProgress((double)progressBar / scene.height / scene.width);
    //    }
    //};

    int id = 0;
    const int numx = 3;
    const int numy = 3;
    std::thread threads[numx * numy];
    int strideX = std::ceil(scene.width / (double)numx);
    int strideY = std::ceil(scene.height / (double)numy);
    for(int y = 0;y < scene.height;y += strideY){
        for(int x = 0;x < scene.width;x += strideX){
            int rowStart = y;
            int rowEnd = std::min(y + strideY,scene.height);
            int colStart = x;
            int colEnd = std::min(x + strideX,scene.width);
            threads[id++] = std::thread(castRayMultiThreading,rowStart,rowEnd,colStart,colEnd);
        }
    }
    for(int i = 0;i < numx * numy; ++i){
        if(threads[i].joinable())
            threads[i].join();
    }

    // save framebuffer to file
    FILE* fp = fopen("binary.ppm", "wb");
    (void)fprintf(fp, "P6\n%d %d\n255\n", scene.width, scene.height);
    for (auto i = 0; i < scene.height * scene.width; ++i) {
        static unsigned char color[3];
        framebuffer[i] = framebuffer[i] / spp;
        color[0] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].x), 0.6f));
        color[1] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].y), 0.6f));
        color[2] = (unsigned char)(255 * std::pow(clamp(0, 1, framebuffer[i].z), 0.6f));
        fwrite(color, 1, 3, fp);
    }
    fclose(fp);    
}
