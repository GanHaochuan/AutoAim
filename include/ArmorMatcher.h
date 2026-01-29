#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include "Armor.h" // 必须包含 Armor 结构体定义

/**
 * @brief 装甲板匹配器类
 */
class ArmorMatcher {
public:
    ArmorMatcher(); 

    // 匹配装甲板
    std::vector<Armor> match(const std::vector<cv::RotatedRect>& lights); 

private:
    // 判断灯条对是否有效
    bool isValidPair(const cv::RotatedRect& l1, const cv::RotatedRect& l2); 
};