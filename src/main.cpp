#include <FastLED.h>
#include "NimBLEDevice.h"

#define LED_PIN 8
#define LED_BUILTIN 10
#define NUM_LEDS 300
#define EYE_SIZE 15
#define BRIGHTNESS 200
#define LED_TYPE WS2812
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];
CPixelView<CRGB> *Mouth = new CPixelView<CRGB>(leds, 0, 100);
CPixelView<CRGB> *LeftEye = new CPixelView<CRGB>(leds, 150, 150+EYE_SIZE);
CPixelView<CRGB> *RightEye = new CPixelView<CRGB>(leds, 230, 150+EYE_SIZE);



time_t timestamp;
uint8_t LEDMode = 0x00;

class LEDCallbacks : public NimBLECharacteristicCallbacks
{
  void onWrite(NimBLECharacteristic *Characteristic)
  {
    LEDMode = *Characteristic->getValue().getValue(&timestamp);
  }
};

// Fade to black
void fadeToBlackAndBack(CPixelView<CRGB> *ledSet) {
  // Fade times in milliseconds
  const unsigned long RedFade = 800; 
  const unsigned long BlackFade = 1500;
  unsigned long startTime = millis();

  // Fade to black
  while (millis() - startTime < BlackFade) {
    float progress = (float)(millis() - startTime) / BlackFade;
    uint8_t brightness = 255 * (1.0 - progress); 
    fill_solid(*ledSet, ledSet->size(), CRGB(brightness, 0, 0));
    FastLED.show();
  }

  // Ensure LEDs are completely off
  fill_solid(*ledSet, ledSet->size(), CRGB::Black);
  FastLED.show();

  // Fade to red
  startTime = millis();
  while (millis() - startTime < RedFade) {
    float progress = (float)(millis() - startTime) / RedFade;
    uint8_t brightness = 255 * progress;
    fill_solid(*ledSet, ledSet->size(), CRGB(brightness, 0, 0));
    FastLED.show();
  }

  // Ensure LEDs are fully red
  fill_solid(*ledSet, ledSet->size(), CRGB::Red);
  FastLED.show();
}

void setup()
{
  Serial.begin(9600);
  Serial.println("Hello from ESP32");
  pinMode(LED_BUILTIN, OUTPUT);
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  //Default all LEDS to black
  fill_solid(leds, NUM_LEDS, CRGB::Black);

  NimBLEDevice::init("Meep");

  NimBLEServer *pServer = NimBLEDevice::createServer();
  NimBLEService *pService = pServer->createService("ABCD");
  NimBLECharacteristic *pCharacteristic = pService->createCharacteristic("1234");

  pService->start();
  pCharacteristic->setValue("1");
  pCharacteristic->setCallbacks(new LEDCallbacks());

  NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->addServiceUUID("ABCD");
  pAdvertising->start();

}

void loop()
{
  //Mouth Switch
  switch (LEDMode & 0xF0) {
  case 0x00:
    Mouth->fill_solid(CRGB::Black);
    break;
  case 0x10:
    Mouth->fill_solid(CRGB::Red);
    break;
  case 0x20:
    Mouth->fill_solid(CRGB::Green);
    break;
  case 0x30:
    Mouth->fill_solid(CRGB::Blue);
    break;
  case 0x40:
    fadeToBlackAndBack(Mouth);
    break;
  default:
    Mouth->fill_solid(CRGB::Yellow);
  }

  //Eyes Switch
  switch (LEDMode & 0x0F) {
  case 0x0:
    LeftEye->fill_solid(CRGB::Black);
    RightEye->fill_solid(CRGB::Black);
    break;
  case 0x1:
    LeftEye->fill_solid(CRGB::Red);
    RightEye->fill_solid(CRGB::Red);
    break;
  case 0x2:
    LeftEye->fill_solid(CRGB::Green);
    RightEye->fill_solid(CRGB::Green);
    break;
  case 0x3:
    LeftEye->fill_solid(CRGB::Blue);
    RightEye->fill_solid(CRGB::Blue);
    break;
  default:
    LeftEye->fill_solid(CRGB::Yellow);
    RightEye->fill_solid(CRGB::Yellow);
  }

  FastLED.show();
  delay(50);
}
