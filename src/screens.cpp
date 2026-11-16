#include "screens.h"
#include "modern_font.h"
#include <Adafruit_GFX.h>
#include <Preferences.h>
#include <stdio.h>
#include <string.h>

namespace {
constexpr uint16_t Blue = 0x02DF;  // #005BFF в RGB565.
constexpr uint16_t Pink = 0xFC16;
constexpr uint16_t White = 0xFFFF;
constexpr uint16_t CardBackground = 0x119F;
constexpr uint16_t ModernBlue = 0x015B;
constexpr uint16_t Lime = 0x976B;

ScreenTheme activeTheme = ScreenTheme::Modern;
char lastTemperature[16] = "";
char lastCondition[64] = "ПОГОДА";
WeatherIcon lastIcon = WeatherIcon::None;

void renderWeather(Adafruit_GFX& screen, const char* temperature,
                   const char* condition, WeatherIcon icon);

// Глифы Century Gothic Bold для заголовка.
struct HeaderGlyph {
  char16_t letter;
  uint8_t advance;
  uint32_t rows[26];
};

constexpr HeaderGlyph headerFont[] = {
  {0x6f, 20, {
    0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
    0x000000, 0x000000, 0x000F80, 0x003FE0, 0x00FFF0, 0x00FFF8, 0x01F07C, 0x01E03C, 0x03C01E,
    0x03C01E, 0x03C01E, 0x03C01E, 0x01E03C, 0x01F07C, 0x00FFF8, 0x00FFF0, 0x003FE0,
  }},
  {0x7a, 14, {
    0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
    0x000000, 0x000000, 0x001FFE, 0x001FFE, 0x000FFE, 0x000FFE, 0x000780, 0x0007C0, 0x0003E0,
    0x0001E0, 0x0001F0, 0x0000F0, 0x0000F8, 0x000078, 0x001FFC, 0x001FFC, 0x001FFE,
  }},
  {0x6e, 19, {
    0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
    0x000000, 0x000000, 0x001E3C, 0x007FBC, 0x007FFC, 0x00FFFC, 0x00F8FC, 0x00F07C, 0x00F03C,
    0x00F03C, 0x00F03C, 0x00F03C, 0x00F03C, 0x00F03C, 0x00F03C, 0x00F03C, 0x00F03C,
  }},
  {0x431, 20, {
    0x000000, 0x000000, 0x000000, 0x000000, 0x01FE00, 0x01FFC0, 0x01FFF0, 0x01FFF8, 0x0001F8,
    0x00007C, 0x00003C, 0x001F1C, 0x007F9C, 0x00FFDE, 0x01FFFE, 0x03F0FE, 0x03E07E, 0x03C03E,
    0x03C03C, 0x03C01C, 0x03C03C, 0x03E03C, 0x01F0F8, 0x01FFF8, 0x00FFF0, 0x007FE0,
  }},
  {0x430, 20, {
    0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
    0x000000, 0x000000, 0x03C7C0, 0x03DFF0, 0x03FFF8, 0x03FFF8, 0x03F07C, 0x03E03C, 0x03C03C,
    0x03C01E, 0x03C01E, 0x03C03C, 0x03E03C, 0x03F07C, 0x03FFF8, 0x03FFF8, 0x03DFE0,
  }},
  {0x43d, 19, {
    0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
    0x000000, 0x000000, 0x00F03C, 0x00F03C, 0x00F03C, 0x00F03C, 0x00F03C, 0x00F03C, 0x00FFFC,
    0x00FFFC, 0x00FFFC, 0x00FFFC, 0x00F03C, 0x00F03C, 0x00F03C, 0x00F03C, 0x00F03C,
  }},
  {0x43a, 17, {
    0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000,
    0x000000, 0x000000, 0x00F03C, 0x00783C, 0x007C3C, 0x003E3C, 0x001F3C, 0x000FBC, 0x0007FC,
    0x0003FC, 0x0003FC, 0x0007FC, 0x000FFC, 0x001FBC, 0x003F3C, 0x003E3C, 0x007C3C,
  }},
};

void drawHeading(Adafruit_GFX& screen, const char16_t* text, int offset = 0) {
  constexpr int spaceWidth = 8;
  int width = 0;
  for (const char16_t* letter = text; *letter; ++letter) {
    if (*letter == u' ') width += spaceWidth;
    for (const auto& glyph : headerFont) {
      if (glyph.letter == *letter) width += glyph.advance;
    }
  }

  int x = (screen.width() - width) / 2 + offset;
  for (; *text; ++text) {
    if (*text == u' ') x += spaceWidth;
    for (const auto& glyph : headerFont) {
      if (glyph.letter != *text) continue;
      for (int row = 0; row < 26; ++row) {
        for (int col = 0; col < 24; ++col) {
          if (glyph.rows[row] & (1UL << col)) {
            screen.drawPixel(x + col, 22 + row, White);
          }
        }
      }
      x += glyph.advance;
      break;
    }
  }
}

// При размере шрифта 1 каждый символ занимает 6 пикселей по ширине, включая пробел до следующего символа.
void centered(Adafruit_GFX& screen, const char* text, int y, uint8_t size, uint16_t color, uint16_t background = Blue) {
  const int width = static_cast<int>(strlen(text)) * 6 * size;
  screen.setTextSize(size);
  screen.setTextColor(color, background);
  screen.setCursor((screen.width() - width) / 2, y);
  screen.print(text);
}

// Семь строк по пять пикселей для заглавных букв А–Я и Ё.
constexpr uint8_t cyrillic[][7] = {
  {14,17,17,31,17,17,17}, {31,16,16,30,17,17,30},
  {30,17,17,30,17,17,30}, {31,16,16,16,16,16,16},
  {14,10,10,10,10,31,17}, {31,16,16,30,16,16,31},
  {21,21,14,4,14,21,21}, {30,1,1,14,1,1,30},
  {17,17,19,21,25,17,17}, {10,4,17,19,21,25,17},
  {17,18,20,24,20,18,17}, {7,9,9,9,9,9,17},
  {17,27,21,21,17,17,17}, {17,17,17,31,17,17,17},
  {14,17,17,17,17,17,14}, {31,17,17,17,17,17,17},
  {30,17,17,30,16,16,16}, {14,17,16,16,16,17,14},
  {31,4,4,4,4,4,4}, {17,17,17,15,1,1,14},
  {4,14,21,21,14,4,4}, {17,17,10,4,10,17,17},
  {18,18,18,18,18,31,1}, {17,17,17,15,1,1,1},
  {21,21,21,21,21,21,31}, {21,21,21,21,21,31,1},
  {24,8,8,14,9,9,14}, {17,17,17,29,21,21,29},
  {16,16,16,30,17,17,30}, {14,17,1,7,1,17,14},
  {18,21,21,29,21,21,18}, {15,17,17,15,5,9,17},
  {10,0,31,16,30,16,31},
};

void drawSun(Adafruit_GFX& screen, int x, int y, uint16_t color) {
  screen.fillCircle(x, y, 5, color);
  screen.drawFastHLine(x - 11, y, 4, color);
  screen.drawFastHLine(x + 8, y, 4, color);
  screen.drawFastVLine(x, y - 11, 4, color);
  screen.drawFastVLine(x, y + 8, 4, color);
  for (int dx = -1; dx <= 1; dx += 2) {
    for (int dy = -1; dy <= 1; dy += 2) {
      screen.drawLine(x + dx * 6, y + dy * 6, x + dx * 8, y + dy * 8, color);
    }
  }
}

void drawCloud(Adafruit_GFX& screen, int x, int y) {
  screen.fillRoundRect(x - 13, y, 27, 9, 4, White);
  screen.fillCircle(x - 5, y + 1, 5, White);
  screen.fillCircle(x + 3, y, 7, White);
}

void drawWeatherIcon(Adafruit_GFX& screen, WeatherIcon icon, int x, int y, uint16_t sunColor, uint16_t background) {

  switch (icon) {
    case WeatherIcon::Sun:
      drawSun(screen, x, y, sunColor);
      break;
    case WeatherIcon::PartlyCloudy:
      drawSun(screen, x - 7, y - 5, sunColor);
      drawCloud(screen, x + 5, y);
      break;
    case WeatherIcon::Moon:
      screen.fillCircle(x, y, 10, White);
      screen.fillCircle(x + 5, y - 4, 8, background);
      break;
    case WeatherIcon::PartlyCloudyNight:
      screen.fillCircle(x - 7, y - 5, 9, White);
      screen.fillCircle(x - 3, y - 9, 7, background);
      drawCloud(screen, x + 5, y);
      break;
    case WeatherIcon::Cloud:
      drawCloud(screen, x, y - 2);
      break;
    case WeatherIcon::Rain:
      drawCloud(screen, x, y - 8);
      for (int dx = -6; dx <= 6; dx += 6) {
        screen.drawLine(x + dx + 1, y + 5, x + dx - 2, y + 12, 0x7DFF);
        screen.drawLine(x + dx + 2, y + 5, x + dx - 1, y + 12, 0x7DFF);
      }
      break;
    case WeatherIcon::Snow:
      drawCloud(screen, x, y - 8);
      for (int dx = -7; dx <= 7; dx += 7) {
        screen.drawFastHLine(x + dx - 2, y + 8, 5, White);
        screen.drawFastVLine(x + dx, y + 6, 5, White);
      }
      break;
    case WeatherIcon::Fog:
      screen.drawFastHLine(x - 11, y - 5, 22, White);
      screen.drawFastHLine(x - 14, y, 28, White);
      screen.drawFastHLine(x - 10, y + 5, 20, White);
      break;
    case WeatherIcon::Thunder:
      drawCloud(screen, x, y - 8);
      screen.fillTriangle(x + 1, y + 1, x - 5, y + 9, x + 5, y + 6, 0xFFE0);
      screen.fillTriangle(x + 5, y + 6, x - 2, y + 15, x, y + 7, 0xFFE0);
      break;
    case WeatherIcon::None:
      break;
  }
}

void centeredWeather(Adafruit_GFX& screen, const char* text, int y, uint8_t size, int centerX, int maxWidth,
                     uint16_t background) {
  int count = 0;
  for (const unsigned char* p = reinterpret_cast<const unsigned char*>(text); *p; ++p) {
    if ((*p & 0xC0) != 0x80) ++count;
  }
  if (count * 6 * size > maxWidth) size = 1;
  int x = centerX - count * 6 * size / 2;
  const unsigned char* p = reinterpret_cast<const unsigned char*>(text);
  while (*p) {
    uint16_t code = *p++;
    if (code >= 0xC0 && *p) {
      code = ((code & 0x1F) << 6) | (*p++ & 0x3F);
    }
    if ((code >= 0x410 && code <= 0x42F) || code == 0x401) {
      const uint8_t* rows = cyrillic[code == 0x401 ? 32 : code - 0x410];
      for (int row = 0; row < 7; ++row) {
        for (int col = 0; col < 5; ++col) {
          if (rows[row] & (16 >> col)) {
            screen.fillRect(x + col * size, y + row * size, size, size, White);
          }
        }
      }
    } else if (code == 0xB0) {
      screen.drawCircle(x + 2 * size, y + 2 * size, size, White);
    } else {
      screen.drawChar(x, y, code < 128 ? code : '?', White, background, size);
    }
    x += 6 * size;
  }
}

// Смешиваем белый цвет глифа с фоном в RGB565.
uint16_t smoothTextColor(uint16_t background, unsigned level) {
  const unsigned fade = 3 - level;
  const unsigned red = (((background >> 11) & 31) * fade + 31 * level) / 3;
  const unsigned green = (((background >> 5) & 63) * fade + 63 * level) / 3;
  const unsigned blue = ((background & 31) * fade + 31 * level) / 3;
  return (red << 11) | (green << 5) | blue;
}

template <size_t Count>
void drawSmoothText(Adafruit_GFX& screen, const char* text, int centerX, int y,
                    uint16_t background, const ModernFont::Glyph (&glyphs)[Count],
                    const uint8_t* pixels, int height) {
  // Первый проход измеряет строку, второй рисует её по центру.
  const int spaceWidth = height / 4;
  int width = 0;
  int x = 0;
  for (int pass = 0; pass < 2; ++pass) {
    const auto* next = reinterpret_cast<const unsigned char*>(text);
    while (*next) {
      uint16_t code = *next++;
      if (code >= 0xC0 && *next) code = ((code & 0x1F) << 6) | (*next++ & 0x3F);
      if (code == ' ') {
        x += spaceWidth;
        continue;
      }
      for (const auto& glyph : glyphs) {
        if (glyph.code != code) continue;
        if (pass == 1) {
          const int stride = (glyph.width + 3) / 4;
          for (int row = 0; row < height; ++row) {
            for (int col = 0; col < glyph.width; ++col) {
              const uint8_t packed = pixels[glyph.offset + row * stride + col / 4];
              const unsigned level = (packed >> (2 * (col % 4))) & 3;
              if (level) screen.drawPixel(x + col, y + row, smoothTextColor(background, level));
            }
          }
        }
        x += glyph.width;
        break;
      }
    }
    if (pass == 0) {
      width = x;
      x = centerX - width / 2;
    }
  }
}

void drawModernBackground(Adafruit_GFX& screen) {
  screen.fillScreen(ModernBlue);
  const int width = screen.width();
  const int height = screen.height();
  // Цветные углы формируем горизонтальными линиями.
  for (int row = 0; row < 42; ++row) {
    const int remaining = 42 - row;
    const int green = remaining * remaining / 30 + 1;
    const int pink = remaining * remaining / 22 + 1;
    screen.drawFastHLine(width - green, row, green, Lime);
    screen.drawFastHLine(0, height - 1 - row, pink, Pink);
  }

  // Сердце рядом с надписью и метка рядом с названием города.
  screen.fillCircle(34, 33, 6, White);
  screen.fillCircle(44, 33, 6, White);
  screen.fillTriangle(28, 35, 50, 35, 39, 47, White);
  drawHeading(screen, u"ozon банк", 18);
  screen.fillCircle(77, 210, 6, White);
  screen.fillTriangle(71, 211, 83, 211, 77, 222, White);
  screen.fillCircle(77, 210, 2, ModernBlue);
  drawSmoothText(screen, "МОСКВА", 128, 202, ModernBlue,
                 ModernFont::TextGlyphs, ModernFont::TextPixels, ModernFont::TextHeight);
}

void drawModernWeather(Adafruit_GFX& screen, const char* temperature,
                       const char* condition, WeatherIcon icon) {
  screen.fillRoundRect(18, 130, 204, 65, 15, CardBackground);
  screen.drawFastVLine(87, 143, 38, White);
  drawWeatherIcon(screen, icon, 52, 161, Pink, CardBackground);

  char degrees[16];
  snprintf(degrees, sizeof(degrees), "%s°C", temperature);
  drawSmoothText(screen, degrees, 154, 132, CardBackground,
                 ModernFont::TemperatureGlyphs, ModernFont::TemperaturePixels,
                 ModernFont::TemperatureHeight);

  const char* space = strchr(condition, ' ');
  if (strlen(condition) > 20 && space) {
    char firstLine[48];
    snprintf(firstLine, sizeof(firstLine), "%.*s", static_cast<int>(space - condition), condition);
    drawSmoothText(screen, firstLine, 154, 163, CardBackground,
                   ModernFont::DescriptionGlyphs, ModernFont::DescriptionPixels,
                   ModernFont::DescriptionHeight);
    drawSmoothText(screen, space + 1, 154, 178, CardBackground,
                   ModernFont::DescriptionGlyphs, ModernFont::DescriptionPixels,
                   ModernFont::DescriptionHeight);
  } else {
    drawSmoothText(screen, condition, 154, 168, CardBackground,
                   ModernFont::DescriptionGlyphs, ModernFont::DescriptionPixels,
                   ModernFont::DescriptionHeight);
  }
}
}

