#pragma once
#include <opencv2/opencv.hpp>
#include <vector>

/**
 * @brief 灯条检测器类
 * 
 * 负责从输入图像中检测机器人装甲板上的LED灯条。
 * 使用颜色分割和形态学处理来识别候选区域，并通过几何特征筛选出灯条。
 */
class LightBarDetector {
public:
    LightBarDetector(); // 默认构造函数

    std::vector<cv::RotatedRect> detect(const cv::Mat& frame); // 检测灯条并返回旋转矩形列表

    void setEnemyColor(int color); // 设置敌方颜色  // 0-blue, 1-red

private:
    int enemyColor; // 敌方颜色标志
};
