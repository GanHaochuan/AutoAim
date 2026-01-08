#include "ArmorMatcher.h"
#include <cmath>

ArmorMatcher::ArmorMatcher() {}

/**
 * @brief 判断灯条对是否有效的私有方法
 * 
 * @param[in] l1 第一个灯条
 * @param[in] l2 第二个灯条
 * @return bool 是否构成有效装甲板对
 * 
 */
bool ArmorMatcher::isValidPair(const cv::RotatedRect& l1, const cv::RotatedRect& l2) {
    // 条件一：角度差异不超过15度
    float angleDiff = std::abs(l1.angle - l2.angle);
    if (angleDiff > 15) return false;

    // 条件二：高度比例在0.5到2.0之间
    float h1 = std::max(l1.size.height, l1.size.width);
    float h2 = std::max(l2.size.height, l2.size.width);
    float heightRatio = h1 / h2;
    if (heightRatio < 0.5 || heightRatio > 2.0) return false;

    // 条件三：中心点y坐标差异不超过最大高度的一半（垂直对齐）
    float yDiff = std::abs(l1.center.y - l2.center.y);
    if (yDiff > std::max(h1, h2) * 0.5) return false;

    // 条件四：中心点x坐标差异在最大高度的0.5到5倍之间（水平间距）
    float xDiff = std::abs(l1.center.x - l2.center.x);
    if (xDiff < std::max(h1, h2) * 0.5 || xDiff > std::max(h1, h2) * 5.0) return false;

    return true;
}

/**
 * @brief 装甲板匹配主函数
 * 
 * @param[in] lights 输入灯条集合
 * @return 匹配成功的装甲板集合
 * 
 */
std::vector<Armor> ArmorMatcher::match(const std::vector<cv::RotatedRect>& lights) {
    std::vector<Armor> armors; // 存储匹配到的装甲板

    // 遍历所有可能的灯条对
    for (size_t i = 0; i < lights.size(); ++i) {
        for (size_t j = i + 1; j < lights.size(); ++j) {
            const auto& l1 = lights[i];
            const auto& l2 = lights[j];

            if (!isValidPair(l1, l2)) continue;

            std::vector<cv::Point2f> pts; // 存储两个灯条的所有顶点（共8个点）

            cv::Point2f p1[4], p2[4]; // 分别获取两个灯条的4个顶点
            l1.points(p1);
            l2.points(p2);

            // 将两个灯条的顶点加入到点集
            for (int k = 0; k < 4; k++) {
                pts.push_back(p1[k]);
                pts.push_back(p2[k]);
            }

            // 计算最小外接矩形作为装甲板区域
            cv::RotatedRect armorRect = cv::minAreaRect(pts); // 最小外接矩形
            cv::Rect bound = armorRect.boundingRect(); // 轴对齐外接矩形（用于ROI提取）

            Armor armor;
            armor.rect = armorRect;
            armor.boundingRect = bound;

            armors.push_back(armor);
        }
    }

    return armors;
}
