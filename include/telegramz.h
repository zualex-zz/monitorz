#ifndef TELEGRAMZ_H
#define TELEGRAMZ_H

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <vector>
#include <esp_camera.h>


class Telegramz {

    const char* botName = TELEGRAM_BOTNAME;
    const char* botUsername = TELEGRAM_BOTUSERNAME;
    const char* botToken = TELEGRAM_BOTTOKEN;
    const char* chatId = TELEGRAM_CHATID;
    const unsigned long BOT_MTBS = 1000; // mean time between scan messages

    unsigned long bot_lasttime;          // last time messages' scan has been done
    bool Start = false;

    WiFiClientSecure* secured_client;
    
    UniversalTelegramBot* bot;

    bool armed = false;

    typedef void (*TelegramCallBack)(Telegramz*);

    typedef struct Action {
        String text;
        TelegramCallBack execute;
    } Action;

    std::vector< Action > actions = {
        { "/arm", [](Telegramz* t) {
            t->armed = true;
            t->send("Armed!");
        }},
        { "/disarm", [](Telegramz* t) {
            t->send("Disarmed!");
            t->armed = false;
        }}
    };

    static uint8_t* fb_buffer;
    static size_t fb_length;
    static int currentByte;

    void handleNewMessages(int numNewMessages) {
        //Serial.println("handleNewMessages");
        //Serial.println(String(numNewMessages));

        for (int i = 0; i < numNewMessages; i++) {
            String chat_id = bot->messages[i].chat_id;
            String text = bot->messages[i].text;
            Serial.println(text);
            String from_name = bot->messages[i].from_name;
            if (from_name == "") {
                from_name = "Guest";
            }

            if (text == "/start") {
                String welcome = "Welcome to zHome, " + from_name + ".\n";
                for (int i = 0; i < actions.size(); i++) {
                    welcome += actions.at(i).text + " \n";
                }
                bot->sendChatAction(chat_id, "typing");
                delay(1000);
                bot->sendMessage(chat_id, welcome);
            }

            for (int i = 0; i < actions.size(); i++) {
                Action action = actions.at(i);
                if (text == action.text) {
                    Serial.print("executing "); Serial.println(text);
                    action.execute(this);
                }
            }
        }
    }

    static bool isMoreDataAvailable() {
        return (fb_length - currentByte);
    }

    static uint8_t photoNextByte() {
        currentByte++;
        return (fb_buffer[currentByte - 1]);
    }

    public:

    Telegramz() {
        secured_client = new WiFiClientSecure();
        secured_client->setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
        // secured_client->setInsecure(); // TEST az

        bot = new UniversalTelegramBot(botToken, *secured_client);

        configTime(0, 0, "pool.ntp.org"); // get UTC time via NTP
        time_t now = time(nullptr);
        while (now < 24 * 3600) {
            Serial.print(".");
            delay(100);
            now = time(nullptr);
        }

        Serial.print("Telegramz init ");Serial.println(now);

        // send("started!");
    }

    void addAction(String text, TelegramCallBack toExecute) {
        Serial.print("adding  "); Serial.println(text);
        actions.push_back({text, toExecute});
    }
    
    void send(const String& message) {
        if (armed) {
            Serial.print("sending ");Serial.println(message);
            bot->sendMessage(chatId, message);
            Serial.print("sended ");Serial.println(message);
        }
    }

    void sendPhoto(camera_fb_t* fb) {
        if (armed) {
            Serial.print("sending photo");
            bot->sendChatAction(chatId, "upload_photo");
            currentByte = 0;
            fb_length = fb->len;
            fb_buffer = fb->buf;

            bot->sendPhotoByBinary(chatId, "image/jpeg", fb->len, isMoreDataAvailable, photoNextByte, nullptr, nullptr);

            fb_length = NULL;
            fb_buffer = NULL;
            Serial.print("sended ");
        }
    }

    void loop() {
        if (millis() - bot_lasttime > BOT_MTBS) {
            int numNewMessages = bot->getUpdates(bot->last_message_received + 1);

            while (numNewMessages) {
                handleNewMessages(numNewMessages);
                numNewMessages = bot->getUpdates(bot->last_message_received + 1);
            }

            bot_lasttime = millis();
        }
    }

};
uint8_t* Telegramz::fb_buffer;
size_t Telegramz::fb_length;
int Telegramz::currentByte;
#endif