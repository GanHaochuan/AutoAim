#include "NumberRecognizer.h"
#include <iostream>
#include <algorithm>

NumberRecognizer::NumberRecognizer() {
    // ---------------------------------------------------------
    // 核心修改：根据你训练输出的字典，顺序必须完全一致！
    // 你的字典: {'1': 0, '2': 1, '3': 2, '4': 3, '5': 4, '7': 5, 'negative': 6}
    // ---------------------------------------------------------
    classNames = {"1", "2", "3", "4", "5", "7", "negative"};
}

bool NumberRecognizer::init(const std::string& modelPath) {
    try {
        std::cout << "Loading model from: " << modelPath << std::endl;
        net = cv::dnn::readNetFromONNX(modelPath);
        
        // 优先使用 GPU (如果有的话)，否则使用 CPU
        if (cv::cuda::getCudaEnabledDeviceCount() > 0) {
            net.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
            net.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);
        } else {
            net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
            net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
        }
        return !net.empty();
    } catch (const cv::Exception& e) {
        std::cerr << "DNN Load Error: " << e.what() << std::endl;
        return false;
    }
}

void NumberRecognizer::sortPoints(cv::Point2f pts[4]) {
    // 1. 先按 x 坐标排序
    std::sort(pts, pts + 4, [](const cv::Point2f& a, const cv::Point2f& b) {
        return a.x < b.x;
    });

    // 此时 pts[0], pts[1] 是左边的点；pts[2], pts[3] 是右边的点
    // 2. 左边排序区分上下 (y小的是TL，y大的是BL)
    if (pts[0].y > pts[1].y) std::swap(pts[0], pts[1]);
    
    // 3. 右边排序区分上下 (y小的是TR，y大的是BR)
    if (pts[2].y > pts[3].y) std::swap(pts[2], pts[3]);

    // 目前顺序: TL(0), BL(1), TR(2), BR(3)
    // 目标顺序: TL, TR, BR, BL (这是透视变换的标准顺序)
    cv::Point2f finalPts[4] = {pts[0], pts[2], pts[3], pts[1]};
    
    for(int i=0; i<4; i++) pts[i] = finalPts[i];
}

int NumberRecognizer::recognize(const cv::Mat& frame, const Armor& armor) {
    if (net.empty()) return -1;

    // --- 1. 透视变换提取装甲板 ---
    cv::Point2f srcPts[4];
    for(int i=0; i<4; i++) srcPts[i] = armor.points[i];
    sortPoints(srcPts);

    // 变换到一个正方形区域，包含灯条
    int warpSize = 50; 
    cv::Point2f dstPts[4] = {
        cv::Point2f(0, 0),
        cv::Point2f(warpSize, 0),
        cv::Point2f(warpSize, warpSize),
        cv::Point2f(0, warpSize)
    };
    cv::Mat M = cv::getPerspectiveTransform(srcPts, dstPts);
    cv::Mat warped;
    cv::warpPerspective(frame, warped, M, cv::Size(warpSize, warpSize));

    // --- 2. 预处理 (必须与 Python 训练代码逻辑一致) ---
    cv::cvtColor(warped, warped, cv::COLOR_BGR2GRAY); // 转灰度

    // 裁剪掉左右灯条，只留中间数字
    // 假设中间 20px 宽是数字 (x从15开始，宽20)
    cv::Rect cropRoi(15, 0, 20, 50); 
    // 边界检查
    if (cropRoi.x < 0) cropRoi.x = 0;
    if (cropRoi.x + cropRoi.width > warped.cols) cropRoi.width = warped.cols - cropRoi.x;
    
    cv::Mat numberROI = warped(cropRoi);

    // --- 3. 构建 DNN 输入 Blob ---
    // Python: Resize((40, 20)) -> 高40，宽20
    // C++ Size: (width, height) -> (20, 40)
    // Python ToTensor: 0-255 -> 0.0-1.0 (所以这里 scale = 1/255.0)
    cv::Mat blob;
    cv::dnn::blobFromImage(numberROI, blob, 1.0/255.0, cv::Size(20, 40), cv::Scalar(0), false, false);

    // --- 4. 推理 ---
    net.setInput(blob);
    cv::Mat prob = net.forward();

    // --- 5. 解析结果 ---
    cv::Point maxLoc;
    double maxVal;
    cv::minMaxLoc(prob, NULL, &maxVal, NULL, &maxLoc);
    int classId = maxLoc.x; // 获取概率最大的索引

    // --- 6. 过滤逻辑 ---
    // 阈值过滤 (如果置信度 < 0.6，视为识别不可靠)
    if (maxVal < 0.6) return -1;

    // 获取类别名称
    std::string resultStr = classNames[classId];

    // 如果是 "negative"，说明识别为非数字
    if (resultStr == "negative") return -1;

    // 否则返回对应的数字
    return std::stoi(resultStr);
}