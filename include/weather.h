#pragma once

class Adafruit_GFX;
void loadWeather(Adafruit_GFX& screen, int localMinute);
void updateWeather(Adafruit_GFX& screen, bool online, bool timeSynced, int localMinute);
