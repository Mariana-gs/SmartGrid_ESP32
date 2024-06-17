// Bibliotecas
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <Adafruit_NeoPixel.h>
#include <SinricPro.h>
#include <SinricProSwitch.h>

// Wifi
#define WIFI_SSID ""        // Nome da rede
#define WIFI_PASSWORD ""    // Senha da rede

// Firebase
#define FIREBASE_HOST "https://drawer-57cde-default-rtdb.firebaseio.com/"      // Url do projeto 
#define FIREBASE_AUTH "X9Io7QZYS7HudcHYIsxKe7xauAAqbwl5dNGNmzxb"      // Token de autenticacao 

// LED
#define LED_PIN 2           // Pino do EPS32 
#define NUM_LEDS 29         // Numero de LEDs na fita
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// SinricPro
#define APP_KEY "00994d24-7469-4941-a075-8e14e059060f"                                             // Credencial 
#define APP_SECRET "232b62e8-f588-46d8-aeb2-1239c96e679c-1a6f0934-260a-45ea-8131-17c97bc18b1d"     // Credencial 
#define SWITCH_ID "665b80d55d818a66fab35577"                                                       // Identificador do dispositivo 

// Objeto firebase
FirebaseData firebaseData;
FirebaseConfig firebaseConfig;   
FirebaseAuth firebaseAuth; 

// Mapeamento de gavetas para os LEDs
// Cada linha -> uma gaveta, cada coluna -> um LED associado a ela
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

// Variavel de controle do efeito visual basico
bool visualEffect = true;

void setup() {
  Serial.begin(115200);

  // Conexão com Wi-Fi
  
  connectToWiFi();

  // Inicializacao dos LEDs
  strip.begin();
  strip.show();


  // Inicializacao do Firebase
  firebaseConfig.host = FIREBASE_HOST;
  firebaseConfig.api_key = FIREBASE_AUTH;
  firebaseConfig.signer.tokens.legacy_token = FIREBASE_AUTH;

  Firebase.begin(&firebaseConfig, &firebaseAuth);
  Firebase.reconnectWiFi(true);

  // Inicializacao do SinricPro
  SinricProSwitch &mySwitch = SinricPro[SWITCH_ID];
  mySwitch.onPowerState(onPowerState);
  SinricPro.begin(APP_KEY, APP_SECRET);
}

void loop() {
  //SinricPro.handle();

  // Verifica a conexao Wi-Fi e reconecta se necessario
  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
  }

  // Le o estado do Firebase para obter o ID da gaveta acionada
  if(Firebase.getInt(firebaseData, "/drawer/id_drawer")) {
    int drawerId = firebaseData.intData();

    if(drawerId >= 0 && drawerId < 9) {
      lightUpDrawer(drawerId);
      visualEffect = false;
    }
    else {
      visualEffect = true;
    }
  }
  
  else {
    Serial.println("Falha ao ler os dados do Firebase");
    visualEffect = true;
  }

  if(visualEffect) {
    showVisualEffect();
  }
}

// Conecta Wi-Fi
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

// Efeito visual 
void showVisualEffect() {
  int R = random(0, 255);
  int G = random(0, 255);
  int B = random(0, 255);

  for(int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(R, G, B));
  }

  strip.show();
}

// Ascende determinada gaveta
void lightUpDrawer(int drawerId) {
  for(int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 0));
  }

  for(int i = 0; i < 4; i++) {
    int ledIndex = drawerLEDMap[drawerId][i];

    if(ledIndex != -1) {
      strip.setPixelColor(ledIndex, strip.Color(247, 166, 135));    // Cor pode ser modificada com o valor da tabela
    }
  }
  strip.show();
}

// Callback - mudança do estado de energia do dispositivo 
bool onPowerState(const String &deviceId, bool &state) {
  Serial.printf("Dispositivo %s ligado %s\r\n", deviceId.c_str(), state ? "on" : "off");
  return true;
}

