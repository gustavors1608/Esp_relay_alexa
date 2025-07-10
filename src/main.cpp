#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include "fauxmoESP.h"

#define DEVICE_NAME "ESP32-ALEXA"

// --- Constantes para os IDs dos dispositivos virtuais ---
#define ID_RELE1 "Relé 1"
#define ID_RELE2 "Relé 2"
#define ID_RELE3 "Relé 3"
#define ID_RELE4 "Relé 4"

// --- Constantes para os pinos GPIO ---
#define PIN_WIFI_LED 12 
#define PIN_STATUS_LED 14
#define PIN_WIFI_RESET_BUTTON 34 // <--- Botão para reset WiFi

#define PIN_LED_OUT_1 27 
#define PIN_LED_OUT_2 26 
#define PIN_LED_OUT_3 25 
#define PIN_LED_OUT_4 33 

#define PIN_OUT_1 16 
#define PIN_OUT_2 18 
#define PIN_OUT_3 21 
#define PIN_OUT_4 23 

#define PIN_OUT_1_FEEDBACK 4 
#define PIN_OUT_2_FEEDBACK 17 
#define PIN_OUT_3_FEEDBACK 19 
#define PIN_OUT_4_FEEDBACK 22  

// --- Variáveis Globais ---
fauxmoESP fauxmo;

// --- Protótipos de Funções ---
void wifiSetup();
void set_out(uint8_t pin, bool state);

void setup() {
  Serial.begin(115200);

  // --- Configuração dos pinos GPIO ---
  pinMode(PIN_WIFI_LED, OUTPUT);
  pinMode(PIN_STATUS_LED, OUTPUT);
  pinMode(PIN_WIFI_RESET_BUTTON, INPUT); // Botão no pino 34

  pinMode(PIN_LED_OUT_1, OUTPUT);
  pinMode(PIN_LED_OUT_2, OUTPUT);
  pinMode(PIN_LED_OUT_3, OUTPUT);
  pinMode(PIN_LED_OUT_4, OUTPUT);

  pinMode(PIN_OUT_1, OUTPUT);
  pinMode(PIN_OUT_2, OUTPUT);
  pinMode(PIN_OUT_3, OUTPUT);
  pinMode(PIN_OUT_4, OUTPUT);

  // --- Verifica se o botão de reset está pressionado no boot ---
  if (digitalRead(PIN_WIFI_RESET_BUTTON) == LOW) { // Pressionado (gnd)
    delay(5000);
      if (digitalRead(PIN_WIFI_RESET_BUTTON) == LOW) { // Pressionado (gnd)
        Serial.println("Botão de reset pressionado. Apagando configurações WiFi...");
        WiFiManager wm;
        wm.resetSettings(); // Apaga SSID e senha salvos
        delay(1000);
        ESP.restart();
      }
  }

  // --- Conecta à rede Wi-Fi com WiFiManager ---
  wifiSetup();
  digitalWrite(PIN_WIFI_LED, HIGH);

  // --- Configuração do FauxmoESP ---
  fauxmo.createServer(true);
  fauxmo.setPort(80);
  fauxmo.enable(true);

  fauxmo.addDevice(ID_RELE1);
  fauxmo.addDevice(ID_RELE2);
  fauxmo.addDevice(ID_RELE3);
  fauxmo.addDevice(ID_RELE4);

  fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state, unsigned char value) {
    Serial.printf("[MAIN] Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);

    if (strcmp(device_name, ID_RELE1) == 0) {
      set_out(PIN_OUT_1, state);
      set_out(PIN_LED_OUT_1, state);
    } else if (strcmp(device_name, ID_RELE2) == 0) {
      set_out(PIN_OUT_2, state);
      set_out(PIN_LED_OUT_2, state);
    } else if (strcmp(device_name, ID_RELE3) == 0) {
      set_out(PIN_OUT_3, state);
      set_out(PIN_LED_OUT_3, state);
    } else if (strcmp(device_name, ID_RELE4) == 0) {
      set_out(PIN_OUT_4, state);
      set_out(PIN_LED_OUT_4, state);
    } 
  });
}

void loop() {  
  fauxmo.handle();

  static unsigned long lastMillis = 0;
  if (millis() - lastMillis > 100) {
    lastMillis = millis();
    digitalWrite(PIN_STATUS_LED, !digitalRead(PIN_STATUS_LED));
  }
}

void wifiSetup() {
  WiFiManager wm;

  // Inicia o modo portal se necessário
  bool res = wm.autoConnect("ESP32-Config");

  if (!res) {
    Serial.println("Falha na conexão. Reiniciando...");
    ESP.restart();
  }

  Serial.println("WiFi conectado com sucesso!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void set_out(uint8_t pin, bool state) {
  digitalWrite(pin, state);
}
