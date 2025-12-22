#pragma once

#include <keyauth/skStr.h>
#include <Lmcons.h>
#include <functional>
namespace Notifier {
    //std::string botName = skCrypt("8453817061:AAFzZ0Xl6C8VivHLaw_V6bcb7Io1Uf0Mw6k").decrypt();
   /* TgBot::Bot botCreate(std::string botName = skCrypt("8453817061:AAFzZ0Xl6C8VivHLaw_V6bcb7Io1Uf0Mw6k").decrypt()) {
         return TgBot::Bot(botName);
    }*/
    std::string ANSIToUTF8(const char* ansiStr, UINT fromCodePage = CP_ACP) {
        if (!ansiStr) return "";

        int wlen = MultiByteToWideChar(fromCodePage, 0, ansiStr, -1, nullptr, 0);
        if (wlen == 0) return "";

        wchar_t* wbuf = new wchar_t[wlen];
        MultiByteToWideChar(fromCodePage, 0, ansiStr, -1, wbuf, wlen);

        int utf8len = WideCharToMultiByte(CP_UTF8, 0, wbuf, -1, nullptr, 0, nullptr, nullptr);
        std::string utf8Str(utf8len, 0);

        WideCharToMultiByte(CP_UTF8, 0, wbuf, -1, &utf8Str[0], utf8len, nullptr, nullptr);

        delete[] wbuf;
        utf8Str.pop_back();
        return utf8Str;
    }
    void OnStartCommand(TgBot::Bot& bot, TgBot::Message::Ptr message) {
        
        std::string id = std::to_string(message->chat->id);
        std::string messagee = "This bot will send you message if ingame bot broken\n Id: " + id + "\n You need to pass the id to ingame bot \n";// System name: " + userStr;
        bot.getApi().sendMessage(message->chat->id, messagee);
    }
    
    
    TgBot::TgLongPoll longPoll(bot);

    void getId(TgBot::Bot& bot) {
        
        bot.getEvents().onCommand("start",
            std::bind(OnStartCommand, std::ref(bot), std::placeholders::_1));
    }

    void brokenMessage(TgBot::Bot& bot ,std::string idStr) {
        char userChar[UNLEN + 1];
        DWORD size = UNLEN + 1;
        GetUserNameA(userChar, &size);
        std::string userStr = ANSIToUTF8(userChar);
        std::string bMessage = "Bot is broken on user: " + userStr;
        try {
           bot.getApi().sendMessage(idStr, bMessage);
        }
        catch (TgBot::TgException& e) {
            printf("error: %s\n", e.what());
        }
    }
    void sendLongPollEvent(TgBot::Bot& bot) {

        

        for (int i = 0; i < 2; i++) {
            getId(bot);
            try {
                longPoll.start();
            }
            catch (TgBot::TgException& e) {
                printf("error: %s\n", e.what());
            }
        }
        
    }

   
};
