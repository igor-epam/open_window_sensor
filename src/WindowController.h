#pragma once

#include <vector>
#include <cstdint>
#include <Arduino.h>

class WindowController
{
public:
    enum class Status
    {
        Unknown,
        Opened,
        Closed
    };

    struct WindowWithStatus
    {
        friend class WindowController;
        WindowWithStatus(std::uint32_t id,
                         std::uint8_t opened_message)
            : _id{id}, _status{Status::Unknown}, _last_change_time{millis()}, _opened_message{opened_message}
        {
        }

        std::uint32_t _id;
        Status _status;
        unsigned long _last_change_time;

    private:
        std::uint8_t _opened_message;
    };

    WindowController() = default;

    bool add_window(std::uint32_t id, std::uint8_t opened_message, std::uint8_t closed_message);

    std::vector<WindowWithStatus> const &get_statuses() const;

    bool handle_message(std::uint32_t id, std::uint8_t message);

    // TODO: once needed
    // bool remove_window(std::uint32_t id);
    // void enumerate_windows();

private:
    std::vector<WindowWithStatus> _windowsWithStatuses;
};