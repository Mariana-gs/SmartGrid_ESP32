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

// Cor fixa de encontrado
const int foundR = 255; // R (255)
const int foundG = 214; // G (226)
const int foundB = 75; // B (125)

// Cor de procura
const int searchR = 166; // R (169)
const int searchG = 98; // G (136)
const int searchB = 255; // B (249)

// Cor inicial
const int initialR = 166; // R (169)
const int initialG = 98; // G (136)
const int initialB = 255; // B (249)

void setup() {
  Serial.begin(115200);

  connectToWiFi();

  strip.begin();
  strip.show();

  // Configura os LEDs na inicialização
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(initialR, initialG, initialB)); // Cor roxa
  }

  // LEDs correspondentes ao ID 3 (amarelo)
  for (int i = 0; i < 4; i++) {
    int ledIndex = drawerLEDMap[2][i]; // ID 3 é index 2
    if (ledIndex != -1) {
      strip.setPixelColor(ledIndex, strip.Color(foundR, foundG, foundB)); // Cor amarela
    }
  }

  // LEDs correspondentes ao ID 7 (apagado)
  for (int i = 0; i < 4; i++) {
    int ledIndex = drawerLEDMap[6][i]; // ID 7 é index 6
    if (ledIndex != -1) {
      strip.setPixelColor(ledIndex, strip.Color(0, 0, 0)); // Apagado
    }
  }

  strip.show();
  delay(2000); // Mantém os LEDs acesos por 2 segundos

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
        lightUpDrawerWithEffect(drawerId - 1, foundR, foundG, foundB);
        lastDrawerId = drawerId;
        visualEffect = false;
      }
    } else {
      if (lastDrawerId != -1) {
        lightUpDrawer(lastDrawerId, 0, 0, 0); // Apaga a gaveta acesa
        lastDrawerId = -1;
      }
    }
  }

  if (visualEffect) {
    showVisualEffect();
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

void showVisualEffect() {
  int R = random(0, 255);
  int G = random(0, 255);
  int B = random(0, 255);

  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(R, G, B));
  }
  strip.show();
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

void lightUpDrawerWithEffect(int drawerId, int r, int g, int b) {
  // Apaga todos os LEDs
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 0));
  }
  strip.show();
  delay(500);

  // Acende LEDs em sequência até o ID da gaveta selecionada
  for (int i = 0; i <= drawerId; i++) {
    for (int j = 0; j < 4; j++) {
      int ledIndex = drawerLEDMap[i][j];
      if (ledIndex != -1) {
        strip.setPixelColor(ledIndex, strip.Color(searchR, searchG, searchB));
      }
    }
    strip.show();
    delay(250); // Atraso para o efeito de sequência
  }

  // Mostra apenas os LEDs da gaveta selecionada
  delay(500);
  lightUpDrawer(drawerId, r, g, b);
}
