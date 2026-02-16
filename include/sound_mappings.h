// sound_mappings.h
// Hardcoded mappings for perfect loops and non-loops
// Update these if you add/remove files!
// Folder numbers and file counts based on your SD card structure (Feb 2026)

#ifndef SOUND_MAPPINGS_H
#define SOUND_MAPPINGS_H

#include <vector>
#include <utility>
#include <cstdint>

// Perfect loops: folder 01 only (001–060)
extern std::vector<std::pair<uint8_t, uint8_t>> perfectLoops;

// Non-loops: folder 11 (001–092) + folder 12 (001 only)
extern std::vector<std::pair<uint8_t, uint8_t>> nonLoops;

// Call this once in setup() to populate the vectors
void initSoundMappings();

#endif