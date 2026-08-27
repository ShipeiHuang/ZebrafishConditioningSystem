#include <LedControl.h>
#include "TM1637.h"

#define HARDWARE_TYPE MD_MAX72XX:: FC16_HW
#define MAX_DEVICES 4

// -------------------- Pins / Hardware --------------------
const int DIN1 = 11;
const int CS1  = 10;
const int CLK1 = 13;

const int DIN2 = 9;
const int CS2  = 6;
const int CLK2 = 7;

const int VoltageCircuit = 2;
const int NumberofLEDPanels = 4;

// TM1637 pins
#define CLK A5
#define DIO A4
TM1637 tm1637(CLK, DIO);

const bool RELAY_ACTIVE_HIGH = true;

// -------------------- LED objects --------------------
LedControl ledPanel1 = LedControl(DIN1, CLK1, CS1, NumberofLEDPanels);
LedControl ledPanel2 = LedControl(DIN2, CLK2, CS2, NumberofLEDPanels);

// -------------------- Patterns --------------------
const byte HalfHalf2[8] = {
  B11001100, B11001100, B00110011, B00110011,
  B11001100, B11001100, B00110011, B00110011
};

// -------------------- Protocol timing --------------------
const unsigned long SEC_MS = 1000UL;
const unsigned long MIN_MS = 60UL * 1000UL;

const unsigned long LOADING_MS  = 1UL   * MIN_MS;
const unsigned long BASELINE_MS = 24UL  * MIN_MS;
const unsigned long TRAINING_MS = 3UL   * MIN_MS;
const unsigned long ANALYSIS_MS = (153UL * MIN_MS) - (3UL * SEC_MS);

const unsigned long TOTAL_MS = LOADING_MS + BASELINE_MS + TRAINING_MS + ANALYSIS_MS;

const unsigned long BASE_PHASE_MS = 8UL * MIN_MS;

const unsigned long SHOCK_ON_MS    = 2000UL;
const unsigned long SHOCK_OFF_MS   = 13000UL;
const unsigned long SHOCK_CYCLE_MS = SHOCK_ON_MS + SHOCK_OFF_MS;
const int TRAINING_CYCLES = 12;

const unsigned long TEST_TOTAL_MS = 3UL  * MIN_MS;
const unsigned long TEST_HALF_MS  = 90UL * SEC_MS;

const unsigned long DARK_NORMAL_MS = 27UL * MIN_MS;
const unsigned long DARK_SHORT_MS  = (27UL * MIN_MS) - (1UL * SEC_MS);

// -------------------- Display / Relay state caching --------------------
enum DisplayMode { MODE_OFF, MODE_P1, MODE_P2, MODE_BOTH };
DisplayMode currentMode = MODE_OFF;
bool currentRelayOn = false;

// -------------------- Timer display --------------------
int8_t TimeDisp[] = {0, 0, 0, 0};

unsigned long protocolStartMs = 0;
unsigned long lastTimerUpdate = 0;

// -------------------- Helpers --------------------
void relayWrite(bool on) {
  if (on == currentRelayOn) return;

  currentRelayOn = on;

  bool pinLevel = RELAY_ACTIVE_HIGH ? on : !on;
  digitalWrite(VoltageCircuit, pinLevel ? HIGH : LOW);
}

void clearPanel(LedControl &p) {
  for (int dev = 0; dev < NumberofLEDPanels; dev++) {
    p.clearDisplay(dev);
  }
}

void drawPatternAllDevices(LedControl &p, const byte pat[8]) {
  for (int dev = 0; dev < NumberofLEDPanels; dev++) {
    for (int row = 0; row < 8; row++) {
      p.setRow(dev, row, pat[row]);
    }
  }
}

void applyDisplayMode(DisplayMode mode) {
  if (mode == currentMode) return;

  currentMode = mode;

  clearPanel(ledPanel1);
  clearPanel(ledPanel2);

  switch (mode) {
    case MODE_OFF:
      break;

    case MODE_P1:
      drawPatternAllDevices(ledPanel1, HalfHalf2);
      break;

    case MODE_P2:
      drawPatternAllDevices(ledPanel2, HalfHalf2);
      break;

    case MODE_BOTH:
      drawPatternAllDevices(ledPanel1, HalfHalf2);
      drawPatternAllDevices(ledPanel2, HalfHalf2);
      break;
  }
}

// -------------------- Timer logic --------------------
void showTime(unsigned int hours, unsigned int minutes) {
  if (hours > 99) {
    hours = 99;
    minutes = 59;
  }

  int8_t h_tens = hours / 10;
  int8_t h_ones = hours % 10;
  int8_t m_tens = minutes / 10;
  int8_t m_ones = minutes % 10;

  tm1637.point(POINT_ON);

  tm1637.display(0, h_tens);
  tm1637.display(1, h_ones);
  tm1637.display(2, m_tens);
  tm1637.display(3, m_ones);
}

void updateCountdown(unsigned long elapsedMs, bool forceUpdate = false) {
  unsigned long nowMs = millis();

  if (!forceUpdate && (nowMs - lastTimerUpdate < 1000UL)) {
    return;
  }

  lastTimerUpdate = nowMs;

  unsigned long remainingMs;

  if (elapsedMs >= TOTAL_MS) {
    remainingMs = 0UL;
  } else {
    remainingMs = TOTAL_MS - elapsedMs;
  }

  unsigned long remainingMin = (remainingMs + 59999UL) / 60000UL;

  unsigned int hours = remainingMin / 60UL;
  unsigned int minutes = remainingMin % 60UL;

  showTime(hours, minutes);
}

