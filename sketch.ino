#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define DHTPIN 14 // define io pin
#define DHTTYPE DHT22 // define dht sensor type

#define BUTTON_PIN 2
#define SPEAKER_PIN 9

// DHT22 declaration
DHT dht(DHTPIN, DHTTYPE);

int lastPressed = 1;
bool isFahrenheit = true;

// fahrenheit
const float HOT_THRESHOLD = 85;
const float COLD_THRESHOLD = 30;

// https://rickkas7.github.io/DisplayGenerator/
const uint8_t warningIcon[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x7e, 0x00, 0x00, 0xf7, 0x00, 0x00, 0xe7, 0x00, 0x01, 0xc3, 0x80, 0x01, 0xc3, 0x80, 0x03, 0x99, 0xc0, 0x07, 0x18, 0xe0, 0x07, 0x18, 0xe0, 0x0e, 0x18, 0x70, 0x0e, 0x18, 0x70, 0x1c, 0x18, 0x38, 0x38, 0x00, 0x1c, 0x38, 0x00, 0x1c, 0x70, 0x18, 0x0e, 0x70, 0x18, 0x0e, 0xe0, 0x00, 0x07, 0xe0, 0x00, 0x07, 0x7f, 0xff, 0xfe, 0x3f, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// ms
unsigned long elapsedTime = 0;
unsigned long previousTick = 0;
const long WARNING_PERIOD = 10000;

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
  previousTick = millis();
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

  displayInfo(temp, humidity, heatIndex);
  lightUp(temp);

  elapsedTime = millis();
  if (elapsedTime - previousTick > WARNING_PERIOD) {
    if (temp > convert(HOT_THRESHOLD) || temp < convert(COLD_THRESHOLD)) {
      playSound(temp);
      displayWarning(temp, humidity);
      previousTick = elapsedTime;
    }
  }

  lastPressed = measurement;
  }

/** Light up LED bars based on temperature severity, hot and cold. About 10f for each additional bar,
 *  starting from ~20f.
 */
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

/** Put relevant sensor readings & calculated heat index to the SSD1306 Display. */
void displayInfo(float temp, float humidity, float heatIndex) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  String tempUnit = isFahrenheit ? " F" : " C";

  display.println("Temperature: "+ String(temp) + tempUnit);
  display.println("\nHeat Index: " + String(heatIndex) + tempUnit);
  display.println("\nHumidity: " +String(humidity) + "%");

  display.display();
}

/** Put warnings to the screen based on hot/cold thresholds. */
void displayWarning(float temperature, float humidity) {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setFont(NULL);
  display.setCursor(0, 10);
  display.println("");
  display.drawBitmap(51, 5, warningIcon, 24, 24, 1);

  if (temperature > convert(HOT_THRESHOLD)) {
    display.setCursor(13, 13);
    display.println("Hot!");
    display.setCursor(89, 13);
    display.println("Hot!");
  } else {
    display.setCursor(13, 13);
    display.println("Cold!");
    display.setCursor(89, 13);
    display.println("Cold!");
  }
  String tempUnit = isFahrenheit ? " F" : " C";

  display.setCursor(0, 33);
  display.println("Temperature: " + String(temperature) + tempUnit);
  display.println("\nHumidity: " + String(humidity) + "%");

  display.display();

  delay(2000);
}

/** Buzz the buzzer with a sound that increases in pitch the higher the temperature, and vice versa. */
void playSound(float temperature) {
  tone(SPEAKER_PIN, 30 * temperature, 100);
}

/** Convert temperature to celsius or fahrenheit, or do nothing depending on the desired unit. */
float convert(float temperature) {
  if (!isFahrenheit) {
    return (temperature * 9/5) + 32;
  }
  return temperature;
}