#ifndef OLEDZ_H
#define OLEDZ_H

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "utilz.h"

class Oledz {
    uint8_t addr;
    uint8_t width;
    uint8_t height;
    Adafruit_SSD1306* display;
    static const int16_t oledBandsLen = 128; 
    double oledBands[oledBandsLen];
    char ylim = 32; // 60;

public:

    Oledz(uint8_t addrz = 0x3C, uint8_t widthz = 128, uint8_t heightz = 64) {
        addr = addrz;
        width = widthz;
        height = heightz;
    }

    void begin() {
        Wire.begin(21, 22);
        display = new Adafruit_SSD1306(width, height, &Wire, -1);

        if(!display->begin(SSD1306_SWITCHCAPVCC, addr)) { // 0x3C or 0x3D
            SERIALZ.println("Display allocation failed");
        }
        display->clearDisplay();
        showSplashScreen();
    }

    void showSplashScreen() {
        display->clearDisplay();
        display->setTextColor(SSD1306_WHITE);
        display->setCursor(0,0);
        display->print("z-home!");
        display->display();
    }

    void show(const char* text) {
        display->clearDisplay();
        display->setTextColor(SSD1306_WHITE);
        
        display->drawLine(0, 32, 127, 32, WHITE);
        // display->setCursor(0,0);
        // display->print(from);
        
        display->setCursor(0,8);
        display->print(text);

        // display->setCursor(0,16);
        // display->print("ciao");

        display->display();
        SERIALZ.println("oled displayed");
    }

    template <typename T>
    void drawBands(T* buffer, uint16_t bufferLen, /*char* top, char* bottom,*/ const char* menu, int32_t* menuValue, const int32_t* scale) {

        sampling<T>(buffer, bufferLen, oledBands, oledBandsLen);

        display->clearDisplay();

        if (scale != 0) {
            for (uint8_t i = 0; i < oledBandsLen; i++) {
                display->drawLine(i, ylim, i, ylim - (oledBands[i] / *scale * ylim), WHITE);
            };
        }

        // display.drawLine(0, 10, bands[0], 10, WHITE);
        // display.drawLine(0, 11, bands[0], 11, WHITE);

        display->setTextColor(SSD1306_WHITE);
        
        display->setCursor(0,0);
        display->print(menu);
        
        if (menuValue != NULL) {
            display->setCursor(0,8);
            display->print(*menuValue);
        }

        // display->setCursor(0,48);
        // display->print(top);

        // display->setCursor(0,56);
        // display->print(bottom);

        display->setCursor(0,56);
        display->print(*scale);

        display->display();
    }

};

#endif