// -------------------- Protocol logic --------------------
void runLoadingPeriod() {
  relayWrite(false);
  applyDisplayMode(MODE_OFF);
}

void runBaseline(unsigned long t) {
  relayWrite(false);

  if (t < BASE_PHASE_MS) {
    applyDisplayMode(MODE_P1);
  } else if (t < 2UL * BASE_PHASE_MS) {
    applyDisplayMode(MODE_P2);
  } else {
    applyDisplayMode(MODE_OFF);
  }
}

void runTraining(unsigned long t) {
  unsigned long cycleIndex  = t / SHOCK_CYCLE_MS;
  unsigned long withinCycle = t % SHOCK_CYCLE_MS;

  if ((int)cycleIndex >= TRAINING_CYCLES) {
    relayWrite(false);
    applyDisplayMode(MODE_OFF);
    return;
  }

  if (withinCycle < SHOCK_ON_MS) {
    relayWrite(true);
    applyDisplayMode(MODE_BOTH);
  } else {
    relayWrite(false);
    applyDisplayMode(MODE_OFF);
  }
}

void runAnalysis(unsigned long t) {
  relayWrite(false);

  unsigned long cursor = 0;

  if (t < cursor + TEST_TOTAL_MS) {
    unsigned long withinTest = t - cursor;
    if (withinTest < TEST_HALF_MS) {
      applyDisplayMode(MODE_P1);
    } else {
      applyDisplayMode(MODE_P2);
    }
    return;
  }
  cursor += TEST_TOTAL_MS;

  if (t < cursor + DARK_NORMAL_MS) {
    applyDisplayMode(MODE_OFF);
    return;
  }
  cursor += DARK_NORMAL_MS;

  if (t < cursor + TEST_TOTAL_MS) {
    unsigned long withinTest = t - cursor;
    if (withinTest < TEST_HALF_MS) {
      applyDisplayMode(MODE_P1);
    } else {
      applyDisplayMode(MODE_P2);
    }
    return;
  }
  cursor += TEST_TOTAL_MS;

  if (t < cursor + DARK_NORMAL_MS) {
    applyDisplayMode(MODE_OFF);
    return;
  }
  cursor += DARK_NORMAL_MS;

  if (t < cursor + TEST_TOTAL_MS) {
    unsigned long withinTest = t - cursor;
    if (withinTest < TEST_HALF_MS) {
      applyDisplayMode(MODE_P1);
    } else {
      applyDisplayMode(MODE_P2);
    }
    return;
  }
  cursor += TEST_TOTAL_MS;

  if (t < cursor + DARK_SHORT_MS) {
    applyDisplayMode(MODE_OFF);
    return;
  }
  cursor += DARK_SHORT_MS;

  if (t < cursor + TEST_TOTAL_MS) {
    unsigned long withinTest = t - cursor;
    if (withinTest < TEST_HALF_MS) {
      applyDisplayMode(MODE_P1);
    } else {
      applyDisplayMode(MODE_P2);
    }
    return;
  }
  cursor += TEST_TOTAL_MS;

  if (t < cursor + DARK_SHORT_MS) {
    applyDisplayMode(MODE_OFF);
    return;
  }
  cursor += DARK_SHORT_MS;

  if (t < cursor + TEST_TOTAL_MS) {
    unsigned long withinTest = t - cursor;
    if (withinTest < TEST_HALF_MS) {
      applyDisplayMode(MODE_P1);
    } else {
      applyDisplayMode(MODE_P2);
    }
    return;
  }
  cursor += TEST_TOTAL_MS;

  if (t < cursor + DARK_SHORT_MS) {
    applyDisplayMode(MODE_OFF);
    return;
  }
  cursor += DARK_SHORT_MS;

  if (t < cursor + TEST_TOTAL_MS) {
    unsigned long withinTest = t - cursor;
    if (withinTest < TEST_HALF_MS) {
      applyDisplayMode(MODE_P1);
    } else {
      applyDisplayMode(MODE_P2);
    }
    return;
  }

  applyDisplayMode(MODE_OFF);
}

// -------------------- Setup / Loop --------------------
void setup() {
  pinMode(VoltageCircuit, OUTPUT);
  relayWrite(false);

  tm1637.set();
  tm1637.init();
  tm1637.point(POINT_ON);

  for (int i = 0; i < NumberofLEDPanels; i++) {
    ledPanel1.shutdown(i, false);
    ledPanel1.setIntensity(i, 8);
    ledPanel1.clearDisplay(i);

    ledPanel2.shutdown(i, false);
    ledPanel2.setIntensity(i, 8);
    ledPanel2.clearDisplay(i);
  }

  applyDisplayMode(MODE_BOTH);
  relayWrite(false);
  delay(800);
  applyDisplayMode(MODE_OFF);

  protocolStartMs = millis();
  lastTimerUpdate = 0;

  updateCountdown(0, true);
}

void loop() {
  unsigned long now = millis();
  unsigned long elapsed = now - protocolStartMs;

  updateCountdown(elapsed);

  if (elapsed < LOADING_MS) {
    runLoadingPeriod();

  } else if (elapsed < LOADING_MS + BASELINE_MS) {
    runBaseline(elapsed - LOADING_MS);

  } else if (elapsed < LOADING_MS + BASELINE_MS + TRAINING_MS) {
    runTraining(elapsed - LOADING_MS - BASELINE_MS);

  } else if (elapsed < TOTAL_MS) {
    runAnalysis(elapsed - LOADING_MS - BASELINE_MS - TRAINING_MS);

  } else {
    relayWrite(false);
    applyDisplayMode(MODE_OFF);
    updateCountdown(TOTAL_MS, true);
  }
}