#include <eigen3/Eigen>
#include <eigen3/src/Core/Matrix.h>
#include <cmath>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <Triangle.hpp>
#include <rasterizer.hpp>

// constexpr double MY_PI = 3.1415926;

Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos)
{
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f translate;
    translate << 1, 0, 0, -eye_pos[0], 0, 1, 0, -eye_pos[1], 0, 0, 1,
        -eye_pos[2], 0, 0, 0, 1;

    view = translate * view;

    return view;
}

Eigen::Matrix4f get_model_matrix(float rotation_angle)
{
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();
    // TODO: Implement this function
    // Create the model matrix for rotating the triangle around the Z axis.
    // Then return it.
    
    float radian = rotation_angle * std::acos(-1)/180; 
    model << std::cos(radian) ,-std::sin(radian),0,0,
             std::sin(radian) , std::cos(radian),0,0,
             0                , 0               ,1,0,
             0                , 0               ,0,1;
    
    return model;
}
//45, 1, 0.1, 50
Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
                                      float zNear, float zFar)
{
    Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();
    // Students will implement this function
    // TODO: Implement this function
    // Create the projection matrix for the given parameters.
    // Then return it.
    
    //透视矩阵:将平截头体压缩成长方体。
    projection << zNear , 0     , 0            , 0             ,
                  0     , zNear , 0            , 0             ,
                  0     , 0     , zNear + zFar , -zNear * zFar ,
                  0     , 0     , 1            , 0             ;
    
    //获取压缩后平截头体的长宽高。
    float h = zNear * std::tan(eye_fov/2 * acos(-1)/180) * 2;
    float w = h * aspect_ratio;
    float z = zFar - zNear;
    //正交矩阵：由平移和缩放所得。
    Eigen::Matrix4f translation,scale;
    translation << 1,0,0,0,//x不改变，一是没有具体的左右边界，二是field of view 是看向中心点
                   0,1,0,0,//y同理
                   0,0,1,-(zFar + zNear)/2,
                   0,0,0,1;
    scale << 2/w,0  ,0  ,0,
             0  ,2/h,0  ,0,
             0  ,0  ,2/z,0,
             0  ,0  ,0  ,1;

    projection = scale * translation * projection;

    return projection;
}

int main(int argc, const char** argv)
{
    float angle = 0;
    bool command_line = false;
    std::string filename = "output.png";

    if (argc >= 3) {
        command_line = true;
        angle = std::stof(argv[2]); // -r by default
        if (argc == 4) {
            filename = std::string(argv[3]);
        }
    }

    rst::rasterizer r(700, 700);

    Eigen::Vector3f eye_pos = {0, 0, 5};

    std::vector<Eigen::Vector3f> pos{{2, 0, -2}, {0, 2, -2}, {-2, 0, -2}};

    std::vector<Eigen::Vector3i> ind{{0, 1, 2}};

    auto pos_id = r.load_positions(pos);
    auto ind_id = r.load_indices(ind);

    int key = 0;
    int frame_count = 0;

    if (command_line) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);

        cv::imwrite(filename, image);

        return 0;
    }

    while (key != 27) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);

        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::imshow("image", image);
        key = cv::waitKey(10);

        std::cout << "frame count: " << frame_count++ << '\n';

        if (key == 'a') {
            angle += 10;
        }
        else if (key == 'd') {
            angle -= 10;
        }
    }

    return 0;
}
