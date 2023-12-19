#include "Arduino.h"
#include "timekeeping.h"

void processSerialCommands() {
    if (Serial.available()) {
        String receivedCommand = "";
        receivedCommand = Serial.readString();

        std::vector<String> substrings;

        // Split the string into substrings
        while (receivedCommand.length() > 0) {
            int index = receivedCommand.indexOf(' ');
            if (index == -1) // No space found
            {
                substrings.push_back(receivedCommand);
                break;
            } else {
                substrings.push_back(receivedCommand.substring(0, index));
                receivedCommand = receivedCommand.substring(index + 1);
            }
        }

        if (!substrings.empty()) {
            Serial.print("Received command: ");
            for (const auto& str : substrings) { Serial.printf("%s ", str); }
            Serial.printf("\n");

            if (substrings[0] == "T") {
                // YYYY MM DD HH MM SS
                if (substrings.size() == 7) {
                    int year = substrings[1].toInt();
                    int month = substrings[2].toInt();
                    int day = substrings[3].toInt();
                    int hour = substrings[4].toInt();
                    int min = substrings[5].toInt();
                    int sec = substrings[6].toInt();

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