#include "weather.h"
#include "constants.h"
#include "screens.h"
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <stdlib.h>
#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

namespace {
struct WeatherResult {
  char temperature[8];
  const char* condition;
  WeatherIcon icon;
};

QueueHandle_t weatherResults = nullptr;

class WeatherBodyReader {
 public:
  explicit WeatherBodyReader(WiFiClient& client)
      : client_(client), started_(millis()) {}

  int read() {
    while (millis() - started_ < 10000UL) {
      if (client_.available() > 0) return client_.read();
      if (!client_.connected()) return -1;
      delay(5);
    }
    timedOut_ = true;
    return -1;
  }

  bool timedOut() const { return timedOut_; }

 private:
  WiFiClient& client_;
  uint32_t started_;
  bool timedOut_ = false;
};

// Грозу и снег проверяем первыми, поскольку в описании могут быть осадки сразу нескольких типов.
WeatherIcon pickWeatherIcon(const char* description) {
  String name(description);
  name.trim();
  name.toLowerCase();
  if (name.indexOf("thunder") >= 0) return WeatherIcon::Thunder;
  if (name.indexOf("snow") >= 0 || name.indexOf("blizzard") >= 0 ||
      name.indexOf("sleet") >= 0 || name.indexOf("ice pellets") >= 0) {
    return WeatherIcon::Snow;
  }
  if (name.indexOf("rain") >= 0 || name.indexOf("drizzle") >= 0 ||
      name.indexOf("shower") >= 0) return WeatherIcon::Rain;
  if (name.indexOf("fog") >= 0 || name.indexOf("mist") >= 0) return WeatherIcon::Fog;
  if (name == "partly cloudy") return WeatherIcon::PartlyCloudy;
  if (name == "cloudy" || name == "overcast") return WeatherIcon::Cloud;
  if (name == "sunny" || name == "clear") return WeatherIcon::Sun;
  return WeatherIcon::None;
}

WeatherResult fetchWeather() {
  WeatherResult result{};
  // Для проверки срока сертификата нужны настроенные системные часы.
  WiFiClientSecure client;
  client.useBuiltinCACertBundle();
  client.setHandshakeTimeout(10);

  HTTPClient http;
  // HTTP/1.0 позволяет читать ответ из потока без разметки chunked.
  http.useHTTP10(true);
  http.setConnectTimeout(8000);
  http.setTimeout(3000);
  const char* url = "https://wttr.in/Moscow?m&lang=en&format=%25t%7C%25C%7C";

  Serial.println("Weather: requesting wttr.in for Moscow.");
  int status = 0;
  bool valid = false;
  if (http.begin(client, url)) {
    status = http.GET();
    Serial.printf("Weather GET returned %d.\n", status);
    if (status == HTTPC_ERROR_CONNECTION_REFUSED) {
      // Уточняем причину сбоя подключения.
      char detail[160]{};
      const int clientError = client.lastError(detail, sizeof(detail));
      IPAddress address;
      Serial.println("Weather: checking DNS.");
      const bool dnsOk = WiFi.hostByName("wttr.in", address) == 1;
      if (dnsOk) {
        // Проверяем, доступен ли порт HTTPS без запуска TLS.
        WiFiClient probe;
        const bool tcpOk = probe.connect(address, 443, 4000) == 1;
        probe.stop();
        Serial.printf("Weather TCP 443: %s\n", tcpOk ? "OK" : "failed");
      }
      Serial.printf("Weather DNS: %s; secure client: %d (%s)\n",
                    dnsOk ? "OK" : "failed", clientError, detail);
    }
    if (status == HTTP_CODE_OK) {
      // Формат ответа: +2°C|Clear|. Последний разделитель отмечает конец строки.
      Serial.println("Weather: reading compact response.");
      WeatherBodyReader body(http.getStream());
      char line[96]{};
      size_t length = 0;
      int separators = 0;
      while (length < sizeof(line) - 1 && separators < 2) {
        const int byte = body.read();
        if (byte < 0) break;
        line[length++] = static_cast<char>(byte);
        if (byte == '|') ++separators;
      }

      char* units = nullptr;
      const long celsius = strtol(line, &units, 10);
      char* description = strchr(line, '|');
      if (separators == 2 && description && units != line &&
          celsius >= -100 && celsius <= 100 &&
          strcmp(units, "") != 0 &&
          (strncmp(units, "\xC2\xB0" "C|", 4) == 0 || strncmp(units, "C|", 2) == 0) &&
          description[1] != '|') {
        line[length - 1] = '\0';
        snprintf(result.temperature, sizeof(result.temperature), "%+ld", celsius);
        result.condition = WeatherTranslations::translate(description + 1);
        result.icon = pickWeatherIcon(description + 1);
        Serial.printf("Weather updated: %s C, %s\n", result.temperature, description + 1);
        valid = true;
      } else if (body.timedOut()) {
        Serial.printf("Weather: body timed out after 10 seconds (%u bytes).\n",
                      static_cast<unsigned>(length));
      } else {
        Serial.printf("Weather: invalid compact response (%u bytes).\n",
                      static_cast<unsigned>(length));
      }
    }

    http.end();
  }

  if (!valid) {
    Serial.printf("Weather update failed (HTTP %d).\n", status);
  }
  return result;
}

void requestWeather(void*) {
  const WeatherResult result = fetchWeather();
  xQueueSend(weatherResults, &result, 0);
  vTaskDelete(nullptr);
}
}

void updateWeather(Adafruit_GFX& screen, bool online, bool clockReady) {
  static bool requestRunning = false;
  static uint32_t lastRequest = 0;
  static uint32_t interval = 0;

  // Только основной цикл рисует на дисплее.
  WeatherResult result{};
  if (requestRunning && xQueueReceive(weatherResults, &result, 0) == pdTRUE) {
    requestRunning = false;
    lastRequest = millis();
    interval = result.temperature[0] ? 15UL * 60 * 1000 : 60UL * 1000;
    if (result.temperature[0]) showWeatherLine(screen, result.temperature, result.condition, result.icon);
  }

  if (requestRunning || !online || !clockReady || millis() - lastRequest < interval) return;

  // Запрос работает в отдельной задаче, чтобы часы продолжали обновляться.
  if (!weatherResults) weatherResults = xQueueCreate(1, sizeof(WeatherResult));
  if (weatherResults &&
      xTaskCreate(requestWeather, "weather", 16384, nullptr, 1, nullptr) == pdPASS) {
    requestRunning = true;
  } else {
    Serial.println("Weather: could not start background task.");
    lastRequest = millis();
    interval = 60UL * 1000;
  }
}
