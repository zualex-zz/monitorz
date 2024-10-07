#ifndef CAMZ_H
#define CAMZ_H

#include <Arduino.h>
#include <esp_camera.h>
// #include "soc/soc.h"
// #include "soc/rtc_cntl_reg.h"
// #include "driver/rtc_io.h"

//Pin definition for CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM  32
#define RESET_GPIO_NUM  -1
#define XCLK_GPIO_NUM  0
#define SIOD_GPIO_NUM  26
#define SIOC_GPIO_NUM  27
#define Y9_GPIO_NUM  35
#define Y8_GPIO_NUM  34
#define Y7_GPIO_NUM  39
#define Y6_GPIO_NUM  36
#define Y5_GPIO_NUM  21
#define Y4_GPIO_NUM  19
#define Y3_GPIO_NUM  18
#define Y2_GPIO_NUM  5
#define VSYNC_GPIO_NUM  25
#define HREF_GPIO_NUM  23
#define PCLK_GPIO_NUM  22

#define FLASH_PIN 4

class Camz {

    boolean flashEnabled = false;
    camera_fb_t* fb;
    
    public:

    Camz() {

        pinMode (FLASH_PIN, OUTPUT);

        // WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG,0);//disable brownout detector

        camera_config_t configg;
        configg.ledc_channel = LEDC_CHANNEL_0;
        configg.ledc_timer = LEDC_TIMER_0;
        configg.pin_d0 = Y2_GPIO_NUM;
        configg.pin_d1 = Y3_GPIO_NUM;
        configg.pin_d2 = Y4_GPIO_NUM;
        configg.pin_d3 = Y5_GPIO_NUM;
        configg.pin_d4 = Y6_GPIO_NUM;
        configg.pin_d5 = Y7_GPIO_NUM;
        configg.pin_d6 = Y8_GPIO_NUM;
        configg.pin_d7 = Y9_GPIO_NUM;
        configg.pin_xclk = XCLK_GPIO_NUM;
        configg.pin_pclk = PCLK_GPIO_NUM;
        configg.pin_vsync = VSYNC_GPIO_NUM;
        configg.pin_href = HREF_GPIO_NUM;
        configg.pin_sscb_sda = SIOD_GPIO_NUM;
        configg.pin_sscb_scl = SIOC_GPIO_NUM;
        configg.pin_pwdn = PWDN_GPIO_NUM;
        configg.pin_reset = RESET_GPIO_NUM;
        configg.xclk_freq_hz = 20000000;
        configg.pixel_format = PIXFORMAT_JPEG;

        // rtc_gpio_hold_dis(GPIO_NUM_4);

        if (psramFound()) {
            Serial.printf("Ps ram found!");  
            // configg.frame_size = FRAMESIZE_QXGA;
            configg.frame_size = FRAMESIZE_UXGA;
            configg.jpeg_quality = 10;
            configg.fb_count = 2;
            configg.grab_mode = CAMERA_GRAB_LATEST;
        } else {
            configg.frame_size = FRAMESIZE_SVGA;
            configg.jpeg_quality = 12;
            configg.fb_count = 1;
        }

        //Init Camera
        esp_err_t err = esp_camera_init(&configg);
        if(err != ESP_OK) {
            Serial.printf("Camera init failed with error");  
        }
    }

    camera_fb_t* takePhoto() {
        return esp_camera_fb_get();
    }

    void reuseBuffer(camera_fb_t* fb) {
        esp_camera_fb_return(fb);
    }

    bool toggleFlash() {
        flashEnabled = !flashEnabled;
        digitalWrite(FLASH_PIN, flashEnabled ? HIGH : LOW);
        return flashEnabled;
    }

};

// bool Wifiz::connected = false;

#endif