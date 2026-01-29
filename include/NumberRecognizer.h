#pragma once
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp> // 必须包含深度学习模块
#include <vector>
#include <string>
#include "Armor.h"

class NumberRecognizer {
public:
    NumberRecognizer();
    
    // 初始化模型 (传入 .onnx 路径)
    bool init(const std::string& modelPath);
    
    // 识别主函数
    int recognize(const cv::Mat& frame, const Armor& armor);

private:
    cv::dnn::Net net;
    std::vector<std::string> classNames; // 存放 "1", "2"...
    
    // 顶点排序辅助函数
    void sortPoints(cv::Point2f pts[4]);
};