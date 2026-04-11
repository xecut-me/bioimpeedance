#include <Arduino.h>
#include "DYPlayerArduino.h"  // https://github.com/SnijderC/dyplayer
#include <vector>
#include <utility>

// UART for the DYPlayer (same pins as your original code)
HardwareSerial FPSerial(1);
DY::Player myDYPlayer(&FPSerial);

// Dynamic lists built from actual SD card
std::vector<std::pair<const char*, uint16_t>> perfectLoops;  // "PERFECT" + index
std::vector<std::pair<const char*, uint16_t>> nonLoops;      // "MUSIC" + "MEME" + index

// State machine (unchanged)
enum State { IDLE, PLAYING_NON_LOOP, LOOPING };
State currentState = IDLE;

unsigned long fadeStartTime = 0;
const unsigned long FADE_DURATION_MS = 3000;
bool fadingOut = false;
const int originalVolume = 30;        // 0-30, never changes at runtime

// Hall-switch debounce
bool switchActive = false;
unsigned long confirmationTimer = 0;
const unsigned long ON_HOLD_MS  = 1000;
const unsigned long OFF_HOLD_MS = 50;

const int switchPin = 10;

// Forward declaration
void loopRandomPerfect();

// ===================================================================
// Safe path builder (library requires non-const char*)
// ===================================================================
void buildPath(char* buf, size_t bufSize, const char* folder, uint16_t fileNum) {
  snprintf(buf, bufSize, "/%s/%05u.mp3", folder, fileNum);
}

// ===================================================================
// Get number of files in a folder (while muted)
// ===================================================================
uint16_t getFolderFileCount(const char* folderName) {
  if (strlen(folderName) == 0) return 0;

  char dummyBuf[64];
  buildPath(dummyBuf, sizeof(dummyBuf), folderName, 1);

  myDYPlayer.playSpecifiedDevicePath(DY::device_t::Sd, dummyBuf);
  delay(80);

  uint16_t count = myDYPlayer.getSoundCountDir();

  myDYPlayer.stop();
  delay(20);
  return count;
}

// ===================================================================
// Playback helpers
// ===================================================================
void playRandomOnceThenLoop() {
  if (nonLoops.empty()) {
    Serial.println(F("No non-loop files available."));
    return;
  }
  size_t idx = random(nonLoops.size());
  auto p = nonLoops[idx];

  myDYPlayer.setCycleMode(DY::play_mode_t::OneOff);

  char pathBuf[64];
  buildPath(pathBuf, sizeof(pathBuf), p.first, p.second);
  myDYPlayer.playSpecifiedDevicePath(DY::device_t::Sd, pathBuf);

  currentState = PLAYING_NON_LOOP;
  Serial.print(F("Playing non-loop: Folder "));
  Serial.print(p.first);
  Serial.print(F(", File "));
  Serial.println(p.second);
}

void loopRandomPerfect() {
  if (perfectLoops.empty()) {
    Serial.println(F("No perfect loop files available."));
    return;
  }
  size_t idx = random(perfectLoops.size());
  auto p = perfectLoops[idx];

  myDYPlayer.setCycleMode(DY::play_mode_t::RepeatOne);

  char pathBuf[64];
  buildPath(pathBuf, sizeof(pathBuf), p.first, p.second);
  myDYPlayer.playSpecifiedDevicePath(DY::device_t::Sd, pathBuf);

  currentState = LOOPING;
  Serial.print(F("Looping perfect loop: Folder "));
  Serial.print(p.first);
  Serial.print(F(", File "));
  Serial.println(p.second);
}

// ===================================================================
// Fade-out
// ===================================================================
void startFadeOutAndStop() {
  if (fadingOut) return;
  fadingOut = true;
  fadeStartTime = millis();
  Serial.println(F("Starting 3s volume fade-out..."));
}

