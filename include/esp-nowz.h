#ifndef ESPNOWZ_H
#define ESPNOWZ_H

#include <esp_now.h>
#include <WiFi.h>
#include <vector>

enum NodeCapability {AudioIn, AudioOut, Pir, Base};

typedef struct EspNode {
  const char* name;
  uint8_t address[6];
  std::vector< NodeCapability > capabilities;
} EspNode;

// Structure example to send data
// Must match the receiver structure
typedef struct EspMessage {
  bool btnPresed = false;
  // char a[32];
  // int b;
  // float c;
  // String d;
  // bool e;
} EspMessage;

// esp_now_peer_info_t peerInfo;


class EspNowz {
  static std::vector< EspNode > espNodes;

  static EspMessage espMessage;
  static void (*onMessage)(EspMessage*);
  static esp_now_peer_info_t peerInfo; // works only if it is a global variable: see https://rntlab.com/question/espnow-peer-interface-is-invalid/

public:
  static EspNode* currentNode;
  static EspNode* broadcast;

private:
  // callback when data is sent
  static void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    if (status != ESP_NOW_SEND_SUCCESS) {
      Serial.print("Delivery Fail ");Serial.print(status);
    }
    
  }

  // callback function that will be executed when data is received
/*  static void onDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
    memcpy(&espMessage, incomingData, sizeof(espMessage));

    onMessage(&espMessage);

    // Serial.print("Bytes received: ");
    // Serial.println(len);
    // Serial.print("Char: ");
    // Serial.println(espMessage.a);
    // Serial.print("Int: ");
    // Serial.println(espMessage.b);
    // Serial.print("Float: ");
    // Serial.println(espMessage.c);
    // Serial.print("String: ");
    // Serial.println(espMessage.d);
    // Serial.print("Bool: ");
    // Serial.println(espMessage.e);
    // Serial.println();
  }*/

  static void findCurrentNodeAndRegisterOthers() {
    uint8_t myMac[6];
    WiFi.macAddress(myMac);

    for (int i = 0; i < espNodes.size(); i++) {
      EspNode espNode = espNodes.at(i);
      if (memcmp(espNode.address, &myMac, sizeof(espNode.address)) == 0) { /* returns zero for a match */
        Serial.print("I'm ");Serial.println(espNode.name);
        currentNode = &espNodes.at(i);
      } else {
        Serial.print("Registering ");Serial.println(espNode.name);

        memcpy(peerInfo.peer_addr, espNode.address, sizeof(espNode.address));
        peerInfo.channel = 0;  
        peerInfo.encrypt = false;
        
        // Add peer        
        esp_err_t result = esp_now_add_peer(&peerInfo);
        if (result != ESP_OK) {
          Serial.print("Failed to add peer ");Serial.println(espNode.name);
          Serial.println(result, HEX); // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/error-codes.html
        }
      }
    }
  }

public:
  
  static void init(void (*onMessage)(EspMessage*)) {
    Serial.println("Initializing ESP-NOW");
    EspNowz::onMessage = onMessage;
    // Set device as a Wi-Fi Station
    WiFi.mode(WIFI_STA);
    
    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
      Serial.println("Error initializing ESP-NOW");
      return;
    }
    
    findCurrentNodeAndRegisterOthers();

    esp_now_register_send_cb(onDataSent);
    // esp_now_register_recv_cb(onDataRecv);
  }
  
  static void send(EspNode espNode, EspMessage espMessage) {
    // Set values to send
    // strcpy(espMessage.a, "THIS IS A CHAR");
    // espMessage.b = random(1,20);
    // espMessage.c = 1.2;
    // espMessage.d = "Hello";
    // espMessage.e = false;

    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(espNode.address, (uint8_t *) &espMessage, sizeof(espMessage));
    
    if (result == ESP_OK) {
      Serial.println("Sent with success");
    }
    else {
      Serial.print("Error sending the data ");
      Serial.println(result, HEX);
    }
    // delay(2000);
  }

};

std::vector< EspNode > EspNowz::espNodes = { 
  {"broadcast",   {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}},
  {"microphone",  {0x24, 0x0A, 0xC4, 0x5E, 0xE1, 0xE4}, {AudioIn, Pir}},
  {"testPir",       {0x24, 0x62, 0xAB, 0xF2, 0x8C, 0x08}, {Pir}},
  {"battery",     {0x98, 0xF4, 0xAB, 0x23, 0x4A, 0x20}, {Base}}
};
EspNode* EspNowz::currentNode = NULL;
EspNode* EspNowz::broadcast = &espNodes.at(0);
EspMessage EspNowz::espMessage;
void (*EspNowz::onMessage)(EspMessage*);
esp_now_peer_info_t EspNowz::peerInfo;
#endif