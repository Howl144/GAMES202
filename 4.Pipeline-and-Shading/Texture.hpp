//
// Created by LEI XU on 4/27/19.
//

#ifndef RASTERIZER_TEXTURE_H
#define RASTERIZER_TEXTURE_H
#include "global.hpp"
#include <eigen3/Eigen>
#include <opencv2/opencv.hpp>
class Texture{
private:
    cv::Mat image_data;

public:
    Texture(const std::string& name)
    {
        image_data = cv::imread(name);
        cv::cvtColor(image_data, image_data, cv::COLOR_RGB2BGR);
        width = image_data.cols;
        height = image_data.rows;
    }

    int width, height;

    Eigen::Vector3f getColor(float u, float v)
    {
        auto u_img = u * width;
        auto v_img = (1 - v) * height;
        auto color = image_data.at<cv::Vec3b>(v_img, u_img);
        return Eigen::Vector3f(color[0], color[1], color[2]);
    }
    // Bilinear Interpolation
      Eigen::Vector3f getColorBilinear(float u, float v)
    {
        if (u < 0) u = 0;
        if (u > 1) u = 1;
        if (v < 0) v = 0;
        if (v > 1) v = 1;
 
        //纹理坐标转换到图像坐标
        auto u_img = u * width;
        auto v_img = (1 - v) * height;

        float u_min = std::floor(u_img);
        float u_max = std::ceil(u_img);
        float v_min = std::floor(v_img);
        float v_max = std::ceil(v_img);

        //获取4个整数坐标处的颜色
        auto U00 = image_data.at<cv::Vec3b>(v_min, u_min);
        auto U01 = image_data.at<cv::Vec3b>(v_max, u_min);
        auto U10 = image_data.at<cv::Vec3b>(v_min, u_max);
        auto U11 = image_data.at<cv::Vec3b>(v_max, u_max);

        //计算权重
        float u_weight = (u_max - u_min) !=0 ? (u_img - u_min) / (u_max - u_min) : 1;
        float v_weight = (v_max - v_min) !=0 ? (v_img - v_min) / (v_max - v_min) : 1;

        //计算插值,先插值u方向，再插值v方向
        auto U0 = (1 - u_weight) * U00 + u_weight * U10;
        auto U1 = (1 - u_weight) * U01 + u_weight * U11;

        Eigen::Vector3f result = (1 - v_weight) * Eigen::Vector3f(U0[0],U0[1],U0[2]) + v_weight * Eigen::Vector3f(U1[0],U1[1],U1[2]);

        float tmp = result[0] + result[1] + result[2];

        return result;
    }
};
#endif //RASTERIZER_TEXTURE_H
