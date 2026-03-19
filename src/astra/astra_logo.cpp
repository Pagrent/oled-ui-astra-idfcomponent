//
// Created by Fir on 2024/3/21 021.
//
#include <cmath>
#include <random>
#include "../../include/astra/astra_logo.h"

namespace astra {

// 辅助动画函数
inline void animate(float &pos, float posTrg, float speed) {
    if (pos != posTrg) {
        if (std::fabs(pos - posTrg) < 0.15f) pos = posTrg;
        else pos += (posTrg - pos) / ((100 - speed) / 1.0f);
    }
}

// ==================== LogoPlayer 实现 ====================
LogoPlayer::LogoPlayer(uint16_t duration)
    : totalDuration(duration), currentTime(0), isInit(false),
      yTitle(0), yTitleTrg(0), yCopyRight(0), yCopyRightTrg(0),
      yBackGround(0), yBackGroundTrg(0), xBackGround(0),
      text("astra UI"), copyRight("powered by") {
    // 构造函数不执行具体初始化，星星等放在 update 中首次执行
}

bool LogoPlayer::update() {
    // 如果动画已经结束（currentTime >= totalDuration 且背景已退场），返回 false
    if (currentTime >= totalDuration && yBackGround == -HAL::getSystemConfig().screenHeight - 1) {
        return false;
    }

    // 首次调用时初始化星星和静态坐标
    if (!isInit) {
        yStars.clear();
        yStarsTrg.clear();
        xStars.clear();

        // 设置随机种子（使用 HAL 提供的随机种子）
        srand(HAL::getRandomSeed() * 7);

        for (unsigned char i = 0; i < getUIConfig().logoStarNum; i++) {
            yStars.push_back(0 - getUIConfig().logoStarLength - 1);
            // 产生从1到screenHeight的随机数
            yStarsTrg.push_back(1 + rand() % (uint16_t)(HAL::getSystemConfig().screenHeight - 2 * getUIConfig().logoStarLength - 2 + 1));
            // 产生从1到screenWeight的随机数
            xStars.push_back(1 + rand() % (uint16_t)(HAL::getSystemConfig().screenWeight - 2 * getUIConfig().logoStarLength - 2 + 1));
        }

        // 初始化背景位置（从屏幕外上方）
        yBackGround = -HAL::getSystemConfig().screenHeight - 1;
        yBackGroundTrg = 0;

        // 标题初始位置（屏幕外上方）
        yTitle = -getUIConfig().logoTextHeight - 1;
        yCopyRight = -getUIConfig().logoCopyRightHeight - 1;

        isInit = true;
    }

    // 更新 currentTime（帧计数）
    currentTime++;

    // 根据当前时间设置目标值
    if (currentTime < totalDuration) {
        yBackGroundTrg = 0;
        yTitleTrg = HAL::getSystemConfig().screenHeight / 2 - getUIConfig().logoTextHeight / 2;
        yCopyRightTrg = yTitleTrg - getUIConfig().logoCopyRightHeight - 4;
        // 星星目标已经初始化，保持不变
    } else {
        // 退场阶段
        yBackGroundTrg = -HAL::getSystemConfig().screenHeight - 1;
        for (auto &val : yStarsTrg) val = -getUIConfig().logoStarLength - 1;
        yTitleTrg = -getUIConfig().logoTextHeight - 1;
        yCopyRightTrg = -getUIConfig().logoCopyRightHeight - 1;
    }

    // 开始绘制一帧
    HAL::canvasClear();

    // 遮罩
    HAL::setDrawType(0);
    HAL::drawBox(xBackGround, yBackGround, HAL::getSystemConfig().screenWeight, HAL::getSystemConfig().screenHeight);
    animate(yBackGround, yBackGroundTrg, getUIConfig().logoAnimationSpeed);
    HAL::setDrawType(1);
    HAL::drawHLine(0, yBackGround + HAL::getSystemConfig().screenHeight, HAL::getSystemConfig().screenWeight);

    // 星星
    for (unsigned char i = 0; i < getUIConfig().logoStarNum; i++) {
        HAL::drawHLine(xStars[i] - getUIConfig().logoStarLength - 1, yStars[i], getUIConfig().logoStarLength);
        HAL::drawHLine(xStars[i] + 2, yStars[i], getUIConfig().logoStarLength);
        HAL::drawVLine(xStars[i], yStars[i] - getUIConfig().logoStarLength - 1, getUIConfig().logoStarLength);
        HAL::drawVLine(xStars[i], yStars[i] + 2, getUIConfig().logoStarLength);
        animate(yStars[i], yStarsTrg[i], getUIConfig().logoAnimationSpeed);
    }

    // 标题文字（每次绘制前计算居中坐标，因为字体可能变化）
    HAL::setFont(getUIConfig().logoTitleFont);
    xTitle = (HAL::getSystemConfig().screenWeight - HAL::getFontWidth(text)) / 2;
    HAL::drawEnglish(xTitle, yTitle + getUIConfig().logoTextHeight, text);

    // 版权文字
    HAL::setFont(getUIConfig().logoCopyRightFont);
    xCopyRight = (HAL::getSystemConfig().screenWeight - HAL::getFontWidth(copyRight)) / 2;
    HAL::drawEnglish(xCopyRight, yCopyRight + getUIConfig().logoCopyRightHeight, copyRight);

    // 标题动画
    animate(yTitle, yTitleTrg, getUIConfig().logoAnimationSpeed);
    animate(yCopyRight, yCopyRightTrg, getUIConfig().logoAnimationSpeed);

    HAL::canvasUpdate();

    return true; // 动画仍在进行
}

// ==================== 保留原阻塞函数（基于 LogoPlayer 实现） ====================
void drawLogo(uint16_t _time) {
    LogoPlayer player(_time);
    while (player.update()) {
        // 空循环，update 已经绘制并更新内部状态
        // 注意：这里没有延时，动画速度由帧计数控制，与原来一致
        // 如果需要控制帧率，可加入 HAL::delay(10)，但原代码没有，所以保持原样
    }
}

void LogoPlayer::reset(uint16_t newDuration) {
    if (newDuration > 0) totalDuration = newDuration;
    currentTime = 0;
    isInit = false;   // 下次 update 会重新初始化星星
    yBackGround = -HAL::getSystemConfig().screenHeight - 1;
    yBackGroundTrg = 0;
    yTitle = -getUIConfig().logoTextHeight - 1;
    yCopyRight = -getUIConfig().logoCopyRightHeight - 1;
    // 其他坐标会在 update 中重新计算，不需要额外处理
}

} // namespace astra