#include "Settings.h"

#include <LittleFS.h>
#include <ArduinoJson.h>

namespace
{
    auto const filePath = "/config.json";
    auto const telegramBotKeyName = "telegramBotKey";
    auto const telegramChatIdsName = "telegramChatIds";
    auto const windowDescriptionsName = "windowDescriptions";
    auto const windowDescriptionId = "id";
    auto const windowDescriptionOpenCode = "openCode";
    auto const windowDescriptionCloseCode = "closeCode";
    auto const windowDescriptionName = "windowDescriptionName";
    auto const numbefOfMinutesBeforeNotification = "numbefOfMinutesBeforeNotificationKey";
    auto const numberOfMinutesBetweenNotifications = "numberOfMinutesBetweenNotificationsKey";

    void chatIdsToJson(JsonArray jsonArray, Settings::ChatIds const &chatIds)
    {
        for (const auto &chatId : chatIds)
        {
            if (chatId.front() != 0)
            {
                jsonArray.add(chatId.data());
            }
        }
    }

    template <typename T>
    void CopyToArray(char const *str, T &array)
    {
        std::fill(array.begin(), array.end(), 0);
        auto const asView = std::string_view{str};
        std::memcpy(array.data(), asView.data(), std::min(asView.size(), array.size()));
    }

    Settings::ChatIds jsonToChatIds(JsonArray const &jsonArray)
    {
        auto chatIds = Settings::ChatIds{};
        for (auto i = 0U; i < chatIds.size(); ++i)
        {
            auto &item = chatIds.at(i);
            std::fill(item.begin(), item.end(), 0);
            if (i >= jsonArray.size())
            {
                continue;
            }
            auto const str = jsonArray[i].as<char const *>();
            CopyToArray(str, item);
        }
        return chatIds;
    }

    Settings::WindowDescriptions deserializeWindowDescriptions(JsonArray const &jsonArray)
    {
        auto receivedWindows = Settings::WindowDescriptions{};
        receivedWindows.reserve(jsonArray.size());
        std::transform(jsonArray.begin(), jsonArray.end(), std::back_inserter(receivedWindows), [](JsonVariant const &jsonElement)
                       { return Settings::WindowDescription{
                             jsonElement[windowDescriptionName].as<char const *>(),
                             jsonElement[windowDescriptionId].as<std::uint64_t>(),
                             jsonElement[windowDescriptionOpenCode].as<std::uint64_t>(),
                             jsonElement[windowDescriptionCloseCode].as<std::uint64_t>()}; });
        return receivedWindows;
    }

    void windowDescriptionsToJson(JsonArray jsonArray, Settings::WindowDescriptions const &windowDescriptions)
    {
        for (auto const &windowDescription : windowDescriptions)
        {
            JsonObject obj = jsonArray.createNestedObject();
            obj[windowDescriptionName] = windowDescription._name;
            obj[windowDescriptionId] = windowDescription._id;
            obj[windowDescriptionOpenCode] = windowDescription._openedCode;
            obj[windowDescriptionCloseCode] = windowDescription._closedCode;
        }
    }
}

void Settings::writeConfigFile()
{
    DynamicJsonDocument json{1024};

    // JSONify local configuration parameters
    json[telegramBotKeyName] = _telegramBotKey.data();
    json[numbefOfMinutesBeforeNotification] = _numbefOfMinutesBeforeNotification;
    json[numberOfMinutesBetweenNotifications] = _numberOfMinutesBetweenNotifications;
    chatIdsToJson(json.createNestedArray(telegramChatIdsName), _chatIds);
    windowDescriptionsToJson(json.createNestedArray(windowDescriptionsName), _windowDescriptions);

    // Open file for writing
    File f = LittleFS.open(filePath, "w");
    if (!f)
    {
        Serial.println("Failed to open config file for writing");
    }

    serializeJson(json, Serial);
    // Write data to file and close it
    serializeJson(json, f);
    f.close();
}

Settings Settings::readConfigFile()
{
    auto const parse = []() -> std::optional<Settings>
    {
        // this opens the config file in read-mode
        File f = LittleFS.open(filePath, "r");

        if (!f)
        {
            return std::nullopt;
        }
        else
        {
            // Using dynamic JSON buffer which is not the recommended memory model, but anyway
            // See https://github.com/bblanchon/ArduinoJson/wiki/Memory%20model
            DynamicJsonDocument doc(1024);
            DeserializationError error = deserializeJson(doc, f);
            // Test if parsing succeeds.
            if (error)
            {
                return std::nullopt;
            }
            serializeJson(doc, Serial);

            // Parse all config file parameters, override
            // local config variables with parsed values
            if (!doc.containsKey(telegramBotKeyName) ||
                !doc.containsKey(telegramChatIdsName) || !doc.containsKey(windowDescriptionsName) || !doc.containsKey(numbefOfMinutesBeforeNotification) || !doc.containsKey(numberOfMinutesBetweenNotifications))
            {
                return std::nullopt;
            }
            TelegramApiKey key;
            CopyToArray(doc[telegramBotKeyName].as<char const *>(), key);

            return Settings{
                std::move(key),
                jsonToChatIds(doc[telegramChatIdsName].as<JsonArray>()),
                deserializeWindowDescriptions(doc[windowDescriptionsName].as<JsonArray>()),
                doc[numbefOfMinutesBeforeNotification].as<std::uint64_t>(),
                doc[numberOfMinutesBetweenNotifications].as<std::uint64_t>()};
        }
    };
    auto optionalSettins = parse();
    if (!optionalSettins.has_value())
    {
        Settings{}.writeConfigFile();
        return {};
    }

    return std::move(optionalSettins.value());
}

void Settings::reset()
{
    *this = Settings{};
}
