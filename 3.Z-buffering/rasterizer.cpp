// clang-format off
//
// Created by goksu on 4/6/19.
//

#include <algorithm>
#include <vector>
#include "rasterizer.hpp"
#include <opencv2/opencv.hpp>
#include <math.h>

#define SSAA false

rst::pos_buf_id rst::rasterizer::load_positions(const std::vector<Eigen::Vector3f> &positions)
{
    auto id = get_next_id();
    pos_buf.emplace(id, positions);

    return {id};
}

rst::ind_buf_id rst::rasterizer::load_indices(const std::vector<Eigen::Vector3i> &indices)
{
    auto id = get_next_id();
    ind_buf.emplace(id, indices);
    return {id};
}

rst::col_buf_id rst::rasterizer::load_colors(const std::vector<Eigen::Vector3f> &cols)
{
    auto id = get_next_id();
    col_buf.emplace(id, cols);

    return {id};
}

auto to_vec4(const Eigen::Vector3f& v3, float w = 1.0f)
{
    return Vector4f(v3.x(), v3.y(), v3.z(), w);
}

//一定要是float类型，否则精度缺失后，SSAA和正常采样的结果是一样的！
static bool insideTriangle(float x, float y, const Vector3f* _v)
{   
    // TODO : Implement this function to check if the point (x, y) is inside the triangle represented by _v[0], _v[1], _v[2]
    
    // 三角形内部的点满足：三角形的三个顶点到该点的向量的叉积方向相同
    Vector3f A = _v[0];
    Vector3f B = _v[1];
    Vector3f C = _v[2];
    //向量AB, BC, CA
    Vector3f AB = B - A;
    Vector3f BC = C - B;
    Vector3f CA = A - C;
    //深度值可以不考虑，随便取顶点的一个深度值作为p点的深度值。
    Vector3f P;
    P << x, y , C[2];

    Vector3f AP = P - A;
    Vector3f BP = P - B;
    Vector3f CP = P - C;

    Vector3f AB_cross_AP = AB.cross(AP);
    Vector3f BC_cross_BP = BC.cross(BP);
    Vector3f CA_cross_CP = CA.cross(CP);
    //叉乘得到的向量做点乘，如果都大于0，说明三个向量的方向相同，点在三角形内部
    return AB_cross_AP.dot(BC_cross_BP)>0 && 
            BC_cross_BP.dot(CA_cross_CP)>0 && 
            CA_cross_CP.dot(AB_cross_AP)>0;
}

static std::tuple<float, float, float> computeBarycentric2D(float x, float y, const Vector3f* v)
{
    float c1 = (x*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*y + v[1].x()*v[2].y() - v[2].x()*v[1].y()) / (v[0].x()*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*v[0].y() + v[1].x()*v[2].y() - v[2].x()*v[1].y());
    float c2 = (x*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*y + v[2].x()*v[0].y() - v[0].x()*v[2].y()) / (v[1].x()*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*v[1].y() + v[2].x()*v[0].y() - v[0].x()*v[2].y());
    float c3 = (x*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*y + v[0].x()*v[1].y() - v[1].x()*v[0].y()) / (v[2].x()*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*v[2].y() + v[0].x()*v[1].y() - v[1].x()*v[0].y());
    return {c1,c2,c3};
}

void rst::rasterizer::draw(pos_buf_id pos_buffer, ind_buf_id ind_buffer, col_buf_id col_buffer, Primitive type)
{
    auto& buf = pos_buf[pos_buffer.pos_id];
    auto& ind = ind_buf[ind_buffer.ind_id];
    auto& col = col_buf[col_buffer.col_id];

    float f1 = (50 - 0.1) / 2.0;
    float f2 = (50 + 0.1) / 2.0;

    Eigen::Matrix4f mvp = projection * view * model;
    for (auto& i : ind)
    {
        Triangle t;
        Eigen::Vector4f v[] = {
                mvp * to_vec4(buf[i[0]], 1.0f),
                mvp * to_vec4(buf[i[1]], 1.0f),
                mvp * to_vec4(buf[i[2]], 1.0f)
        };
        //Homogeneous division
        for (auto& vec : v) {
            vec /= vec.w();
        }
        //Viewport transformation
        for (auto & vert : v)
        {
            vert.x() = 0.5*width*(vert.x()+1.0);
            vert.y() = 0.5*height*(vert.y()+1.0);
            vert.z() = vert.z() * f1 + f2;
        }

        for (int i = 0; i < 3; ++i)
        {
            //v[i].head<3>()返回的是v[i]的前三个元素
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
        }

        auto col_x = col[i[0]];
        auto col_y = col[i[1]];
        auto col_z = col[i[2]];

        t.setColor(0, col_x[0], col_x[1], col_x[2]);
        t.setColor(1, col_y[0], col_y[1], col_y[2]);
        t.setColor(2, col_z[0], col_z[1], col_z[2]);

        rasterize_triangle(t);
    }
    if(SSAA){
        //SSAA begin
        for(int x = 0;x < width;++x){
            for(int y = 0;y < height;++y){
                //对之前设置好的frame_buf_2xSSAA进行平均
                Eigen::Vector3f color{0.0f,0.0f,0.0f};
                for(int i = 0;i < 4;++i){
                    color += frame_buf_2xSSAA[get_index(x,y)][i];
                }
                color /= 4.0f;
                set_pixel(Eigen::Vector3f(x,y,1.0f),color);
            }
        }
        //SSAA end
    }
    
}

