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
  constexpr int16_t pixelSize = 6;
  constexpr char cat[][33] = {
    "    ####                ####    ",
    "   #pppp#              #pppp#   ",
    "  #pppppp#            #pppppp#  ",
    "  #ppppppp############ppppppp#  ",
    "  #pppppwwwwwwwwwwwwwwwwppppp#  ",
    "  #pppwwwwwwwwwwwwwwwwwwwwppp#  ",
    "  #wwwwwwwwwwwwwwwwwwwwwwwwww#  ",
    " #wwwwwwwwwwwwwwwwwwwwwwwwwwww# ",
    " #www#######wwwwwwww#######www# ",
    " #ww#wwwwwww#wwwwww#wwwwwww#ww# ",
    " ###wwwwwwwww######wwwwwwwww### ",
    " #w#www##wwww#wwww#wwww##www#w# ",
    " #w#www##wwww#wwww#wwww##www#w# ",
    " #ww#ww##www#wwwwww#www##ww#ww# ",
    " #wwr###w###wwwwwwww###w###rww# ",
    " #wrrrwwwwwwwwwwwwwwwwwwwwrrrw# ",
    " #wrrrwwwwwww#wwww#wwwwwwwrrrw# ",
    "  #wwwwwwwwwww####wwwwwwwwwww#  ",
    "   #wwwwwwwwwwwwwwwwwwwwwwww#   ",
    "    ##wwwwwwwwwwwwwwwwwwww##    ",
    "      ####################      ",
  };

  const uint16_t outline = tft.color565(29, 24, 46);
  const uint16_t fur = tft.color565(255, 244, 222);
  const uint16_t ears = tft.color565(255, 175, 194);
  const uint16_t cheeks = tft.color565(245, 116, 156);

  tft.fillScreen(tft.color565(0, 91, 255));
  constexpr int16_t columns = sizeof(cat[0]) - 1;
  constexpr int16_t rows = sizeof(cat) / sizeof(cat[0]);
  const int16_t left = (tft.width() - columns * pixelSize) / 2;
  const int16_t top = (tft.height() - rows * pixelSize) / 2;

  tft.startWrite();
  for (int16_t y = 0; y < rows; ++y) {
    for (int16_t x = 0; x < columns;) {
      const char pixel = cat[y][x];
      int16_t end = x + 1;
      while (end < columns && cat[y][end] == pixel) ++end;

      if (pixel != ' ') {
        uint16_t color = outline;
        if (pixel == 'w') color = fur;
        if (pixel == 'p') color = ears;
        if (pixel == 'r') color = cheeks;
        tft.writeFillRect(left + x * pixelSize, top + y * pixelSize,
                          (end - x) * pixelSize, pixelSize, color);
      }
      x = end;
    }
  }
  tft.endWrite();

  // Replace only the eye pixels inside the glasses.
  auto drawEyes = [&](bool open) {
    tft.startWrite();
    constexpr int16_t eyeColumns[] = {7, 23};
    for (const int16_t eyeColumn : eyeColumns) {
      const int16_t x = left + eyeColumn * pixelSize;
      const int16_t y = top + 11 * pixelSize;
      tft.writeFillRect(x, y, 2 * pixelSize, 3 * pixelSize, fur);
      tft.writeFillRect(x, open ? y : y + pixelSize,
                        2 * pixelSize, (open ? 3 : 1) * pixelSize, outline);
    }
    tft.endWrite();
  };

  constexpr uint16_t blinkDuration = 120;
  delay(1100);
  drawEyes(false);
  delay(blinkDuration);
  drawEyes(true);
  delay(1480);
  drawEyes(false);
  delay(blinkDuration);
  drawEyes(true);
  delay(1180);
}

Adafruit_GFX& displayGraphics() {
  return tft;
}
