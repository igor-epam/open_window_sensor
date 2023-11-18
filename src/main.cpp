#include <Arduino.h>
#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <FastBot.h>

#include <optional>

#include "Sniffer.h"
#include "Settings.h"
#include "StringViewUtils.h"
#include "WindowController.h"
#include "EV1527.h"

namespace
{
  auto constexpr recieverPin = 4;

  auto sniffer = Sniffer{recieverPin};
  auto telegram_bot = FastBot{};
  std::optional<Settings> settings = std::nullopt;
  bool learning_mode = false;
  auto window_controller = WindowController{};

}

void telegram_bot_new_message(FB_msg &msg)
{
  if (msg.text == "/help")
  {
    auto const reply = "/add DEVICE_ID OPENED_ID SINGLE_WORD_NAME - adds device as the opened window.\n"
                       "/enable_learning - enables learning mode, when unknown devices are highlited, othewise they are ignored.\n"
                       "/disable_learning - disables learning mode, when unknown devices ignored.\n"
                       "/allowed_time - the time in minutes before the first notification.\n"
                       "/notify_every - the time in minutes between notifications";
    telegram_bot.replyMessage(reply, msg.messageID, msg.chatID);
  }
  else if (msg.text == "/enable_learning")
  {
    learning_mode = true;
    auto const reply = "Learning mode is enabled.";
    telegram_bot.replyMessage(reply, msg.messageID, msg.chatID);
  }
  else if (msg.text == "/disable_learning")
  {
    learning_mode = false;
    auto const reply = "Learning mode is disabled.";
    telegram_bot.replyMessage(reply, msg.messageID, msg.chatID);
  }
  else if (msg.text.startsWith("/allowed_time"))
  {
    auto const tokens = split_as_vector(msg.text.c_str(), ' ');
    if (2U != tokens.size())
    {
      auto const reply = "Incorrect parameters for /allowed_time command. See help.";
      telegram_bot.replyMessage(reply, msg.messageID, msg.chatID);
    }
    auto const time = std::stoul(tokens[1]);
    settings->_numbefOfMinutesBeforeNotification = time;
    settings->writeConfigFile();
    auto const reply = String{"It's allowed to keep a window opened for "} + settings->_numbefOfMinutesBeforeNotification + " minutes.";
    telegram_bot.replyMessage(reply, msg.messageID, msg.chatID);
  }
  else if (msg.text.startsWith("/notify_every"))
  {
    auto const tokens = split_as_vector(msg.text.c_str(), ' ');
    if (2U != tokens.size())
    {
      auto const reply = "Incorrect parameters for /notify_every command. See help.";
      telegram_bot.replyMessage(reply, msg.messageID, msg.chatID);
    }
    auto const time = std::stoul(tokens[1]);
    settings->_numberOfMinutesBetweenNotifications = time;
    settings->writeConfigFile();
    auto const reply = String{"Will notify every "} + settings->_numberOfMinutesBetweenNotifications + " minutes if the window is opened after allowed time.";
    telegram_bot.replyMessage(reply, msg.messageID, msg.chatID);
  }
  else if (msg.text.startsWith("/add"))
  {
    auto const tokens = split_as_vector(msg.text.c_str(), ' ');
    if (4U != tokens.size())
    {
      auto const reply = "Incorrect parameters for /add command. See help.";
      telegram_bot.replyMessage(reply, msg.messageID, msg.chatID);
    }
    auto const id = std::stoul(tokens[1]);
    auto const message = std::stoul(tokens[2]);
    auto const &name = tokens[3];
    settings->_windowDescriptions.emplace_back(Settings::WindowDescription{name, id, message, 0U /*don't care*/});
    window_controller.add_window(id, message, 0U);
    {
      auto const reply = String{"Message "} + message + " from device " + id + " added as an opened window " + name.c_str() + ".";
      telegram_bot.replyMessage(reply, msg.messageID, msg.chatID);
      settings->writeConfigFile();
    }
  }
  else
  {
    auto const reply = "Recieved unsupported command, see help";
    telegram_bot.replyMessage(reply, msg.messageID, msg.chatID);
  }
}

