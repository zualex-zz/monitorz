#include <Arduino.h>
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
Wifiz* wifiz;
Camz* camz;
Telegramz* telegramz;
Shellyz* shellyz;

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
    Serial.println("Up!");
    menuz->next();
  } else {
    Serial.println("Down!");
    menuz->prev();
  }
}

volatile void pressedHandlerz() {
  Serial.println("Pressed!");
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
  Serial.print("Message recieved ");Serial.println(message->btnPresed);
}

// Called when ESP-Now receives.
void onDataRecv(const uint8_t *mac, const uint8_t *incomingRaw, int samples) {
  // if (EspNowz::currentNode->type == AudioOut) {
  if (contains(EspNowz::currentNode->capabilities, AudioOut)) {
    Serial.print("Playing ");Serial.println(samples);
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
      Serial.print("Error i2s_write ");Serial.println(result, HEX);
    }

    oledz.drawBands<uint16_t>(incoming16, ESP_NOW_MAX_DATA_LEN, menuz->getCurrentItem()->label, menuz->getCurrentItem()->value, &scale);
  }
}*/

void setup() {

  Serial.begin(115200);
  setupLedBuiltin();

  /*pinMode(BOOT_BTN, INPUT);
  attachInterrupt(digitalPinToInterrupt(BOOT_BTN), [](){
    cli();
    Serial.println("Boot btn!");
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
      Serial.println("AudioIn type");
      I2sz::startMic();

      Encoderz::begin(rotationHandlerz, pressedHandlerz);
      // oledz = new Oledz(); // 0x3D
      oledz.begin(); 
    // } else if (EspNowz::currentNode->type == AudioOut) {
    } else if (contains(EspNowz::currentNode->capabilities, AudioOut)) {
      Serial.println("AudioOut type");
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

      camz = new Camz();

      /*auto takeAndsendPhoto = [](Telegramz* t) {
        camera_fb_t* fb = camz->takePhoto();
        t->sendPhoto(fb);
        camz->reuseBuffer(fb);
      };*/

      telegramz = new Telegramz();
      shellyz = new Shellyz();
      telegramz->addAction("/takePicture", [](Telegramz* t) {
        camera_fb_t* fb = camz->takePhoto();
        t->sendPhoto(fb);
        camz->reuseBuffer(fb);
      });
      telegramz->addAction("/picture", [](Telegramz* t) {
        photoOnPir = !photoOnPir;
        t->send(photoOnPir ? "on" : "off");
      });
      telegramz->addAction("/led", [](Telegramz* t) {
        ledBuiltinBlinkOnPir = !ledBuiltinBlinkOnPir;
        t->send(ledBuiltinBlinkOnPir ? "on" : "off");
      });
      telegramz->addAction("/flash", [](Telegramz* t) {
        t->send(camz->toggleFlash() ? "on" : "off");
      });;
      telegramz->addAction("/soggiorno", [](Telegramz* t) {
        t->send(shellyz->toggle(shellyz->relaies[4]));
      });
      telegramz->addAction("/corridoio", [](Telegramz* t) {
        t->send(shellyz->toggle(shellyz->relaies[5]));
      });
      telegramz->addAction("/cancel", [](Telegramz* t) {
        t->send(shellyz->toggle(shellyz->relaies[6]));
      });

      // for (const RelayItem &relay : shellyz->relaies) {
      //   telegramz->addAction(relay.label, [](Telegramz* t) {
      //   t->send(shellyz->toggle(relay));
      // });
      // }
      
      Pirz::setup([]() {
        // oledz.show("motion");
        if (ledBuiltinBlinkOnPir) {
          ledBuiltinOn();
        }
        Serial.print("motion.. ");
        if (!photoOnPir) {
          telegramz->send("motionz");
        } else {
          camera_fb_t* fb = camz->takePhoto();
          telegramz->sendPhoto(fb);
          camz->reuseBuffer(fb);
        }
      },[]() {
        // oledz.show("motion end");
        if (ledBuiltinBlinkOnPir) {
          ledBuiltinOff();
        }
        Serial.println("end!");
        telegramz->send("motionz end");
      });
    /*}
  } else {
    Serial.print("Unknow mac ardess! ");Serial.println(WiFi.macAddress());
  }*/
  // camz->toggleFlash();
}


// int32_t maxAmplitude = 5000; //FIXME 
void loop() {

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
      //Serial.print("Sending ");Serial.println(samplesRead);
      // Send to the other ESP32.
      if (ESP_OK != esp_now_send(EspNowz::broadcast->address, (uint8_t *)buffer8, samplesRead)) {
          Serial.println("Error: esp_now_send");
          delay(500);
      }
      
      MenuItem* menuItem = menuz->getCurrentItem();
      oledz.drawBands<int8_t>(buffer8, ESP_NOW_MAX_DATA_LEN, menuItem->label, menuItem->value, &scale);

    } else {

    }

  }*/

if (wifiz->connected) {
  telegramz->loop();
}

  // Pirz::loop();

  // Serial.print(buffer16[0]);Serial.print(" ");Serial.print(buffer16[1]);Serial.print(" ");Serial.print(buffer16[2]);Serial.print(" ");Serial.print(buffer16[3]);

if (wifiz->justConnected()) {
    //webServerz.begin();
    Serial.println("wifi!");
    telegramz->begin();
    camz->startCameraServer();
  }

  // Wifiz::handleOTA();
  wifiz->handleOTA();
}