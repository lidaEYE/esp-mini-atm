#pragma once

#include <Arduino.h>
#include <SPI.h>

constexpr int8_t TFT_SCLK = 12;
constexpr int8_t TFT_MOSI = 11;
constexpr int8_t TFT_RST = 10;
constexpr int8_t TFT_DC = 9;
constexpr int8_t TFT_CS = -1;  // На разъёме дисплея нет контакта CS.
constexpr uint16_t SCREEN_WIDTH = 240;
constexpr uint16_t SCREEN_HEIGHT = 240;
constexpr uint8_t TFT_SPI_MODE = SPI_MODE3;

// Координаты города для погоды. По умолчанию Москва.
constexpr double weatherLatitude = 55.7558;
constexpr double weatherLongitude = 37.6176;
