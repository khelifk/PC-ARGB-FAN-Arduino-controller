/*
 * ARGB Controller for PC Fan with YK-901 Remote
 * ACTUAL CODES AND LAYOUT OF YOUR REMOTE
 * * Hardware Configuration:
 * - ARGB Data Pin: Digital Pin 3
 * - IR Receiver: Digital Pin 5
 * - Fan PWM Control (optional): Digital Pin 9
 * * Required Libraries:
 * - FastLED (for ARGB control)
 * - IRremote (for infrared remote)
 */

#include <FastLED.h>
#include <IRremote.hpp>

// ===== PIN DEFINITIONS =====
#define ARGB_PIN 3
#define IR_RECEIVE_PIN 5
#define FAN_PWM_PIN 9  // Optional

// ===== LED CONFIGURATION =====
#define NUM_LEDS 16  // Adjust based on your fan (12-24 LEDs)
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
#define BRIGHTNESS 128  // 0-255

CRGB leds[NUM_LEDS];

// ===== YK-001 REMOTE CODES =====
// Real layout of the remote:
//
//   [POWER] [MODE] [MUTE]      Line 1
//   [◄◄]    [▶||]  [►►]     Line 2 
//   [─]     [+]    [EQ]       Line 3
//   [0]     [100+] [200+]      Line 4
//   [1]     [2]    [3]         Line 5
//   [4]     [5]    [6]         Line 6
//   [7]     [8]    [9]         Line 7

// Line 1 - Main controls
#define IR_POWER 0x45          // Power ON/OFF
#define IR_MODE 0x46           // Change animation mode
#define IR_MUTE 0x47           // Pause animation

// Line 2 - Playback controls
#define IR_PREV 0x44           // Previous (slower animation)
#define IR_PLAY_PAUSE 0x40     // Play/Pause
#define IR_NEXT 0x43           // Next (faster animation)

// Line 3 - Volume/Brightness controls
#define IR_VOL_MINUS 0x07      // Volume - (brightness -)
#define IR_VOL_PLUS 0x15       // Volume + (brightness +)
#define IR_EQ 0x09             // EQ (rainbow effect)

// Line 4 - Special functions
#define IR_ZERO 0x16           // 0 - White
#define IR_100_PLUS 0x19       // 100+ - Special effect 1
#define IR_200_PLUS 0x0D       // 200+ - Default reset

// Line 5 - Keys 1-2-3 (Primary colors)
#define IR_ONE 0x0C            // 1 - Red
#define IR_TWO 0x18            // 2 - Green
#define IR_THREE 0x5E          // 3 - Blue

// Line 6 - Keys 4-5-6 (Secondary colors)
#define IR_FOUR 0x08           // 4 - Yellow
#define IR_FIVE 0x1C           // 5 - Cyan
#define IR_SIX 0x5A            // 6 - Magenta

// Line 7 - Keys 7-8-9 (Tertiary colors)
#define IR_SEVEN 0x42          // 7 - Orange
#define IR_EIGHT 0x52          // 8 - Purple
#define IR_NINE 0x4A           // 9 - Pink

// ===== STATE VARIABLES =====
bool powerOn = true;
uint8_t currentBrightness = BRIGHTNESS;
uint8_t animationMode = 0;
uint8_t animationSpeed = 50;
bool animationPaused = false;
uint8_t fanSpeed = 255;
CRGB currentColor = CRGB::White;

// Animation variables
uint8_t hue = 0;
uint8_t animationStep = 0;

