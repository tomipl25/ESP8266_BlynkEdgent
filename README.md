# PiwnicaESP8266 Blynk.Edgent

Projekt dla platformy **ESP8266 (WeMos D1 R2 / NodeMCU)** oparty na **Blynk.Edgent**.

## 📖 Opis
Sterownik urządzenia „Piwnica” działający z aplikacją **Blynk 2.0**.  
Kod zawiera obsługę:
- trybu konfiguracji WiFi przez AP (`blynk.setup`),
- automatyczne połączenie z chmurą Blynk,
- aktualizacje OTA (Over-The-Air),
- diodę LED statusu i przycisk resetu,
- lokalną konsolę (`Serial` lub terminal w Blynk).

## ⚙️ Wymagania
- Płytka: **WeMos D1 R2 / NodeMCU ESP8266**
- Biblioteki:
  - Blynk (najnowsza)
  - ESP8266WiFi
  - Ticker
  - EEPROM
  - FS (SPIFFS / LittleFS)
  - Adafruit_NeoPixel *(opcjonalnie dla WS2812)*

## 🔧 Kompilacja
1. Otwórz `PiwnicaESP8266v100.ino` w **Arduino IDE**.
2. Wybierz płytkę: `LOLIN(WEMOS) D1 R2 & mini`.
3. Skonfiguruj `BLYNK_TEMPLATE_ID` i `BLYNK_TEMPLATE_NAME`.
4. Wgraj projekt na ESP8266.

## 🌐 Tryb konfiguracji
Po uruchomieniu, jeśli urządzenie nie ma zapisanych danych WiFi:
1. ESP uruchomi hotspot o nazwie np. `Blynk Piwnica-XXXX`.
2. Połącz się z tym WiFi.
3. Otwórz w przeglądarce `http://blynk.setup`.
4. Wprowadź dane WiFi i token z Blynk Cloud.

---

Autor: **Tom**  
Licencja: MIT
