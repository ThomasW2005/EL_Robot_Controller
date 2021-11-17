#include <Arduino.h>
#include <BluetoothSerial.h>
#include <FastLED.h>
#include <vector>

// #define DEBUG

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

namespace led_pos
{
    enum
    {
        front_right = 0,
        front_left = 1,
        back_left = 2,
        back_right = 3
    };
}

namespace mask
{
    enum
    {
        FORWARD = 1,
        LEFT = 2,
        DOWN = 3,
        RIGHT = 4,
        ROT_LEFT = 5,
        ROT_RIGHT = 6,
        SET_SPEED = 7,
        CLIENT,
        NOCLIENT
    };
}

std::vector<byte> keys;
BluetoothSerial bluetooth;
CRGB leds[NUM_LEDS];
byte speed = 255;
byte clientState = mask::NOCLIENT;

void vector_add_or_remove(std::vector<byte> &v, byte value);
void print_vector(std::vector<byte> v);

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
#ifdef DEBUG
    Serial.println("Setup done");
#endif
}

void loop()
{
    EVERY_N_MILLISECONDS(50)
    {
        if (bluetooth.hasClient() && clientState == mask::NOCLIENT)
        {
            clientState = mask::CLIENT;
            fill_solid(leds, NUM_LEDS, CRGB::Green);
            FastLED.show();
#ifdef DEBUG
            Serial.println("Client connected");
#endif
        }
        else if (!bluetooth.hasClient() && clientState == mask::CLIENT)
        {
            clientState = mask::NOCLIENT;
            fill_solid(leds, NUM_LEDS, CRGB::Red);
            FastLED.show();
#ifdef DEBUG
            Serial.println("Client disconnected");
#endif
            ledcWrite(LEFT_SPEED, 0);
            ledcWrite(RIGHT_SPEED, 0);
        }
    }

    if (bluetooth.isReady() && bluetooth.available())
    {
        byte state = bluetooth.read(); // each keypress gets put onto the stack and gets removed if release event is triggered (namespace mask)

        if (state == mask::SET_SPEED)
            speed = bluetooth.read();
        else
            vector_add_or_remove(keys, state);
#ifdef DEBUG
        print_vector(keys);
#endif

        FastLED.setBrightness(speed);
        fill_solid(leds, NUM_LEDS, CRGB::Green);
        ledcWrite(LEFT_SPEED, speed);
        ledcWrite(RIGHT_SPEED, speed);

        switch (keys[keys.size() - 1])
        {
        case mask::FORWARD:
            digitalWrite(LEFT_DIRECTION, LOW);
            digitalWrite(RIGHT_DIRECTION, HIGH);
            leds[led_pos::front_left] = CRGB::Blue;
            leds[led_pos::front_right] = CRGB::Blue;
            break;
        case mask::LEFT:
            digitalWrite(LEFT_DIRECTION, LOW);
            digitalWrite(RIGHT_DIRECTION, LOW);
            ledcWrite(LEFT_SPEED, 0);
            leds[led_pos::front_left] = CRGB::Blue;
            break;
        case mask::DOWN:
            digitalWrite(LEFT_DIRECTION, HIGH);
            digitalWrite(RIGHT_DIRECTION, LOW);
            leds[led_pos::back_left] = CRGB::Blue;
            leds[led_pos::back_right] = CRGB::Blue;
            break;
        case mask::RIGHT:
            digitalWrite(LEFT_DIRECTION, HIGH);
            digitalWrite(RIGHT_DIRECTION, HIGH);
            ledcWrite(RIGHT_SPEED, 0);
            leds[led_pos::front_right] = CRGB::Blue;
            break;
        case mask::ROT_LEFT:
            digitalWrite(LEFT_DIRECTION, LOW);
            digitalWrite(RIGHT_DIRECTION, LOW);
            leds[led_pos::front_left] = CRGB::Blue;
            leds[led_pos::back_right] = CRGB::Blue;
            break;
        case mask::ROT_RIGHT:
            digitalWrite(LEFT_DIRECTION, HIGH);
            digitalWrite(RIGHT_DIRECTION, HIGH);
            leds[led_pos::back_left] = CRGB::Blue;
            leds[led_pos::front_right] = CRGB::Blue;
            break;
        default:
            ledcWrite(LEFT_SPEED, 0);
            ledcWrite(RIGHT_SPEED, 0);
            break;
        }
        FastLED.show();
    }
}

void print_vector(std::vector<byte> v)
{
    Serial.printf("<Stack size=%d>\n", v.size());
    std::for_each(v.begin(), v.end(), [](byte b)
                  { Serial.println(b); });
    Serial.printf("</Stack>\n");
}

void vector_add_or_remove(std::vector<byte> &v, byte value)
{
    auto it = std::find(v.begin(), v.end(), value);
    if (it != v.end())
        v.erase(it);
    else
        v.push_back(value);
}