#include <Arduino.h>
#include <BluetoothSerial.h>
#include <FastLED.h>

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

#define CLIENT CRGB::Green
#define NOCLIENT CRGB::Red

namespace mask
{
    enum
    {
        FORWARD = 1 << 0,
        LEFT = 1 << 1,
        DOWN = 1 << 2,
        RIGHT = 1 << 3,
        ROT_LEFT = 1 << 4,
        ROT_RIGHT = 1 << 5,
    };
}

BluetoothSerial bluetooth;
CRGB leds[NUM_LEDS];

int clientState = NOCLIENT;

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
    }

    if (bluetooth.available() >= 2)
    {
        byte state = bluetooth.read();
        byte speed = bluetooth.read();
        FastLED.setBrightness(speed);
        fill_solid(leds, NUM_LEDS, CRGB::Green);

        ledcWrite(LEFT_SPEED, speed);
        ledcWrite(RIGHT_SPEED, speed);

        if (!state)
        {
            ledcWrite(LEFT_SPEED, 0);
            ledcWrite(RIGHT_SPEED, 0);
        }
        if (state & mask::FORWARD)
        {
            digitalWrite(LEFT_DIRECTION, LOW);
            digitalWrite(RIGHT_DIRECTION, HIGH);
            leds[BL] = CRGB::Blue;
            leds[BR] = CRGB::Blue;
        }
        if (state & mask::LEFT)
        {
            digitalWrite(LEFT_DIRECTION, LOW);
            digitalWrite(RIGHT_DIRECTION, LOW);
            ledcWrite(LEFT_SPEED, 0);
            leds[FR] = CRGB::Blue;
            leds[BR] = CRGB::Blue;
        }
        if (state & mask::DOWN)
        {
            digitalWrite(LEFT_DIRECTION, HIGH);
            digitalWrite(RIGHT_DIRECTION, LOW);
            leds[FL] = CRGB::Blue;
            leds[BL] = CRGB::Blue;
        }
        if (state & mask::RIGHT)
        {
            digitalWrite(LEFT_DIRECTION, HIGH);
            digitalWrite(RIGHT_DIRECTION, HIGH);
            ledcWrite(RIGHT_SPEED, 0);
            leds[FL] = CRGB::Blue;
            leds[BR] = CRGB::Blue;
        }
        if (state & mask::ROT_LEFT)
        {
            digitalWrite(LEFT_DIRECTION, LOW);
            digitalWrite(RIGHT_DIRECTION, LOW);
            leds[BL] = CRGB::Blue;
            leds[FR] = CRGB::Blue;
        }
        if (state & mask::ROT_RIGHT)
        {
            digitalWrite(LEFT_DIRECTION, HIGH);
            digitalWrite(RIGHT_DIRECTION, HIGH);
            leds[FL] = CRGB::Blue;
            leds[BR] = CRGB::Blue;
        }

        FastLED.show();
    }
}