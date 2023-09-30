#include <cmath>
#include <eigen3/Core>
#include <eigen3/Dense>
#include <eigen3/src/Core/Matrix.h>
#include <iostream>


union f2b{
    float fvalue;
    unsigned char bvalue[4];
};
//获取float的小数部分
Eigen::Vector4f fract(Eigen::Vector4f frac) {
    float intPart;
    frac[0] = modf(frac[0], &intPart);
    frac[1] = modf(frac[1], &intPart);
    frac[2] = modf(frac[2], &intPart);
    frac[3] = modf(frac[3], &intPart);
    return frac;
}

//深度值float -> 颜色值RGBA  
Eigen::Vector4f pack(float depth) {
    // 使用rgba 4字节共32位来存储z值,1个字节精度为1/256
    const Eigen::Vector4f bitShift = Eigen::Vector4f(1.0, 256.0, 256.0 * 256.0, 256.0 * 256.0 * 256.0);//阶码信息数组
    const Eigen::Vector4f bitMask = Eigen::Vector4f(1.0 / 256.0, 1.0 / 256.0, 1.0 / 256.0, 0);
    Eigen::Vector4f vecDepth(depth, depth, depth, depth);

    f2b tmp[4];
    tmp[0].fvalue = vecDepth.cwiseProduct(bitShift).x();
    tmp[1].fvalue = vecDepth.cwiseProduct(bitShift).y();
    tmp[2].fvalue = vecDepth.cwiseProduct(bitShift).z();
    tmp[3].fvalue = vecDepth.cwiseProduct(bitShift).w();
    for (int index = 0; index < 4; index++) {
        for (int i = 3; i >= 0; i--) {
            for (int j = 7; j >= 0; j--) {
                printf("%d", (tmp[index].bvalue[i] >> j) & 1);
            }
            printf(" ");
        }
        printf("\n");
    }

    // gl_FragCoord:片元的坐标,fract():返回数值的小数部分
    Eigen::Vector4f rgbaDepth = fract(vecDepth.cwiseProduct(bitShift)); //改变阶码信息后保留小数部分的float数组（位移后的小数部分）。
    Eigen::Vector4f unRepresent(Eigen::Vector4f(rgbaDepth[1], rgbaDepth[2], rgbaDepth[3], rgbaDepth[3]).cwiseProduct(bitMask));//计算位移后小数部分由于精度无法表示的值
    rgbaDepth -= unRepresent; //将无法表示的值减掉，减掉的意义在于让32位float尽可能在数值上逼近8位float,随后32位的float四舍五入至8位float（在这里还是32到32），即便会丢失精度，但是我们已经减掉了无法表示的精度部分。
    //我们已经尽力了让结果保持正确了，这里还得看glsl里面RGBA单通道里面float是怎么设计的了，csapp里面有关8位float的设计为：
    //s e    n
    //0 0000 000
    //精度为1/8,0.125。

    f2b tmp1[4];
    tmp1[0].fvalue = rgbaDepth.x();
    tmp1[1].fvalue = rgbaDepth.y();
    tmp1[2].fvalue = rgbaDepth.z();
    tmp1[3].fvalue = rgbaDepth.w();
    for (int index = 0; index < 4; index++) {
        for (int i = 3; i >= 0; i--) {
            for (int j = 7; j >= 0; j--) {
                printf("%d", (tmp1[index].bvalue[i] >> j) & 1);
            }
            printf(" ");
        }
        printf("\n");
    }


    return rgbaDepth;
}

//颜色值RGBA -> 深度值float
float unpack(Eigen::Vector4f rgbaDepth) {
    //阶码信息数组的第一位1.0和frac函数就注定了这种变换只适用于-1-1之间。
    const Eigen::Vector4f bitShift = Eigen::Vector4f(1.0, 1.0 / 256.0, 1.0 / (256.0 * 256.0), 1.0 / (256.0 * 256.0 * 256.0));//恢复阶码信息
    return bitShift.dot(rgbaDepth);
}

int main(){
    //0.123456789f
    //s E=e-bias frac
    //0 01111011 11111001101011011101010
    int frac = 0b11111001101011011101010;
    double precision = 1 / (double)8388608;//1 / 2^23
    double E = 1 / (double)16;
    printf("%f \n", (frac * precision + 1) * E );

    //有关glsl，depth转RGBA的小实验
    Eigen::Vector4f vec = pack(-0.123456789f);
    float ret = unpack(vec);
    

    // Basic Example of cpp
    std::cout << "Example of cpp \n";
    float a = 1.0, b = 2.0;
    std::cout << a << std::endl;
    std::cout << a/b << std::endl;
    std::cout << std::sqrt(b) << std::endl;
    std::cout << std::acos(-1) << std::endl;//cos 180 的值为 -1，acos返回的是弧度
    std::cout << std::sin(30.0/180.0*acos(-1)) << std::endl;//传入的是弧度，30° * π/180 

    // Example of vector
    std::cout << "Example of vector \n";
    // vector definition
    Eigen::Vector3f v(1.0f,2.0f,3.0f);
    Eigen::Vector3f w(1.0f,0.0f,0.0f);
    // vector output
    std::cout << "Example of output \n";
    std::cout << v << std::endl;
    // vector add
    std::cout << "Example of add \n";
    std::cout << v + w << std::endl;
    // vector scalar multiply
    std::cout << "Example of scalar multiply \n";
    std::cout << v * 3.0f << std::endl;
    std::cout << 2.0f * v << std::endl;

    // Example of matrix
    std::cout << "Example of matrix \n";
    // matrix definition
    Eigen::Matrix3f i,j;
    i << 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0;
    j << 2.0, 3.0, 1.0, 4.0, 6.0, 5.0, 9.0, 7.0, 8.0;
    // matrix output
    std::cout << "Example of output \n";
    std::cout << i << std::endl;
    
    //给定一个点 P=(2,1), 将该点绕原点先逆时针旋转 45◦，再平移 (1,2), 计算出
    //变换后点的坐标（要求用齐次坐标进行计算）。
    Eigen::Vector3f point(1,2,1);
    Eigen::Matrix3f rotate,translate;
    rotate << std::cos(45 * acos(-1)/180) , -std::sin(45 * acos(-1)/180) , 0,
              std::sin(45 * acos(-1)/180) , std::cos(45 * acos(-1)/180)  , 0,
              0                           , 0                            , 1;
    translate << 1 , 0 , 1,
                 0 , 1 , 2,
                 0 , 0 , 1;

    std::cout << "rotate \n";
    std::cout << rotate << std::endl;
    std::cout << "translate \n";
    std::cout << translate << std::endl;

    Eigen::Matrix3f model(translate * rotate);//先旋转后平移，顺序不能反过来。
    std::cout << "model \n";
    std::cout << model << std::endl;

    std::cout << "The result of the point translation \n";
    std::cout << model * point << std::endl;

    return 0;
}