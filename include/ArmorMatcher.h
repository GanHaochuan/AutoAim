#pragma once
#include <opencv2/opencv.hpp>
#include <vector>

/**
 * @brief 装甲板结构体
 * 
 * 存储匹配到的装甲板相关信息，包含几何特征和边界信息。
 * 主要用于后续的目标跟踪和识别。
 */
struct Armor {
    cv::RotatedRect rect;  // 装甲板整体旋转矩形
    cv::Rect boundingRect; // 外接矩形（用于ROI）
};

/**
 * @brief 装甲板匹配器类
 * 
 * 根据检测到的灯条对进行装甲板匹配。
 * 使用几何约束条件筛选有效的灯条对，并组合成装甲板区域。
 */
class ArmorMatcher {
public:
    ArmorMatcher(); // 构造函数

    std::vector<Armor> match(const std::vector<cv::RotatedRect>& lights); // 装甲板匹配函数

private:
    bool isValidPair(const cv::RotatedRect& l1, const cv::RotatedRect& l2); // 判断两个灯条是否构成有效装甲板对
};