void handleFadeOut() {
  if (!fadingOut) return;

  unsigned long elapsed = millis() - fadeStartTime;
  if (elapsed >= FADE_DURATION_MS) {
    myDYPlayer.setVolume(0);
    myDYPlayer.stop();
    fadingOut = false;
    currentState = IDLE;
    Serial.println(F("Fade complete → playback stopped."));
    myDYPlayer.setVolume(originalVolume);
    Serial.println(F("Volume restored."));
  } else {
    float progress = (float)elapsed / FADE_DURATION_MS;
    int newVol = originalVolume * (1.0f - progress);
    if (newVol < 0) newVol = 0;
    myDYPlayer.setVolume(newVol);
  }
}

// ===================================================================
// Play-state polling (replaces DFPlayer event)
// ===================================================================
DY::play_state_t lastPlayState = DY::play_state_t::Stopped;

void setup() {
  FPSerial.begin(9600, SERIAL_8N1, 20, 21);
  Serial.begin(115200);
  pinMode(switchPin, INPUT_PULLUP);

  Serial.println(F("DYPlayer Mini Demo"));
  Serial.println(F("Initializing DYPlayer ..."));
  delay(3000);

  // Quick connection test
  if (myDYPlayer.checkPlayState() == DY::play_state_t::Fail) {
    Serial.println(F("Unable to communicate with DYPlayer! Check wiring/SD."));
  } else {
    Serial.println(F("DYPlayer online."));
  }
  myDYPlayer.stop();

  // ── Build file lists dynamically (replaces sound_mappings.cpp) ──
  Serial.println(F("Scanning SD card folders..."));
  myDYPlayer.setVolume(0);                     // mute during scan

  // PERFECT
  uint16_t count = getFolderFileCount("PERFECT");
  perfectLoops.clear();
  for (uint16_t i = 1; i <= count; ++i) perfectLoops.emplace_back("PERFECT", i);

  // MUSIC
  count = getFolderFileCount("MUSIC");
  nonLoops.clear();
  for (uint16_t i = 1; i <= count; ++i) nonLoops.emplace_back("MUSIC", i);

  // MEME
  count = getFolderFileCount("MEME");
  for (uint16_t i = 1; i <= count; ++i) nonLoops.emplace_back("MEME", i);

  myDYPlayer.setVolume(originalVolume);

  Serial.print(F("perfectLoops (PERFECT): ")); Serial.println(perfectLoops.size());
  Serial.print(F("nonLoops (MUSIC+MEME): "));  Serial.println(nonLoops.size());

  randomSeed(analogRead(0));

  // Boot confirmation sound
  if (!nonLoops.empty()) {
    char bootBuf[64];
    buildPath(bootBuf, sizeof(bootBuf), "MEME", 1);
    myDYPlayer.playSpecifiedDevicePath(DY::device_t::Sd, bootBuf);
    Serial.println(F("Boot confirmation: MEME/00001.mp3"));
  }
}

void loop() {
  // Play-state polling
  DY::play_state_t curr = myDYPlayer.checkPlayState();

  if (lastPlayState == DY::play_state_t::Playing &&
      curr == DY::play_state_t::Stopped) {

    if (!fadingOut && currentState == PLAYING_NON_LOOP) {
      if (digitalRead(switchPin) == LOW) {
        loopRandomPerfect();
        currentState = LOOPING;
      } else {
        currentState = IDLE;
      }
    }
  }
  lastPlayState = curr;

  handleFadeOut();

  // Switch debounce (exactly as you tested)
  bool rawConnected = (digitalRead(switchPin) == LOW);
  unsigned long now = millis();

  if (rawConnected != switchActive) {
    if (confirmationTimer == 0) confirmationTimer = now;

    unsigned long requiredHold = rawConnected ? ON_HOLD_MS : OFF_HOLD_MS;

    if (now - confirmationTimer >= requiredHold) {
      switchActive = rawConnected;
      confirmationTimer = 0;

      if (switchActive && currentState == IDLE) {
        playRandomOnceThenLoop();
      } else if (!switchActive && (currentState == PLAYING_NON_LOOP || currentState == LOOPING)) {
        if (!fadingOut) startFadeOutAndStop();
      }
    }
  } else {
    confirmationTimer = 0;
  }

  delay(5);
}
