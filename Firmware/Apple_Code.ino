
const uint8_t ROW_PINS[8] = {4, 5, 13, 14, 16, 17, 18, 19};
const uint8_t COL_PINS[9] = {21, 22, 23, 25, 26, 27, 32, 33, 34};

uint8_t framebuff[9] = {0};


inline void setPixel(int8_t x, int8_t y, bool on) {
  if (x < 0 || x >= 9 || y < 0 || y >= 8) return;
  if (on) framebuff[x] |= (1 << y);
  else framebuff[x] &= ~(1 << y);
}

void clearFrame() { 
  for (int i = 0; i < 9; i++) framebuff[i] = 0;
}

volatile uint8_t currentCol = 0;
hw_timer_t* refreshTimer = nullptr;

void IRAM_ATTR refreshISR() {
  for (int c = 0; c < 9; c++) digitalWrite(COL_PINS[c], HIGH);

  uint8_t bits = framebuff[currentCol];
  for (int r = 0; r < 8; r++) {
    digitalWrite(ROW_PINS[r], (bits >> r) & 1 ? HIGH : LOW);
  }

  digitalWrite(COL_PINS[currentCol], LOW);

  currentCol = (currentCol + 1) % 9;
}


const int MAX_DROPS = 6;

struct Drop { int8_t x; int8_t y; };
Drop Drops[MAX_DROPS];

void initRain() {
  for (int i = 0; i < MAX_DROPS; i++) Drops[i] = {-1, -1};
}

void stepRain() {
  clearFrame();
  for (int i = 0; i < MAX_DROPS; i++) { 
    if (Drops[i].y < 0) {


      if (random(0,4) == 0) {
        Drops[i].x = random(0,9);
        Drops[i].y = 0;
      }
    } else {
      Drops[i].y++;
      if (Drops[i].y >= 8) Drops[i].y = -1;
    }
    if (Drops[i].y >= 0) setPixel(Drops[i].x, Drops[i].y, true);
  };
};

void setup() {
  for (int r = 0; r < 8; r++) {
    pinMode(ROW_PINS[r], OUTPUT);
    digitalWrite(ROW_PINS[r], LOW);
  }

  for (int c = 0; c < 9; c++) { 
    pinMode(COL_PINS[c], OUTPUT);
    digitalWrite(COL_PINS[c], HIGH);
  }


refreshTimer = timerBegin(0, 80, true);
timerAttachInterrupt(refreshTimer, &refreshISR, true);
timerAlarmWrite(refreshTimer, 1000, true);
timerAlarmEnable(refreshTimer);

randomSeed(esp_random());
initRain();

}


void loop() {
  static uint32_t lastStep = 0;
  const uint32_t STEP_INTERVALS_MS = 140;

  if (millis() - lastStep > STEP_INTERVALS_MS) { 
    lastStep = millis();
    stepRain();
  }
}
