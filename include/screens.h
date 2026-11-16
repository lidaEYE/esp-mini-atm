#pragma once

class Adafruit_GFX;
enum class ScreenTheme { Classic, Modern };
ScreenTheme screenTheme();
bool setScreenTheme(ScreenTheme theme);
void showClockScreen(Adafruit_GFX& screen);
void updateClockScreen(Adafruit_GFX& screen, const char* clockText, const char* dateText, const char* weekday = "");

enum class WeatherIcon { None, Sun, PartlyCloudy, Cloud, Rain, Snow, Fog, Thunder };

void showWeatherLine(Adafruit_GFX& screen, const char* temperature, const char* condition,
                     WeatherIcon icon = WeatherIcon::None);
