#pragma once

#include <array>
#include <cstdint>
#include <vector>
#include <string>

struct Settings
{
  using TelegramApiKey = std::array<char, 100>;
  using ChatIds = std::array<std::array<char, 10>, 10>;
  struct WindowDescription
  {
    std::string _name;
    std::uint64_t _id;
    std::uint64_t _openedCode;
    std::uint64_t _closedCode;
  };
  using WindowDescriptions = std::vector<WindowDescription>;

  TelegramApiKey _telegramBotKey;
  ChatIds _chatIds;
  WindowDescriptions _windowDescriptions;
  std::uint64_t _numbefOfMinutesBeforeNotification = 1U;
  std::uint64_t _numberOfMinutesBetweenNotifications = 1U;

  void
  writeConfigFile();
  static Settings readConfigFile();

  void reset();

private:
  Settings() = default;
};
