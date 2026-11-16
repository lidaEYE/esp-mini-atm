#pragma once

#include <Arduino.h>

class WeatherTranslations {
  struct WeatherTranslation {
    const char* english;
    const char* russian;
  };

 public:
  static const char* translate(const char* description) {
    // Описания wttr.in и подписи для экрана.
    static constexpr WeatherTranslation translations[] = {
      {"Sunny", "ЯСНО"}, {"Clear", "ЯСНО"},
      {"Partly Cloudy", "ЧАСТИЧНАЯ ОБЛАЧНОСТЬ"},
      {"Cloudy", "ОБЛАЧНО"}, {"Overcast", "ПАСМУРНО"},
      {"Mist", "ДЫМКА"}, {"Fog", "ТУМАН"}, {"Freezing fog", "ТУМАН"},
      {"Patchy rain nearby", "МЕСТАМИ ДОЖДЬ"},
      {"Patchy rain possible", "ВОЗМОЖЕН ДОЖДЬ"},
      {"Light drizzle", "МОРОСЬ"}, {"Patchy light drizzle", "МОРОСЬ"},
      {"Light rain", "СЛАБЫЙ ДОЖДЬ"}, {"Patchy light rain", "МЕСТАМИ ДОЖДЬ"},
      {"Moderate rain", "ДОЖДЬ"}, {"Heavy rain", "СИЛЬНЫЙ ДОЖДЬ"},
      {"Moderate rain at times", "ВРЕМЕНАМИ ДОЖДЬ"},
      {"Heavy rain at times", "ВРЕМЕНАМИ ЛИВЕНЬ"},
      {"Light rain shower", "НЕБОЛЬШОЙ ЛИВЕНЬ"},
      {"Moderate or heavy rain shower", "ЛИВЕНЬ"},
      {"Torrential rain shower", "СИЛЬНЫЙ ЛИВЕНЬ"},
      {"Patchy snow nearby", "МЕСТАМИ СНЕГ"},
      {"Patchy snow possible", "ВОЗМОЖЕН СНЕГ"},
      {"Patchy light snow", "МЕСТАМИ СНЕГ"},
      {"Light snow", "СЛАБЫЙ СНЕГ"}, {"Moderate snow", "СНЕГ"},
      {"Patchy moderate snow", "МЕСТАМИ СНЕГ"},
      {"Heavy snow", "СИЛЬНЫЙ СНЕГ"}, {"Patchy heavy snow", "МЕСТАМИ СНЕГОПАД"},
      {"Light snow showers", "НЕБОЛЬШОЙ СНЕГ"},
      {"Moderate or heavy snow showers", "СНЕГОПАД"},
      {"Blowing snow", "ПОЗЁМКА"}, {"Blizzard", "МЕТЕЛЬ"},
      {"Sleet", "МОКРЫЙ СНЕГ"}, {"Light sleet", "МОКРЫЙ СНЕГ"},
      {"Moderate or heavy sleet", "МОКРЫЙ СНЕГ"},
      {"Patchy sleet possible", "ВОЗМОЖЕН МОКРЫЙ СНЕГ"},
      {"Light sleet showers", "МОКРЫЙ СНЕГ"},
      {"Moderate or heavy sleet showers", "МОКРЫЙ СНЕГ"},
      {"Ice pellets", "ЛЕДЯНАЯ КРУПА"},
      {"Light showers of ice pellets", "ЛЕДЯНАЯ КРУПА"},
      {"Moderate or heavy showers of ice pellets", "ЛЕДЯНАЯ КРУПА"},
      {"Patchy freezing drizzle possible", "ЛЕДЯНАЯ МОРОСЬ"},
      {"Freezing drizzle", "ЛЕДЯНАЯ МОРОСЬ"},
      {"Heavy freezing drizzle", "ЛЕДЯНАЯ МОРОСЬ"},
      {"Light freezing rain", "ЛЕДЯНОЙ ДОЖДЬ"},
      {"Moderate or heavy freezing rain", "ЛЕДЯНОЙ ДОЖДЬ"},
      {"Thundery outbreaks possible", "ВОЗМОЖНА ГРОЗА"},
      {"Thundery outbreaks in nearby", "ГРОЗА ПОБЛИЗОСТИ"},
      {"Patchy light rain with thunder", "ДОЖДЬ С ГРОЗОЙ"},
      {"Moderate or heavy rain with thunder", "ЛИВЕНЬ С ГРОЗОЙ"},
      {"Patchy light snow with thunder", "СНЕГ С ГРОЗОЙ"},
      {"Moderate or heavy snow with thunder", "СНЕГ С ГРОЗОЙ"},
    };

    String name(description);
    name.trim();
    for (const auto& entry : translations) {
      if (name.equalsIgnoreCase(entry.english)) return entry.russian;
    }
    return "НЕТ ОПИСАНИЯ";
  }
};
