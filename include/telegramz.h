#ifndef TELEGRAMZ_H
#define TELEGRAMZ_H

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <AsyncTelegram2.h>
#include <vector>
#include <esp_camera.h>

class Telegramz
{

    const char *botName = TELEGRAM_BOTNAME;
    const char *botUsername = TELEGRAM_BOTUSERNAME;
    const char *botToken = TELEGRAM_BOTTOKEN;
    const int64_t chatId = TELEGRAM_CHATID;
    const unsigned long BOT_MTBS = 1000; // mean time between scan messages

    unsigned long bot_lasttime; // last time messages' scan has been done
    bool Start = false;

    WiFiClientSecure *secured_client;

    AsyncTelegram2 *bot;

    // bool armed = false;

    typedef void (*TelegramCallBack)(Telegramz *);

    typedef struct Action
    {
        String text;
        TelegramCallBack execute;
    } Action;

    std::vector<Action> actions = {};
    //     {"/arm", [](Telegramz *t)
    //      {
    //          t->armed = true;
    //          t->send("Armed!");
    //      }},
    //     {"/disarm", [](Telegramz *t)
    //      {
    //          t->send("Disarmed!");
    //          t->armed = false;
    //      }}};

    void handleNewMessages(TBMessage msg)
    {
        const int64_t chat_id = msg.chatId;
        String text = msg.text;
        SERIALZ.println(text);
        String from_name = msg.sender.firstName;
        if (from_name == "")
        {
            from_name = "Guest";
        }

        if (text == "/start")
        {
            String welcome = "Welcome to zHome, " + from_name + ".\n";
            for (int i = 0; i < actions.size(); i++)
            {
                welcome += actions.at(i).text + " \n";
            }
            // bot->sendChatAction(chat_id, "typing");
            // delay(1000);
            bot->sendTo(chat_id, welcome);
        }

        for (int i = 0; i < actions.size(); i++)
        {
            Action action = actions.at(i);
            if (text == action.text)
            {
                SERIALZ.print("executing ");
                SERIALZ.println(text);
                action.execute(this);
            }
        }
    }

public:
    Telegramz()
    {
        secured_client = new WiFiClientSecure();
        secured_client->setCACert(telegram_cert);

        bot = new AsyncTelegram2(*secured_client);

        bot->addSentCallback([](bool sent)
                             {
            const char* res = sent ? "Picture delivered!" : "Error! Picture NOT delivered";
            if (!sent) {
                SERIALZ.print("Sent");
                // bot->sendTo(chatId, res); 
            } }, 3000);

        // Set the Telegram bot properies
        bot->setUpdateTime(1000);
        bot->setTelegramToken(botToken);

        // Check if all things are ok
        // SERIALZ.print("\nTest Telegram connection... ");
        // bot->begin() ? SERIALZ.println("OK") : SERIALZ.println("NOK");

        configTime(0, 0, "pool.ntp.org"); // get UTC time via NTP
        // time_t now = time(nullptr);
        // while (now < 24 * 3600)
        // {
        //     SERIALZ.print(".");
        //     delay(100);
        //     now = time(nullptr);
        // }

        // SERIALZ.print("Telegramz init ");
        // SERIALZ.println(now);
    }

    void begin()
    {
        bot->begin();
    }

    void addAction(String text, TelegramCallBack toExecute)
    {
        SERIALZ.print("adding  ");
        SERIALZ.println(text);
        actions.push_back({text, toExecute});
    }

    void send(const String &message)
    {
        // if (armed)
        // {
        SERIALZ.print("sending ");
        SERIALZ.println(message);
        bot->sendTo(chatId, message);
        SERIALZ.print("sended ");
        SERIALZ.println(message);
        // }
    }

    void sendPhoto(camera_fb_t *fb)
    {
        bot->sendPhoto(chatId, fb->buf, fb->len);
    }

    void loop()
    {
        TBMessage msg;
        // if there is an incoming message...
        if (bot->getNewMessage(msg))
        {
            SERIALZ.print("New message from chat_id: ");
            SERIALZ.println(msg.sender.id);
            MessageType msgType = msg.messageType;

            if (msgType == MessageText)
            {
                // Received a text message
                handleNewMessages(msg);
            }
        }
    }
};

#endif