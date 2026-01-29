#include "LightBarDetector.h"
#include <algorithm>

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

    // 1st： 预处理 - 颜色通道分离与相减
    std::vector<cv::Mat> channels;
    cv::split(frame, channels);

    cv::Mat gray;
    // 核心改进：根据敌方颜色进行通道相减，增强灯条对比度
    if (enemyColor == 0) {
        // 敌方为蓝色：B - R
        cv::subtract(channels[0], channels[2], gray);
    } else {
        // 敌方为红色：R - B
        cv::subtract(channels[2], channels[0], gray);
    }

    // 2nd： 二值化处理
    // 核心改进：使用 OTSU 算法自动寻找最佳阈值，适应不同亮度环境
    cv::Mat binary;
    cv::threshold(gray, binary, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

    // 3rd： 形态学处理
    // 使用椭圆核或矩形核进行膨胀，使断裂的灯条闭合
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 5));
    cv::morphologyEx(binary, binary, cv::MORPH_DILATE, kernel);

    // 4th： 轮廓提取
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(binary, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // 5th： 灯条筛选（几何特征约束）
    for (auto& c : contours) {
        if (c.size() < 6) continue; // 点数过少不稳定

        cv::RotatedRect rect = cv::minAreaRect(c);
        
        // 规范化角度和长宽：确保 width 是短边，height 是长边
        float w = rect.size.width;
        float h = rect.size.height;
        
        if (w > h) {
            std::swap(w, h);
            rect.angle += 90.0f; // 调整角度
            std::swap(rect.size.width, rect.size.height); // 交换尺寸数据
        }

        // 筛选条件1：面积
        if (w * h < 20) continue; 

        // 筛选条件2：长宽比 (灯条通常细长，比例在 1.5 ~ 10 之间)
        float ratio = h / w;
        if (ratio < 1.5 || ratio > 12.0) continue;

        // 筛选条件3：角度 (灯条大体竖直，容忍左右倾斜)
        // 注意：OpenCV 高版本 angle 范围可能有变，但经过上面规范化后，
        // 竖直灯条的 angle 应该接近 0 或 180 (或 +/- 90 视版本而定)。
        // 简单判断：只要不是横着的(接近90度)就可以
        // 这里做一个宽松的判断：倾斜角度不应过大
        float angle = std::abs(rect.angle);
        // 如果规范化后角度是0度表示竖直，那么超过45度通常不是灯条
        // 具体取决于 OpenCV 版本，若发现筛选太严需调整此值
        if (angle > 45 && angle < 135) continue; 

        lightBars.push_back(rect);
    }

    return lightBars;
}