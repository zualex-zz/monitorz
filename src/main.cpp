#include <Arduino.h>

#define DEBUG false // set to true for debug output
#define SERIALZ \
    if (DEBUG)  \
    Serial

// #include <driver/i2s.h>
// #include "i2sz.h"
// #include "painless-meshz.h"
// #include "esp-nowz.h"
// #include "encoderz.h"
// #include "oledz.h"
// #include "menuz.h"
#include "pirz.h"
#include "wifiz.h"
#include "telegramz.h"
#include "shellyz.h"
#include "camz.h"
#include "utilz.h"

boolean ledBuiltinBlinkOnPir = false;
boolean photoOnPir = false;

// PainlessMeshz* painlessMeshz;
// Encoderz* encoderz;
// Oledz oledz;
// Menuz* menuz;
Wifiz *wifiz;
Camz *camz;
Telegramz *telegramz;
Shellyz *shellyz;

/*int32_t functionGenEnabled = 0; // FIXME: type was bool but menu supports only int
int32_t frequency = 100;
int32_t amplitude = 100;
*/
// int32_t pirGenEnabled = true;
/*
std::vector< MenuItem > menuItems = {
  { "Microphone", {
    { "Auto scale", {}, &scaleAuto, [](){scaleAuto = true;}, [](){scaleAuto = false;} },
    { "Scale", {}, &scale, [](){scale++;}, [](){scale--;} }
  }},
 // {"Web radio", NULL, {"Station"}},
 // {"Bluetooth", toggleBluetooth},
  { "Function generator", {
    {"On/off", {}, &functionGenEnabled, [](){functionGenEnabled = true;}, [](){functionGenEnabled = false;} },
    {"Frequency", {}, &frequency, [](){frequency++;}, [](){frequency--;} },
    {"Amplitude", {}, &amplitude, [](){amplitude++;}, [](){amplitude--;} }
  }},
  { "Pir", {
    {"On/off", {}, &pirGenEnabled, [](){pirGenEnabled = true;}, [](){pirGenEnabled = false;} }
  }}
};
*/

/*volatile void rotationHandlerz(boolean up) {
  if (up) {
    SERIALZ.println("Up!");
    menuz->next();
  } else {
    SERIALZ.println("Down!");
    menuz->prev();
  }
}

volatile void pressedHandlerz() {
  SERIALZ.println("Pressed!");
  menuz->select();
}*/
/*
void toggleLed() {
  int led = LED_BUILTIN;
  if (contains(EspNowz::currentNode->capabilities, Base)) {
    led = TTGO_T_Energy_LED_BUILTIN;
  }
  if (ledBuiltInOn) {
    digitalWrite(led, HIGH);
  } else {
    digitalWrite(led, LOW);
  }
  ledBuiltInOn = !ledBuiltInOn;
}*/

/*void onMessage(EspMessage* message) {
  SERIALZ.print("Message recieved ");SERIALZ.println(message->btnPresed);
}

// Called when ESP-Now receives.
void onDataRecv(const uint8_t *mac, const uint8_t *incomingRaw, int samples) {
  // if (EspNowz::currentNode->type == AudioOut) {
  if (contains(EspNowz::currentNode->capabilities, AudioOut)) {
    SERIALZ.print("Playing ");SERIALZ.println(samples);
    // Convert it from 8 bit signed to 16 bit unsigned with an 0x80 delta which is what the DAC requires.
    int8_t *incoming8 = (int8_t *)incomingRaw;
    uint16_t incoming16[ESP_NOW_MAX_DATA_LEN] = {0};
    for (int i=0; i<samples; i++) {
        int32_t value = incoming8[i];
        value += 0x80; // DAC wants unsigned values with a bias, not signed!
        incoming16[i] = value << 8;
    }

    // Forward it to the DAC.
    size_t bytesWritten=0;
    esp_err_t result = i2s_write(I2S_NUM_0, incoming16, samples * 2, &bytesWritten, 500);

    if (result != ESP_OK) {
      SERIALZ.print("Error i2s_write ");SERIALZ.println(result, HEX);
    }

    oledz.drawBands<uint16_t>(incoming16, ESP_NOW_MAX_DATA_LEN, menuz->getCurrentItem()->label, menuz->getCurrentItem()->value, &scale);
  }
}*/

httpd_handle_t stream_httpd = NULL;
httpd_handle_t camera_httpd = NULL;