// ===== INITIAL CONFIGURATION =====
void setup() {
  Serial.begin(9600);
  Serial.println(F("═══════════════════════════════════════════════"));
  Serial.println(F("  ARGB Controller - YK-901 Remote"));
  Serial.println(F("═══════════════════════════════════════════════"));

  // Initialize FastLED
  FastLED.addLeds<LED_TYPE, ARGB_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(currentBrightness);
  FastLED.clear();
  FastLED.show();
  
  // Initialize IR receiver
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);

  // Initialize Fan PWM
  pinMode(FAN_PWM_PIN, OUTPUT);
  analogWrite(FAN_PWM_PIN, fanSpeed);
  
  Serial.println(F("\n✓ System Initialized"));
  Serial.println(F("\n🎮 Key Guide:"));
  Serial.println(F("┌─────────────────────────────────────────┐"));
  Serial.println(F("│ LINE 1: [⚡Power] [🔄Mode] [⏸Mute]    │"));
  Serial.println(F("│ LINE 2: [◄◄Slow] [▶Pause] [►►Fast]    │"));
  Serial.println(F("│ LINE 3: [─Dim]   [+Bright] [🎵EQ]      │"));
  Serial.println(F("│ LINE 4: [0-White] [100+] [200+Reset]   │"));
  Serial.println(F("│ LINE 5: [1-Red] [2-Green] [3-Blue]    │"));
  Serial.println(F("│ LINE 6: [4-Yellow] [5-Cyan] [6-Magenta] │"));
  Serial.println(F("│ LINE 7: [7-Orange][8-Purple][9-Pink]   │"));
  Serial.println(F("└─────────────────────────────────────────┘"));
  Serial.println(F("\n═══════════════════════════════════════════════\n"));

  // Startup animation
  startupAnimation();
}

// ===== MAIN LOOP =====
void loop() {
  if (IrReceiver.decode()) {
    handleIRCommand(IrReceiver.decodedIRData.command);
    IrReceiver.resume();
  }
  
  if (powerOn && !animationPaused) {
    updateAnimation();
  }
  
  delay(10);
}

// ===== IR COMMAND HANDLER =====
void handleIRCommand(uint8_t command) {
  Serial.print(F("📡 Code: 0x"));
  if (command < 0x10) Serial.print(F("0"));
  Serial.print(command, HEX);
  Serial.print(F(" → "));
  
  switch (command) {
    // ===== LINE 1 - MAIN CONTROLS =====
    case IR_POWER:
      togglePower();
      Serial.print(F("⚡ POWER → "));
      Serial.println(powerOn ? "ON" : "OFF");
      break;
      
    case IR_MODE:
      changeMode();
      Serial.print(F("🔄 MODE → Mode "));
      Serial.println(animationMode);
      printModeName();
      break;
      
    case IR_MUTE:
      togglePause();
      Serial.print(F("⏸ MUTE/PAUSE → "));
      Serial.println(animationPaused ? "PAUSE" : "PLAY");
      break;
      
    // ===== LINE 2 - PLAYBACK CONTROLS =====
    case IR_PREV:
      adjustSpeed(false);
      Serial.print(F("◄◄ SLOW → Speed: "));
      Serial.println(animationSpeed);
      break;
      
    case IR_PLAY_PAUSE:
      togglePause();
      Serial.print(F("▶|| PLAY/PAUSE → "));
      Serial.println(animationPaused ? "PAUSE" : "PLAY");
      break;
      
    case IR_NEXT:
      adjustSpeed(true);
      Serial.print(F("►► FAST → Speed: "));
      Serial.println(animationSpeed);
      break;
      
    // ===== LINE 3 - VOLUME/BRIGHTNESS =====
    case IR_VOL_MINUS:
      adjustBrightness(false);
      Serial.print(F("─ DIM → Brightness: "));
      Serial.println(currentBrightness);
      break;
      
    case IR_VOL_PLUS:
      adjustBrightness(true);
      Serial.print(F("+ BRIGHT → Brightness: "));
      Serial.println(currentBrightness);
      break;
      
    case IR_EQ:
      setRainbowMode();
      Serial.println(F("🎵 EQ → Rainbow"));
      break;

    // ===== LINE 4 - SPECIAL FUNCTIONS =====
    case IR_ZERO:
      setStaticColor(CRGB::White);
      Serial.println(F("0 → ⚪ WHITE"));
      break;
      
    case IR_100_PLUS:
      setFireMode();
      Serial.println(F("100+ → 🔥 FIRE"));
      break;

    case IR_200_PLUS:
      resetToDefault();
      Serial.println(F("200+ → 🔄 RESET"));
      break;

    // ===== LINE 5 - PRIMARY COLORS =====
    case IR_ONE:
      setStaticColor(CRGB::Red);
      Serial.println(F("1 → 🔴 RED"));
      break;
      
    case IR_TWO:
      setStaticColor(CRGB::Green);
      Serial.println(F("2 → 🟢 GREEN"));
      break;

    case IR_THREE:
      setStaticColor(CRGB::Blue);
      Serial.println(F("3 → 🔵 BLUE"));
      break;

    // ===== LINE 6 - SECONDARY COLORS =====
    case IR_FOUR:
      setStaticColor(CRGB::Yellow);
      Serial.println(F("4 → 🟡 YELLOW"));
      break;
      
    case IR_FIVE:
      setStaticColor(CRGB::Cyan);
      Serial.println(F("5 → 🩵 CYAN"));
      break;

    case IR_SIX:
      setStaticColor(CRGB::Magenta);
      Serial.println(F("6 → 🩷 MAGENTA"));
      break;

    // ===== LINE 7 - TERTIARY COLORS =====
    case IR_SEVEN:
      setStaticColor(CRGB::Orange);
      Serial.println(F("7 → 🟠 ORANGE"));
      break;
      
    case IR_EIGHT:
      setStaticColor(CRGB::Purple);
      Serial.println(F("8 → 🟣 PURPLE"));
      break;

    case IR_NINE:
      setStaticColor(CRGB::Pink);
      Serial.println(F("9 → 🩷 PINK"));
      break;

    default:
      Serial.println(F("❓ Unknown"));
      break;
  }
}

