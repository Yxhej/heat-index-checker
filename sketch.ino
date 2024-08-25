#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <arduino-timer.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define DHTPIN 14 // define io pin
#define DHTTYPE DHT22 // define dht sensor type

#define BUTTON_PIN 2
#define SPEAKER_PIN 9

// DHT22 declaration
DHT dht(DHTPIN, DHTTYPE);

Timer<1, millis, float *> timer;

int lastPressed = 1;
bool isFahrenheit = true;
const float TEMP_THRESHOLD = 85;

int barPins[] = {28, 27, 26, 22, 21, 20, 19, 18, 17, 16};
const int numBars = 10;

void setup() {
  Serial1.begin(115200);
  Serial1.println("Hello, Raspberry Pi Pico W!");

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial1.println("SSD1306 allocation failed");
  }

  for (int ledPin = 0; ledPin < numBars; ledPin++) {
    pinMode(barPins[ledPin], OUTPUT);
  }
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(SPEAKER_PIN, OUTPUT);

  dht.begin();
  float temp = dht.readTemperature(isFahrenheit);
  timer.every(1000, playSound, &temp);
}

void loop() {  
  delay(100);
  int measurement = digitalRead(BUTTON_PIN);

  if (measurement != lastPressed) {
    if (measurement == 0) {
      isFahrenheit = !isFahrenheit;
    }
  }

  float humidity = dht.readHumidity();
  float temp = dht.readTemperature(isFahrenheit);
  float heatIndex = dht.computeHeatIndex(temp, humidity, isFahrenheit);

  if (isnan(humidity) || isnan(temp) || isnan(heatIndex)) {
    Serial1.println(F("Failed to read from DHT sensor!"));
  }

  displayScreen(temp, humidity, heatIndex);
  lightUp(temp);

  timer.tick();
  lastPressed = measurement;
  }

void lightUp(float temperature) {
  temperature = int(abs(trunc(convert(temperature) / 10)) - 2);

  for (int i = 0; i < numBars; i++) {
    if (temperature >= i) {
      digitalWrite(barPins[i], HIGH);
    } else {
      digitalWrite(barPins[i], LOW);
    }
  }
}

void displayScreen(float temp, float humidity, float heatIndex) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);

  if (isFahrenheit) {
    display.println("Temperature: "+ String(temp) + " F");
    display.println("\nHeat Index: " + String(heatIndex) + " F");
  } else {
    display.println("Temperature: " + String(temp) + " C");
    display.println("\nHeat Index: " + String(heatIndex) + " C");
  }
  display.println("\nHumidity: " +String(humidity) + "%");

  display.display();
}

bool playSound(float* temp) {
  float temperature = convert(*temp);
  Serial1.println(temperature);
  if (temperature > convert(TEMP_THRESHOLD)) {
    tone(SPEAKER_PIN, 30 * temperature, 1000);
  }
  return true;
}

float convert(float temperature) {
  if (!isFahrenheit) {
    return (temperature * 9/5) + 32;
  }
  return temperature;
}
