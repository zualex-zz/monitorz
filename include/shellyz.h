#ifndef SHELLYZ_H
#define SHELLYZ_H

#include <Arduino.h>
#include <HTTPClient.h>

typedef struct ShellyItem
{
  const char *label;
  const char *user;
  const char *host;
  // std::vector<RelayItem> relays;
} ShellyItem;

typedef struct RelayItem
{
  const char *label;
  const int number;
  const ShellyItem *shelly;
} RelayItem;

class Shellyz
{
  const char *urlTemplate = "http://%s%s/relay/%d?turn=toggle";
  const char *coIoTurlTemplate = "http://%s/cit/d";
  
  const ShellyItem gardenShelly = {"Giardin", "admin:d8xHUxwpkipWmz7", "192.168.1.150"};
  const ShellyItem kitchenShelly = {"Cusina", "", "192.168.1.151"};
  const ShellyItem tvShelly = {"Soggiorno", "", "192.168.1.152"};
  const ShellyItem gateShelly = {"Cancel", "", "192.168.1.153"};
  
  const std::vector<ShellyItem> shellies = {gardenShelly, kitchenShelly, tvShelly, gateShelly};
  public:
  const std::vector<RelayItem> relaies = {
      {"Casa", 0, &gardenShelly},
      {"Giardin", 1, &gardenShelly},
      {"Sala pranzo", 0, &kitchenShelly},
      {"Cucina", 1, &kitchenShelly},
      {"Soggiorno", 0, &tvShelly},
      {"Corridoio", 1, &tvShelly},
      {"Cancel", 0, &gateShelly}};

    HTTPClient http;

public:
  Shellyz()
  {
    // HTTPClient http;

    // for (const ShellyItem &shelly : shellies)
    // {
    //   char url[30];
    //   sprintf(url, coIoTurlTemplate, shelly.host);
    //   // Start the request
    //   http.begin(url);
    //   // Use HTTP GET request
    //   http.GET();
    //   // Response from server
    //   String response = "";
    //   response = http.getString();

    //   Serial.print(shelly.label);
    //   Serial.print("Shellyz get status ");
    //   Serial.println(response);
    // }
  }

  String toggle(RelayItem relay) {
      char url[66];
      sprintf(url, urlTemplate, relay.shelly->user, relay.shelly->host, relay.number);
      // Start the request
      http.begin(url);
      // Use HTTP GET request
      http.GET();
      // Response from server
      String response = "";
      response = http.getString();

      Serial.print(relay.label);
      Serial.print(" Shellyz toggle ");
      Serial.print(url);
      Serial.print(" response ");
      Serial.println(response);
      return response;
  }

  static void loop()
  {
  }
};
// unsigned long Shellyz::coIoT;
#endif