//Screen space rasterization
void rst::rasterizer::rasterize_triangle(const Triangle& t) {
    auto v = t.toVector4();
    
    // TODO : Find out the bounding box of current triangle.
    // iterate through the pixel and find if the current pixel is inside the triangle 
    
    //bounding box
    float min_x = std::min(v[0].x(), std::min(v[1].x(), v[2].x()));
    float max_x = std::max(v[0].x(), std::max(v[1].x(), v[2].x()));
    float min_y = std::min(v[0].y(), std::min(v[1].y(), v[2].y()));
    float max_y = std::max(v[0].y(), std::max(v[1].y(), v[2].y()));
    //整数化
    int min_x_int = std::floor(min_x);
    int max_x_int = std::ceil(max_x);
    int min_y_int = std::floor(min_y);
    int max_y_int = std::ceil(max_y);

    // If so, use the following code to get the interpolated z value.
    //auto[alpha, beta, gamma] = computeBarycentric2D(x, y, t.v);
    //float w_reciprocal = 1.0/(alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
    //float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
    //z_interpolated *= w_reciprocal;

    //遍历bounding box内的像素，判断是否在三角形内，是的话就插值z值，然后判断是否需要更新深度缓冲区。
    for(int x = min_x_int;x <= max_x_int;++x){
        for(int y = min_y_int;y <= max_y_int;++y){
            if(SSAA){
                //pixel内的采样点
                int index = 0;
                for(float x_pixel = 0.25;x_pixel < 1.0;x_pixel += 0.5){
                    for(float y_pixel = 0.25;y_pixel < 1.0;y_pixel += 0.5){
                        if(insideTriangle(x + x_pixel,y + y_pixel,t.v)){
                            auto[alpha, beta, gamma] = computeBarycentric2D(x + x_pixel, y + y_pixel, t.v);
                            float w_reciprocal = 1.0/(alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
                            float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
                            z_interpolated *= w_reciprocal;
                            if(z_interpolated < depth_buf_2xSSAA[get_index(x,y)][index]){
                                depth_buf_2xSSAA[get_index(x,y)][index] = z_interpolated;
                                frame_buf_2xSSAA[get_index(x,y)][index] = t.getColor();
                            }        
                        }
                        ++index;
                    }
                }
            }else{
                if(insideTriangle(x + 0.5,y + 0.5,t.v)){
                    //c++17 tuple元组,计算该点在三角形内的重心坐标
                    auto[alpha, beta, gamma] = computeBarycentric2D(x + 0.5, y + 0.5, t.v);
                    float w_reciprocal = 1.0/(alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
                    float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
                    z_interpolated *= w_reciprocal;
                    //深度值更小的片段覆盖深度值更大的片段，并且更新颜色。
                    if(z_interpolated < depth_buf[get_index(x,y)]){
                        depth_buf[get_index(x,y)] = z_interpolated;
                        set_pixel(Vector3f(x,y,1.0f),t.getColor());
                    }
                }
            }
        }
    }

    // TODO : set the current pixel (use the set_pixel function) to the color of the triangle (use getColor function) if it should be painted.
}

void rst::rasterizer::set_model(const Eigen::Matrix4f& m)
{
    model = m;
}

void rst::rasterizer::set_view(const Eigen::Matrix4f& v)
{
    view = v;
}

void rst::rasterizer::set_projection(const Eigen::Matrix4f& p)
{
    projection = p;
}

void rst::rasterizer::clear(rst::Buffers buff)
{
    if ((buff & rst::Buffers::Color) == rst::Buffers::Color)
    {
        std::fill(frame_buf.begin(), frame_buf.end(), Eigen::Vector3f{0, 0, 0});
        //SSAA Begin
        for(unsigned int i = 0;i < frame_buf_2xSSAA.size();++i){
            frame_buf_2xSSAA[i].resize(4);
            std::fill(frame_buf_2xSSAA[i].begin(), frame_buf_2xSSAA[i].end(), Eigen::Vector3f{0, 0, 0});            
        }
        //SSAA End
    }
    if ((buff & rst::Buffers::Depth) == rst::Buffers::Depth)
    {
        std::fill(depth_buf.begin(), depth_buf.end(), std::numeric_limits<float>::infinity());
        //SSAA Begin
        for(unsigned int i = 0;i < depth_buf_2xSSAA.size();++i){
            depth_buf_2xSSAA[i].resize(4);
            std::fill(depth_buf_2xSSAA[i].begin(), depth_buf_2xSSAA[i].end(), std::numeric_limits<float>::infinity());
        }
        //SSAA End
    }
}

rst::rasterizer::rasterizer(int w, int h) : width(w), height(h)
{
    frame_buf.resize(w * h);
    depth_buf.resize(w * h);
    //SSAA Begin
    frame_buf_2xSSAA.resize(w * h);
    depth_buf_2xSSAA.resize(w * h);
    //SSAA End
}

int rst::rasterizer::get_index(int x, int y)
{
    return (height-1-y)*width + x;
}

void rst::rasterizer::set_pixel(const Eigen::Vector3f& point, const Eigen::Vector3f& color)
{
    //old index: auto ind = point.y() + point.x() * width;
    auto ind = (height-1-point.y())*width + point.x();
    frame_buf[ind] = color;

}

// clang-format on