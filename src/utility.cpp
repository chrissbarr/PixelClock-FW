/* Project Scope */
#include "utility.h"

namespace utility {

namespace printFormatting {

void printSolidLine(uint8_t width) {
    for (int i = 0; i < width; i++) { Serial.printf("-"); }
    Serial.printf("\n");
}

void printTextCentred(const char* text, uint8_t width) {
    int padlen = (width - (strlen(text) + 2)) / 2;
    for (int i = 0; i < padlen; i++) { Serial.printf("-"); }
    Serial.printf(" %s ", text);
    for (int i = 0; i < padlen; i++) { Serial.printf("-"); }
    Serial.printf("\n");
}

} // namespace printFormatting

void listAllI2CDevices(TwoWire& wire) {
    for (uint8_t address = 1; address < 127; address++) {
        wire.beginTransmission(address);
        if (wire.endTransmission() == 0) {
            Serial.printf("%-*s 0x%02x\n", printFormatting::textPadding, "Found device at:", address);
        }
    }
}

} // namespace utility