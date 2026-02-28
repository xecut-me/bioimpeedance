// sound_mappings.cpp
#include <Arduino.h>
#include "sound_mappings.h"

std::vector<std::pair<uint8_t, uint8_t>> perfectLoops;
std::vector<std::pair<uint8_t, uint8_t>> nonLoops;

void initSoundMappings() {
    perfectLoops.clear();
    nonLoops.clear();

    // Folder 01 – perfect loops – 60 files 14feb26
    for (uint8_t i = 1; i <= 60; i++) {
        perfectLoops.emplace_back(1, i);
    }

    // Folder 10 – short meme sounds preferably short that turn into perfect loops
    for (uint8_t i = 1; i <= 72; i++) {
        nonLoops.emplace_back(10, i);
    }

    // Folder 11 – non-loops 120 files 28feb26
    for (uint8_t i = 1; i <= 127; i++) {
        nonLoops.emplace_back(11, i);
    }

    // Optional: print sizes for debug
    Serial.print(F("Hardcoded perfectLoops: ")); Serial.println(perfectLoops.size());
    Serial.print(F("Hardcoded nonLoops: "));    Serial.println(nonLoops.size());
}