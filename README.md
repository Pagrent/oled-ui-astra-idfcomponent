## powered by astra UI.

# astra UI 的 ESP-IDF 组件移植版

本项目是 [AstraThreshold/oled-ui-astra](https://github.com/AstraThreshold/oled-ui-astra) 的**移植版本**。原项目是一个基于C++、适用于STM32平台的流畅且易扩展的OLED UI框架。

本组件已将核心UI框架**成功移植为 ESP-IDF 组件**，使得在 **ESP32** 系列芯片上可以方便地使用该框架构建OLED用户界面。该组件仅在ESP32-S3芯片上测试

## ✨ 主要特点
*   **源自优秀项目**：继承了原项目`oled-ui-astra`的平滑动画和易用架构。
*   **ESP-IDF 组件化**：符合ESP-IDF组件标准，易于集成到你的ESP32项目中。
*   **跨平台延续**：让原STM32框架的优秀设计在ESP32生态中得以延续。

## 📌 致谢
感谢 [AstraThreshold](https://github.com/AstraThreshold) 开发的原始项目。

## 🚀 快速开始
下载整个仓库，然后直接放到ESP-IDF框架的components文件夹里即可

您可能需要修改/src/astra/ESP32S3_HAL.cpp内的按键引脚定义和oled引脚定义，使其符合您的物理连接。

## 📄 许可证
继承原项目的许可证（根据原仓库信息为 [LICENSE](LICENSE)）。

这个readme是ai写的。
