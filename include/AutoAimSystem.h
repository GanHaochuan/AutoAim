#pragma once
#include "LightBarDetector.h"
#include "ArmorMatcher.h"

/**
 * @brief 自动瞄准系统总控类
 * 
 * 负责整合灯条检测功能，实现完整的自动瞄准系统流程。
 * 包含视频流处理、目标检测结果可视化等功能。
 */
class AutoAimSystem {
public:
    AutoAimSystem(); // 默认构造函数

    void run(const std::string& videoPath); // 运行自动瞄准系统，处理视频流

private:
    LightBarDetector detector; // 灯条检测器实例
    ArmorMatcher matcher; // 装甲板匹配器实例
};