void startHttpServer()
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;

    httpd_uri_t index_uri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = Camz::stream_handler,
        .user_ctx = NULL};

    httpd_uri_t capture_uri = {
        .uri = "/capture",
        .method = HTTP_GET,
        .handler = Camz::capture_handler,
        .user_ctx = NULL};

    httpd_uri_t send_uri = {
        .uri = "/send",
        .method = HTTP_GET,
        .handler = [](httpd_req *r)
        {
            camera_fb_t *fb = camz->takePhoto();
            telegramz->sendPhoto(fb);
            camz->reuseBuffer(fb);
            return ESP_OK;
        },
        .user_ctx = NULL};

    SERIALZ.printf("Starting stream server on port: '%d'\n", config.server_port);
    if (httpd_start(&stream_httpd, &config) == ESP_OK)
    {
        httpd_register_uri_handler(stream_httpd, &index_uri);
    }

    config.server_port += 1;
    config.ctrl_port += 1;
    SERIALZ.printf("Starting snapshot server on port: '%d'\n", config.server_port);
    if (httpd_start(&camera_httpd, &config) == ESP_OK)
    {
        httpd_register_uri_handler(camera_httpd, &capture_uri);
        httpd_register_uri_handler(camera_httpd, &send_uri);
    }
}

void setup()
{

    SERIALZ.begin(115200);
    setupLedBuiltin();

    /*pinMode(BOOT_BTN, INPUT);
    attachInterrupt(digitalPinToInterrupt(BOOT_BTN), [](){
      cli();
      SERIALZ.println("Boot btn!");
      toggleLed();

      EspMessage espMessage;
      espMessage.btnPresed = true;
      EspNowz::send(*EspNowz::broadcast, espMessage);
      // oledz.show(0, "Pressed!");
      sei();
    }, RISING);*/

    // menuz = new Menuz(menuItems);

    // painlessMeshz->init(receivedCallback);
    /*EspNowz::init(onMessage);

    if (EspNowz::currentNode != NULL) {
      //if (EspNowz::currentNode->type == AudioIn) {
      if (contains(EspNowz::currentNode->capabilities, AudioIn)) {
        SERIALZ.println("AudioIn type");
        I2sz::startMic();

        Encoderz::begin(rotationHandlerz, pressedHandlerz);
        // oledz = new Oledz(); // 0x3D
        oledz.begin();
      // } else if (EspNowz::currentNode->type == AudioOut) {
      } else if (contains(EspNowz::currentNode->capabilities, AudioOut)) {
        SERIALZ.println("AudioOut type");
        I2sz::startDacExternal();
        oledz.begin();
        scale = 127; // FIXME remove
        esp_now_register_recv_cb(onDataRecv);
      }

      if (contains(EspNowz::currentNode->capabilities, Pir)) {*/
    // oledz.begin();
    wifiz = new Wifiz(WIFI_SSID, WIFI_PASS);
    wifiz->begin();
    // Wifiz::connect();
    // Wifiz::enableOTA();

    camz = new Camz([]()
                    {
                        // telegramz->send("Xe movimento!");
                        // camz->cameraImageSettings(FRAME_SIZE_PHOTO);
                        camera_fb_t *fb = camz->takePhoto();
                        telegramz->sendPhoto(fb);
                        camz->reuseBuffer(fb);
                        // camz->cameraImageSettings(FRAME_SIZE_MOTION);
                    });

    /*auto takeAndsendPhoto = [](Telegramz* t) {
      camera_fb_t* fb = camz->takePhoto();
      t->sendPhoto(fb);
      camz->reuseBuffer(fb);
    };*/

    telegramz = new Telegramz();
    shellyz = new Shellyz();
    telegramz->addAction("/takePicture", [](Telegramz *t)
                         {
        camz->setFramesizePhoto();
        camera_fb_t* fb = camz->takePhoto();
        t->sendPhoto(fb);
        camz->reuseBuffer(fb);
        camz->setFramesizeMotion(); });
    // telegramz->addAction("/picture", [](Telegramz* t) {
    //   photoOnPir = !photoOnPir;
    //   t->send(photoOnPir ? "on" : "off");
    // });
    // telegramz->addAction("/led", [](Telegramz* t) {
    //   ledBuiltinBlinkOnPir = !ledBuiltinBlinkOnPir;
    //   t->send(ledBuiltinBlinkOnPir ? "on" : "off");
    // });
    // telegramz->addAction("/flash", [](Telegramz* t) {
    //   t->send(camz->toggleFlash() ? "on" : "off");
    // });
    telegramz->addAction("/arm", [](Telegramz *t)
                         { camz->armed = true; t->send("Armed!"); });
    telegramz->addAction("/disarm", [](Telegramz *t)
                         { camz->armed = false; t->send("Disarmed!"); });
    telegramz->addAction("/casa", [](Telegramz *t)
                         { t->send(shellyz->toggle(shellyz->relaies[0])); });
    telegramz->addAction("/giardin", [](Telegramz *t)
                         { t->send(shellyz->toggle(shellyz->relaies[1])); });
    telegramz->addAction("/pranzo", [](Telegramz *t)
                         { t->send(shellyz->toggle(shellyz->relaies[2])); });
    telegramz->addAction("/cucina", [](Telegramz *t)
                         { t->send(shellyz->toggle(shellyz->relaies[3])); });
    telegramz->addAction("/soggiorno", [](Telegramz *t)
                         { t->send(shellyz->toggle(shellyz->relaies[4])); });
    telegramz->addAction("/corridoio", [](Telegramz *t)
                         { t->send(shellyz->toggle(shellyz->relaies[5])); });
    telegramz->addAction("/cancel", [](Telegramz *t)
                         { t->send(shellyz->toggle(shellyz->relaies[6])); });

    // for (const RelayItem &relay : shellyz->relaies) {
    //   telegramz->addAction(relay.label, [](Telegramz* t) {
    //   t->send(shellyz->toggle(relay));
    // });
    // }

    Pirz::setup([]()
                {
        // oledz.show("motion");
        if (ledBuiltinBlinkOnPir) {
          ledBuiltinOn();
        }
        SERIALZ.print("motion.. ");
        if (!photoOnPir) {
          telegramz->send("motionz");
        } else {
          camera_fb_t* fb = camz->takePhoto();
          telegramz->sendPhoto(fb);
          camz->reuseBuffer(fb);
        } }, []()
                {
        // oledz.show("motion end");
        if (ledBuiltinBlinkOnPir) {
          ledBuiltinOff();
        }
        SERIALZ.println("end!");
        telegramz->send("motionz end"); });
    /*}
  } else {
    SERIALZ.print("Unknow mac ardess! ");SERIALZ.println(WiFi.macAddress());
  }*/
    // camz->toggleFlash();
}

