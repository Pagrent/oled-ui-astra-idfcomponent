cmake_minimum_required(VERSION 3.16)

idf_component_register(
    SRCS
        "src/astra/astra_app.cpp"
        "src/astra/astra_logo.cpp"
        "src/astra/astra_rocket.cpp"
        "src/astra/camera.cpp"
        "src/astra/hal.cpp"
        "src/astra/launcher.cpp"
        "src/astra/menu.cpp"
        "src/astra/plugin.cpp"
        "src/astra/selector.cpp"
        "src/astra/widget.cpp"
        "src/astra/ESP32S3_HAL.cpp"
    
    INCLUDE_DIRS 
        "include"
        "src/astra"
    
    REQUIRES
        u8g2 freertos driver esp_timer)