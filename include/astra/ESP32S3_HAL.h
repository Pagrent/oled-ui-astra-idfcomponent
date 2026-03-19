#pragma once
#ifndef ESP32S3_HAL_H
#define ESP32S3_HAL_H

#include "hal.h"
#include "u8g2.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_random.h"

class ESP32S3_HAL : public HAL {
private:
    u8g2_t u8g2;
    spi_device_handle_t spi;
    
    // 按键状态
    bool key_states[4] = {false, false, false, false};
    uint64_t key_press_time[4] = {0, 0, 0, 0};
    
    // 按键映射
    static const gpio_num_t KEY_PINS[4];
    
    // 按键去抖动时间（ms）
    static const uint32_t DEBOUNCE_TIME = 20;
    
    // 长按时间（ms）
    static const uint32_t LONG_PRESS_TIME = 1000;

    // 四个逻辑按键对应的物理引脚索引
    static const uint8_t LOGICAL_TO_PHYSICAL[4];

    // 去抖动后的稳定状态
    bool debounced_state[4];
    bool last_debounced_state[4];
    uint64_t press_start_time[4];
    bool long_press_triggered[4];
    
public:
    ESP32S3_HAL();
    ~ESP32S3_HAL() override = default;
    
    // 初始化函数
    void init() override;
    
    // 图形相关函数
    void* _getCanvasBuffer() override;
    uint8_t _getBufferTileHeight() override;
    uint8_t _getBufferTileWidth() override;
    void _canvasUpdate() override;
    void _canvasClear() override;
    void _setFont(const uint8_t *_font) override;
    uint8_t _getFontWidth(std::string &_text) override;
    uint8_t _getFontHeight() override;
    void _setDrawType(uint8_t _type) override;
    void _drawPixel(float _x, float _y) override;
    void _drawEnglish(float _x, float _y, const std::string &_text) override;
    void _drawChinese(float _x, float _y, const std::string &_text) override;
    void _drawVDottedLine(float _x, float _y, float _h) override;
    void _drawHDottedLine(float _x, float _y, float _l) override;
    void _drawVLine(float _x, float _y, float _h) override;
    void _drawHLine(float _x, float _y, float _l) override;
    void _drawBMP(float _x, float _y, float _w, float _h, const uint8_t *_bitMap) override;
    void _drawBox(float _x, float _y, float _w, float _h) override;
    void _drawRBox(float _x, float _y, float _w, float _h, float _r) override;
    void _drawCircle(float _x0, float _y0, float _r) override;
    void _drawDisc(float _x0, float _y0, float _r) override;
    void _drawFrame(float _x, float _y, float _w, float _h) override;
    void _drawRFrame(float _x, float _y, float _w, float _h, float _r) override;
    
    // 系统函数
    void _delay(uint32_t _mill) override;
    uint32_t _millis() override;
    uint32_t _getTick() override;
    uint32_t _getRandomSeed() override;
    
    // 屏幕控制
    void _screenOn() override;
    void _screenOff() override;
    
    // 按键函数
    bool _getKey(key::KEY_INDEX _keyIndex) override;
    bool _getAnyKey() override;
    void _keyScan() override;
    void _keyTest() override;
    
private:
    // SPI 初始化
    void initSPI();
    
    // u8g2 初始化
    void initU8G2();
    
    // GPIO 初始化
    void initGPIO();
    
    // 按键扫描辅助函数
    void scanPhysicalKeys();
    bool isKeyPressed(int physical_key);
    
    // 映射物理按键到逻辑按键
    key::KEY_INDEX mapPhysicalToLogical(int physical_key);
};

#endif // ESP32S3_HAL_H