#include "control_panel.h"
#include "screens.h"

#include <Arduino.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <WiFi.h>
#include <esp_random.h>

namespace {
constexpr char hostname[] = "ozonatm";
WebServer server(80);
String token;

// Принимаем запросы по IP платы и её имени mDNS.
bool isLocalAddress() {
  const String address = WiFi.localIP().toString();
  const String host = server.hostHeader();
  const String name = String(hostname) + ".local";
  if (host == address || host == address + ":80" ||
      host.equalsIgnoreCase(name) || host.equalsIgnoreCase(name + ":80")) return true;
  server.send(400, "text/plain", "Open this page using the device IP address.");
  return false;
}

void showPanel() {
  if (!isLocalAddress()) return;
  String page = R"html(<!doctype html><html lang="ru"><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>OzonATM</title>
<style>
*{box-sizing:border-box}body{margin:0;padding:24px;background:#05218c;color:#fff;
font:16px system-ui,sans-serif}main{max-width:420px;margin:12vh auto}
h1{font-size:36px;margin:0 0 20px}section{background:#123bb2;border-radius:20px;padding:22px}
h2{margin:0 0 12px;font-size:20px}p{margin:0 0 18px;color:#e0e8ff}
form{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:8px}
button{border:0;border-radius:10px;padding:12px 4px;font:inherit;font-weight:600;
background:#e2ebff;color:#102879;cursor:pointer}button:disabled{background:#ffb9ec;cursor:default}
</style><main><h1>OzonATM</h1><section><h2>Тема экрана</h2><p>Сейчас включена )html";
  const bool modern = screenTheme() == ScreenTheme::Modern;
  page += modern ? "современная тема" : "классическая тема";
  page += R"html(.</p><form method="post" action="/theme"><input type="hidden" name="token" value=")html";
  page += token;
  page += R"html("><button name="theme" value="modern")html";
  if (modern) page += " disabled";
  page += R"html(>Современная</button><button name="theme" value="classic")html";
  if (!modern) page += " disabled";
  page += ">Классическая</button></form></section></main></html>";
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "text/html; charset=utf-8", page);
}
}

bool updateControlPanel(bool wifiConnected) {
  static bool registered = false;
  static bool running = false;
  static bool themeChanged = false;

  if (!wifiConnected) {
    if (running) {
      MDNS.end();
      server.stop();
    }
    running = false;
    return false;
  }
  if (!registered) {
    char nonce[17];
    snprintf(nonce, sizeof(nonce), "%08lx%08lx",
             static_cast<unsigned long>(esp_random()), static_cast<unsigned long>(esp_random()));
    token = nonce;
    server.on("/", HTTP_GET, showPanel);
    server.on("/theme", HTTP_POST, [] {
      if (!isLocalAddress()) return;
      if (server.arg("token") != token) {
        server.send(403, "text/plain", "Reload the control panel and try again.");
        return;
      }
      const String value = server.arg("theme");
      if (value != "modern" && value != "classic") {
        server.send(400, "text/plain", "Unknown theme.");
        return;
      }
      themeChanged = setScreenTheme(value == "modern" ? ScreenTheme::Modern : ScreenTheme::Classic);
      server.sendHeader("Location", "/");
      server.send(303, "text/plain", "");
    });
    registered = true;
  }
  if (!running) {
    server.begin();
    if (MDNS.begin(hostname)) {
      MDNS.addService("http", "tcp", 80);
      Serial.printf("OzonATM control panel: http://%s.local/\n", hostname);
    } else {
      Serial.println("OzonATM: mDNS startup failed. Use the IP address.");
    }
    running = true;
    Serial.print("OzonATM control panel: http://");
    Serial.println(WiFi.localIP());
  }
  themeChanged = false;
  server.handleClient();
  return themeChanged;
}
