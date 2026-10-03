#include <Wire.h>
#include <SparkFun_ENS160.h>
#include <SparkFunBME280.h>

SparkFun_ENS160 myENS;
BME280 myBME;

// These lines give names to pin numbers so the code is easier to read.
const int TRIG_PIN = 6;      // Ultrasonic sensor trigger pin
const int ECHO_PIN = 5;      // Ultrasonic sensor echo pin
const int MIC_PIN = A1;      // Microphone analog input
const int LED_R = 2;         // Red LED pin (PWM capable)
const int LED_G = A3;        // Green LED pin
const int LED_B = A2;        // Blue LED pin
const int BUZZER_PIN = 3;    // Buzzer for audible alert
const int AIN1 = 13;
const int AIN2 = 12;
const int PWMA = 11;
const int STBY = 7;
const int BIN1 = 8;
const int BIN2 = 9;
const int PWMB = 10;
const int BLINKER_L = A0;    // Left blinker LED
const int BLINKER_R = 4;     // Right blinker LED

// These are fixed values used in calculations.
// We set them once here instead of scattering numbers throughout the code.

const float OBSTACLE_FAR_CM = 30.48;   // 1 foot = 30.48 cm (far detection)
const float OBSTACLE_NEAR_CM = 30.48;  // Same threshold for near stop
const float FIVE_FEET_MS = 3500;       // Approximate time in milliseconds to travel 5 feet
// we will need to tune FIVE_FEET_MS by testing our actual bot.
// Start with 3500ms and adjust up or down until the bot travels exactly 5 feet.

const int SPEED_LEFT = 255;   // Slightly slower to match right motor
const int SPEED_RIGHT = 255;  // Base speed
const int BASE_SPEED = 255;
const int FAST_SPEED = 255;            // Speed used in the hot temperature zone
const int ROOM_TEMP_THRESHOLD = 28;    // Degrees Celsius considered "room temperature"
const int HOT_TEMP_THRESHOLD = 35;     // Degrees Celsius that triggers heat response
const int MIC_THRESHOLD = 600;         // Raw analog value that counts as a loud sound
const int VOC_THRESHOLD = 400;         // Raw analog value that counts as VOC detection

// This function fires the ultrasonic sensor and returns the distance in cm.
float measureDistance() {
  // Pull TRIG_PIN LOW for 2 microseconds to reset the sensor.
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  // Pull TRIG_PIN HIGH for 10 microseconds to send a sound pulse.
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // pulseIn waits for ECHO_PIN to go HIGH, then measures how long it stays HIGH.
  // That duration is how long the sound pulse took to bounce back (in microseconds).
  long duration = pulseIn(ECHO_PIN, HIGH);

  // Convert time to distance.
  // Sound travels at 0.0343 cm per microsecond.
  // The pulse travels TO the obstacle and BACK, so divide by 2.
  float distance = (duration * 0.0343) / 2.0;
  return distance;
}


// Reads the analog temperature sensor and converts it to Celsius.
float readTemperatureCelsius() {
  return myBME.readTempC();
}

int readVOC() {
  if (myENS.checkDataStatus()) {
    return myENS.getAQI();
  }
  return 0;
}

// Sets the RGB LED to a specific color.
// Pass values 0-255 for red, green, blue.
void setLED(int r, int g, int b) {
  analogWrite(LED_R, r);
  analogWrite(LED_G, g);
  analogWrite(LED_B, b);
}


// Stops all motor movement by writing LOW to all motor pins.
void stopMotors() {
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, LOW);
  analogWrite(PWMA, 0);
  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, LOW);
  analogWrite(PWMB, 0);
}


// Drives both motors forward at a given speed.
void moveForward(int speed) {
  digitalWrite(STBY, HIGH);
  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, LOW);
  analogWrite(PWMA, SPEED_LEFT);
  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);
  analogWrite(PWMB, SPEED_RIGHT);
}


// Turns the bot 90 degrees left.
// It does this by running the RIGHT motor forward and the LEFT motor backward.
// The delay time controls how far it turns. Tune TURN_DURATION_MS for our bot.
void turnLeft90() {
  digitalWrite(BLINKER_L, HIGH);
  const int TURN_DURATION_MS = 800;
  digitalWrite(STBY, HIGH);
  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, LOW);
  analogWrite(PWMA, BASE_SPEED);
  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, HIGH);
  analogWrite(PWMB, BASE_SPEED);
  delay(TURN_DURATION_MS);
  stopMotors();
  digitalWrite(BLINKER_L, LOW);
}

// Same idea as turnLeft90, but mirrored.
// Left motor runs forward, right motor runs backward.
void turnRight90() {
  digitalWrite(BLINKER_R, HIGH);
  const int TURN_DURATION_MS = 800;
  digitalWrite(STBY, HIGH);
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, HIGH);
  analogWrite(PWMA, BASE_SPEED);
  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);
  analogWrite(PWMB, BASE_SPEED);
  delay(TURN_DURATION_MS);
  stopMotors();
  digitalWrite(BLINKER_R, LOW);
}


// Turns 180 degrees by running turnLeft90 twice.
void turnAround() {
  turnLeft90();
  delay(200);
  turnLeft90();
}