// int32_t maxAmplitude = 5000; //FIXME
void loop()
{

    /*if (contains(EspNowz::currentNode->capabilities, AudioIn)) {

      if (!functionGenEnabled) {

        size_t bytesRead = 0;
        uint8_t buffer32[ESP_NOW_MAX_DATA_LEN * 4] = {0};
        i2s_read(I2S_NUM_1, &buffer32, sizeof(buffer32), &bytesRead, 1000);
        int samplesRead = bytesRead / 4;

          // Convert to 16-bit signed.
          // It's actually 24-bit, but the lowest byte is just noise, even in a quiet room.
          // If we go to 16 bit we don't have to worry about extending a sign byte.
          // Quiet room seems to be values maxing around 7.
          // Max seems around 300 with me at 0.5m distance talking at normal loudness.
        int16_t buffer16[ESP_NOW_MAX_DATA_LEN] = {0};
        for (int i=0; i<samplesRead; i++) {
              // Offset + 0 is always E0 or 00, regardless of the sign of the other bytes,
              // because our mic is only 24-bits, so discard it.
              // Offset + 1 is the LSB of the sample, but is just fuzz, discard it.
            uint8_t mid = buffer32[i * 4 + 2];
            uint8_t msb = buffer32[i * 4 + 3];
            uint16_t raw = (((uint32_t)msb) << 8) + ((uint32_t)mid);
            memcpy(&buffer16[i], &raw, sizeof(raw)); // Copy so sign bits aren't interfered.
        }

        if (scaleAuto) {

              // Find the maximum scale.
            int16_t max = 0;
            for (int i=0; i<samplesRead; i++) {
                int16_t val = buffer16[i];
                if (val < 0) { val = -val; }
                if (val > max) { max = val; }
            }

            // Push up the scale if volume went up.
            if (max > scale) { scale = max; }
            // Gradually drop the scale when things are quiet.
            if (max < scale && scale > RESTING_SCALE) { scale -= 300; }
            if (scale < RESTING_SCALE) { scale = RESTING_SCALE; } // Dropped too far.
        }

        // Scale it to int8s so we aren't transmitting too much data.
        int8_t buffer8[ESP_NOW_MAX_DATA_LEN] = {0};
        for (int i=0; i<samplesRead; i++) {
            int32_t scaled = ((int32_t)buffer16[i]) * 127 / scale;
            if (scaled <= -127) {
                buffer8[i] = -127;
            } else if (scaled >= 127) {
                buffer8[i] = 127;
            } else {
                buffer8[i] = scaled;
            }
        }
        //SERIALZ.print("Sending ");SERIALZ.println(samplesRead);
        // Send to the other ESP32.
        if (ESP_OK != esp_now_send(EspNowz::broadcast->address, (uint8_t *)buffer8, samplesRead)) {
            SERIALZ.println("Error: esp_now_send");
            delay(500);
        }

        MenuItem* menuItem = menuz->getCurrentItem();
        oledz.drawBands<int8_t>(buffer8, ESP_NOW_MAX_DATA_LEN, menuItem->label, menuItem->value, &scale);

      } else {

      }

    }*/

    // Pirz::loop();

    // SERIALZ.print(buffer16[0]);SERIALZ.print(" ");SERIALZ.print(buffer16[1]);SERIALZ.print(" ");SERIALZ.print(buffer16[2]);SERIALZ.print(" ");SERIALZ.print(buffer16[3]);

    if (wifiz->justConnected())
    {
        // webServerz.begin();
        SERIALZ.println("wifi!");
        startHttpServer();
        telegramz->begin();
    }

    if (wifiz->connected)
    {
        camz->loop();
        telegramz->loop();
        wifiz->handleOTA();
    }
}