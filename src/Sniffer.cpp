#include "Sniffer.h"

Sniffer::Sniffer(int interruptionPin)
    : _listener{}
{
    _listener.enableReceive(interruptionPin);
}

std::optional<Sniffer::Message> Sniffer::get_message()
{
    if (!_listener.available())
    {
        return std::nullopt;
    }

    auto const payload = _listener.getReceivedValue();
    auto const length = _listener.getReceivedBitlength();
    _listener.resetAvailable();

    // TODO: check that the message is short enough to fit into value and size
    return Sniffer::Message{static_cast<std::uint32_t>(payload), static_cast<std::uint8_t>(length)};
}
