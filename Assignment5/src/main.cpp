#include <fstream>
#include <string>
#include <filesystem>
#include <chrono>
#include "denoiser.h"
#include "util/image.h"
#include "util/mathutil.h"

std::vector<Matrix4x4> ReadMatrix(const std::string &filename) {
    std::ifstream is;
    is.open(filename, std::ios::binary);
    CHECK(is.is_open());
    int shapeNum;
    is.read(reinterpret_cast<char *>(&shapeNum), sizeof(int));
    std::vector<Matrix4x4> matrix(shapeNum + 2);
    for (int i = 0; i < shapeNum + 2; i++) {
        is.read(reinterpret_cast<char *>(&matrix[i]), sizeof(Matrix4x4));
    }
    is.close();
    return matrix;
}

FrameInfo LoadFrameInfo(const filesystem::path &inputDir, const int &idx) {
    Buffer2D<Float3> beauty =
        ReadFloat3Image((inputDir / ("beauty_" + std::to_string(idx) + ".exr")).str());
    Buffer2D<Float3> normal =
        ReadFloat3Image((inputDir / ("normal_" + std::to_string(idx) + ".exr")).str());
    Buffer2D<Float3> position =
        ReadFloat3Image((inputDir / ("position_" + std::to_string(idx) + ".exr")).str());
    Buffer2D<float> depth =
        ReadFloatImage((inputDir / ("depth_" + std::to_string(idx) + ".exr")).str());
    Buffer2D<float> id =
        ReadFloatImage((inputDir / ("ID_" + std::to_string(idx) + ".exr")).str());
    std::vector<Matrix4x4> matrix =
        ReadMatrix((inputDir / ("matrix_" + std::to_string(idx) + ".mat")).str());

    FrameInfo frameInfo = {beauty, depth, normal, position, id, matrix};
    return frameInfo;
}

void Denoise(const filesystem::path &inputDir, const filesystem::path &outputDir,
             const int &frameNum) {
    Denoiser_JBF denoiser_JBF;
    Denoiser_SVGF denoiser_SVGF;
    for (int i = 0; i < frameNum; i++) {
        std::cout << "Frame: " << i << std::endl;
        if (i == 22)
            int i = 0;
        FrameInfo frameInfo = LoadFrameInfo(inputDir, i);
        Buffer2D<Float3> image = denoiser_SVGF.ProcessFrame(frameInfo);
        std::string filename =
            (outputDir / ("result_" + std::to_string(i) + ".exr")).str();
        WriteFloat3Image(image, filename);
    }
}

int main() {

    //std::filesystem::path current_path = std::filesystem::current_path();
    //std::cout << "currentSourcePath : " << current_path << std::endl;

    //// Box
    //filesystem::path inputDir("./Debug/examples/box/input");
    //filesystem::path outputDir("./Debug/examples/box/output");
    //int frameNum = 20;

    
    // Pink room
    filesystem::path inputDir("./Debug/examples/pink-room/input");
    filesystem::path outputDir("./Debug/examples/pink-room/output");
    int frameNum = 80;
    
    auto start = std::chrono::high_resolution_clock::now();
    Denoise(inputDir, outputDir, frameNum);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Denoise函数的执行时间为： " << elapsed.count() << "秒.\n";
    return 0;
}