// ===== CONTROL FUNCTIONS =====
void togglePower() {
  powerOn = !powerOn;
  if (powerOn) {
    FastLED.setBrightness(currentBrightness);
    if (animationMode == 0) {
      fill_solid(leds, NUM_LEDS, currentColor);
    }
    FastLED.show();
  } else {
    FastLED.clear();
    FastLED.show();
  }
}

void adjustBrightness(bool increase) {
  if (increase) {
    currentBrightness = min(255, currentBrightness + 25);
  } else {
    currentBrightness = max(10, currentBrightness - 25);
  }
  FastLED.setBrightness(currentBrightness);
  FastLED.show();
}

void changeMode() {
  animationMode = (animationMode + 1) % 11;
  animationStep = 0;
  animationPaused = false;
}

void togglePause() {
  animationPaused = !animationPaused;
}

void adjustSpeed(bool increase) {
  if (increase) {
    animationSpeed = max(10, animationSpeed - 10); // Faster
  } else {
    animationSpeed = min(150, animationSpeed + 10); // Slower
  }
}

void setStaticColor(CRGB color) {
  currentColor = color;
  animationMode = 0;
  animationPaused = false;
  fill_solid(leds, NUM_LEDS, color);
  FastLED.show();
}

void setRainbowMode() {
  animationMode = 5; // Rainbow Cycle
  animationPaused = false;
}

void setFireMode() {
  animationMode = 9; // Fire Effect
  animationPaused = false;
}

void resetToDefault() {
  currentBrightness = BRIGHTNESS;
  animationMode = 0;
  animationSpeed = 50;
  animationPaused = false;
  currentColor = CRGB::White;
  
  FastLED.setBrightness(currentBrightness);
  fill_solid(leds, NUM_LEDS, currentColor);
  FastLED.show();
  
  Serial.println(F("  Settings restored"));
}

