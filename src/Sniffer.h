#pragma once

#include <optional>
#include <cstdint>
#include <RCSwitch.h>

class FastBot;

class Sniffer
{
public:
  /**
   * Work with short messages only, as the purpose of this sniffer is listening
   * for EV1527 (24 bits at most).
   */
  struct Message
  {
    std::uint32_t _payload;
    std::uint8_t _size;
  };

  Sniffer(int interruptionPin);

  std::optional<Message> get_message();

private:
  RCSwitch _listener;
};
