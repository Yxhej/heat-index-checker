#include "DHT.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define DHTPIN 22 // define io pin
#define DHTTYPE DHT22 // define dht sensor type

#define SPEAKER_PIN 5

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, , -1);


// DHT22 declaration
DHT dht(DHTPIN, DHTTYPE);

float humidity;
float temp;
float heatIndex;

void setup() {
  Serial1.begin(115200);
  Serial1.println("Hello, Raspberry Pi Pico W!");

  dht.begin();
  update();
}

void loop() {
  delay(2000);

}

void playSound(float temp, double seconds) {
  tone(SPEAKER_PIN, 3.8 * temp, seconds * 1000);
}

// void displayCategories() {
//   display.
// }

/** Read and updates sensors. Returns true if operation was successful. */
boolean update() {
  humidity = dht.readHumidity();
  temp = dht.readTemperature(true);
  heatIndex = dht.computeHeatIndex(temp, humidity);

  if (isnan(humidity) || isnan(temp) || isnan(heatIndex)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return false;
  }
  return true;
}

