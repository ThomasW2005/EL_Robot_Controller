#include <Arduino.h>
#include <BluetoothSerial.h>
#include <FastLED.h>
#include <HCSR04.h>

#define NUM_LEDS 4
#define DATA_PIN 23
#define CLOCK_PIN 18

#define US_TRIG_PIN 25
#define US_ECHO_PIN 26

#define LEFT_SPEED 0
#define LEFT_DIRECTION 33
#define LEFT_PIN 32
#define RIGHT_SPEED 1
#define RIGHT_DIRECTION 15
#define RIGHT_PIN 2

#define FR 0
#define FL 1
#define BL 2
#define BR 3

UltraSonicDistanceSensor sensor(US_TRIG_PIN, US_ECHO_PIN, 100);

enum
{
    CLIENT = CRGB::Green,
    NOCLIENT = CRGB::Red,
    DIR_FORWARD = 1 << 7,
    DIR_LEFT = 1 << 6,
    DIR_BACKWARD = 1 << 5,
    DIR_RIGHT = 1 << 4,
    ROT_LEFT = 1 << 3,
    ROT_RIGHT = 1 << 2,
    STOP = 0
};

BluetoothSerial bluetooth;
CRGB leds[NUM_LEDS];

int clientState = NOCLIENT;
int change = 0;
int vel = 128;

void setup()
{
    Serial.begin(115200);
    bluetooth.begin("EL-Robot Thomas Weichhart");
    bluetooth.setTimeout(1);
    FastLED.addLeds<SK9822, DATA_PIN, CLOCK_PIN, RBG>(leds, NUM_LEDS);
    fill_solid(leds, NUM_LEDS, CRGB::Red);
    FastLED.show();
    pinMode(LEFT_DIRECTION, OUTPUT);
    pinMode(RIGHT_DIRECTION, OUTPUT);
    ledcSetup(LEFT_SPEED, 50, 8);
    ledcSetup(RIGHT_SPEED, 50, 8);
    ledcAttachPin(LEFT_PIN, RIGHT_SPEED);
    ledcAttachPin(RIGHT_PIN, LEFT_SPEED);
    Serial.println("Setup done");
}

void loop()
{
    EVERY_N_MILLISECONDS(50)
    {
        if (bluetooth.hasClient() && clientState == NOCLIENT)
        {
            clientState = CLIENT;
            fill_solid(leds, NUM_LEDS, CRGB::Green);
            FastLED.show();
            Serial.println("Client connected");
        }
        else if (!bluetooth.hasClient() && clientState == CLIENT)
        {
            clientState = NOCLIENT;
            fill_solid(leds, NUM_LEDS, CRGB::Red);
            FastLED.show();
            Serial.println("Client disconnected");
            ledcWrite(LEFT_SPEED, 0);
            ledcWrite(RIGHT_SPEED, 0);
        }
        if (bluetooth.connected())
            bluetooth.write(map(sensor.measureDistanceCm(), 0, 100, 0, 255));
    }

    if (bluetooth.available())
    {
        char c = bluetooth.read();
        if (c & 1)
            vel = c;
        FastLED.setBrightness(vel);
        fill_solid(leds, NUM_LEDS, CRGB::Green);
        switch (c)
        {
        case ROT_LEFT:
            digitalWrite(LEFT_DIRECTION, LOW);
            digitalWrite(RIGHT_DIRECTION, LOW);
            ledcWrite(LEFT_SPEED, vel);
            ledcWrite(RIGHT_SPEED, vel);
            leds[BL] = CRGB::Blue;
            leds[FR] = CRGB::Blue;
            break;
        case ROT_RIGHT:
            digitalWrite(LEFT_DIRECTION, HIGH);
            digitalWrite(RIGHT_DIRECTION, HIGH);
            ledcWrite(LEFT_SPEED, vel);
            ledcWrite(RIGHT_SPEED, vel);
            leds[FL] = CRGB::Blue;
            leds[BR] = CRGB::Blue;
            break;
        case DIR_BACKWARD:
            digitalWrite(LEFT_DIRECTION, HIGH);
            digitalWrite(RIGHT_DIRECTION, LOW);
            ledcWrite(LEFT_SPEED, vel);
            ledcWrite(RIGHT_SPEED, vel);
            leds[FL] = CRGB::Blue;
            leds[FR] = CRGB::Blue;
            break;
        case DIR_RIGHT:
            digitalWrite(LEFT_DIRECTION, HIGH);
            digitalWrite(RIGHT_DIRECTION, HIGH);
            ledcWrite(LEFT_SPEED, vel);
            ledcWrite(RIGHT_SPEED, 0);
            leds[FL] = CRGB::Blue;
            leds[BL] = CRGB::Blue;
            break;
        case DIR_LEFT:
            digitalWrite(LEFT_DIRECTION, LOW);
            digitalWrite(RIGHT_DIRECTION, LOW);
            ledcWrite(LEFT_SPEED, 0);
            ledcWrite(RIGHT_SPEED, vel);
            leds[FR] = CRGB::Blue;
            leds[BR] = CRGB::Blue;
            break;
        case DIR_FORWARD:
            digitalWrite(LEFT_DIRECTION, LOW);
            digitalWrite(RIGHT_DIRECTION, HIGH);
            ledcWrite(LEFT_SPEED, vel);
            ledcWrite(RIGHT_SPEED, vel);
            leds[BL] = CRGB::Blue;
            leds[BR] = CRGB::Blue;
            break;
        case STOP:
            ledcWrite(LEFT_SPEED, 0);
            ledcWrite(RIGHT_SPEED, 0);
            FastLED.clear();
        }
        FastLED.show();
        Serial.println((int)c);
    }
}