// setup() runs once when the Arduino powers on.
// we use it to configure every pin as either INPUT or OUTPUT.
void setup() {
  // Ultrasonic sensor
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Sensors (analog pins are INPUT by default)
  pinMode(MIC_PIN, INPUT);

  // Outputs
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(PWMB, OUTPUT);
  pinMode(STBY, OUTPUT);
  digitalWrite(STBY, HIGH);
  pinMode(BLINKER_L, OUTPUT);
  pinMode(BLINKER_R, OUTPUT);

  Wire.begin();
  if (!myBME.beginI2C()) {
    Serial.println("BME280 not detected. Check wiring.");
    while (1);
  }
  if (!myENS.begin()) {
    Serial.println("ENS160 not detected. Check wiring.");
    while (1);
  }

  myENS.setOperatingMode(SFE_ENS160_STANDARD);

  // Start serial monitor so we can debug sensor values.
  Serial.begin(9600);
}


// --- LOOP ---
// loop() runs repeatedly forever after setup() finishes.
// The bot follows the track in order, using flags to track its current phase.
void loop() {

  // ============================================================
  // PHASE 1: Move forward, detect obstacle
  // ============================================================
  setLED(0, 0, 0);  // LED off at start

  while (true) {
    float dist = measureDistance();
    Serial.print("Distance: "); Serial.println(dist);

    if (dist > OBSTACLE_FAR_CM) {
      // Far detection (more than 1 foot away): turn LED blue and keep moving.
      setLED(0, 0, 255);  // Blue
      moveForward(BASE_SPEED);
    }

    else if (dist <= OBSTACLE_NEAR_CM && dist > 0) {
      // Within 1 foot: stop, red LED, buzzer on.
      setLED(255, 0, 0);  // Red
      tone(BUZZER_PIN, 1000);  // Play 1000 Hz tone
      stopMotors();
      delay(500);
      noTone(BUZZER_PIN);

      // Turn left 90 degrees.
      turnLeft90();
      break;  // Exit this phase and move to Phase 2.
    }

    delay(50);  // Wait 50ms between distance readings.
  }


  // ============================================================
  // PHASE 2: Travel exactly 5 feet forward, then turn right
  // ============================================================
  setLED(0, 0, 0);
  moveForward(BASE_SPEED);

  // Drive for the amount of time that equals 5 feet of travel.
  // FIVE_FEET_MS was defined at the top. Tune it during testing.
  delay(FIVE_FEET_MS);

  stopMotors();
  delay(200);

  // Turn right 90 degrees.
  turnRight90();


  // ============================================================
  // PHASE 3: Detect high temperature, accelerate through zone
  // ============================================================
  moveForward(BASE_SPEED);

  while (true) {
    float temp = readTemperatureCelsius();
    Serial.print("Temp: "); Serial.println(temp);

    if (temp >= HOT_TEMP_THRESHOLD) {
      // Hot zone detected: red blinking LED, accelerate.
      // Blink by turning LED on and off inside the loop.
      setLED(255, 0, 0);  // Red on
      moveForward(FAST_SPEED);
      delay(150);
      setLED(0, 0, 0);    // Red off
      delay(150);
    } else if (temp <= ROOM_TEMP_THRESHOLD) {
      // Temperature returned to room level: green LED briefly, then off.
      setLED(0, 255, 0);  // Green
      delay(500);
      setLED(0, 0, 0);    // Off
      moveForward(BASE_SPEED);
      break;  // Done with hot zone, move to Phase 4.
    } else {
      // Still in transition zone: keep going fast.
      moveForward(FAST_SPEED);
      delay(50);
    }
  }


  // ============================================================
  // PHASE 4: Listen for microphone command to turn right
  // ============================================================
  // Keep moving forward until a loud sound is detected.

  while (true) {
    int micVal = analogRead(MIC_PIN);
    Serial.print("Mic: "); Serial.println(micVal);

    moveForward(BASE_SPEED);

    if (micVal > MIC_THRESHOLD) {
      // Loud sound heard: stop and turn right.
      stopMotors();
      delay(300);
      turnRight90();
      break;
    }

    delay(50);
  }


  // ============================================================
  // PHASE 5: Detect VOC (toxic gas), evacuate toward finish
  // ============================================================
  moveForward(BASE_SPEED);

  while (true) {
    int vocVal = readVOC();
    Serial.print("VOC: "); Serial.println(vocVal);

    if (vocVal >= 3){
      // VOC detected. Option A: turn around and move toward finish.
      stopMotors();
      delay(200);
      turnAround();
      moveForward(BASE_SPEED);
      break;

      // Option B (alternative): back up then turn left.
      // Comment out Option A above and uncomment the block below if we prefer.
      /*
      stopMotors();
      delay(200);
      // Reverse
      analogWrite(AIN2, BASE_SPEED);
      analogWrite(STBY, BASE_SPEED);
      delay(1000);
      stopMotors();
      delay(200);
      turnLeft90();
      moveForward(BASE_SPEED);
      break;
      */
    }

    delay(50);
  }


  // ============================================================
  // PHASE 6: Stop within finish zone
  // ============================================================
  // The bot drives forward after Phase 5.
  // Use the ultrasonic sensor to stop before hitting the wall.

  while (true) {
    float dist = measureDistance();
    Serial.print("Finish dist: "); Serial.println(dist);

    if (dist < 15.0) {
      // Wall is within 15 cm. Stop.
      stopMotors();
      setLED(0, 255, 0);  // Green to signal completion.
      break;
    }

    moveForward(BASE_SPEED);
    delay(50);
  }

  // Bot is done. Stop permanently.
  // loop() would normally run again, but stopMotors() keeps it idle.
  stopMotors();
  while (true) { delay(1000); }  // Infinite idle loop.
}
