// ##### Server Imports #####
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <esp_task_wdt.h>

#include "ArduinoJson.h"
#include "AsyncJson.h"

// ##### OLED Imports #####
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

// ##### LED-strip Imports #####
#include "FastLED.h"
#include "FastLED_RGBW.h"

// ##### General variables #####
int animation_counter = 0;

// ##### Server variables #####
AsyncWebServer server(80);
const char* ssid = "wifi-name";
const char* password = "wifi-password";

// ##### OLED variables #####
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 32  // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ##### LED-Strip variables
#define NUM_LEDS 24
#define DATA_PIN 23

// FastLED with RGBW
CRGBW leds[NUM_LEDS];
CRGB* ledsRGB = (CRGB*)&leds[0];

//  ##### Relay variables #####
const int relay1 = 32;
const int relay2 = 33;
const int relay3 = 25;
const int relay4 = 26;

// ~~~~~ General Functions ~~~~~

void custom_delay(int ms) {
  if (ms == 0) {
    return;
  }

  unsigned long start_point = millis();
  unsigned long current_time = millis();

  // counting down
  while (current_time - start_point < (unsigned long)ms) {
    current_time = millis();
  }
}

// ~~~~~ Server Functions ~~~~~

void notFound(AsyncWebServerRequest* request) {
  request->send(404, "text/plain", "Not found");
}

int ml_to_ms(int pumpspeed, int ml) {
  if (ml == 0) {
    return 0;
  }
  int ms = ((ml / pumpspeed) + 1) * 1000;
  return ms;
}

void server_setup() {
  // Send a POST request to <IP>/drink with json
  AsyncCallbackJsonWebHandler* handler = new AsyncCallbackJsonWebHandler(
      "/drink", [](AsyncWebServerRequest* request, JsonVariant& json) {
        JsonObject jsonObj = json.as<JsonObject>();

        int pumpspeed = 13;  // ml/sec
        int drink1_ml = jsonObj["1"];
        int drink2_ml = jsonObj["2"];
        int drink3_ml = jsonObj["3"];
        int drink4_ml = jsonObj["4"];

        int drink1_ms = ml_to_ms(pumpspeed, drink1_ml);
        int drink2_ms = ml_to_ms(pumpspeed, drink2_ml);
        int drink3_ms = ml_to_ms(pumpspeed, drink3_ml);
        int drink4_ms = ml_to_ms(pumpspeed, drink4_ml);

        request->send(200, "text/plain", "de json:");

        display_text("Your drink is being\nmade");

        single_color(CRGB::Red);
        open_relay(relay1, drink1_ms);

        single_color(CRGB::Blue);
        open_relay(relay2, drink2_ms);

        single_color(CRGB::Green);
        open_relay(relay3, drink3_ms);

        CRGB pink = CRGB(255, 20, 147);
        single_color(pink);
        open_relay(relay4, drink4_ms);

        main_display();
      });
  server.addHandler(handler);

  server.onNotFound(notFound);
  server.begin();
}

void wifi_setup() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.printf("WiFi Failed!\n");
    display_text("WiFi Failed!");
    return;
  }

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

// ~~~~~ OLED Functions ~~~~~

void oled_setup() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  custom_delay(1000);
  display.clearDisplay();

  main_display();
}

void main_display() { display_text("Welcome to the\nCocktail Maker"); }

void display_text(String text) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // Display static text
  display.println(text);
  display.display();
}

void clear_display() { display.clearDisplay(); }

// ~~~~~ LED-Strip Functions ~~~~~

void ledstrip_setup() {
  // FastLED with RGBW
  FastLED.addLeds<WS2812B, DATA_PIN, RGB>(ledsRGB, getRGBWsize(NUM_LEDS));
}

void ledstrip_loop() {
  CRGB color = CRGB::Red;
  int width = 5;
  int time = 10;

  rainbow();
}

void single_color(CRGB color) {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = color;
  }
  FastLED.show();
}

void rainbow() {
  for (int j = 0; j < 256; j++) {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = Scroll((i * 256 / NUM_LEDS + j) % 256);
    }

    FastLED.show();
    custom_delay(1);
  }
}

void chase(CRGB color, int width, int time) {
  // starting the line
  for (int i = 0; i < width; i++) {
    leds[i] = color;
    FastLED.show();
  }
  // turning back-led off and front-led on
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
    leds[(i + width) % NUM_LEDS] = color;
    FastLED.show();
    custom_delay(time);
  }
}

CRGB Scroll(int pos) {
  CRGB color(0, 0, 0);
  if (pos < 85) {
    color.g = 0;
    color.r = ((float)pos / 85.0f) * 255.0f;
    color.b = 255 - color.r;
  } else if (pos < 170) {
    color.g = ((float)(pos - 85) / 85.0f) * 255.0f;
    color.r = 255 - color.g;
    color.b = 0;
  } else if (pos < 256) {
    color.b = ((float)(pos - 170) / 85.0f) * 255.0f;
    color.g = 255 - color.b;
    color.r = 1;
  }
  return color;
}

// ~~~~~ Relay Functions ~~~~~

void relay_setup() {
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);

  // relay is low-active so we first have to put all the relays to high to turn
  // them off
  digitalWrite(relay1, HIGH);
  digitalWrite(relay2, HIGH);
  digitalWrite(relay3, HIGH);
  digitalWrite(relay4, HIGH);
}

void open_relay(int relay, int time_amount) {
  if (time_amount == 0) {
    return;
  }
  digitalWrite(relay, LOW);  // turning pump on

  custom_delay(time_amount);

  digitalWrite(relay, HIGH);  // turning pump off
}

void setup() {
  esp_task_wdt_init(30, false);
  Serial.begin(115200);

  relay_setup();
  wifi_setup();
  oled_setup();
  ledstrip_setup();
  server_setup();
}

void loop() {
  int width = 10;
  int interval = 25;
  CRGB color = CRGB::Blue;

  // Letting the LED Strip run through some animations
  if (animation_counter <= 25) {
    rainbow();
    animation_counter++;
    return;
  } else if (25 < animation_counter && animation_counter <= 50) {
    color = CRGB::Red;
  } else if (50 < animation_counter && animation_counter <= 75) {
    color = CRGB::Green;
  } else if (75 < animation_counter && animation_counter <= 100) {
    color = CRGB::Blue;
  } else {
    animation_counter = -1;
  }

  animation_counter++;
  chase(color, width, interval);
}
