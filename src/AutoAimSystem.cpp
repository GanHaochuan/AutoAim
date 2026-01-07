#include "AutoAimSystem.h"
#include <opencv2/opencv.hpp>
#include <iostream>

AutoAimSystem::AutoAimSystem() {
    detector.setEnemyColor(1); // 0-blue, 1-red
}

/**
 * @brief 自动瞄准系统主运行函数
 * 
 * @param[in] videoPath 视频文件路径
 * 
 * @note 支持常见的视频格式（如.mp4）
 *       窗口名称为"AutoAim - LightBars"
 *       按ESC键可退出程序
 */
void AutoAimSystem::run(const std::string& videoPath) {
    // 1st： 打开视频文件
    cv::VideoCapture cap(videoPath);

    if (!cap.isOpened()) {
        std::cerr << "Failed to open video: " << videoPath << std::endl;
        return;
    }

    cv::Mat frame; //储存当前帧

    // 2nd: 逐帧处理视频
    while (true) {
        cap >> frame;
        if (frame.empty()) break;

        // 3rd: 检测灯条
        auto lights = detector.detect(frame); // 返回所有检测到的灯条矩形列表

        // 4th: 绘制检测结果
        for (auto& rect : lights) {
            cv::Point2f pts[4]; // 储存矩形四个顶点
            rect.points(pts); // 获取矩形顶点坐标
            // 绘制矩形
            for (int i = 0; i < 4; i++) {
                cv::line(frame, pts[i], pts[(i + 1) % 4], cv::Scalar(0, 255, 0), 2); // 绿色线条、宽度2
            }
        }

        // 5th: 显示结果
        cv::imshow("AutoAim - LightBars", frame);

        // 6th: 处理退出按键
        if (cv::waitKey(1) == 27) break;
    }
}
