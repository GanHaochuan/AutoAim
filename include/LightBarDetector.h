#pragma once
#include <opencv2/opencv.hpp>
#include <vector>

/**
 * @brief 灯条检测器类
 */
class LightBarDetector {
public:
    LightBarDetector(); 

    // 设置敌方颜色
    void setEnemyColor(int color); 

    // 检测灯条
    std::vector<cv::RotatedRect> detect(const cv::Mat& frame); 

private:
    int enemyColor; // 0-blue, 1-red
};