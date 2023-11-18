#pragma once

#include <cstdint>
#include <optional>

struct EV1527
{
    std::uint32_t _reciever_id;
    std::uint8_t _message;
};

std::optional<EV1527> ToEV1527(std::uint32_t payload, std::uint8_t length)
{
    if (24U != length)
    {
        return std::nullopt;
    }
    auto const get_id = [](std::uint32_t value)
    {
        auto const mask = 0xFFFFFUL;
        auto const extracted_bits = (value >> 4) & mask;
        return extracted_bits;
    };

    auto const get_message = [](std::uint32_t value)
    {
        auto const mask = 0xF;
        auto const extracted_bits = value & mask;
        return static_cast<std::uint8_t>(extracted_bits);
    };

    return EV1527{get_id(payload), get_message(payload)};
}
