#pragma once
#include <opencv2/opencv.hpp>
#include <vector>

/**
 * @brief 装甲板类型枚举
 * 用于区分大装甲板和小装甲板（加分项）
 */
enum class ArmorType {
    SMALL, // 小装甲板
    LARGE, // 大装甲板
    UNKNOWN // 未知
};

/**
 * @brief 装甲板结构体
 * 
 * 存储匹配到的装甲板相关信息，包含几何特征、边界信息及类型。
 * 主要用于后续的目标跟踪和识别。
 */
struct Armor {
    cv::RotatedRect rect;      // 装甲板整体旋转矩形
    cv::Rect boundingRect;     // 外接矩形（用于ROI）
    cv::Point2f points[4];     // 装甲板四个顶点（透视变换用）
    ArmorType type;            // 装甲板类型
    float distance;            // 目标距离（单位：mm）
    
    Armor() : type(ArmorType::UNKNOWN), distance(0.0f) {}
};