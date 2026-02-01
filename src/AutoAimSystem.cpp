#include "AutoAimSystem.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <algorithm> // 必须包含，用于 std::sort

AutoAimSystem::AutoAimSystem() {
    detector.setEnemyColor(1); // 1-RED, 0-BLUE

    // ---------------------------------------------------------
    // 核心修改：加载 ONNX 模型
    // 请确保 D:/AutoAim/models/armor_model.onnx 文件存在！
    // ---------------------------------------------------------
    std::string modelPath = "D:/AutoAim/models/armor_model.onnx";
    if (!recognizer.init(modelPath)) {
        std::cerr << "严重错误: 无法加载模型文件: " << modelPath << std::endl;
        // 可以选择在这里 exit(-1) 或者继续运行
    } else {
        std::cout << "模型加载成功!" << std::endl;
    }

    initCameraParams(); 
}

void AutoAimSystem::initCameraParams() {
    // 假设是 1280x720 的视频，估算内参
    double fx = 1000.0, fy = 1000.0;
    double cx = 640.0, cy = 360.0;
    cameraMatrix = (cv::Mat_<double>(3, 3) << fx, 0, cx, 0, fy, cy, 0, 0, 1);
    distCoeffs = cv::Mat::zeros(5, 1, CV_64F); 
}

void AutoAimSystem::solvePnP(Armor& armor) {
    // 定义装甲板的世界坐标 (单位：mm)
    float halfX = (armor.type == ArmorType::SMALL) ? 135.0f / 2.0f : 230.0f / 2.0f;
    float halfY = 55.0f / 2.0f;

    std::vector<cv::Point3f> objPoints;
    // 世界坐标系顺序：TL, TR, BR, BL
    objPoints.push_back(cv::Point3f(-halfX, -halfY, 0)); // TL
    objPoints.push_back(cv::Point3f(halfX, -halfY, 0));  // TR
    objPoints.push_back(cv::Point3f(halfX, halfY, 0));   // BR
    objPoints.push_back(cv::Point3f(-halfX, halfY, 0));  // BL

    // 图像点处理
    std::vector<cv::Point2f> imgPoints;
    for(int i=0; i<4; i++) imgPoints.push_back(armor.points[i]);

    // 对图像点进行简单的 X 轴排序，然后区分上下
    // 这一步是为了确保 imgPoints 的顺序也是 TL, TR, BR, BL
    std::sort(imgPoints.begin(), imgPoints.end(), [](const cv::Point2f& a, const cv::Point2f& b){ 
        return a.x < b.x; 
    });

    // 此时 imgPoints[0], imgPoints[1] 是左边的点；imgPoints[2], imgPoints[3] 是右边的点
    if (imgPoints[0].y > imgPoints[1].y) std::swap(imgPoints[0], imgPoints[1]); // 确保 [0] 是 TL
    if (imgPoints[2].y > imgPoints[3].y) std::swap(imgPoints[2], imgPoints[3]); // 确保 [2] 是 TR
    
    // 现在的顺序是: TL(0), BL(1), TR(2), BR(3) -> 需要调整为 TL, TR, BR, BL
    // 目标: [0]=TL, [1]=TR, [2]=BR, [3]=BL
    std::vector<cv::Point2f> sortedPoints(4);
    sortedPoints[0] = imgPoints[0]; // TL
    sortedPoints[1] = imgPoints[2]; // TR
    sortedPoints[2] = imgPoints[3]; // BR
    sortedPoints[3] = imgPoints[1]; // BL

    cv::Mat rvec, tvec;
    bool success = cv::solvePnP(objPoints, sortedPoints, cameraMatrix, distCoeffs, rvec, tvec);

    if (success) {
        armor.distance = (float)tvec.at<double>(2, 0); // Z轴距离
    }
}

void AutoAimSystem::run(const std::string& videoPath) {
    cv::VideoCapture cap(videoPath);
    if (!cap.isOpened()) {
        std::cerr << "Cannot open video: " << videoPath << std::endl;
        return;
    }

    cv::Mat frame;
    while (true) {
        cap >> frame;
        if (frame.empty()) break;

        // 1. 检测与匹配
        std::vector<cv::RotatedRect> lights = detector.detect(frame);
        std::vector<Armor> armors = matcher.match(lights);

        // 2. 识别与绘制
        for (auto& armor : armors) {
            // 识别数字
            int number = recognizer.recognize(frame, armor);
            
            // PnP 测距
            solvePnP(armor);

            // 绘制装甲板轮廓
            for (int i = 0; i < 4; i++) {
                cv::line(frame, 
                         (cv::Point)armor.points[i], 
                         (cv::Point)armor.points[(i + 1) % 4], 
                         cv::Scalar(0, 255, 0), 2);
            }

            // --- 新增/修改：准备显示信息 ---
            std::string numText = "Num: " + (number == -1 ? "?" : std::to_string(number));
            std::string distText = "Dist: " + std::to_string((int)armor.distance) + "mm";
            
            // 根据 armor.type 转换为字符串
            std::string typeStr;
            if (armor.type == ArmorType::SMALL) typeStr = "Type: SMALL";
            else if (armor.type == ArmorType::LARGE) typeStr = "Type: LARGE";
            else typeStr = "Type: UNKNOWN";

            // --- 绘制信息到画面 ---
            // 1. 显示数字 (上方)
            cv::putText(frame, numText, (cv::Point)armor.rect.center + cv::Point(0, -25), 
                        cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 255, 255), 2);
            
            // 2. 显示距离 (中间)
            cv::putText(frame, distText, (cv::Point)armor.rect.center + cv::Point(0, 0), 
                        cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 255, 255), 2);
            
            // 3. 显示类型 (下方)
            cv::putText(frame, typeStr, (cv::Point)armor.rect.center + cv::Point(0, 25), 
                        cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 100, 0), 2); // 用蓝色/橙色区分
        }

        cv::imshow("AutoAim System", frame);
        if (cv::waitKey(1) == 27) break;
    }
}