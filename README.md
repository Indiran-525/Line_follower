# ESP32 PID Line Follower (BLE Tunable)

A fast and configurable **ESP32-based line follower** using **QTR-8 analog sensors**, **TB6612 motor driver**, and **Bluetooth Low Energy (BLE)** for live PID tuning.

---

## Features
- PID based line following
- BLE tuning using Bluetooth Serial Terminal app (no re-upload required)
- QTR-8 analog sensor array
- All-black detection handling
- Auto calibration mode
- Adjustable base speed
- ESP32 BluetoothSerial interface

---

## Hardware
- ESP32
- QTR-8A Analog Sensor Array
- TB6612FNG Motor Driver
- 2x n20 Motors
- LiPo Battery (7.4V)

---

## Wiring

### QTR-8A → ESP32
```
S0  → 26
S1  → 27
S2  → 14
S3  → 15
S4  → 13
S5  → 4
S6  → 35
S7  → 34
VCC → 3.3V
GND → GND
```

### TB6612 → ESP32
```
AIN1 → 21
AIN2 → 22
BIN1 → 25
BIN2 → 33
PWMA → 23
PWMB → 32
STBY → 19
```

### Motors
```
Left Motor  → A01 / A02
Right Motor → B01 / B02
```

---

## How It Works
1. Robot starts in **WAITING mode**
2. Connect via BLE
3. Tune PID if needed
4. Send `calibrate`
5. Sensors calibrate
6. Robot switches to **RUNNING mode**
7. Line following begins

---

## BLE Commands
```
kp10       -> Set proportional gain
ki0        -> Set integral gain
kd0.1      -> Set derivative gain
base70     -> Set base speed
calibrate  -> Start calibration and run
```

Example tuning:
```
kp12
kd0.15
base85
```

---

## PID Formula
```
correction = Kp * error + Ki * integral + Kd * derivative
```

Motor speeds:
```
left  = baseSpeed - correction
right = baseSpeed + correction
```

---

## Default Parameters
```
Kp = 10
Ki = 0
Kd = 0.1
baseSpeed = 70
maxSpeed = 255
```

---

## Calibration
- Place robot over track
- Send `calibrate`
- Robot samples sensors
- Automatically enters running mode

---

## BLE Device Name
```
LineBot
```

---

## Modes
| Mode | Description |
|------|-------------|
| WAITING | BLE tuning allowed |
| RUNNING | Line following active |

---

## All-Black Detection
When all sensors detect black:
- Robot continues forward
- Uses last error for correction
- Prevents stop at intersections

---

## Libraries Required
Install from Arduino Library Manager:
- BluetoothSerial (ESP32 core)
- QTRSensors

---



## NOTED POINTS FROM LFR

- ADC2 pins of esp32 will get blocked if BLE/WIFI is enabled
- getsensorvalues() function from qtr returns the raw analog value
  that value depends on the resolution of the ADC in the ESP
- QTR-8A is different from QTR-8RC, 8A can work only in typeAnalog mode
