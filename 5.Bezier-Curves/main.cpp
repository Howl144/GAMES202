#include <chrono>
#include <iostream>
#include <opencv2/opencv.hpp>

std::vector<cv::Point2f> control_points;

void mouse_handler(int event, int x, int y, int flags, void *userdata)
{
    if (event == cv::EVENT_LBUTTONDOWN && control_points.size() < 4)
    {
        std::cout << "Left button of the mouse is clicked - position (" << x << ", "
                  << y << ")" << '\n';
        control_points.emplace_back(x, y);
    }
}

void naive_bezier(const std::vector<cv::Point2f> &points, cv::Mat &window)
{
    auto &p_0 = points[0];
    auto &p_1 = points[1];
    auto &p_2 = points[2];
    auto &p_3 = points[3];

    for (double t = 0.0; t <= 1.0; t += 0.001)
    {
        auto point = std::pow(1 - t, 3) * p_0 + 3 * t * std::pow(1 - t, 2) * p_1 +
                     3 * std::pow(t, 2) * (1 - t) * p_2 + std::pow(t, 3) * p_3;

        // window.at<cv::Vec3b>(point.y, point.x)[2] = 255;
    }
}

cv::Point2f recursive_bezier(const std::vector<cv::Point2f> &control_points, float t)
{
    // TODO: Implement de Casteljau's algorithm

    if (control_points.size() > 2)
    {
        //points用于存储插值出的新的控制点，每次迭代后，控制点数量减少1
        std::vector<cv::Point2f> points;
        for (int i = 0; i < control_points.size() - 1; i++)
        {
            cv::Point2f point((1 - t) * control_points[i] + t * control_points[i + 1]);
            points.emplace_back(point.x, point.y);
        }
        //递归新的控制点
        return recursive_bezier(points, t);
    }
    else
    {
        //de Casteljau's 每迭代一次, 控制点数量减少1, 当控制点数量为2时, 递归结束
        return cv::Point2f((1 - t) * control_points[0] + t * control_points[1]);
    }
}

void bezier(const std::vector<cv::Point2f> &control_points, cv::Mat &window)
{
    // TODO: Iterate through all t = 0 to t = 1 with small steps, and call de Casteljau's
    // recursive Bezier algorithm.
    for (float t = 0.0; t < 1.0; t += 0.001)
    {
        //获取bezier曲线在t时刻的点
        cv::Point2f point = recursive_bezier(control_points, t);
        //bgr
        // window.at<cv::Vec3b>(point.y, point.x)[1] = 255;
        
        //anti-aliasing
        float x_min = std::floor(point.x);
        float y_min = std::floor(point.y);
        
        cv::Point2f pos_init = cv::Point2f(x_min + 0.5, y_min + 0.5);

        //遍历改点周围的9个点，计算ratio，然后乘以point点的颜色值，得到该点的颜色值
        for(int i = -1; i <= 1;++i){
            for(int j = -1; j <= 1;++j){
                cv::Point2f tmp_pos = pos_init + cv::Point2f(j,i);
                //计算距离，d最大为 3 / sqrt(2)
                float distance = std::sqrt(std::pow(point.y - tmp_pos.y, 2) + std::pow(point.x - tmp_pos.x, 2));
                //计算ratio
                float ratio = 1 - distance * std::sqrt(2) / 3;
                //计算颜色值,需要对比改点已经存在的颜色值，取最大值作为改点的颜色值
                window.at<cv::Vec3b>(tmp_pos.y,tmp_pos.x)[1] = std::fmax(window.at<cv::Vec3b>(tmp_pos.y,tmp_pos.x)[1], 255 * ratio);
            }
        }
    }
}

int main()
{
    cv::Mat window = cv::Mat(700, 700, CV_8UC3, cv::Scalar(0));
    cv::cvtColor(window, window, cv::COLOR_BGR2RGB);
    cv::namedWindow("Bezier Curve", cv::WINDOW_AUTOSIZE);

    cv::setMouseCallback("Bezier Curve", mouse_handler, nullptr);

    int key = -1;
    while (key != 27)
    {
        for (auto &point : control_points)
        {
            cv::circle(window, point, 3, {255, 255, 255}, 3);
        }

        if (control_points.size() == 4)
        {
            naive_bezier(control_points, window);
            bezier(control_points, window);

            cv::imshow("Bezier Curve", window);
            cv::imwrite("my_bezier_curve.png", window);
            key = cv::waitKey(0);

            return 0;
        }

        cv::imshow("Bezier Curve", window);
        key = cv::waitKey(20);
    }

    return 0;
}
