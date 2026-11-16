#include "display.h"
#include "config.h"

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

namespace {
SPIClass displaySPI(FSPI);
Adafruit_ST7789 tft(&displaySPI, TFT_CS, TFT_DC, TFT_RST);
}

bool initDisplay() {
  if (!displaySPI.begin(TFT_SCLK, -1, TFT_MOSI, -1)) {
    return false;
  }

  tft.init(SCREEN_WIDTH, SCREEN_HEIGHT, TFT_SPI_MODE);
  tft.setRotation(2);
  tft.invertDisplay(true);
  return true;
}

void showWelcomeScreen() {
  constexpr uint8_t textSize = 5;
  constexpr int16_t lineGap = 8;
  const char* const lines[] = {"OZON", "BANK"};

  tft.fillScreen(tft.color565(0, 91, 255));
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(textSize);
  tft.setTextWrap(false);

  int16_t left[2], top[2];
  uint16_t width[2], height[2];
  for (uint8_t i = 0; i < 2; ++i) {
    tft.getTextBounds(lines[i], 0, 0, &left[i], &top[i], &width[i], &height[i]);
  }

  const int16_t blockHeight = height[0] + lineGap + height[1];
  int16_t y = (tft.height() - blockHeight) / 2;
  for (uint8_t i = 0; i < 2; ++i) {
    const int16_t x = (tft.width() - static_cast<int16_t>(width[i])) / 2;
    tft.setCursor(x - left[i], y - top[i]);
    tft.print(lines[i]);
    y += height[i] + lineGap;
  }
}

Adafruit_GFX& displayGraphics() {
  return tft;
}
