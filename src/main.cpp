/**
 *   ESP32 SvelteKit
 *
 *   A simple, secure and extensible framework for IoT projects for ESP32 platforms
 *   with responsive Sveltekit front-end built with TailwindCSS and DaisyUI.
 *   https://github.com/theelims/ESP32-sveltekit
 *
 *   Copyright (C) 2018 - 2023 rjwats
 *   Copyright (C) 2023 - 2024 theelims
 *
 *   All Rights Reserved. This software may be modified and distributed under
 *   the terms of the LGPL v3 license. See the LICENSE file for details.
 **/

#include <ESP32SvelteKit.h>
#include <LightMqttSettingsService.h>
#include <LightStateService.h>
#include <PsychicHttpServer.h>
#include <OneButton.h>
#include <LaserWar.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif


#define IR_PIN 21
#define SERIAL_BAUD_RATE 115200

PsychicHttpServer server;
ESP32SvelteKit esp32sveltekit(&server, 120);
LightMqttSettingsService lightMqttSettingsService = LightMqttSettingsService(&server,
                                                                             esp32sveltekit.getFS(),
                                                                             esp32sveltekit.getSecurityManager());

LightStateService lightStateService = LightStateService(&server,
                                                        esp32sveltekit.getSocket(),
                                                        esp32sveltekit.getSecurityManager(),
                                                        esp32sveltekit.getMqttClient(),
                                                        &lightMqttSettingsService);

#define PIN       13
#define NUMPIXELS 1 
#define BRIGHTNESS  20
#define BUTTON_PIN 14

#define ADAFRUIT_RMT_CHANNEL_MAX 1
Adafruit_NeoPixel strip(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

LaserWar lwSender(IR_PIN);
LWCommand bangCmd(LwSetting::AdminCommand, LwAdminSetting::Explode);
LWCommand blindCmd(LwSetting::AdminCommand, LwAdminSetting::StunPlayer);

bool fired = false;
bool trigger = false;
bool broke = true;

const int triggertimer = 3000;
const int resettimer = 10000;

static unsigned long resetTime = 0;
static unsigned long triggerTime = 0;
static bool resetInProgress = false;

OneButton btn = OneButton(
  BUTTON_PIN,  // Input pin for the button
  true,        // Button is active LOW
  true         // Enable internal pull-up resistor
);
// Create tasks
TaskHandle_t led_T    = nullptr;  // LED Task for LED control

// Declarations
void ledTask(void *pvParameters); // LED

// Handler function for a single click:
static void handlelongstop() {
  if(fired==false){
    trigger = true;
    triggerTime = millis();
  }
}

static void splintsave()
{  
  if(fired==true && resetInProgress==false && trigger==false){
    resetTime = millis();
    resetInProgress = true;
    Serial.println("resetInProgress");
  } else if(resetInProgress && millis() - resetTime >= resettimer){
    Serial.println("Reset!");
    fired = false;
    resetInProgress = false;
  } else if(fired==true && trigger==false){
    broke = true;
  }
}

void setup()
{
    Serial.begin(SERIAL_BAUD_RATE);
    pinMode(BUTTON_PIN, INPUT);
    pinMode(IR_PIN, OUTPUT);
    if(digitalRead(BUTTON_PIN)==HIGH){
          // start ESP32-SvelteKit
    Serial.println("HIGH");
    esp32sveltekit.begin();
    // load the initial light settings
    lightStateService.begin();
    // start the light service
    lightMqttSettingsService.begin();
    strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
    strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)
    strip.setPixelColor(0, strip.Color(128,0,128));
    strip.show();            // Turn OFF all pixels ASAP
    } else {
                                     // Start Tasks here:
    xTaskCreatePinnedToCore(ledTask,   /* Task function. */
                        "ledTask", /* name of task. */
                        4096,                /* Stack size of task */
                        NULL,                /* parameter of the task */
                        1,                   /* priority of the task */
                        &led_T,            /* Task handle to keep track of created task */
                        0);                  /* pin task to core 0 */
    Serial.println("boot");
    btn.attachLongPressStop(handlelongstop);
    btn.attachDuringLongPress(splintsave);
    btn.setLongPressIntervalMs(500);
    btn.tick();  
    } 
    esp32sveltekit.begin();
    // load the initial light settings
    lightStateService.begin();
    // start the light service
    lightMqttSettingsService.begin();
    Serial.println("ready");
}

void loop()
{
    btn.tick();
    if(broke==false && fired==false && trigger==true && millis() - triggerTime >= triggertimer ){
      trigger = false;
      lwSender.send(bangCmd);
      //lwSender.send(blindCmd);
      Serial.println("Clicked!");
      fired = true;
      trigger = false;
  }
}

void ledTask(void *pvParameters)
 {

  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)
  strip.setPixelColor(0, strip.Color(255,0,0));
 for (;;)
  { 
    if (fired==false && trigger==true){
      static bool ledState = false;
      static unsigned long previousMillis = 0;
      const unsigned long interval = 250; // Blink interval in milliseconds
      unsigned long currentMillis = millis();
      if (currentMillis - previousMillis >= interval) {
        ledState = !ledState;
        if (ledState) {
          strip.setPixelColor(0, strip.Color(0,0,255)); // Blue
        } else {
          strip.setPixelColor(0, strip.Color(0,0,0)); // Off
        }
        strip.show();  
        previousMillis = currentMillis;
      }
    } else if (fired==true && resetInProgress==true){
      static bool ledState = false;
      static unsigned long previousMillis = 0;
      const unsigned long interval = 250; // Blink interval in milliseconds
      unsigned long currentMillis = millis();
      if (currentMillis - previousMillis >= interval) {
        ledState = !ledState;
        if (ledState) {
          strip.setPixelColor(0, strip.Color(0,255,255)); // Blue
        } else {
          strip.setPixelColor(0, strip.Color(0,0,0)); // Off
        }
        strip.show();  
        previousMillis = currentMillis;
      }
    } else if(fired==true){
      static uint16_t current_pixel = 0;
      strip.setPixelColor(0, strip.Color(0,255,0));
      strip.show();  
    } else if (fired==false && trigger==false && resetInProgress==false){
      strip.setPixelColor(0, strip.Color(255,0,0));
      strip.show();  
    } else if( broke== true){
      strip.setPixelColor(0, strip.Color(255,255,0));
      strip.show(); 
    } 
  }
}