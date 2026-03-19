#include "../../include/astra/ESP32S3_HAL.h"
#include "esp_rom_sys.h"
#include "esp_log.h"
#include <string.h>
#include <cmath>

static const char* TAG = "ESP32S3_HAL";

// oled引脚定义
#define SPI_HOST    SPI2_HOST
#define PIN_NUM_CS  GPIO_NUM_16
#define PIN_NUM_DC  GPIO_NUM_15
#define PIN_NUM_RES GPIO_NUM_4
#define PIN_NUM_SCK GPIO_NUM_12
#define PIN_NUM_MOSI GPIO_NUM_11

// 按键引脚定义
const gpio_num_t ESP32S3_HAL::KEY_PINS[4] = {
    GPIO_NUM_39,    // UP
    GPIO_NUM_35,    // DOWN
    GPIO_NUM_37,    // ENTER
    GPIO_NUM_20     // BACK
};

// 逻辑按键映射
const uint8_t ESP32S3_HAL::LOGICAL_TO_PHYSICAL[4] = {0, 1, 2, 3};

// 全局SPI句柄
static spi_device_handle_t g_spi_handle = nullptr;

// U8G2 回调函数
extern "C" uint8_t u8x8_gpio_and_delay_esp32(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
extern "C" uint8_t u8x8_byte_esp32_hw_spi(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);

ESP32S3_HAL::ESP32S3_HAL() {
    // 初始化系统配置
    config.screenWeight = 128;
    config.screenHeight = 64;
    config.screenBright = 255;
}

void ESP32S3_HAL::init() {
    static bool initialized = false;
    
    if (initialized) {
        ESP_LOGI(TAG, "HAL already initialized, skipping...");
        return;
    }
    
    ESP_LOGI(TAG, "=== ESP32S3_HAL Initialization Start ===");
    
    initGPIO();
    initSPI();
    initU8G2();
    
    initialized = true;
    
    ESP_LOGI(TAG, "=== ESP32S3_HAL Initialization Complete ===");
}

void ESP32S3_HAL::initGPIO() {
    static bool gpio_initialized = false;
    
    if (gpio_initialized) {
        ESP_LOGI(TAG, "GPIO already initialized, skipping...");
        return;
    }
    
    ESP_LOGI(TAG, "Initializing GPIO...");
    
    // 配置GPIO
    gpio_config_t io_conf = {};
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    
    // DC引脚
    io_conf.pin_bit_mask = (1ULL << PIN_NUM_DC);
    gpio_config(&io_conf);
    gpio_set_level(PIN_NUM_DC, 1);
    
    // RESET引脚
    io_conf.pin_bit_mask = (1ULL << PIN_NUM_RES);
    gpio_config(&io_conf);
    gpio_set_level(PIN_NUM_RES, 1);
    
    // CS引脚
    io_conf.pin_bit_mask = (1ULL << PIN_NUM_CS);
    gpio_config(&io_conf);
    gpio_set_level(PIN_NUM_CS, 1);
    
    // 执行硬件复位
    ESP_LOGI(TAG, "Performing hardware reset...");
    gpio_set_level(PIN_NUM_RES, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(PIN_NUM_RES, 1);
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // 初始化按键引脚
    for (int i = 0; i < 4; i++) {
        gpio_reset_pin(KEY_PINS[i]);
        gpio_set_direction(KEY_PINS[i], GPIO_MODE_INPUT);
        gpio_pullup_en(KEY_PINS[i]);
    }
    
    gpio_initialized = true;
    ESP_LOGI(TAG, "GPIO initialization complete");
}

void ESP32S3_HAL::initSPI() {
    static bool spi_initialized = false;
    
    if (spi_initialized) {
        ESP_LOGI(TAG, "SPI already initialized, skipping...");
        return;
    }
    
    ESP_LOGI(TAG, "Initializing SPI...");
    
    spi_bus_config_t buscfg;
    memset(&buscfg, 0, sizeof(buscfg));
    buscfg.miso_io_num = -1;
    buscfg.mosi_io_num = PIN_NUM_MOSI;
    buscfg.sclk_io_num = PIN_NUM_SCK;
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    buscfg.max_transfer_sz = 4096;
    
    spi_device_interface_config_t devcfg;
    memset(&devcfg, 0, sizeof(devcfg));
    devcfg.clock_speed_hz = 8 * 1000 * 1000;
    devcfg.mode = 0;
    devcfg.spics_io_num = PIN_NUM_CS;
    devcfg.queue_size = 7;
    devcfg.flags = 0;
    devcfg.pre_cb = nullptr;
    devcfg.post_cb = nullptr;
    
    ESP_ERROR_CHECK(spi_bus_initialize(SPI_HOST, &buscfg, SPI_DMA_CH_AUTO));
    ESP_ERROR_CHECK(spi_bus_add_device(SPI_HOST, &devcfg, &spi));
    
    g_spi_handle = spi;
    spi_initialized = true;
    
    ESP_LOGI(TAG, "SPI initialized at 32MHz, mode 0");
}

void ESP32S3_HAL::initU8G2() {
    ESP_LOGI(TAG, "Initializing U8G2...");
    
    // 使用与工作代码完全相同的初始化函数
    u8g2_Setup_sh1106_128x64_noname_f(
        &u8g2,
        U8G2_R0,
        u8x8_byte_esp32_hw_spi,
        u8x8_gpio_and_delay_esp32
    );
    
    u8g2_InitDisplay(&u8g2);
    u8g2_SetPowerSave(&u8g2, 0);
    u8g2_SetDrawColor(&u8g2, 1);
    u8g2_SetBitmapMode(&u8g2, 0);
    u8g2_ClearBuffer(&u8g2);
    
    ESP_LOGI(TAG, "U8G2 initialized");
}

// 图形函数实现（保持不变）
void* ESP32S3_HAL::_getCanvasBuffer() {
    return u8g2_GetBufferPtr(&u8g2);
}

uint8_t ESP32S3_HAL::_getBufferTileHeight() {
    return u8g2_GetBufferTileHeight(&u8g2);
}

uint8_t ESP32S3_HAL::_getBufferTileWidth() {
    return u8g2_GetBufferTileWidth(&u8g2);
}

void ESP32S3_HAL::_canvasUpdate() {
    u8g2_SendBuffer(&u8g2);
}

void ESP32S3_HAL::_canvasClear() {
    u8g2_ClearBuffer(&u8g2);
}

void ESP32S3_HAL::_setFont(const uint8_t *_font) {
    u8g2_SetFont(&u8g2, _font);
}

uint8_t ESP32S3_HAL::_getFontWidth(std::string &_text) {
    return u8g2_GetUTF8Width(&u8g2, _text.c_str());
}

uint8_t ESP32S3_HAL::_getFontHeight() {
    return u8g2_GetAscent(&u8g2) - u8g2_GetDescent(&u8g2);
}

void ESP32S3_HAL::_setDrawType(uint8_t _type) {
    u8g2_SetDrawColor(&u8g2, _type);
    //u8g2_SetBitmapMode(&u8g2, 0);
}

void ESP32S3_HAL::_drawPixel(float _x, float _y) {
    u8g2_DrawPixel(&u8g2, static_cast<uint16_t>(std::round(_x)), static_cast<uint16_t>(std::round(_y)));
}

void ESP32S3_HAL::_drawEnglish(float _x, float _y, const std::string &_text) {
    u8g2_DrawStr(&u8g2, static_cast<uint16_t>(std::round(_x)), static_cast<uint16_t>(std::round(_y)), _text.c_str());
}

void ESP32S3_HAL::_drawChinese(float _x, float _y, const std::string &_text) {
    u8g2_DrawUTF8(&u8g2, static_cast<uint16_t>(std::round(_x)), static_cast<uint16_t>(std::round(_y)), _text.c_str());
}

void ESP32S3_HAL::_drawVDottedLine(float _x, float _y, float _h) {
    uint16_t x = static_cast<uint16_t>(std::round(_x));
    uint16_t y = static_cast<uint16_t>(std::round(_y));
    uint16_t h = static_cast<uint16_t>(std::round(_h));
    
    for (uint16_t i = 0; i < h; i += 2) {
        u8g2_DrawPixel(&u8g2, x, y + i);
    }
}

void ESP32S3_HAL::_drawHDottedLine(float _x, float _y, float _l) {
    uint16_t x = static_cast<uint16_t>(std::round(_x));
    uint16_t y = static_cast<uint16_t>(std::round(_y));
    uint16_t l = static_cast<uint16_t>(std::round(_l));
    
    for (uint16_t i = 0; i < l; i += 2) {
        u8g2_DrawPixel(&u8g2, x + i, y);
    }
}

void ESP32S3_HAL::_drawVLine(float _x, float _y, float _h) {
    u8g2_DrawVLine(&u8g2, 
        static_cast<uint16_t>(std::round(_x)),
        static_cast<uint16_t>(std::round(_y)),
        static_cast<uint16_t>(std::round(_h))
    );
}

void ESP32S3_HAL::_drawHLine(float _x, float _y, float _l) {
    u8g2_DrawHLine(&u8g2,
        static_cast<uint16_t>(std::round(_x)),
        static_cast<uint16_t>(std::round(_y)),
        static_cast<uint16_t>(std::round(_l))
    );
}

void ESP32S3_HAL::_drawBMP(float _x, float _y, float _w, float _h, const uint8_t *_bitMap) {
    u8g2_DrawXBM(&u8g2,
        static_cast<uint16_t>(std::round(_x)),
        static_cast<uint16_t>(std::round(_y)),
        static_cast<uint16_t>(std::round(_w)),
        static_cast<uint16_t>(std::round(_h)),
        _bitMap
    );
}

void ESP32S3_HAL::_drawBox(float _x, float _y, float _w, float _h) {
    u8g2_DrawBox(&u8g2,
        static_cast<uint16_t>(std::round(_x)),
        static_cast<uint16_t>(std::round(_y)),
        static_cast<uint16_t>(std::round(_w)),
        static_cast<uint16_t>(std::round(_h))
    );
}

void ESP32S3_HAL::_drawRBox(float _x, float _y, float _w, float _h, float _r) {
    u8g2_DrawRBox(&u8g2,
        static_cast<uint16_t>(std::round(_x)),
        static_cast<uint16_t>(std::round(_y)),
        static_cast<uint16_t>(std::round(_w)),
        static_cast<uint16_t>(std::round(_h)),
        static_cast<uint16_t>(std::round(_r))
    );
}

void ESP32S3_HAL::_drawCircle(float _x0, float _y0, float _r) {
    u8g2_DrawCircle(&u8g2,
        static_cast<uint16_t>(std::round(_x0)),
        static_cast<uint16_t>(std::round(_y0)),
        static_cast<uint16_t>(std::round(_r)),
    0);
}

void ESP32S3_HAL::_drawDisc(float _x0, float _y0, float _r) {
    u8g2_DrawDisc(&u8g2,
        static_cast<uint16_t>(std::round(_x0)),
        static_cast<uint16_t>(std::round(_y0)),
        static_cast<uint16_t>(std::round(_r)),
        0);
}

void ESP32S3_HAL::_drawFrame(float _x, float _y, float _w, float _h) {
    u8g2_DrawFrame(&u8g2,
        static_cast<uint16_t>(std::round(_x)),
        static_cast<uint16_t>(std::round(_y)),
        static_cast<uint16_t>(std::round(_w)),
        static_cast<uint16_t>(std::round(_h))
    );
}

void ESP32S3_HAL::_drawRFrame(float _x, float _y, float _w, float _h, float _r) {
    u8g2_DrawRFrame(&u8g2,
        static_cast<uint16_t>(std::round(_x)),
        static_cast<uint16_t>(std::round(_y)),
        static_cast<uint16_t>(std::round(_w)),
        static_cast<uint16_t>(std::round(_h)),
        static_cast<uint16_t>(std::round(_r))
    );
}

// 系统函数实现
void ESP32S3_HAL::_delay(uint32_t _mill) {
    vTaskDelay(pdMS_TO_TICKS(_mill));
}

uint32_t ESP32S3_HAL::_millis() {
    return static_cast<uint32_t>(esp_timer_get_time() / 1000);
}

uint32_t ESP32S3_HAL::_getTick() {
    return static_cast<uint32_t>(esp_timer_get_time());
}

uint32_t ESP32S3_HAL::_getRandomSeed() {
    return esp_random();
}

void ESP32S3_HAL::_screenOn() {
    u8g2_SetPowerSave(&u8g2, 0);
}

void ESP32S3_HAL::_screenOff() {
    u8g2_SetPowerSave(&u8g2, 1);
}

// 按键扫描辅助函数
void ESP32S3_HAL::scanPhysicalKeys() {
    uint64_t now = esp_timer_get_time() / 1000; // 转换为毫秒
    
    for (int i = 0; i < 4; i++) {
        bool current_state = (gpio_get_level(KEY_PINS[i]) == 0); // 低电平为按下
        
        if (current_state != key_states[i]) {
            if (current_state) {
                // 按键按下
                key_press_time[i] = now;
                key_states[i] = true;
            } else {
                // 按键释放
                key_states[i] = false;
                key_press_time[i] = 0;
            }
        }
    }
}

bool ESP32S3_HAL::isKeyPressed(int physical_key) {
    if (physical_key < 0 || physical_key >= 4) return false;
    
    if (!key_states[physical_key]) return false;
    
    uint64_t now = esp_timer_get_time() / 1000;
    uint64_t press_duration = now - key_press_time[physical_key];
    
    // 去抖动
    if (press_duration < DEBOUNCE_TIME) {
        return false;
    }
    
    return true;
}

key::KEY_INDEX ESP32S3_HAL::mapPhysicalToLogical(int physical_key) {
    switch (physical_key) {
        case 0: return key::KEY_UP;
        case 1: return key::KEY_DOWN;
        case 2: return key::KEY_ENTER;
        case 3: return key::KEY_BACK;
        default: return key::KEY_UP;
    }
}

// HAL 按键函数实现
bool ESP32S3_HAL::_getKey(key::KEY_INDEX _keyIndex) {
    int phys = -1;
    for (int i = 0; i < 4; i++) {
        if (mapPhysicalToLogical(i) == _keyIndex) {
            phys = i;
            break;
        }
    }
    if (phys == -1) return false;
    return isKeyPressed(phys);   // 返回去抖动后的稳定状态
}

bool ESP32S3_HAL::_getAnyKey() {
    for (int i = 0; i < 4; i++) {
        if (isKeyPressed(i)) return true;
    }
    return false;
}

void ESP32S3_HAL::_keyScan() {
    // 扫描物理按键，更新实时状态（利用原有的 scanPhysicalKeys）
    scanPhysicalKeys();

    for (int i = 0; i < 4; i++) {
        bool current = isKeyPressed(i);          // 去抖动后的稳定按下状态

        // 边缘检测
        if (current != last_debounced_state[i]) {
            if (current) {                      // 按下
                press_start_time[i] = esp_timer_get_time() / 1000;
                long_press_triggered[i] = false;
            } else {                            // 释放
                if (!long_press_triggered[i]) { // 没有触发过长按 → 短按
                    key::KEY_INDEX logical = mapPhysicalToLogical(i);
                    key[logical] = key::CLICK;
                    keyFlag = key::KEY_PRESSED;
                }
            }
            last_debounced_state[i] = current;
        } else {
            // 长按检测
            if (current && !long_press_triggered[i]) {
                uint64_t now = esp_timer_get_time() / 1000;
                if (now - press_start_time[i] >= LONG_PRESS_TIME) {
                    key::KEY_INDEX logical = mapPhysicalToLogical(i);
                    key[logical] = key::PRESS;
                    keyFlag = key::KEY_PRESSED;
                    long_press_triggered[i] = true;
                }
            }
        }
    }
}

void ESP32S3_HAL::_keyTest() {
    // 调用父类的_keyTest
    HAL::_keyTest();
}

extern "C" {

// u8g2 的 ESP32 GPIO 和延时函数
extern "C" uint8_t u8x8_gpio_and_delay_esp32(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    switch (msg) {
        case U8X8_MSG_DELAY_MILLI:
            vTaskDelay(arg_int / portTICK_PERIOD_MS);
            break;
        case U8X8_MSG_DELAY_10MICRO:
            esp_rom_delay_us(10);
            break;
        case U8X8_MSG_DELAY_100NANO:
            esp_rom_delay_us(1);
            break;
        case U8X8_MSG_GPIO_RESET:
            gpio_set_level(PIN_NUM_RES, arg_int);
            break;
        default:
            u8x8_SetGPIOResult(u8x8, 1);
            break;
    }
    return 1;
}

// u8g2 的 ESP32 硬件 SPI 函数
extern "C" uint8_t u8x8_byte_esp32_hw_spi(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    switch (msg) {
        case U8X8_MSG_BYTE_SEND: {
            spi_transaction_t t;
            memset(&t, 0, sizeof(t));
            t.length = arg_int * 8;
            t.tx_buffer = arg_ptr;
            spi_device_transmit(g_spi_handle, &t);
            break;
        }
        case U8X8_MSG_BYTE_INIT:
            // SPI初始化已在外部完成
            break;
        case U8X8_MSG_BYTE_SET_DC:
            gpio_set_level(PIN_NUM_DC, arg_int);
            break;
        case U8X8_MSG_BYTE_START_TRANSFER:
            spi_device_acquire_bus(g_spi_handle, portMAX_DELAY);
            gpio_set_level(PIN_NUM_CS, 0);
            break;
        case U8X8_MSG_BYTE_END_TRANSFER:
            gpio_set_level(PIN_NUM_CS, 1);
            spi_device_release_bus(g_spi_handle);
            break;
        default:
            return 0;
    }
    return 1;
}


} // extern "C"