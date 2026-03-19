//
// Created by Fir on 2024/3/21 021
//

#pragma once
#ifndef ASTRA_CORE_SRC_ASTRA_ASTRA_LOGO_H_
#define ASTRA_CORE_SRC_ASTRA_ASTRA_LOGO_H_

#include "../../include/astra/hal.h"
#include <vector>
#include <string>

namespace astra {

// 阻塞式启动动画（保持原有函数）
void drawLogo(uint16_t _time);

// 非阻塞动画播放器，用于页面内逐帧驱动
class LogoPlayer {
public:
    // 构造函数，duration 为动画总帧数（与原 drawLogo 的 _time 参数含义一致）
    LogoPlayer(uint16_t duration);
    
    // 每帧调用一次，绘制一帧动画，返回 true 表示动画仍在进行，false 表示结束
    bool update();

    void reset(uint16_t newDuration = 0);

    bool isPlaying() const { return currentTime < totalDuration; }

private:
    uint16_t totalDuration;
    uint16_t currentTime;
    bool isInit;

    // 星星坐标
    std::vector<float> yStars;
    std::vector<float> yStarsTrg;
    std::vector<float> xStars;

    // 文字坐标
    float yTitle, yTitleTrg;
    float yCopyRight, yCopyRightTrg;
    float yBackGround, yBackGroundTrg;
    float xBackGround;

    float xTitle, xCopyRight;   // 居中坐标，每次绘制前计算

    std::string text;
    std::string copyRight;

    // 内置动画函数（无需暴露）
    void animate(float &pos, float posTrg, float speed) {
        if (pos != posTrg) {
            if (std::fabs(pos - posTrg) < 0.15f) pos = posTrg;
            else pos += (posTrg - pos) / ((100 - speed) / 1.0f);
        }
    }
};

} // namespace astra

#endif //ASTRA_CORE_SRC_ASTRA_ASTRA_LOGO_H_