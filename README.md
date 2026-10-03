# HAZBOT

An Arduino robot that drives a hazard course on its own. It uses five sensors to spot an obstacle, a hot zone, a voice command, and toxic gas, then reacts to each one and parks at the finish line.

<!-- Add a photo: upload it to an images/ folder, then replace this comment with:
![HAZBOT](images/hazbot.jpg)
-->

## What it does

The code runs the course as six phases in order:

| Phase | Sensor | What the bot does |
|---|---|---|
| 1. Obstacle | HC-SR04 ultrasonic | Drives forward with a blue LED. Inside 1 ft (30.48 cm) it stops, turns the LED red, sounds a 1 kHz buzzer, and turns left 90° |
| 2. Fixed distance | Timer | Drives 5 ft (tuned by time, 3500 ms), then turns right 90° |
| 3. Hot zone | BME280 temperature | At 35 °C or above it blinks red and speeds up. Back at 28 °C or below it flashes green and moves on |
| 4. Voice command | Analog microphone | Keeps driving until it hears a sound above the threshold, then turns right 90° |
| 5. Toxic gas | ENS160 air quality | When the air quality index reaches 3, it turns 180° and heads for the exit |
| 6. Finish | HC-SR04 ultrasonic | Stops within 15 cm of the end wall and shows a solid green LED |

Left and right blinker LEDs light up during each turn.

## Hardware

- Arduino-compatible board
- TB6612FNG dual motor driver and two DC drive motors
- HC-SR04 ultrasonic distance sensor
- SparkFun BME280 (temperature) over I2C
- SparkFun ENS160 (air quality / VOC) over I2C
- Analog microphone module
- RGB LED, piezo buzzer, two blinker LEDs

### Pin map

| Part | Pin |
|---|---|
| Ultrasonic TRIG / ECHO | 6 / 5 |
| Microphone | A1 |
| RGB LED R / G / B | 2 / A3 / A2 |
| Buzzer | 3 |
| Motor A: AIN1 / AIN2 / PWMA | 13 / 12 / 11 |
| Motor B: BIN1 / BIN2 / PWMB | 8 / 9 / 10 |
| Motor driver STBY | 7 |
| Blinker left / right | A0 / 4 |
| BME280 + ENS160 | SDA / SCL (I2C) |

## Run it

1. Install the Arduino IDE.
2. In Library Manager, install **SparkFun BME280** and **SparkFun Indoor Air Quality Sensor - ENS160**.
3. Open `hazbot.ino`, pick your board and port, and upload.
4. Open the Serial Monitor at 9600 baud to watch distance, temperature, mic, and VOC readings live.

## Tuning

Every surface and battery level changes how far the bot travels, so set these constants at the top of the file by testing on your course:

- `FIVE_FEET_MS`: raise or lower until the bot covers exactly 5 ft
- `TURN_DURATION_MS` (inside `turnLeft90` and `turnRight90`): adjust until each turn is 90°
- `MIC_THRESHOLD`: set just above the room's background noise reading
- `HOT_TEMP_THRESHOLD` / `ROOM_TEMP_THRESHOLD`: match the heat source used on the course