void showClockScreen(Adafruit_GFX& screen) {
  screen.setFont(nullptr);
  screen.setTextWrap(false);
  if (activeTheme == ScreenTheme::Modern) {
    drawModernBackground(screen);

    renderWeather(screen, lastTemperature, lastCondition, lastIcon);
    return;
  }
  screen.fillScreen(Blue);
  drawHeading(screen, u"ozon банк");
  screen.drawFastHLine(96, 148, 48, Pink);

  renderWeather(screen, lastTemperature, lastCondition, lastIcon);
}

void updateClockScreen(Adafruit_GFX& screen, const char* clockText, const char* dateText,
                       const char* weekday) {
  if (activeTheme == ScreenTheme::Modern) {
    if (clockText) {
      screen.fillRect(15, 51, 210, 55, ModernBlue);
      drawSmoothText(screen, clockText, screen.width() / 2, 51, ModernBlue,
                     ModernFont::ClockGlyphs, ModernFont::ClockPixels, ModernFont::ClockHeight);
    }
    if (dateText) {
      screen.fillRect(30, 106, 180, 18, ModernBlue);
      char dateLine[24];
      snprintf(dateLine, sizeof(dateLine), "%s  %s", dateText, weekday);
      drawSmoothText(screen, dateLine, screen.width() / 2, 106, ModernBlue,
                     ModernFont::TextGlyphs, ModernFont::TextPixels, ModernFont::TextHeight);
    }
    return;
  }
  if (clockText) {
    centered(screen, clockText, 91, 6, White);
  }
  if (dateText) {
    centered(screen, dateText, 169, 2, White);
  }
}

