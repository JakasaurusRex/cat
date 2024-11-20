/**************************************************************************
HTTP Request Example
Fetch a random color palette from colormind.io and draw the colors on the display

To fetch a new color, press either button on the LILYGO (GPIO 0 or 35)     
**************************************************************************/
#include "TFT_eSPI.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>

// TODO: replace with your own SSID & Password
const char* ssid = "Columbia University";
const char* password = "";

#define BUTTON_LEFT 0
#define BUTTON_RIGHT 35

#include "image.h"
uint16_t imageW=183;
uint16_t imageH=182;

int WIDTH=135;
int HEIGHT=240;

volatile bool leftButtonPressed = false;
volatile bool rightButtonPressed = false;

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite bg = TFT_eSprite(&tft);
TFT_eSprite catSprite = TFT_eSprite(&tft);

void setup() {
  Serial.begin(115200);
  
  // setup our display
  tft.begin();
  tft.setRotation(2);  // 1 = landscape, 2 = portrait
  tft.fillScreen(TFT_BLACK);
  tft.setSwapBytes(true);
  
  bg.createSprite(WIDTH, HEIGHT);
  bg.setSwapBytes(true);

  catSprite.createSprite(WIDTH, HEIGHT);
  bg.fillSprite(TFT_BLACK);
  catSprite.pushImage(0, 0, imageW, imageH, image);
  catSprite.pushToSprite(&bg, (int)0, (int)80, TFT_BLACK);
  bg.pushSprite(0, 0);

  Serial.print("display dimensions: ");
  Serial.print(tft.width());
  Serial.print(" x ");
  Serial.println(tft.height());

  // connect to wifi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  // Send the HTTP POST request
  if (WiFi.status() == WL_CONNECTED) {
    fetchCat();
  } else {
    Serial.println("WiFi not connected");
  }

  // setup our buttons
  pinMode(BUTTON_LEFT, INPUT_PULLUP);
  pinMode(BUTTON_RIGHT, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_LEFT), pressedLeftButton, FALLING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_RIGHT), pressedRightButton, FALLING);
}

void fetchCat() {
    HTTPClient http;

    String url = "https://meowfacts.herokuapp.com/";
    http.begin(url);

    // Send the request
    int httpResponseCode = http.GET();

    // Check the response
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(response);
      JSONVar responseJSON = JSON.parse(response);

      if (JSON.typeof(responseJSON) == "undefined") {
        Serial.println("Parsing input failed!");
        return;
      }

      Serial.println(responseJSON);
      JSONVar keys = responseJSON.keys();

      catSprite.createSprite(WIDTH, HEIGHT);
      bg.fillSprite(TFT_BLACK);
      catSprite.pushImage(0, 0, imageW, imageH, image);
      catSprite.pushToSprite(&bg, (int)0, (int)80, TFT_BLACK);
      bg.pushSprite(0, 0);

      JSONVar value = responseJSON[keys[0]];
      String valueString = JSON.stringify(value);
      String valueSubbed = valueString.substring(2, valueString.length() - 2);
      int y = 0;
      for(int i = 0; i < valueSubbed.length(); i+=22) {
        tft.drawString(valueSubbed.substring(i, i+22), 0, y);
        y += 10;
      }
    } else {
      Serial.println("Error on sending GET request");
    }

    // Free resources
    http.end();
}

void pressedLeftButton() {
  leftButtonPressed = true;
}

void pressedRightButton() {
  rightButtonPressed = true;
}

void loop() {
  // fetch colors when either button is pressed
  if (leftButtonPressed) {
    fetchCat();
    leftButtonPressed = false;
  }
  if (rightButtonPressed) {
    fetchCat();
    rightButtonPressed = false;
  }
}
