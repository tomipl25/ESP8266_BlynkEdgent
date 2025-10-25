/***************************************************
 * Sterowanie piwnicą - Blynk.Edgent
 * Wersja uproszczona BEZ ADRESÓW czujników
 ***************************************************/

#define BLYNK_TEMPLATE_ID "......"
#define BLYNK_TEMPLATE_NAME "Piwnica"
#define BLYNK_FIRMWARE_VERSION "2.1.1"

#define BLYNK_PRINT Serial

// Konfiguracja dla Wemos D1 R2
#define USE_WEMOS_D1_MINI

#include "BlynkEdgent.h"
#include <OneWire.h>
#include <DallasTemperature.h>

// ---- Konfiguracja pinów ----
#define ONE_WIRE_BUS D3       // GPIO0 - czujniki temperatury
#define RELAY_OPEN D6         // GPIO12 - przekaźnik otwierania
#define RELAY_EXIT D7         // GPIO13 - przekaźnik zamykania
// D1, D2, D5 - WOLNE DO WYKORZYSTANIA

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// --- Stany przekaźników ---
#define RELAY_ON LOW
#define RELAY_OFF HIGH

// ---- Zmienne ----
BlynkTimer timer;

// --- Zmienne do obsługi przekaźników ---
bool openActive = false;
bool exitActive = false;
unsigned long openStartTime = 0;
unsigned long exitStartTime = 0;

// --- Zmienne do optymalizacji ---
bool wifiWasConnected = false;
float lastTemp[4] = {-999, -999, -999, -999};
int sensorCount = 0;

// ====== Monitorowanie siły sygnału Wi-Fi ======
void checkWiFiSignal() {
  if (WiFi.status() == WL_CONNECTED) {
    int rssi = WiFi.RSSI();
    Blynk.virtualWrite(V10, rssi);
    
    // Informuj tylko o ponownym połączeniu
    if (!wifiWasConnected) {
      Serial.print("WiFi połączone, IP: ");
      Serial.println(WiFi.localIP());
      wifiWasConnected = true;
    }
  } else {
    // Informuj tylko o utracie połączenia
    if (wifiWasConnected) {
      Serial.println("WiFi rozłączone!");
      wifiWasConnected = false;
    }
  }
}

// ====== Odczyt temperatur ======
void sendTemperatures() {
  sensors.requestTemperatures();
  
  // Odczytuj tyle czujników ile znaleziono (max 4)
  for (int i = 0; i < sensorCount && i < 4; i++) {
    float temp = sensors.getTempCByIndex(i);
    
    if (temp != DEVICE_DISCONNECTED_C && temp > -50 && temp < 85) {
      // Wysyłaj do V1, V2, V3, V4
      Blynk.virtualWrite(V1 + i, temp);
      
      // Wyświetl tylko jeśli zmiana > 0.5°C
      if (abs(temp - lastTemp[i]) > 0.5 || lastTemp[i] == -999) {
        Serial.print("Czujnik ");
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.print(temp, 1);
        Serial.println("°C");
        lastTemp[i] = temp;
      }
    } else {
      Blynk.virtualWrite(V1 + i, "Błąd");
      // Błąd czujnika - tylko raz na 10 prób
      static int errorCount = 0;
      if (++errorCount >= 10) {
        Serial.print("Błąd czujnika ");
        Serial.println(i + 1);
        errorCount = 0;
      }
    }
  }
}

// ====== Obsługa timerów przekaźników ======
void handleRelayTimers() {
  // Obsługa przekaźnika otwierania
  if (openActive && (millis() - openStartTime >= 500)) {
    digitalWrite(RELAY_OPEN, RELAY_OFF);
    openActive = false;
    Blynk.virtualWrite(V6, 0); // Reset przycisku w aplikacji
  }
  
  // Obsługa przekaźnika zamykania
  if (exitActive && (millis() - exitStartTime >= 500)) {
    digitalWrite(RELAY_EXIT, RELAY_OFF);
    exitActive = false;
    Blynk.virtualWrite(V5, 0); // Reset przycisku w aplikacji
  }
}

// ====== Sterowanie przekaźnikami ======
BLYNK_WRITE(V6) { // Otwieranie
  int openCommand = param.asInt();
  if (openCommand == 1 && !openActive && !exitActive) {
    // Zabezpieczenie przed zbyt częstymi impulsami
    static unsigned long lastOpenCmd = 0;
    if (millis() - lastOpenCmd < 1000) return;
    lastOpenCmd = millis();
    
    digitalWrite(RELAY_OPEN, RELAY_ON);
    openActive = true;
    openStartTime = millis();
    Serial.println("Impuls: OTWIERANIE");
  }
}

BLYNK_WRITE(V5) { // Zamykanie
  int exitCommand = param.asInt();
  if (exitCommand == 1 && !exitActive && !openActive) {
    // Zabezpieczenie przed zbyt częstymi impulsami
    static unsigned long lastExitCmd = 0;
    if (millis() - lastExitCmd < 1000) return;
    lastExitCmd = millis();
    
    digitalWrite(RELAY_EXIT, RELAY_ON);
    exitActive = true;
    exitStartTime = millis();
    Serial.println("Impuls: ZAMYKANIE");
  }
}

// ====== BLYNK CONNECTED ======
BLYNK_CONNECTED() {
  Blynk.syncVirtual(V5, V6);
  Serial.println("Blynk połączony");
  
  // Wyślij aktualną wersję firmware
  Blynk.sendInternal("meta", "fw", BLYNK_FIRMWARE_VERSION);
  
  // Reset przycisków w aplikacji
  Blynk.virtualWrite(V5, 0);
  Blynk.virtualWrite(V6, 0);
}

// ====== BLYNK DISCONNECTED ======
BLYNK_DISCONNECTED() {
  Serial.println("Blynk rozłączony");
}

void setup() {
  Serial.begin(115200);
  delay(100);

  // ---- Konfiguracja pinów ----
  pinMode(RELAY_OPEN, OUTPUT);
  pinMode(RELAY_EXIT, OUTPUT);

  // ---- Stan początkowy ----
  digitalWrite(RELAY_OPEN, RELAY_OFF);
  digitalWrite(RELAY_EXIT, RELAY_OFF);

  // ---- Inicjalizacja czujników temperatury ----
  sensors.begin();
  sensors.setResolution(9); // Niższa rozdzielczość = szybszy odczyt dla wszystkich
  
  // Zlicz czujniki
  sensorCount = sensors.getDeviceCount();
  Serial.print("Znaleziono czujników: ");
  Serial.println(sensorCount);
  
  if (sensorCount == 0) {
    Serial.println("UWAGA: Brak czujników temperatury!");
  }

  // ---- Optymalizacja WiFi ----
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.setOutputPower(20.0);

  // ---- Inicjalizacja Blynk.Edgent ----
  BlynkEdgent.begin();

  // ---- Konfiguracja timerów ----
  timer.setInterval(5000L, sendTemperatures);     // Co 5s
  timer.setInterval(10000L, checkWiFiSignal);      // Co 10s

  // Tylko najważniejsze info startowe
  Serial.println("System: START");
  Serial.print("Wersja: ");
  Serial.println(BLYNK_FIRMWARE_VERSION);
}

void loop() {
  BlynkEdgent.run();
  timer.run();
  handleRelayTimers();
  
  // Heartbeat - minimalna sygnalizacja że system żyje
  static unsigned long lastHeartbeat = 0;
  if (millis() - lastHeartbeat > 60000) { // Co minutę
    Serial.print(".");
    lastHeartbeat = millis();
  }
}