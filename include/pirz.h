#ifndef PIRZ_H
#define PIRZ_H

#include <Arduino.h>

#ifndef PIR_PIN
    #define PIR_PIN 12
#endif

typedef void (*PirCallBack)();

class Pirz {
        static PirCallBack pirMotion;
        static PirCallBack pirMotionEnd;
        // Timer: Auxiliary variables
        /*static const uint8_t timeSeconds = 10;
        static unsigned long now;
        static unsigned long lastTrigger;
        static boolean startTimer;
*/


        // Checks if motion was detected, sets LED HIGH and starts a timer
        /*static void detectsMovement() {
            SERIALZ.println("MOTION DETECTED!!!");
            startTimer = true;
            lastTrigger = millis();
            motionDetected = true;
        }*/

    public:
        
        static bool motionDetected;

        static void setup(PirCallBack pirMotion = [](){}, PirCallBack pirMotionEnd = [](){}) {
            Pirz::pirMotion = pirMotion;
            Pirz::pirMotionEnd = pirMotionEnd;
            
            pinMode(PIR_PIN, INPUT_PULLUP);
            
            // attachInterrupt(digitalPinToInterrupt(motionSensor), Pirz::detectsMovement, RISING);

            SERIALZ.print("Pirz init pin ");SERIALZ.println(PIR_PIN);
        }

        static void loop() {
            int pirVal = digitalRead(PIR_PIN);
            if (motionDetected == false && pirVal == HIGH) {
                motionDetected = true;
                // SERIALZ.println("Pirz!");
                pirMotion();
            }
            if (motionDetected == true && pirVal == LOW) {
                motionDetected = false;
                // SERIALZ.println("Pirz end motion!");
                pirMotionEnd();
            }

            // Current time
    /*        now = millis();
            // Turn off the LED after the number of seconds defined in the timeSeconds variable
            if(startTimer && (now - lastTrigger > (timeSeconds*1000))) {
                SERIALZ.println("Motion stopped...");
                motionDetected = false;
                startTimer = false;
            }*/
        }

};
/*unsigned long Pirz::now;
unsigned long Pirz::lastTrigger = 0;
boolean Pirz::startTimer = false;*/
bool Pirz::motionDetected = false;
PirCallBack Pirz::pirMotion;
PirCallBack Pirz::pirMotionEnd;
#endif