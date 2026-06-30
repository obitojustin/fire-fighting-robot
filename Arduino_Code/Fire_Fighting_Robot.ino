#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include <Servo.h>

// ---------------------------
// Object Declaration
// ---------------------------
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
Servo servo;

// ---------------------------
// Pin Definitions
// ---------------------------
#define IN1         7
#define IN2         6
#define IN3         5
#define IN4         4

#define FLAME1      A0
#define FLAME2      A1
#define FLAME3      A2

#define RELAY_PIN   11
#define SWITCH_PIN  13

// ---------------------------
// Global Variables
// ---------------------------
bool manualMode = true;

int servoPin = 12;
int angle = 90;

// ===================================================
// Setup
// ===================================================

void setup()
{
    Serial.begin(9600);

    if (!mlx.begin())
    {
        Serial.println("Error connecting to MLX90614 sensor!");
        while (1);
    }

    servo.attach(servoPin);
    servo.write(90);

    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);

    pinMode(FLAME1, INPUT);
    pinMode(FLAME2, INPUT);
    pinMode(FLAME3, INPUT);

    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);

    pinMode(SWITCH_PIN, INPUT);

    Serial.println("System initialized!");
}

// ===================================================
// Main Loop
// ===================================================

void loop()
{
    manualMode = (digitalRead(SWITCH_PIN) == HIGH);

    if (manualMode)
    {
        if (Serial.available())
        {
            char command = Serial.read();
            handleManualControl(command);
        }
    }
    else
    {
        handleAutomaticControl();
    }
}

// ===================================================
// Servo Sweep
// ===================================================

void sweepServo()
{
    for (int pos = 50; pos <= 130; pos++)
    {
        servo.write(pos);
        delay(10);
    }

    for (int pos = 130; pos >= 50; pos--)
    {
        servo.write(pos);
        delay(10);
    }
}

// ===================================================
// Fire Extinguishing
// ===================================================

void put_off_fire()
{
    stopMotors();

    digitalWrite(RELAY_PIN, HIGH);

    sweepServo();

    digitalWrite(RELAY_PIN, LOW);

    servo.write(90);
}

// ===================================================
// Manual Control
// ===================================================

void handleManualControl(char command)
{
    switch (command)
    {
        case 'F':
            stopMotors();
            moveForward();
            break;

        case 'B':
            stopMotors();
            moveBackward();
            break;

        case 'L':
            stopMotors();
            turnLeft();
            break;

        case 'R':
            stopMotors();
            turnRight();
            break;

        case 'S':
            stopMotors();
            break;

        case 'X':
            put_off_fire();
            break;

        case 'x':
            digitalWrite(RELAY_PIN, LOW);
            servo.write(90);
            break;
    }
}

// ===================================================
// Automatic Control
// ===================================================

void handleAutomaticControl()
{
    int flame1 = analogRead(FLAME1);
    int flame2 = analogRead(FLAME2);
    int flame3 = analogRead(FLAME3);

    float objectTemp = mlx.readObjectTempC();
    float ambientTemp = mlx.readAmbientTempC();

    Serial.print("Object Temp: ");
    Serial.print(objectTemp);
    Serial.print(" C, Ambient Temp: ");
    Serial.println(ambientTemp);

    Serial.print(flame1);
    Serial.print("\t");

    Serial.print(flame2);
    Serial.print("\t");

    Serial.println(flame3);

    delay(1000);

    if (objectTemp > 50.0)
    {
        stopMotors();

        digitalWrite(RELAY_PIN, HIGH);

        Serial.println("High temperature detected! Activating pump...");

        put_off_fire();

        delay(1000);

        servo.write(90);
    }
    else if (objectTemp > 35.0 && objectTemp <= 50.0)
    {
        digitalWrite(RELAY_PIN, LOW);

        moveForward();

        delay(500);
    }
    else
    {
        stopMotors();

        digitalWrite(RELAY_PIN, LOW);

        Serial.println("Low temperature detected. Stopping motors...");
    }

    if (flame1 < 350 || flame3 < 350)
    {
        stopMotors();

        digitalWrite(RELAY_PIN, HIGH);

        put_off_fire();

        delay(500);

        servo.write(90);
    }
    else if (flame2 < 350)
    {
        stopMotors();

        digitalWrite(RELAY_PIN, HIGH);

        put_off_fire();

        delay(500);

        servo.write(90);
    }
    else if (flame1 >= 351 && flame1 <= 800)
    {
        digitalWrite(RELAY_PIN, LOW);

        moveForward();

        delay(100);

        turnRight();

        delay(200);
    }
    else if (flame2 >= 351 && flame2 <= 800)
    {
        digitalWrite(RELAY_PIN, LOW);

        moveForward();

        delay(100);
    }
    else if (flame3 >= 351 && flame3 <= 800)
    {
        digitalWrite(RELAY_PIN, LOW);

        moveBackward();

        delay(100);

        turnLeft();

        delay(200);
    }
    else
    {
        digitalWrite(RELAY_PIN, LOW);

        stopMotors();
    }

    delay(100);
}

// ===================================================
// Motor Control Functions
// ===================================================

void moveForward()
{
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);

    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
}

void moveBackward()
{
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);

    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
}

void turnLeft()
{
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);

    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
}

void turnRight()
{
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);

    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
}

void stopMotors()
{
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);

    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
}