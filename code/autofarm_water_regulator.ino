/*
  Project: AutoFarm Water Regulator
  Controller: Arduino UNO
  Original academic project: Around 2020

  Description:
  Automated agricultural water-management system.
  Monitors pH, salinity, water level and rainfall.
  Controls the barrage and drainage motors.
  Uses GSM communication to send SMS alerts.
*/


#include<LiquidCrystal.h>

// LCD CONFIGURATION
LiquidCrystal lcd(7, 6, 5, 4, 3, 2); // LCD pins: RS, EN, D4, D5, D6, D7 

// Barrage motor control pins
#define barrage1 8
#define barrage2 9

// Drainage motor control pins
#define drainage1 10
#define drainage2 11

int ph, sal, level, rain;  // Sensor variable

// Analog sensor connections
// pH sensor       -> A0
// Salinity sensor -> A1
// Water level     -> A2
// Rain sensor     -> A3

// GSM SMS FUNCTION: RAIN ALER
void rain_sms() {
  Serial.print("AT");  // Check GSM modem communication
  delay(500);
  Serial.print("AT+CMGF=1");  // Set GSM module to SMS text mode
  delay(500);

  Serial.print("AT+CMGS="); // Specify recipient phone number
  Serial.write('"');
  Serial.print("YOUR_PHONE_NUMBER"); // Rewrite recipient phone number here
  Serial.println('\"');
  Serial.write('"');
  
  Serial.write(0X0D);
  Serial.print(" ALERT: Rain has been detected, barrage is closed"); // SMS message
  Serial.write(0X1A); // Ctrl+Z sends the SMS
  delay(1000);
}

// GSM SMS FUNCTION: WATER QUALITY ALERT
void sensor_sms() {
  Serial.print("AT"); // Check GSM modem communication
  delay(500);
  Serial.print("AT+CMGF=1"); // Set GSM module to SMS text mode
  delay(500);

  Serial.print("AT+CMGS="); // Specify recipient phone number
  Serial.write('"');
  Serial.print("YOUR_PHONE_NUMBER"); // Rewrite recipient phone number here
  Serial.write('"');

  Serial.write(0X0D);
  Serial.print(" ALERT: Change in canal water parameter has been detected, barrage is closed"); //SMS message
  Serial.write(0X1A); // Ctrl+Z sends the SMS
  delay(1000);
}

// BARRAGE MOTOR CONTROL

// Open the barrage
void barrage_open() {
  digitalWrite(barrage1, HIGH);
  digitalWrite(barrage2, LOW);
}

// Close the barrage
void barrage_close() {
  digitalWrite(barrage1, LOW);
  digitalWrite(barrage2, HIGH);
}

// DRAINAGE MOTOR CONTROL

// Open the drainage
void drainage_open() {
  digitalWrite(drainage1, HIGH);
  digitalWrite(drainage2, LOW);
}

// Close the drainage
void drainage_close() {
  digitalWrite(drainage1, LOW);
  digitalWrite(drainage2, HIGH);
}

// STOP ALL MOTORS
void motor_stop() {
  digitalWrite(drainage1, LOW);
  digitalWrite(drainage2, LOW);

  digitalWrite(barrage1, LOW);
  digitalWrite(barrage2, LOW);
}


void setup() {
  lcd.begin(16, 2); // Initialize 16x2 LCD
  Serial.begin(9600); // Initialize GSM serial communication

  // Configure barrage motor pins
  pinMode(barrage1, OUTPUT);
  pinMode(barrage2, OUTPUT);

  // Configure drainage motor pins
  pinMode(drainage1, OUTPUT);
  pinMode(drainage2, OUTPUT);

  motor_stop(); // Stop all motors initially

  // Display startup message
  lcd.setCursor(2,0);
  lcd.print("Agriculture");
  lcd.setCursor(2,1);
  lcd.print("Monitoring");
  delay(1000);
  lcd.clear();
}

// MAIN PROGRAM LOOP
void loop() {
  // 1. pH SENSOR READING
  ph = analogRead(A0);
  delay(200);

  // Display pH sensor raw ADC reading
  lcd.setCursor(0,0);
  lcd.print("PH:");
  lcd.setCursor(3,0);
  lcd.print(ph);

  // 2. SALINITY SENSOR READING
  sal = analogRead(A1);
  delay(200);

  // Display salinity raw ADC reading
  lcd.setCursor(6, 0);
  lcd.print("SAL:");
  lcd.setCursor(10, 0);
  lcd.print(sal);

  // 3. WATER LEVEL SENSOR READING
  level = analogRead(A2);
  delay(200);

  // Display water-level label
  lcd.setCursor(0, 1);
  lcd.print("LEVEL:");

  // 4. RAIN SENSOR READING
  rain = analogRead(A3);
  delay(200);

  // Check rain sensor threshold
  if (rain > 500) {
    barrage_open(); // Open drainage during rain
    delay(1000);

    motor_stop(); // Stop motors

    rain_sms(); // Send rain alert to farmer

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Alert send"); // Display SMS status
    delay(1000);
  }

  // 5. WATER LEVEL CONTROL

  // High water level
  if ( level > 900) {
    lcd.setCursor(0, 6);
    lcd.print("HIGH"); // Display high water condition
    delay(500);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("water level HIGH");
    lcd.setCursor(0, 1);
    lcd.print("DRAINAGE: ON");

    // Open drainage and close barrage
    drainage_open();
    barrage_close();
    delay(1000);

    motor_stop(); // Stop motors
  }

  // Low water level
  else if (level < 100) {
    lcd.setCursor(6, 1);
    lcd.print("LOW"); // Display low water condition

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("water level LOW");
    lcd.setCursor(1, 0);

    // Check whether water-quality readings are within the programmed limits
    if( sal > 200 && sal < 800 && ph > 200 && ph < 800) {
      // Acceptable sensor readings: Open barrage to allow irrigation water
      barrage_open();
      delay(1000);

      motor_stop(); // Stop motor
    }
    else
    {
      sensor_sms(); // Water-quality readings outside limits: Send alert to farmer
    }
   }
  else
  {
    lcd.setCursor(6, 1);
    lcd.print("NORMAL"); // Display normal water condition

    barrage_close(); // Keep barrage closed
    delay(1000);

    motor_stop(); // Stop motor
  }
}
