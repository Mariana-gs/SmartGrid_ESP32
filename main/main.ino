#include <WiFi.h>
#include <Adafruit_NeoPixel.h>
#include <IOXhop_FirebaseESP32.h>
#include <ArduinoJson.h>

// Wifi
#define WIFI_SSID "SOARES"
#define WIFI_PASSWORD "31060810#*"

// Firebase
#define FIREBASE_HOST "https://drawer-57cde-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH "X9Io7QZYS7HudcHYIsxKe7xauAAqbwl5dNGNmzxb"

// LED
#define LED_PIN 13
#define NUM_LEDS 29
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Mapeamento de gavetas para os LEDs
const int drawerLEDMap[9][4] = {
  {0, 1, 2, -1},
  {3, 4, 5, -1},
  {6, 7, 8, -1},
  {9, 10, 11, -1},
  {12, 13, 14, 15},
  {16, 17, 18, -1},
  {19, 20, 21, -1},
  {22, 23, 24, 25},
  {26, 27, 28, -1}
};

bool visualEffect = true;
int lastDrawerId = -1;

const int fixedR = 255; // Valor fixo de R (vermelho)
const int fixedG = 0;   // Valor fixo de G (verde)
const int fixedB = 0;   // Valor fixo de B (azul)

void setup() {
  Serial.begin(115200);

  connectToWiFi();

  strip.begin();
  strip.show();

  // Acende todos os LEDs para indicar que o dispositivo foi ligado
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(fixedR, fixedG, fixedB));
  }
  strip.show();
  delay(2000); // Mantém os LEDs acesos por 2 segundos

  // Apaga todos os LEDs
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 0));
  }
  strip.show();

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
  }

  int drawerId = Firebase.getInt("/drawer/config/id_drawer");

  if (Firebase.failed()) {
    Serial.println("Falha ao ler o ID da gaveta no Firebase");
  } else {
    if (drawerId > 0 && drawerId <= 9) {
      if (drawerId != lastDrawerId) {
        lightUpDrawer(drawerId - 1, fixedR, fixedG, fixedB);
        lastDrawerId = drawerId;
      }
    } else {
      if (lastDrawerId != -1) {
        lightUpDrawer(lastDrawerId, 0, 0, 0); // Apaga a gaveta acesa
        lastDrawerId = -1;
      }
    }
  }

  delay(100);
}

void connectToWiFi() {
  Serial.println("Conectando ao Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Conectado ao Wi-Fi!");
  } else {
    Serial.println("Falha ao conectar ao Wi-Fi. Tentando novamente em 5 segundos.");
    delay(5000);
  }
}

void lightUpDrawer(int drawerId, int r, int g, int b) {
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 0));
  }

  for (int i = 0; i < 4; i++) {
    int ledIndex = drawerLEDMap[drawerId][i];
    if (ledIndex != -1) {
      strip.setPixelColor(ledIndex, strip.Color(r, g, b));
    }
  }
  strip.show();
}
