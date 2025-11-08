// ===== Placar Inteligente com FIWARE e MQTT (sem LED) =====
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define LCD_COLS 16
#define LCD_ROWS 2
#define LCD_ADDR 0x27
LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);

byte trofeu[8] = {
  B11111,B11111,B01110,B00100,B00100,B01110,B11111,
};

const int PIN_MOTION_LEFT  = 23;
const int PIN_MOTION_RIGHT = 18;
const int PIN_BUZZER       = 19;
const int PIN_I2C_SDA = 21;
const int PIN_I2C_SCL = 22;

const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";
const char* BROKER_MQTT = "192.168.0.21";
const int   BROKER_PORT = 1883;

const char* TOPICO_GOLS_BLUE = "/TEF/device006/attrs/gb";
const char* TOPICO_GOLS_RED  = "/TEF/device006/attrs/gr";
const char* ID_MQTT          = "fiware_006";

WiFiClient espClient;
PubSubClient MQTT(espClient);

volatile int goalsBlue = 0;
volatile int goalsRed  = 0;
int lastStateLeft = LOW;
int lastStateRight = LOW;

void showWelcome() {
  lcd.init(); lcd.backlight(); lcd.clear();
  lcd.setCursor(0,0); lcd.print("Bem-vindos ao");
  lcd.setCursor(0,1); lcd.print("Amistoso!");
  lcd.createChar(0, trofeu); lcd.setCursor(15,1); lcd.write(byte(0));
  delay(2000);
}

void updateScoreBoard() {
  lcd.clear();
  lcd.setCursor(0,0); lcd.print("  Blue  x  Red");
  lcd.setCursor(5,1); lcd.print(goalsBlue);
  lcd.setCursor(8,1); lcd.print("x");
  lcd.setCursor(11,1); lcd.print(goalsRed);
}

void beepGoal() {
  tone(PIN_BUZZER, 262, 250);
}

void initSerial() {
  Serial.begin(115200); delay(100);
}

void reconectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;
  Serial.print("Conectando WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) { delay(250); Serial.print("."); }
  Serial.println("Conectado!"); Serial.println(WiFi.localIP());
}

void initWiFi() { reconectWiFi(); }

void initMQTT() { MQTT.setServer(BROKER_MQTT, BROKER_PORT); }

void reconnectMQTT() {
  while (!MQTT.connected()) {
    Serial.print("Conectando ao broker MQTT...");
    if (MQTT.connect(ID_MQTT)) {
      Serial.println("Conectado!");
      char buf[16];
      itoa(goalsBlue, buf, 10); MQTT.publish(TOPICO_GOLS_BLUE, buf);
      itoa(goalsRed , buf, 10); MQTT.publish(TOPICO_GOLS_RED , buf);
    } else {
      Serial.println("Falhou! Tentando de novo...");
      delay(2000);
    }
  }
}

void checkConnections() {
  reconectWiFi();
  if (!MQTT.connected()) reconnectMQTT();
}

void publishScore() {
  char buf[16];
  itoa(goalsBlue, buf, 10); MQTT.publish(TOPICO_GOLS_BLUE, buf);
  itoa(goalsRed , buf, 10); MQTT.publish(TOPICO_GOLS_RED , buf);
  Serial.printf("[PLACAR] Blue=%d | Red=%d\\n", goalsBlue, goalsRed);
}

void handleGoalBlue() {
  goalsBlue++;
  lcd.clear(); lcd.print("Gol do Blue!"); beepGoal();
  delay(800); updateScoreBoard(); publishScore();
}

void handleGoalRed() {
  goalsRed++;
  lcd.clear(); lcd.print("Gol do Red!"); beepGoal();
  delay(800); updateScoreBoard(); publishScore();
}

void setup() {
  pinMode(PIN_MOTION_LEFT, INPUT);
  pinMode(PIN_MOTION_RIGHT, INPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  showWelcome(); updateScoreBoard();
  initSerial(); initWiFi(); initMQTT();
}

void loop() {
  checkConnections(); MQTT.loop();
  int currentLeft = digitalRead(PIN_MOTION_LEFT);
  int currentRight = digitalRead(PIN_MOTION_RIGHT);
  if (currentLeft == HIGH && lastStateLeft == LOW) handleGoalBlue();
  if (currentRight == HIGH && lastStateRight == LOW) handleGoalRed();
  lastStateLeft = currentLeft; lastStateRight = currentRight;
}
