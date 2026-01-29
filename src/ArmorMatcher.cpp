#include "ArmorMatcher.h"
#include <cmath>
#include <algorithm>

ArmorMatcher::ArmorMatcher() {}

/**
 * @brief 判断灯条对是否有效的私有方法
 */
bool ArmorMatcher::isValidPair(const cv::RotatedRect& l1, const cv::RotatedRect& l2) {
    // 条件一：角度差异不超过 10 度 (两灯条应平行)
    float angle1 = l1.size.width > l1.size.height ? l1.angle + 90 : l1.angle;
    float angle2 = l2.size.width > l2.size.height ? l2.angle + 90 : l2.angle;
    float angleDiff = std::abs(angle1 - angle2);
    if (angleDiff > 10) return false;

    // 条件二：高度比例
    float h1 = std::max(l1.size.height, l1.size.width);
    float h2 = std::max(l2.size.height, l2.size.width);
    float heightRatio = h1 / h2;
    if (heightRatio < 0.6 || heightRatio > 1.66) return false; // 0.6 ~ 1.6

    // 条件三：垂直位置对齐
    float yDiff = std::abs(l1.center.y - l2.center.y);
    float avgH = (h1 + h2) / 2.0;
    if (yDiff > avgH * 0.5) return false;

    return true;
}

/**
 * @brief 装甲板匹配主函数
 */
std::vector<Armor> ArmorMatcher::match(const std::vector<cv::RotatedRect>& lights) {
    std::vector<Armor> armors;
    if (lights.size() < 2) return armors;

    struct MatchPair {
        size_t i, j;
        float score;
        Armor armor;
    };
    std::vector<MatchPair> candidates;

    for (size_t i = 0; i < lights.size(); ++i) {
        for (size_t j = i + 1; j < lights.size(); ++j) {
            if (!isValidPair(lights[i], lights[j])) continue;

            // 计算基础数据
            float h1 = std::max(lights[i].size.height, lights[i].size.width);
            float h2 = std::max(lights[j].size.height, lights[j].size.width);
            float avgH = (h1 + h2) / 2.0;
            
            // 计算灯条间距
            float dist = cv::norm(lights[i].center - lights[j].center);
            
            // 核心改进：大小装甲板区分 (加分项)
            // 比例 = 灯条间距 / 灯条高度
            // 小装甲板比例通常在 2.0 ~ 3.2
            // 大装甲板比例通常在 3.5 ~ 5.5
            float ratio = dist / avgH;
            
            ArmorType type = ArmorType::UNKNOWN;
            if (ratio > 1.8 && ratio < 3.2) type = ArmorType::SMALL;
            else if (ratio >= 3.2 && ratio < 5.5) type = ArmorType::LARGE;
            else continue; // 比例不符合装甲板特征，跳过

            // 评分计算
            float angleDiff = std::abs(lights[i].angle - lights[j].angle);
            float yDiff = std::abs(lights[i].center.y - lights[j].center.y);
            float score = (1.0 - angleDiff / 15.0) + (1.0 - yDiff / avgH); // 简化的评分

            // 构建装甲板对象
            Armor armor;
            armor.type = type;
            
            // 获取四个点用于后续 PnP 和透视变换
            cv::Point2f pts_i[4], pts_j[4];
            lights[i].points(pts_i);
            lights[j].points(pts_j);
            
            // 简单组合所有点求外接矩形
            std::vector<cv::Point2f> allPts;
            for(int k=0; k<4; k++) { allPts.push_back(pts_i[k]); allPts.push_back(pts_j[k]); }
            armor.rect = cv::minAreaRect(allPts);
            armor.boundingRect = armor.rect.boundingRect();
            
            // 存储四个顶点 (这里需要暂存，具体顺序在 Recognition 中排序)
            // 这里简单取外接矩形的四个点，更精确的做法是取两灯条的顶点
            armor.rect.points(armor.points);

            candidates.push_back({i, j, score, armor});
        }
    }

    // 按分数排序并去重
    std::sort(candidates.begin(), candidates.end(), 
              [](const MatchPair& a, const MatchPair& b) { return a.score > b.score; });

    std::vector<bool> used(lights.size(), false);
    for (const auto& candidate : candidates) {
        if (used[candidate.i] || used[candidate.j]) continue;
        used[candidate.i] = used[candidate.j] = true;
        armors.push_back(candidate.armor);
    }

    return armors;
}