#include <Arduino.h>
#include <TM1637Display.h>

#define CLK 2
#define DIO 3

const bool LEFT_SIDE = true;
const bool RIGHT_SIDE = false;
const uint8_t MAX_COUNT = 99;
const uint64_t TEST_DELAY = 250;
const uint64_t RESET_GAME_SECONDS = 5000;
const uint8_t DISPLAY_POWER_PIN = 13;
const uint8_t WIN_SCORE = 20;
const int32_t LDR_THRESHOLD = 500;

const uint8_t DISPLAY_WIN_LEFT[] = {
  SEG_F | SEG_B | SEG_E | SEG_C | SEG_G,
  SEG_F | SEG_B | SEG_E | SEG_C | SEG_G,
  0x00,
  0x00
};

const uint8_t DISPLAY_WIN_RIGHT[] = {
  0x00,
  0x00,
  SEG_F | SEG_B | SEG_E | SEG_C | SEG_G,
  SEG_F | SEG_B | SEG_E | SEG_C | SEG_G
};

uint8_t countLeft = 0;
uint8_t countRight = 0;
bool coveredLeft = false;
bool coveredRight = false;
uint8_t data[] = { 0xff, 0xff, 0xff, 0xff };
const uint8_t (*winnerDisplay)[4] = NULL;

TM1637Display display(CLK, DIO);

void resetAll() {
  countLeft = 0;
  countRight = 0;

  for(int i = 0; i < 4; ++i) {
    data[i] = display.encodeDigit(0);
  }
  display.setSegments(data);
  delay(TEST_DELAY);
}

// functions to update counter logic
void resetCount(const bool side) {
  uint8_t index = 0;

  if(side == LEFT_SIDE) {
    countLeft = 0;
    index = 0;
  } else {
    countRight = 0;
    index = 2;
  }

  for (int i = index; i <= index + 1; ++i) {
    data[i] = display.encodeDigit(0);
  }

  display.setSegments(data);
  delay(TEST_DELAY);
}

void encodeCurrentCount(const bool side, const uint8_t count) {
  uint8_t index = side ? 0 : 2;
  data[index] = display.encodeDigit((count / 10) % 10);
  data[index + 1] = display.encodeDigit(count % 10);
}

bool updateCount(const bool side) {
  uint8_t& count = side ? countLeft : countRight;
  if(count == MAX_COUNT) {
    resetCount(side);
    return;
  }
  count += 1;
  encodeCurrentCount(side, count);
  display.setSegments(data);
  delay(TEST_DELAY);
}

void resetScore() {
  winnerDisplay = NULL;
  resetAll();
}

void updateScore(const bool side, const int32_t readValue) {
  bool& covered = side ? coveredLeft : coveredRight;
  if (readValue < LDR_THRESHOLD) {
    if (!covered) {
      updateCount(side);
    }
    covered = true;
  } else {
    covered = false;
  }
}

void checkWinnerDecided() {
  if(countLeft == WIN_SCORE) {
    winnerDisplay = &DISPLAY_WIN_LEFT;
  } else if(countRight == WIN_SCORE) {
    winnerDisplay = &DISPLAY_WIN_RIGHT;
  }
  
  if (winnerDisplay) {
    display.setSegments((const uint8_t *)winnerDisplay);
    delay(TEST_DELAY + RESET_GAME_SECONDS);
    resetScore();
  }
}

// intialization function
void initializeDisplay() {
  pinMode(DISPLAY_POWER_PIN, OUTPUT);
  digitalWrite(DISPLAY_POWER_PIN, HIGH);
  delay(1000);

  display.setBrightness(0x0f);
  display.setSegments(data);
  delay(TEST_DELAY);
}

void setup()
{
  // Serial.begin(9600);
  initializeDisplay();
  resetScore();
}

void loop()
{
  checkWinnerDecided();

  int32_t readLeft = analogRead(A0);
  int32_t readRight = analogRead(A1);
  // Serial.print("Right: ");
  // Serial.print(readRight);
  // Serial.print(", Left: ");
  // Serial.println(readLeft);
  updateScore(LEFT_SIDE, readLeft);
  updateScore(RIGHT_SIDE, readRight);
}