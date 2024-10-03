#ifndef SHELLYZ_H
#define SHELLYZ_H

#include <Arduino.h>
#include <HTTPClient.h>

class Shellyz {
        const char* url = "http://192.168.1.150/";
        const char* coIoT = "http://192.168.1.150/cit/d";
        std::string baseUrl =" http://192.168.1.150/";
        std::string r1Toggle = baseUrl + "/relay/0?turn=toggle";

    public:

        Shellyz() {
            HTTPClient http;

              //Start the request
            http.begin(coIoT);
            //Use HTTP GET request
            http.GET();
            //Response from server
            String response = "";
            response = http.getString();

            Serial.print("Shellyz get status ");Serial.println(response);
        }

        static void loop() {

        }

};
// unsigned long Shellyz::coIoT;
#endif