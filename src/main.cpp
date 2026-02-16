#include <Arduino.h>
#include <HardwareSerial.h>  // For HardwareSerial support
#include "DFRobotDFPlayerMini.h"  // DFPlayer library
#include <vector>  // For std::vector
#include <utility>  // For std::pair
#include "sound_mappings.h"

// we're on standard uart 21 and 20
HardwareSerial FPSerial(1);

// Create an instance of the DFPlayer Mini
DFRobotDFPlayerMini myDFPlayer;

// hardcoded SD card folder structure in sound_mappings.cpp
void initSoundMappings();

// State for playback chaining
enum State {IDLE, PLAYING_NON_LOOP, LOOPING};
State currentState = IDLE;

unsigned long fadeStartTime = 0;
const unsigned long FADE_DURATION_MS = 3000;  // 3 seconds
bool fadingOut = false;
int originalVolume = 25;   // We'll remember last set volume
// debounce shit
unsigned long lastSwitchChange = 0;
const unsigned long DEBOUNCE_MS = 40;   // 40-80ms should be enough for reed
bool lastConnected = false;

// Switch pin
const int switchPin = 10;  // GPIO10 to ground

// Forward declaration for loopRandomPerfect
void loopRandomPerfect();

// Function to print detailed messages from the DFPlayer Mini
void printDetail(uint8_t type, int value) {
  switch (type) {
    case TimeOut:
      Serial.println(F("Time Out!"));
      break;
    case WrongStack:
      Serial.println(F("Stack Wrong!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("Card Inserted!"));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("Card Removed!"));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("Card Online!"));
      break;
    case DFPlayerUSBInserted:
      Serial.println("USB Inserted!");
      break;
    case DFPlayerUSBRemoved:
      Serial.println("USB Removed!");
      break;
case DFPlayerPlayFinished:
  Serial.print(F("Number:"));
  Serial.print(value);
  Serial.println(F(" Play Finished!"));

  if (fadingOut) {
    // Already fading → don't chain anything
    break;
  }

  if (currentState == PLAYING_NON_LOOP) {
    // Only chain if switch is STILL held (LOW)
    if (digitalRead(switchPin) == LOW) {
      loopRandomPerfect();
      currentState = LOOPING;
    } else {
      currentState = IDLE;
    }
  }
  break;
    case DFPlayerError:
      Serial.print(F("DFPlayerError:"));
      switch (value) {
        case Busy:
          Serial.println(F("Card not found"));
          break;
        case Sleeping:
          Serial.println(F("Sleeping"));
          break;
        case SerialWrongStack:
          Serial.println(F("Get Wrong Stack"));
          break;
        case CheckSumNotMatch:
          Serial.println(F("Check Sum Not Match"));
          break;
        case FileIndexOut:
          Serial.println(F("File Index Out of Bound"));
          break;
        case FileMismatch:
          Serial.println(F("Cannot Find File"));
          break;
        case Advertise:
          Serial.println(F("In Advertise"));
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}

// Function to play a specific sound file with a given duration (unchanged)
void playSound(int soundNumber, int duration) {
  myDFPlayer.play(soundNumber);
  delay(duration * 1000); // Convert duration from seconds to milliseconds
}

// Play a random non-loop file once, then automatically chain to function 4 on finish if still connected
void playRandomOnceThenLoop() {
  if (nonLoops.empty()) {
    Serial.println(F("No non-loop files available."));
    return;
  }
  size_t idx = random(nonLoops.size());
  auto p = nonLoops[idx];
  myDFPlayer.playFolder(p.first, p.second);
  currentState = PLAYING_NON_LOOP;
  Serial.print(F("Playing non-loop: Folder "));
  Serial.print(p.first);
  Serial.print(F(", File "));
  Serial.println(p.second);
}

// Loop a random perfect loop file
void loopRandomPerfect() {
  if (perfectLoops.empty()) {
    Serial.println(F("No perfect loop files available."));
    return;
  }
  size_t idx = random(perfectLoops.size());
  auto p = perfectLoops[idx];
  myDFPlayer.playFolder(p.first, p.second);
  myDFPlayer.enableLoop();  // Enable looping for this track
  Serial.print(F("Looping perfect loop: Folder "));
  Serial.print(p.first);
  Serial.print(F(", File "));
  Serial.println(p.second);
}

// Helper to smoothly fade volume down to 0, then stop
void startFadeOutAndStop() {
  if (fadingOut) return;  // already fading

  originalVolume = myDFPlayer.readVolume();  // try to get current (may return -1 on some modules)
  if (originalVolume <= 0 || originalVolume > 25) originalVolume = 25;

  fadingOut = true;
  fadeStartTime = millis();
  Serial.println(F("Starting 5s volume fade-out..."));
}

// Call this regularly during fade
void handleFadeOut() {
  if (!fadingOut) return;

  unsigned long elapsed = millis() - fadeStartTime;
  if (elapsed >= FADE_DURATION_MS) {
    myDFPlayer.volume(0);
    myDFPlayer.stop();
    myDFPlayer.disableLoop();        // just in case
    fadingOut = false;
    currentState = IDLE;
    Serial.println(F("Fade complete → playback stopped."));
    myDFPlayer.volume(originalVolume);
    Serial.println(F("Volume restored to original value."));
  } else {
    // Linear fade (you can also use ease-in/out if desired)
    float progress = (float)elapsed / FADE_DURATION_MS;
    int newVol = originalVolume * (1.0f - progress);
    if (newVol < 0) newVol = 0;
    myDFPlayer.volume(newVol);
  }
}

void setup() {
  // Begin FPSerial on custom pins for UART1
  FPSerial.begin(9600, SERIAL_8N1, 20, 21);
  Serial.begin(115200);

  // Set up switch pin with internal pull-up
  pinMode(switchPin, INPUT_PULLUP);

  Serial.println(F("DFRobot DFPlayer Mini Demo"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));
  delay(3000);  // Give module time to power up

  myDFPlayer.setTimeOut(1000);  // Increase timeout for quirky reads

  if (!myDFPlayer.begin(FPSerial)) {
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1. Please recheck the connection!"));
    Serial.println(F("2. Please insert the SD card!"));
    // while (true) { delay(0); }  // Temporarily commented to continue for debugging
  } else {
    Serial.println(F("DFPlayer Mini online."));
    myDFPlayer.reset();  // Reset module to ensure clean state
    delay(2000);  // Wait after reset
  }

  myDFPlayer.volume(originalVolume); // Set volume value. From 0 to 30

  // play sound on boot if we are okay
  myDFPlayer.playFolder(10, 1);

  // Seed random (use an unused analog pin if GPIO0 is not free)
  randomSeed(analogRead(0));

  // BEWARE THE HARDCODED MP3S
  initSoundMappings();
}

void loop() {
  // Handle DFPlayer events
  if (myDFPlayer.available()) {
    printDetail(myDFPlayer.readType(), myDFPlayer.read());
  }

  // ── Volume fade handling ───────────────────────
  handleFadeOut();

  // ── Switch monitoring ──────────────────────────
  bool rawConnected = (digitalRead(switchPin) == LOW);
  unsigned long now = millis();

  if (rawConnected != lastConnected && (now - lastSwitchChange >= DEBOUNCE_MS)) {
    lastConnected = rawConnected;
    lastSwitchChange = now;

  if (lastConnected && currentState == IDLE) {
      playRandomOnceThenLoop();
    }
    else if (!lastConnected && (currentState == PLAYING_NON_LOOP || currentState == LOOPING)) {
      if (!fadingOut) startFadeOutAndStop();
    }
  }

  // Small delay is usually fine — helps stability
  delay(5);
}