void setup_wifi()
{
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  WiFi.mode(WIFI_STA);

  if (WiFi.SSID() == "" || std::string_view{settings->_telegramBotKey.data()}.empty())
  {
    auto wifi_manager = WiFiManager{};
    wifi_manager.resetSettings();
    auto constexpr maximum_length_of_parameter = 100U;
    auto telegram_key_parameter = WiFiManagerParameter{"telegram_api_key_id", "Telegram API key", "", maximum_length_of_parameter};
    wifi_manager.addParameter(&telegram_key_parameter);
    auto telegram_chat_ids_parameter = WiFiManagerParameter{"telegram_chat_ids_id", "Telegram chat ids", "", maximum_length_of_parameter};
    wifi_manager.addParameter(&telegram_chat_ids_parameter);
    std::vector<const char *> menu = {"wifi", "info", "param", "sep", "restart", "exit"};
    wifi_manager.setMenu(menu);
    wifi_manager.setClass("invert");
    auto const connected = wifi_manager.autoConnect("window_sensor", "password");
    Serial.println(connected ? "connected to wifi" : "no connection");
    std::memcpy(settings->_telegramBotKey.data(), telegram_key_parameter.getValue(), std::min(settings->_telegramBotKey.size(), std::strlen(telegram_key_parameter.getValue())));
    split(telegram_chat_ids_parameter.getValue(), settings->_chatIds);
    settings->writeConfigFile();
  }
  else
  {
    WiFi.begin();
  }
}

void notify_about_opened(std::vector<WindowController::WindowWithStatus> const &windows_with_statuses)
{
  auto now = millis();
  auto static last_notification_time = now;

  // TODO track every window separately
  // TODO add snooze
  // TODO add DND
  if (now - last_notification_time <= settings->_numberOfMinutesBetweenNotifications * 1000 * 60)
  {
    return;
  }
  for (auto const window_with_status : windows_with_statuses)
  {
    if ((WindowController::Status::Opened == window_with_status._status) && (now - window_with_status._last_change_time) > settings->_numbefOfMinutesBeforeNotification * 1000 * 60)
    {
      auto const iterator = std::find_if(settings->_windowDescriptions.cbegin(), settings->_windowDescriptions.cend(), [id = window_with_status._id](auto const &windowDescription)
                                         { return id == windowDescription._id; });

      auto const reply = String{"Window "} + iterator->_name.c_str() + " is opened for longer than " + settings->_numbefOfMinutesBeforeNotification + " minutes. Please close it.";
      for (auto const &chatId : settings->_chatIds)
      {
        if (chatId.front() != 0)
        {
          telegram_bot.sendMessage(reply, chatId.data());
        }
      }
      last_notification_time = now;
    }
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println();
  LittleFS.begin();
  settings = Settings::readConfigFile();
  setup_wifi();
  telegram_bot.setToken(settings->_telegramBotKey.data());
  telegram_bot.attach(telegram_bot_new_message);
  for (auto const window_description : settings->_windowDescriptions)
  {
    window_controller.add_window(window_description._id, window_description._openedCode, window_description._closedCode);
  }
}

void loop()
{
  telegram_bot.tick();
  if (auto const message = sniffer.get_message(); message.has_value())
  {
    if (auto const ev1527 = ToEV1527(message->_payload, message->_size); ev1527.has_value())
    {
      if (!window_controller.handle_message(ev1527->_reciever_id, ev1527->_message) && learning_mode)
      {
        for (auto const &chatId : settings->_chatIds)
        {
          auto const telegram_message = String{"If you want to add this device as a window, send the follwing command appending the name of device to the message. The name should be a single word.\n/add "} + String{ev1527->_reciever_id} + " " + String{ev1527->_message} + " ";
          telegram_bot.sendMessage(telegram_message, chatId.data());
        }
      }
    }
  }

  auto const statuses = window_controller.get_statuses();
  notify_about_opened(statuses);
}
