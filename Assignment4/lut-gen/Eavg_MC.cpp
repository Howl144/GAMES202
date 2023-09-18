﻿#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <sstream>
#include <fstream>
#include <random>
#include "vec.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int resolution = 128;
int channel = 3;

typedef struct samplePoints {
    std::vector<Vec3f> directions;
	std::vector<float> PDFs;
}samplePoints;

void setRGB(int x, int y, float alpha, unsigned char *data) {
	data[3 * (resolution * x + y) + 0] = uint8_t(alpha);
    data[3 * (resolution * x + y) + 1] = uint8_t(alpha);
    data[3 * (resolution * x + y) + 2] = uint8_t(alpha);
}

void setRGB(int x, int y, Vec3f alpha, unsigned char *data) {
	data[3 * (resolution * x + y) + 0] = uint8_t(alpha.x);
    data[3 * (resolution * x + y) + 1] = uint8_t(alpha.y);
    data[3 * (resolution * x + y) + 2] = uint8_t(alpha.z);
}

samplePoints squareToCosineHemisphere(int sample_count){
    samplePoints samlpeList;
    const int sample_side = static_cast<int>(floor(sqrt(sample_count)));

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> rng(0.0, 1.0);
    for (int t = 0; t < sample_side; t++) {
        for (int p = 0; p < sample_side; p++) {
            double samplex = (t + rng(gen)) / sample_side;
            double sampley = (p + rng(gen)) / sample_side;
            
            double theta = 0.5f * acos(1 - 2*samplex);
            double phi =  2 * PI * sampley;
            Vec3f wi = Vec3f(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            float pdf = wi.z / PI;
            
            samlpeList.directions.push_back(wi);
            samlpeList.PDFs.push_back(pdf);
        }
    }
    return samlpeList;
}

Vec3f getEmu(int x, int y, unsigned char *data) {
    return Vec3f(data[3 * (resolution * x + y) + 0],
                 data[3 * (resolution * x + y) + 1],
                 data[3 * (resolution * x + y) + 2]);
}

Vec3f IntegrateEmu(float NdotV, Vec3f Ei) {
    return Ei * NdotV * 2.0f;
}


int main() {
    unsigned char *Edata = stbi_load("./Emu_MC_LUT.png", &resolution, &resolution, &channel, 3);
    if (Edata == NULL) 
    {
		std::cout << "ERROE_FILE_NOT_LOAD" << std::endl;
		return -1;
	}
	else 
    {
		std::cout << resolution << " " << resolution << " " << channel << std::endl;
        // | -----> mu(j)
        // | 
        // | rough（i）
        // flip it if you want to write the data on picture 
        uint8_t* data = new uint8_t[resolution * resolution * 3];
        float step = 1.0 / resolution;
        Vec3f Eavg = Vec3f(0.0);
		for (int i = 0; i < resolution; i++) 
        {
            float roughness = step * (static_cast<float>(i) + 0.5f);
			for (int j = 0; j < resolution; j++) 
            {
                float NdotV = step * (static_cast<float>(j) + 0.5f);
                Vec3f V = Vec3f(std::sqrt(1.f - NdotV * NdotV), 0.f, NdotV);
                //从左下角开始读取
                Vec3f Ei = getEmu((resolution - 1 - i), j,Edata);
                Eavg += IntegrateEmu(NdotV, Ei);
                Eavg = Eavg * step;
                setRGB(i, j, 0.0, data);
			}

            for(int k = 0; k < resolution; k++)
            {
                setRGB(i, k, Eavg, data);
            }

            Eavg = Vec3f(0.0);
		}

		stbi_flip_vertically_on_write(true);
		stbi_write_png("Eavg_MC_LUT.png", resolution, resolution, channel, data, 0);
	}
	stbi_image_free(Edata);
    return 0;
}