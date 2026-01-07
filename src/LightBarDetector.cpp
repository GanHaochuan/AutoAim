#include "LightBarDetector.h"

LightBarDetector::LightBarDetector() {
    enemyColor = 1; // 默认敌方颜色为红色
}

void LightBarDetector::setEnemyColor(int color) {
    enemyColor = color;
}

/**
 * @brief 灯条检测主函数
 * 
 * @param[in] frame 输入的BGR彩色图像
 * @return 符合灯条几何特征的旋转矩形集合
 */
std::vector<cv::RotatedRect> LightBarDetector::detect(const cv::Mat& frame) {
    std::vector<cv::RotatedRect> lightBars; // 存储检测结果的向量

    // 1st： 分离颜色通道
    cv::Mat channels[3];  // 分离BGR通道 
    cv::split(frame, channels);  // channels[0]: B, channels[1]: G, channels[2]: R

    cv::Mat binary;  // 二值化图像

    // 2nd： 颜色差分处理
    // 通过颜色差分增强目标颜色，抑制背景
    if (enemyColor == 0) {
        // 蓝色：B - R
        cv::subtract(channels[0], channels[2], binary);
    } else {
        // 红色：R - B
        cv::subtract(channels[2], channels[0], binary);
    }

    // 3rd： 二值化处理
    cv::threshold(binary, binary, 50, 255, cv::THRESH_BINARY);

    // 4th： 形态学处理
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)); // 3x3矩形核
    cv::morphologyEx(binary, binary, cv::MORPH_DILATE, kernel); // 膨胀操作：填充小孔洞，连接邻近区域

    // 5th： 轮廓提取
    std::vector<std::vector<cv::Point>> contours; // 存储轮廓的向量
    cv::findContours(binary, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    // RETR_EXTERNAL: 只检测最外层轮廓
    // CHAIN_APPROX_SIMPLE: 压缩水平、垂直、对角线方向的点

    // 6th： 灯条筛选
    for (auto& c : contours) {
        if (c.size() < 5) continue; // 拟合最小外接矩形至少需要5个点

        cv::RotatedRect rect = cv::minAreaRect(c); // 最小外接旋转矩形

        // 计算宽高比
        float w = rect.size.width;
        float h = rect.size.height;
        float ratio = std::max(w, h) / std::min(w, h);

        // 筛选条件：宽高比大于2.5，确保是细长形状
        // 且面积大于50，过滤噪声点和小区域
        if (ratio > 2.5 && cv::contourArea(c) > 50) {
            lightBars.push_back(rect);
        }
    }

    return lightBars;
}
