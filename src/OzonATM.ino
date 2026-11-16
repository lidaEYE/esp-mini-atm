#include "control_panel.h"
#include "display.h"
#include "screens.h"
#include "weather.h"

#include <sys/time.h>
#include <time.h>
#include <WiFi.h>
#include <esp_sntp.h>
#include "secrets.h"

namespace {
bool displayReady = false;
bool clockSet = false;

bool ntpReceived = false;
uint32_t lastNetworkReport = 0;
constexpr time_t firstValidNetworkTime = 1704067200;  // 1 января 2024 года
int lastMinuteOfDay = -1;
int lastDate = -1;
constexpr char timeZone[] = "MSK-3";  // Московское время (UTC+3)

bool setClock(const char* input) {
  tm requested{};
  char extra;
  if (sscanf(input, "%4d-%2d-%2d %2d:%2d:%2d %c",
             &requested.tm_year, &requested.tm_mon, &requested.tm_mday,
             &requested.tm_hour, &requested.tm_min, &requested.tm_sec,
             &extra) != 6 ||
      requested.tm_year < 2000 || requested.tm_year > 2099) {
    return false;
  }

  requested.tm_year -= 1900;
  --requested.tm_mon;
  tm normalized = requested;
  const time_t seconds = mktime(&normalized);
  if (seconds == -1 || normalized.tm_year != requested.tm_year ||
      normalized.tm_mon != requested.tm_mon || normalized.tm_mday != requested.tm_mday ||
      normalized.tm_hour != requested.tm_hour || normalized.tm_min != requested.tm_min ||
      normalized.tm_sec != requested.tm_sec) {
    return false;
  }

  timeval value{};
  value.tv_sec = seconds;
  return settimeofday(&value, nullptr) == 0;
}

void readClockInput() {
  static char input[20];
  static size_t length = 0;
  static bool overflow = false;
  while (Serial.available()) {
    const char ch = Serial.read();
    if (ch == '\r' || ch == '\n') {
      if (length || overflow) {
        input[length] = '\0';
        if (!overflow && setClock(input)) {
          clockSet = true;
          Serial.println("Clock set.");
        } else {
          Serial.println("Use YYYY-MM-DD HH:MM:SS");
        }
        length = 0;
        overflow = false;
      }
    } else if (length < sizeof(input) - 1) {
      input[length++] = ch;
    } else {
      overflow = true;
    }
  }
}
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  displayReady = initDisplay();
  if (!displayReady) {
    Serial.println("OzonATM: ESP32 SPI initialization failed.");
    return;
  }

  loadScreenTheme();

  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(wifiName, wifiPassword);
  configTzTime(timeZone, "pool.ntp.org", "time.google.com");
  showWelcomeScreen();

  const time_t now = time(nullptr);
  tm current{};
  const int localMinute = now >= firstValidNetworkTime && localtime_r(&now, &current)
      ? current.tm_hour * 60 + current.tm_min : -1;
  loadWeather(displayGraphics(), localMinute);
  showClockScreen(displayGraphics());
  Serial.println("Set local time: YYYY-MM-DD HH:MM:SS");
}

void loop() {
  if (!displayReady) {
    delay(100);
    return;
  }


  const bool wifiConnected = WiFi.status() == WL_CONNECTED;
  if (updateControlPanel(wifiConnected)) {
    showClockScreen(displayGraphics());
    lastMinuteOfDay = -1;
    lastDate = -1;
  }
  if (!ntpReceived &&
      sntp_get_sync_status() == SNTP_SYNC_STATUS_COMPLETED) {
    ntpReceived = true;
    Serial.println("NTP synchronized.");
  }

  // Печатаем состояние раз в пять секунд, пока ждём время от сервера.
  if (!ntpReceived && millis() - lastNetworkReport >= 5000) {
    lastNetworkReport = millis();
    Serial.println(wifiConnected ? "Wi-Fi connected. Waiting for NTP."
                                 : "Wi-Fi not connected.");
  }

  readClockInput();
  const time_t now = time(nullptr);
  // Рисуем время после синхронизации или ручной настройки.
  const bool clockReady = clockSet || now >= firstValidNetworkTime;
  int localMinute = -1;
  if (clockReady) {
    tm current{};
    localtime_r(&now, &current);
    localMinute = current.tm_hour * 60 + current.tm_min;
    const int date = current.tm_year * 366 + current.tm_yday;
    char clockText[6];
    char dateText[11];
    static const char* const weekdays[] = {"ВС", "ПН", "ВТ", "СР", "ЧТ", "ПТ", "СБ"};
    const bool minuteChanged = localMinute != lastMinuteOfDay;
    const bool dateChanged = date != lastDate;
    if (minuteChanged || dateChanged) {
      strftime(clockText, sizeof(clockText), "%H:%M", &current);
      strftime(dateText, sizeof(dateText), "%d.%m.%Y", &current);
      updateClockScreen(displayGraphics(), minuteChanged ? clockText : nullptr,
                        dateChanged ? dateText : nullptr, weekdays[current.tm_wday]);
      lastMinuteOfDay = localMinute;
      lastDate = date;
    }
  }
  // Первый запрос погоды выполняем после подтверждения синхронизации NTP.
  updateWeather(displayGraphics(), wifiConnected, ntpReceived && now >= firstValidNetworkTime,
                localMinute);
  delay(20);
}
