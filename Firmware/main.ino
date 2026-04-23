#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

// Wifi Credentials 
const char* ssid = "YOUR_WIFI";
const char* password = "YOUR_PASS";
// Tauon Player Ip
String tauon_ip = "http://192.168.1.100:7813";


#define TFT_CS 7
#define TFT_DC 6
#define TFT_RST 5
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);


#define BTN_PLAY 2
#define BTN_UP   3
#define BTN_DOWN 4


struct PlayerState {
  bool playing;
  String title;
  String artist;
  int position;
  int duration;
  int volume;
};

PlayerState current, last;


unsigned long pressStart[3] = {0,0,0};
bool pressed[3] = {false,false,false};

const int SHORT_PRESS = 200;
const int LONG_PRESS  = 800;

unsigned long lastSync = 0;

void sendCommand(String cmd) {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  http.begin(tauon_ip + cmd);
  int code = http.GET();
  Serial.println(cmd + " -> " + String(code));
  http.end();
}

void fetchStatus() {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  http.begin(tauon_ip + "/status");

  int code = http.GET();

  if (code == 200) {
    String payload = http.getString();

    StaticJsonDocument<1024> doc;
    if (!deserializeJson(doc, payload)) {

      current.playing = (String(doc["status"]) == "playing");
      current.title = doc["title"].as<String>();
      current.artist = doc["artist"].as<String>();

      current.position = doc["progress"];
      current.duration = doc["track"]["duration"];
      current.volume = doc["volume"];
    }
  }

  http.end();
}


void updateDisplay() {

  if (current.title != last.title) {
    tft.fillRect(0, 0, 160, 15, ST77XX_BLACK);
    tft.setCursor(0, 0);
    tft.setTextSize(1);
    tft.setTextColor(ST77XX_WHITE);
    tft.println(current.title);
  }

  if (current.artist != last.artist) {
    tft.fillRect(0, 12, 160, 12, ST77XX_BLACK);
    tft.setCursor(0, 12);
    tft.println(current.artist);
  }

  if (current.playing != last.playing) {
    tft.fillRect(0, 24, 160, 10, ST77XX_BLACK);
    tft.setCursor(0, 24);
    tft.println(current.playing ? "Playing" : "Paused");
  }

  if (current.duration > 0) {
    int barWidth = map(current.position, 0, current.duration, 0, 160);

    tft.drawRect(0, 36, 160, 8, ST77XX_WHITE);
    tft.fillRect(0, 36, barWidth, 8, ST77XX_GREEN);
  }
  int sec = current.position / 1000;
  int total = current.duration / 1000;

  tft.fillRect(0, 48, 160, 10, ST77XX_BLACK);
  tft.setCursor(0, 48);
  tft.printf("%d:%02d / %d:%02d",
             sec/60, sec%60,
             total/60, total%60);

  if (current.volume != last.volume) {
    tft.fillRect(0, 60, 160, 10, ST77XX_BLACK);
    tft.setCursor(0, 60);
    tft.print("Vol: ");
    tft.println(current.volume);
  }

  last = current;
}

void togglePlayPause() {
  if (current.playing) {
    sendCommand("/pause");
  } else {
    sendCommand("/play");
  }
}

void handleButton(int pin, int index) {
  bool state = digitalRead(pin);

  if (state && !pressed[index]) {
    pressed[index] = true;
    pressStart[index] = millis();
  }

  if (!state && pressed[index]) {
    pressed[index] = false;
    unsigned long duration = millis() - pressStart[index];

    // PLAY BUTTON
    if (index == 0 && duration > SHORT_PRESS) {
      togglePlayPause();
    }

    // VOL +
    if (index == 1) {
      if (duration > LONG_PRESS) {
        sendCommand("/next");
      } else {
        sendCommand("/setvolumerel/5");
      }
    }

    // VOL -
    if (index == 2) {
      if (duration > LONG_PRESS) {
        sendCommand("/back");
      } else {
        sendCommand("/setvolumerel/-5");
      }
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(BTN_PLAY, INPUT);
  pinMode(BTN_UP, INPUT);
  pinMode(BTN_DOWN, INPUT);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }

  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(ST77XX_BLACK);

  tft.setCursor(10, 5);
  tft.setTextColor(ST77XX_GREEN);
  tft.println("Tauon Remote Ready");
}

void loop() {

  handleButton(BTN_PLAY, 0);
  handleButton(BTN_UP,   1);
  handleButton(BTN_DOWN, 2);

  if (millis() - lastSync > 500) {
    fetchStatus();
    updateDisplay();
    lastSync = millis();
  }

  delay(5);
}
