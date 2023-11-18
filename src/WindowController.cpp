#include "WindowController.h"

#include <Arduino.h>
#include <algorithm>

namespace
{
    template <typename T>
    auto findWindow(std::uint32_t id, T &container)
    {
        return std::find_if(container.begin(), container.end(), [id](auto const &stateOfWindow)
                            { return id == stateOfWindow._id; });
    }
}

bool WindowController::add_window(std::uint32_t id, std::uint8_t opened_message, std::uint8_t closed_message)
{
    if (auto const iterator = findWindow(id, _windowsWithStatuses); _windowsWithStatuses.cend() != iterator)
    {
        // already exists
        return false;
    }
    _windowsWithStatuses.emplace_back(WindowWithStatus{id, opened_message});
    return true;
}

std::vector<WindowController::WindowWithStatus> const &WindowController::get_statuses() const
{
    return _windowsWithStatuses;
}

bool WindowController::handle_message(std::uint32_t id, std::uint8_t message)
{
    if (auto iterator = findWindow(id, _windowsWithStatuses); _windowsWithStatuses.end() != iterator)
    {
        if (message == iterator->_opened_message)
        {
            iterator->_status = Status::Opened;
        }
        // else if (message == iterator->_closed_message)
        // {
        //     iterator->_status = Status::Closed;
        // }
        else
        {
            iterator->_status = Status::Unknown;
        }
        iterator->_last_change_time = millis();
        return true;
    }
    return false;
}
