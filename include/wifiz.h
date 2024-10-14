#ifndef WIFIZ_H
#define WIFIZ_H

#include <WiFi.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>

class Wifiz
{
    const char *ssid;
    const char *password;

public:
    bool connected = false;

    Wifiz(const char *ssid, const char *password) : ssid(ssid), password(password) {}

    void begin()
    {
        SERIALZ.println("WiFi begin");
        WiFi.begin(ssid, password);
    }

    bool justConnected()
    {
        if (this->connected == false && WiFi.isConnected())
        {
            this->connected = true;
            SERIALZ.print("WiFi IP address: ");
            SERIALZ.println(WiFi.localIP());

            if (MDNS.begin("cam"))
            {
                MDNS.addService("http", "tcp", 80);
            }

            enableOTA();
            return true;
        }
        return false;
    }

    void enableOTA()
    {

        // if (!connected) {
        //     SERIALZ.println("Not connected to wifi.. ");
        //     return;
        // }

        // Port defaults to 3232
        // ArduinoOTA.setPort(3232);

        // Hostname defaults to esp3232-[MAC]
        ArduinoOTA.setHostname("cam");

        // No authentication by default
        // ArduinoOTA.setPassword("admin");

        // Password can be set with it's md5 value as well
        // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
        // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

        ArduinoOTA.onStart([]()
                           {
            String type;
            if (ArduinoOTA.getCommand() == U_FLASH)
                type = "sketch";
            else // U_SPIFFS
                type = "filesystem";

            // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
            SERIALZ.println("Start updating " + type); })
            .onEnd([]()
                   { SERIALZ.println("\nEnd"); })
            .onProgress([](unsigned int progress, unsigned int total)
                        { SERIALZ.printf("Progress: %u%%\r", (progress / (total / 100))); })
            .onError([](ota_error_t error)
                     {
            SERIALZ.printf("Error[%u]: ", error);
            if (error == OTA_AUTH_ERROR) SERIALZ.println("Auth Failed");
            else if (error == OTA_BEGIN_ERROR) SERIALZ.println("Begin Failed");
            else if (error == OTA_CONNECT_ERROR) SERIALZ.println("Connect Failed");
            else if (error == OTA_RECEIVE_ERROR) SERIALZ.println("Receive Failed");
            else if (error == OTA_END_ERROR) SERIALZ.println("End Failed"); });

        ArduinoOTA.begin();
        SERIALZ.println("OTA Started");
    }

    void handleOTA()
    {
        ArduinoOTA.handle();
    }
};

#endif

// #ifndef WIFIZ_H
// #define WIFIZ_H

// #include <WiFi.h>
// #include <ESPmDNS.h>
// #include <ArduinoOTA.h>

// #ifndef WIFI_SSID
//     #define WIFI_SSID "(SSID not defined)"
// #endif
// #ifndef WIFI_PASS
//     #define WIFI_PASS "(PASS not defined)"
// #endif

// class Wifiz {
//     static IPAddress ip;
// public:
//     static bool connected;

//     static void connect() {
//         SERIALZ.println("WiFi begin");
//         WiFi.begin(WIFI_SSID, WIFI_PASS);

//         for (int i = 0; i < 10; i++) {
//             if (WiFi.status() == WL_CONNECTED) {
//                 break;
//             }
//             SERIALZ.print(i);
//             delay(500);
//         }
//         if (WiFi.status() != WL_CONNECTED) {
//             SERIALZ.println("Still not connected after 5 seconds!");
//             return;
//         }
//         connected = true;

//         SERIALZ.print("WiFi IP address: ");
//         SERIALZ.println(WiFi.localIP());
//         ip = WiFi.localIP();

//         // Set up mDNS responder:
//         // - first argument is the domain name, in this example
//         //   the fully-qualified domain name is "esp32.local"
//         // - second argument is the IP address to advertise
//         //   we send our IP address on the WiFi network
//         if (!MDNS.begin("EspCam")) {
//             SERIALZ.println("Error setting up MDNS responder!");
//             return;
//         }
//         SERIALZ.println("mDNS responder started");

//         // Add service to MDNS-SD add after webserver started?
//         MDNS.addService("http", "tcp", 80);
//     }

//     static void enableOTA() {

//         if (!connected) {
//             SERIALZ.println("Not connected to wifi.. ");
//             return;
//         }

//         // Port defaults to 3232
//         // ArduinoOTA.setPort(3232);

//         // Hostname defaults to esp3232-[MAC]
//         // ArduinoOTA.setHostname("myesp32");

//         // No authentication by default
//         // ArduinoOTA.setPassword("admin");

//         // Password can be set with it's md5 value as well
//         // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
//         // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

//         ArduinoOTA.onStart([]() {
//             String type;
//             if (ArduinoOTA.getCommand() == U_FLASH)
//                 type = "sketch";
//             else // U_SPIFFS
//                 type = "filesystem";

//             // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
//             SERIALZ.println("Start updating " + type);
//         })
//         .onEnd([]() {
//             SERIALZ.println("\nEnd");
//         })
//         .onProgress([](unsigned int progress, unsigned int total) {
//             SERIALZ.printf("Progress: %u%%\r", (progress / (total / 100)));
//         })
//         .onError([](ota_error_t error) {
//             SERIALZ.printf("Error[%u]: ", error);
//             if (error == OTA_AUTH_ERROR) SERIALZ.println("Auth Failed");
//             else if (error == OTA_BEGIN_ERROR) SERIALZ.println("Begin Failed");
//             else if (error == OTA_CONNECT_ERROR) SERIALZ.println("Connect Failed");
//             else if (error == OTA_RECEIVE_ERROR) SERIALZ.println("Receive Failed");
//             else if (error == OTA_END_ERROR) SERIALZ.println("End Failed");
//         });

//         ArduinoOTA.begin();
//         SERIALZ.println("OTA Started");
//     }

//     static void handleOTA() {
//         ArduinoOTA.handle();
//     }

// };

// bool Wifiz::connected = false;
// IPAddress Wifiz::ip;

// #endif
