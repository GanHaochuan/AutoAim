#pragma once
#include "LightBarDetector.h"
#include "ArmorMatcher.h"
#include "NumberRecognizer.h"

/**
 * @brief 自动瞄准系统总控类
 */
class AutoAimSystem {
public:
    AutoAimSystem(); 

    void run(const std::string& videoPath); 

private:
    LightBarDetector detector;
    ArmorMatcher matcher;
    NumberRecognizer recognizer;

    // 相机参数（PnP 用）
    cv::Mat cameraMatrix;
    cv::Mat distCoeffs;

    // 核心改进：初始化相机参数
    void initCameraParams();
    // 核心改进：PnP 测距
    void solvePnP(Armor& armor);
};