void printModeName() {
  Serial.print(F("  "));
  switch(animationMode) {
    case 0: Serial.println(F("(Static Color)")); break;
    case 1: Serial.println(F("(Flash)")); break;
    case 2: Serial.println(F("(Strobe)")); break;
    case 3: Serial.println(F("(Fade)")); break;
    case 4: Serial.println(F("(Smooth)")); break;
    case 5: Serial.println(F("(Rainbow Cycle)")); break;
    case 6: Serial.println(F("(Rainbow Wave)")); break;
    case 7: Serial.println(F("(Color Chase)")); break;
    case 8: Serial.println(F("(Theater Chase)")); break;
    case 9: Serial.println(F("(Fire)")); break;
    case 10: Serial.println(F("(Twinkle)")); break;
  }
}

// ===== ANIMATIONS =====
void updateAnimation() {
  static unsigned long lastUpdate = 0;
  unsigned long currentMillis = millis();
  if (currentMillis - lastUpdate < animationSpeed) {
    return;
  }
  lastUpdate = currentMillis;

  switch (animationMode) {
    case 0: break; // Static color
    case 1: flashEffect(); break;
    case 2: strobeEffect(); break;
    case 3: fadeEffect(); break;
    case 4: smoothEffect(); break;
    case 5: rainbowCycle(); break;
    case 6: rainbowWave(); break;
    case 7: colorChase(); break;
    case 8: theaterChase(); break;
    case 9: fireEffect(); break;
    case 10: twinkleEffect(); break;
  }
  
  FastLED.show();
}

void flashEffect() {
  if (animationStep % 2 == 0) {
    fill_solid(leds, NUM_LEDS, currentColor);
  } else {
    FastLED.clear();
  }
  animationStep++;
}

void strobeEffect() {
  static uint8_t strobeCount = 0;
  if (strobeCount < 3) {
    fill_solid(leds, NUM_LEDS, CRGB::White);
    strobeCount++;
  } else if (strobeCount < 6) {
    FastLED.clear();
    strobeCount++;
  } else {
    strobeCount = 0;
  }
}

void fadeEffect() {
  uint8_t brightness = beatsin8(30, 50, 255);
  fill_solid(leds, NUM_LEDS, currentColor);
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].nscale8(brightness);
  }
}

void smoothEffect() {
  fill_solid(leds, NUM_LEDS, CHSV(hue, 255, 255));
  hue += 2;
}

void rainbowCycle() {
  fill_rainbow(leds, NUM_LEDS, hue, 255 / NUM_LEDS);
  hue += 3;
}

void rainbowWave() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV((hue + (i * 256 / NUM_LEDS)) % 256, 255, 255);
  }
  hue += 2;
}

void colorChase() {
  fadeToBlackBy(leds, NUM_LEDS, 64);
  leds[animationStep % NUM_LEDS] = CHSV(hue, 255, 255);
  animationStep++;
  hue += 4;
}

void theaterChase() {
  for (int i = 0; i < NUM_LEDS; i++) {
    if ((i + animationStep) % 3 == 0) {
      leds[i] = CHSV(hue, 255, 255);
    } else {
      leds[i] = CRGB::Black;
    }
  }
  animationStep++;
  hue += 2;
}

void fireEffect() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(random8(0, 30), 255, random8(100, 255));
  }
}

void twinkleEffect() {
  fadeToBlackBy(leds, NUM_LEDS, 32);
  if (random8() < 80) {
    leds[random16(NUM_LEDS)] = CHSV(hue, 200, 255);
  }
  hue++;
}

void startupAnimation() {
  // Rapid rainbow
  for (int j = 0; j < 256; j += 4) {
    fill_rainbow(leds, NUM_LEDS, j, 20);
    FastLED.show();
    delay(10);
  }
  
  // White flash
  fill_solid(leds, NUM_LEDS, CRGB::White);
  FastLED.show();
  delay(300);
  
  // Turn off
  FastLED.clear();
  FastLED.show();
  delay(200);
  
  Serial.println(F("✓ Startup complete\n"));
}
