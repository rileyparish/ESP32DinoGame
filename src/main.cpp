/*
Hey there! This code was originally written for our YouTube channel, https://www.youtube.com/RKadeGaming
We're glad that you've taken an interest in our project, and we hope that you have a good time building it!
We've made this code public, and you're free to use it however you like. If you end up sharing the project with others though, 
we'd appreciate some attribution so that people know where to find more stuff like this.
Thanks, and have fun! :)
*/

#include <Arduino.h>
#include <BleKeyboard.h>

class TouchInput {       
    public:             
        uint8_t capacitivePin;  // the GPIO pin that is assigned to this finger
        bool isActive = false;      // whether electricity is flowing to the pin or not
        uint8_t letter;
        bool stateNeedsUpdate = false;   // stores whether the state of this input has changed from the previous iteration     
        
};

class DigitalInput {
    public:
        uint8_t digitalPin;  // the digital pin that is assigned to this finger     
        bool isActive = false;      // whether electricity is flowing to the pin or not
        uint8_t letter;
        bool stateNeedsUpdate = false;   // stores whether the state of this input has changed from the previous iteration

};
// this is the bluetooth keyboard
BleKeyboard bleKeyboard;

// since this script takes control of the keyboard, I want to be able to disable it. The start pin must read LOW before the ESP32 can control the keyboard.
const int START_PIN = 0;       // BOOT button as labeled on the board

// these are the physical GPIO pins on the ESP32
const int NUM_TOUCH_INPUTS = 1;
uint8_t touchPins[NUM_TOUCH_INPUTS] = {T5};  // corresponds to pin D12 on board
const int ACTIVE_THRESHOLD = 25; // Any reading below this value will register as a capacitive touch.

// this list holds the information for each of the 5 finger inputs
TouchInput touchInputs[NUM_TOUCH_INPUTS];


const int NUM_DIGITAL_INPUTS = 1;
uint8_t digitalPins[NUM_DIGITAL_INPUTS] = {33};

DigitalInput digitalInputs[NUM_DIGITAL_INPUTS];

unsigned long previousTime;

void setup() {
    Serial.begin(115200);
    bleKeyboard.begin();
    // don't proceed until I've confirmed that I want to give ESP32 control of the keyboard
    pinMode(START_PIN, INPUT_PULLUP);
    while(digitalRead(START_PIN) == HIGH){
        if(millis() - 500 > previousTime){
            Serial.println("Waiting for START_PIN before continuing...");
            previousTime = millis();
        }
    }

    // init digital inputs:
    for(int i = 0; i< NUM_DIGITAL_INPUTS; i++){
        pinMode(digitalPins[i], INPUT);
    }
    for(int i = 0; i < NUM_DIGITAL_INPUTS; i++){
        digitalInputs[i].digitalPin;
        digitalInputs[i].letter = KEY_UP_ARROW;
    }

    // init capacative inputs:
    for(int i = 0; i < NUM_TOUCH_INPUTS; i++){
        pinMode(touchPins[i], INPUT);
    }
    for(int i = 0; i < NUM_TOUCH_INPUTS; i++){
        touchInputs[i].capacitivePin = touchPins[i];
        touchInputs[i].letter = KEY_DOWN_ARROW;
    }
}

void loop() {
    // no need to do anything if the keyboard is not currently connected to a device
    if(bleKeyboard.isConnected()){
    }
}
