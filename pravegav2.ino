#include <BluetoothSerial.h>
#include <QTRSensors.h>

// === BLE Setup ===
BluetoothSerial SerialBT;
bool bleActive = true;

// === QTR Sensor Setup ===
QTRSensors qtr;
const uint8_t sensorPins[] = {26, 27, 14, 15, 13, 4, 35, 34};  
const uint8_t sensorCount = 8;
uint16_t sensorValues[sensorCount];


// === Motor Pins ===
#define AIN1 21
#define AIN2 22
#define BIN1 25
#define BIN2 33
#define PWMA 23
#define PWMB 32
#define STBY 19

// === PID Control Variables ===
float Kp = 10, Ki = 0.0, Kd = 0.1;
int baseSpeed = 70;
int maxSpeed = 255;
float error = 0, lastError = 0, integral = 0, derivative = 0;

// === Flags ===
enum Mode { WAITING, RUNNING };
Mode mode = WAITING;

// === Motor Function ===
void setMotor(int leftSpeed, int rightSpeed) {
  digitalWrite(AIN1, leftSpeed >= 0);
  digitalWrite(AIN2, leftSpeed < 0);
  analogWrite(PWMA, abs(leftSpeed));

  digitalWrite(BIN1, rightSpeed >= 0);
  digitalWrite(BIN2, rightSpeed < 0);
  analogWrite(PWMB, abs(rightSpeed));
}

// === BLE Setup ===
void setupBLE() {
  if (!SerialBT.begin("LineBot")) {
    Serial.println("Failed to start BLE");
    return;
  }
  Serial.println("BLE started. Awaiting commands...");
  bleActive = true;
}

// === BLE Stop ===
void stopBLE() {
  if (bleActive) {
    SerialBT.end();
    Serial.println("BLE stopped. Freed ADC2 for sensors.");
    bleActive = false;
  }
}

// === PID Update ===
void updatePID(String cmd) {
  if (cmd.startsWith("kp")) {
    Kp = cmd.substring(2).toFloat();
    Serial.printf("Updated Kp = %.2f\n", Kp);
  } else if (cmd.startsWith("kd")) {
    Kd = cmd.substring(2).toFloat();
    Serial.printf("Updated Kd = %.2f\n", Kd);
  } else if (cmd.startsWith("ki")) {
    Ki = cmd.substring(2).toFloat();
    Serial.printf("Updated Ki = %.2f\n", Ki);
  } else if (cmd.startsWith("base")) {
    baseSpeed = cmd.substring(4).toInt();
    Serial.printf("Updated Base Speed = %d\n", baseSpeed);
  }
}

// === Line Following with all-black detection ===
void lineFollow() 
{
  qtr.read(sensorValues);  // read current sensor values
  bool allBlack = true;

  // Check if all sensors are detecting black
  float nsc[]={0,0,0,0,0,0,0,0};
  for (int i = 0; i < sensorCount; i++) 
  {
    nsc[i] = sensorValues[i]/4095.0;
    Serial.print(nsc[i]);
        Serial.print("  ");
    if (nsc[i] < 0.85) 
    {  // some white detected
      allBlack = false;
      break;  
    }
    Serial.println();
  }
  

  if (allBlack) 
  {
    // Keep moving forward based on last PID error
    int leftSpeed  = constrain(baseSpeed - lastError * Kp * 100, 0, maxSpeed);
    int rightSpeed = constrain(baseSpeed + lastError * Kp * 100, 0, maxSpeed);
    setMotor(leftSpeed, rightSpeed);
    Serial.println("All sensors black → moving forward");
    return;
  }

  // --- Normal PID ---
  uint16_t position = qtr.readLineBlack(sensorValues);
  error = ((int)position - 3500) / 1000.0; // 8 sensors → midpoint ~ 3500

  integral += error;
  derivative = error - lastError;
  lastError = error;

  float correction = Kp * error + Ki * integral + Kd * derivative;

  int leftSpeed  = constrain(baseSpeed - correction * 100, 0, maxSpeed);
  int rightSpeed = constrain(baseSpeed + correction * 100, 0, maxSpeed);

  setMotor(leftSpeed, rightSpeed);

  Serial.printf("Pos:%d Err:%.2f L:%d R:%d\n", position, error, leftSpeed, rightSpeed);
}


// === Calibration ===
void calibrateSensors() {
  Serial.println("Calibrating sensors...");
  for (int i = 0; i < 1000; i++) {
    qtr.calibrate();
    delay(5);
  }
  Serial.println("Calibration complete.");
}

// === Setup ===
void setup() {
  Serial.begin(115200);

  // Motor Pins
  pinMode(STBY, OUTPUT); digitalWrite(STBY, HIGH);
  pinMode(AIN1, OUTPUT); pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT); pinMode(BIN2, OUTPUT);
  pinMode(PWMA, OUTPUT); pinMode(PWMB, OUTPUT);

  // QTR Setup
  qtr.setTypeAnalog();
  qtr.setSensorPins(sensorPins, sensorCount);

  // BLE
  setupBLE();

  Serial.println("Commands: kpX, kiX, kdX, baseX, calibrate");
}

// === Loop ===
void loop() {
  // --- BLE Mode (WAITING) ---
  if (mode == WAITING && bleActive && SerialBT.available()) {
    String cmd = SerialBT.readStringUntil('\n');
    cmd.trim();
    Serial.printf("Received: %s\n", cmd.c_str());

    if (cmd == "calibrate") {
      stopBLE();          // 1. Stop BLE
      calibrateSensors(); // 2. Calibrate sensors
      mode = RUNNING;     // 3. Enter running mode
      Serial.println("Calibration done → RUNNING mode");
    } else {
      updatePID(cmd);
    }
  }

  // --- Running Mode ---
  if (mode == RUNNING) {
    lineFollow();
  }
}
