/* Project Scope */
#include "FMTWrapper.h"
#include "timekeeping.h"
#include "utility.h"

/* Arduino Core */
#include "Arduino.h"

/* C++ Standard Library */
#include <string>
#include <vector>

void processSerialCommands() {
    if (Serial.available()) {
        std::string receivedCommand(Serial.readString().c_str());
        std::vector<std::string> substrings;

        // Split the string into substrings
        while (receivedCommand.length() > 0) {
            std::size_t index = receivedCommand.find(' ');
            if (index == receivedCommand.npos) {
                // No space found
                substrings.push_back(receivedCommand);
                break;
            } else {
                substrings.push_back(receivedCommand.substr(0, index));
                receivedCommand = receivedCommand.substr(index + 1);
            }
        }

        if (!substrings.empty()) {
            Serial.print("Received command: ");
            for (const auto& str : substrings) { printing::print(Serial, fmt::to_string(str)); }
            Serial.print("\n");

            if (substrings[0] == "T") {
                // YYYY MM DD HH MM SS
                if (substrings.size() == 7) {
                    int year = std::stoi(substrings[1]);
                    int month = std::stoi(substrings[2]);
                    int day = std::stoi(substrings[3]);
                    int hour = std::stoi(substrings[4]);
                    int min = std::stoi(substrings[5]);
                    int sec = std::stoi(substrings[6]);

                    TimeElements time;
                    time.Year = uint8_t(CalendarYrToTm(year));
                    time.Month = uint8_t(month);
                    time.Day = uint8_t(day);
                    time.Hour = uint8_t(hour);
                    time.Minute = uint8_t(min);
                    time.Second = uint8_t(sec);
                    setTimeGlobally(makeTime(time));
                }
            }
        }
    }
}