namespace {
void renderWeather(Adafruit_GFX& screen, const char* temperature, const char* condition,
                     WeatherIcon icon) {
  if (!temperature[0]) return;
  if (activeTheme == ScreenTheme::Modern) {
    drawModernWeather(screen, temperature, condition, icon);
    return;
  }
  const int centerX = screen.width() / 2;
  screen.fillRect(centerX - 24, 49, 48, 39, Blue);
  drawWeatherIcon(screen, icon, centerX, 69, 0xFFE0, Blue);
  // Город и температура сверху, описание отдельной строкой.
  char heading[48];
  snprintf(heading, sizeof(heading), "МОСКВА  %s°C", temperature);
  screen.fillRect(0, 190, screen.width(), 45, Blue);
  centeredWeather(screen, heading, 194, 2, centerX, screen.width() - 24, Blue);
  centeredWeather(screen, condition, 216, 2, centerX, screen.width() - 24, Blue);
}

}

void loadScreenTheme() {
  Preferences preferences;
  if (!preferences.begin("display", true)) return;

  // При первом запуске или неизвестном значении выбираем современную тему.
  const uint8_t saved = preferences.getUChar("theme", 1);
  activeTheme = saved == 0 ? ScreenTheme::Classic : ScreenTheme::Modern;
  preferences.end();
}

ScreenTheme screenTheme() {
  return activeTheme;
}

bool setScreenTheme(ScreenTheme theme) {
  if (activeTheme == theme) return false;
  activeTheme = theme;

  // Записываем только при смене темы, чтобы не делать лишних записей во flash.
  Preferences preferences;
  if (preferences.begin("display", false)) {
    const size_t written = preferences.putUChar("theme", theme == ScreenTheme::Classic ? 0 : 1);
    preferences.end();
    if (written == 1) return true;
  }
  Serial.println("OzonATM: could not save screen theme.");
  return true;
}

void showWeatherLine(Adafruit_GFX& screen, const char* temperature,
                     const char* condition, WeatherIcon icon, bool redraw) {
  snprintf(lastTemperature, sizeof(lastTemperature), "%s", temperature);
  snprintf(lastCondition, sizeof(lastCondition), "%s", condition);
  lastIcon = icon;
  if (redraw) renderWeather(screen, lastTemperature, lastCondition, lastIcon);
}
