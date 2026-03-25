#include <LiquidCrystal.h>

// LCD pin setup
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// LED pins
const int redLED    = 6;   // Gas alert
const int yellowLED = 7;   // Heat alert
const int greenLED  = 8;   // Person present

// Sensor pins
const int gasSensorPin  = A0;
const int tempSensorPin = A1;
const int trigPin       = 9;
const int echoPin       = 10;
const int buttonPin     = 13;

// Thresholds
const int   GAS_THRESHOLD  = 550;
const float TEMP_THRESHOLD = 50.0;
const float PRESENCE_DIST  = 50.0; // cm

// State
bool alarmActive = false;

void setup() {
  pinMode(redLED,    OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(greenLED,  OUTPUT);
  pinMode(trigPin,   OUTPUT);
  pinMode(echoPin,   INPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  Serial.begin(9600);
  lcd.begin(16, 2);

  lcd.setCursor(0, 0);
  lcd.print("Kitchen Monitor");
  lcd.setCursor(0, 1);
  lcd.print("  Initializing..");
  delay(2000);
  lcd.clear();
}

// --- Ultrasonic distance read ---
float getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH);
  return duration * 0.034 / 2.0;
}

// --- Temperature read ---
float getTemperature() {
  int reading = analogRead(tempSensorPin);
  float voltage = reading * 5.0 / 1024.0;
  return (voltage - 0.5) * 100.0;
}

void allLEDsOff() {
  digitalWrite(redLED,    LOW);
  digitalWrite(yellowLED, LOW);
  digitalWrite(greenLED,  LOW);
}

void loop() {
  // --- Button: Reset alarm ---
  if (digitalRead(buttonPin) == LOW) {
    alarmActive = false;
    allLEDsOff();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Alarm Reset!    ");
    delay(1000);
    lcd.clear();
  }

  // --- Read sensors ---
  int   gasVal  = analogRead(gasSensorPin);
  float tempC   = getTemperature();
  float distCm  = getDistance();

  bool gasDetected  = (gasVal >= GAS_THRESHOLD);
  bool highTemp     = (tempC  >= TEMP_THRESHOLD);
  bool personNear   = (distCm <= PRESENCE_DIST && distCm > 0);

  // --- Serial monitor (debug) ---
  Serial.print("Gas: ");   Serial.print(gasVal);
  Serial.print(" | Temp: "); Serial.print(tempC);
  Serial.print(" C | Dist: "); Serial.print(distCm);
  Serial.println(" cm");

  // --- LED logic ---
  if (gasDetected) {
    alarmActive = true;
    digitalWrite(redLED, HIGH);
  } else {
    digitalWrite(redLED, LOW);
  }

  if (highTemp) {
    alarmActive = true;
    digitalWrite(yellowLED, HIGH);
  } else {
    digitalWrite(yellowLED, LOW);
  }

  if (personNear && !gasDetected && !highTemp) {
  digitalWrite(greenLED, HIGH);
	} else {
  digitalWrite(greenLED, LOW);
  }

  // --- LCD Display ---
  // Line 1: Temperature + Person presence
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print((int)tempC);
  lcd.print("C ");
  lcd.print(personNear ? "PERSON NEAR " : "NO ONE NEAR ");

  // Line 2: Gas status or alert
  lcd.setCursor(0, 1);
  if (gasDetected && highTemp) {
    lcd.print("GAS! + HIGH TEMP");
  } else if (gasDetected) {
    lcd.print("!! GAS LEAK !!  ");
  } else if (highTemp) {
    lcd.print("!! HIGH TEMP !! ");
  } else {
    lcd.print("Status: SAFE    ");
  }

  delay(2400);
}