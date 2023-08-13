/*
Hey there! This code was originally written for our YouTube channel, https://www.youtube.com/RKadeGaming
We're glad that you've taken an interest in our project, and we hope that you have a good time building it!
We've made this code public, and you're free to use it however you like. If you end up sharing the project with others though, 
we'd appreciate some attribution so that people know where to find more stuff like this.
Thanks, and have fun! :)
*/

#include <Arduino.h>
#include <BleKeyboard.h>

class DigitalInput {
    public:
        uint8_t analogPin;  // the analog pin that is assigned to this finger     
        bool isActive = false;      // whether electricity is flowing to the pin or not
        uint8_t letter;
        bool stateNeedsUpdate = false;   // stores whether the state of this input has changed from the previous iteration
};

class TouchInput {       
    public:             
        uint8_t capacitivePin;  // the GPIO pin that is assigned to this finger
        bool isActive = false;      // whether electricity is flowing to the pin or not
        uint8_t letter;
        bool stateNeedsUpdate = false;   // stores whether the state of this input has changed from the previous iteration     
        
};

// this is the bluetooth keyboard
BleKeyboard bleKeyboard;

// since this script takes control of the keyboard, I want to be able to disable it. The start pin must read LOW before the ESP32 can control the keyboard.
const int START_PIN = 0;       // BOOT button as labeled on the board

// declare force inputs (I only expect to use one, but this way the code can be easily adapted to add more)
const int NUM_FORCE_INPUTS = 1;
uint8_t forcePins[NUM_FORCE_INPUTS] = {13};

DigitalInput forceInputs[NUM_FORCE_INPUTS];
const int FORCE_THRESHOLD = 200;

// declare touch inputs
const int NUM_TOUCH_INPUTS = 1;
uint8_t touchPins[NUM_TOUCH_INPUTS] = {T5};  // corresponds to pin D12 on board
const int ACTIVE_THRESHOLD = 3; // Any reading below this value will register as a capacitive touch. Neutral reading is ~12, pressed is ~0.

// this list holds the information for each of the 5 finger inputs
TouchInput touchInputs[NUM_TOUCH_INPUTS];


unsigned long previousTime;

void setup() {
    Serial.begin(115200);
    bleKeyboard.begin();

    pinMode(BUILTIN_LED, OUTPUT);
    // don't proceed until I've confirmed that I want to give ESP32 control of the keyboard
    pinMode(START_PIN, INPUT_PULLUP);
    while(digitalRead(START_PIN) == HIGH){
        if(millis() - 500 > previousTime){
            Serial.println("Waiting for START_PIN before continuing...");
            previousTime = millis();
        }
    }

    for(int i = 0; i < NUM_FORCE_INPUTS; i++){
        forceInputs[i].analogPin = forcePins[i];
        forceInputs[i].letter = KEY_UP_ARROW;
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

// update the keystrokes to match the state of the inputs
void updateForceKeystrokes(){
    // for each key, press or release keys based on whether the pin is active or not
    for(int i = 0; i < NUM_FORCE_INPUTS; i++){
        // if the state of this input hasn't changed from the previous iteration, it doesn't need an update.
        if(forceInputs[i].stateNeedsUpdate){
            // the logic for jumping is inverted; we want to jump when the key is released, not when it's pressed
            if(!forceInputs[i].isActive){
                bleKeyboard.press(forceInputs[i].letter);
            }else{
                bleKeyboard.release(forceInputs[i].letter);
            }
            // now the state of the pin and the state of the keyboard output are correctly synced. No updates are needed for this input
            forceInputs[i].stateNeedsUpdate = false;
        }
    }
}

void updateTouchKeystrokes(){
    // for each key, press or release keys based on whether the plate is being touched
    for(int i = 0; i < NUM_TOUCH_INPUTS; i++){
        // if the state of this input hasn't changed from the previous iteration, it doesn't need an update.
        if(touchInputs[i].stateNeedsUpdate){
            if(touchInputs[i].isActive){
                bleKeyboard.press(touchInputs[i].letter);
            }else{
                bleKeyboard.release(touchInputs[i].letter);
            }
            // now the state of the pin and the state of the keyboard output are correctly synced. No updates are needed for this input
            touchInputs[i].stateNeedsUpdate = false;
        }
    }
}

// read from the pins to determine state
void updateInputs(){
    // update digital inputs
    for(int i = 0; i < NUM_FORCE_INPUTS; i++){
        // if the current reading of the pin does not match the current "active" state in storage, then mark this input as needing an update
        if(forceInputs[i].isActive && analogRead(forceInputs[i].analogPin) < FORCE_THRESHOLD){
            // this means that a key is being pressed, but the force no longer meets the "press" criteria. The input and output are out of sync and need an update.
            forceInputs[i].isActive = false;
            forceInputs[i].stateNeedsUpdate = true;
            updateForceKeystrokes();
        }
        if(!forceInputs[i].isActive && analogRead(forceInputs[i].analogPin) > FORCE_THRESHOLD){
            // this means that no key is currently being pressed, but enough force has been applied to warrant a key press. The input and output are out of sync and need an update.
            forceInputs[i].isActive = true;
            forceInputs[i].stateNeedsUpdate = true;
            updateForceKeystrokes();
        }
    }

    // update capacative inputs
    for(int i = 0; i < NUM_TOUCH_INPUTS; i++){        
        if(touchInputs[i].isActive && touchRead(touchInputs[i].capacitivePin) >= ACTIVE_THRESHOLD){
            // this means that a key is currently being pressed, but a touch is no longer being detected. The key presses are not in line with the state of the device and need an update
            touchInputs[i].isActive = false;
            touchInputs[i].stateNeedsUpdate = true;
            updateTouchKeystrokes();
        }
        if(!touchInputs[i].isActive && touchRead(touchInputs[i].capacitivePin) < ACTIVE_THRESHOLD){
            // this means that no key currently being pressed, but a touch has been registered. The key presses are not in line with the state of the device and need an update
            touchInputs[i].isActive = true;
            touchInputs[i].stateNeedsUpdate = true;
            updateTouchKeystrokes();
        }
    }
}

void loop() {
    // no need to do anything if the keyboard is not currently connected to a device
    if(bleKeyboard.isConnected()){
        digitalWrite(BUILTIN_LED, HIGH);
        updateInputs();
    }else{
        digitalWrite(BUILTIN_LED, LOW);
    }
}
