#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define DHTPIN 14 // define io pin
#define DHTTYPE DHT22 // define dht sensor type

#define SPEAKER_PIN 9

#define buttonPin 2

// DHT22 declaration
DHT dht(DHTPIN, DHTTYPE);

float humidity;
float temp;
float heatIndex;

int lastPressed = 1;
boolean isFahrenheit = true;

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
  pinMode(buttonPin, INPUT_PULLUP);

  dht.begin();
  readSensors();
}

void loop() {  
  delay(100);
  int measurement = digitalRead(buttonPin);

  if (measurement != lastPressed) {
    if (measurement == 0) {
      isFahrenheit = !isFahrenheit;
    }
  }

  readSensors();  
  displayScreen(humidity, temp);
  lightUp(temp);
  lastPressed = measurement;
  }

void lightUp(float temperature) {
  if (!isFahrenheit) {
    temperature = (temperature * 9/5) + 32;
  }

  temperature = int(abs(trunc(temperature / 10)) - 1);
  Serial1.println(temperature);

  for (int i = 0; i < numBars; i++) {
    if (temperature > i) {
      digitalWrite(barPins[i], HIGH);
    } else {
      digitalWrite(barPins[i], LOW);
    }
  }
}

void displayScreen(float humid, float temp) {
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
  display.println("\nHumidity: " +String(humid) + "%");

  display.display();
}

void playSound(float temp, double seconds) {
  tone(SPEAKER_PIN, 3.8 * temp, seconds * 1000);
}

/** Read and updates sensors. Returns true if operation was successful. */
boolean readSensors() {
  humidity = dht.readHumidity();
  temp = dht.readTemperature(isFahrenheit);
  heatIndex = dht.computeHeatIndex(temp, humidity);

  if (isnan(humidity) || isnan(temp) || isnan(heatIndex)) {
    Serial1.println(F("Failed to read from DHT sensor!"));
    return false;
  }
  return true;
}