#define STATE_OFF 0
#define STATE_ACCEL 1
#define STATE_ON 2
#define STATE_DECCEL 3

#define TIMER_POWER_ON_DELAY 0
#define TIMER_ON 1
#define TIMER_OFF 2

const int nSleepPin = 13;
const int buttonPin = 8;
const int pwmPin = 10;

const int cycleTime = 50;  // ms
const int pwmMaxValue = 255;
const int pwmAccelStep = 10;
const int timerPowerOnDelay = 600;
const int timerOnDelay = 100;
const unsigned long timerOffDelay = 288000;

struct pump_state_s {
  int nSleepOut;
  int pwmOut;
  int state;
};

pump_state_s pump_state;

int pinLowCounter = 0;
int start = 0;

void setup() {
  pinMode(nSleepPin, OUTPUT);
  pinMode(buttonPin, INPUT);
  pinMode(pwmPin, OUTPUT);

  pump_state.nSleepOut = LOW;
  pump_state.pwmOut = 0;
  pump_state.state = STATE_OFF;
}

int pumpReadButton(void) {
  int start = 0;
  
  if (digitalRead(buttonPin) == LOW) {
    pinLowCounter++;
  } else {
    pinLowCounter = 0;
    start = 0;
  }

  if (pinLowCounter > 5) {
    pinLowCounter = 0;
    start = 1;
  }

  return start;
}

int pumpTimer(void) {
  static int timerState = TIMER_POWER_ON_DELAY;
  static int powerOnDelayCounter = 0;
  static int onCounter = 0;
  static unsigned long offCounter = 0;
  int start = 0;

  switch (timerState) {
    case TIMER_POWER_ON_DELAY:
      start = 0;
      if (++powerOnDelayCounter >= timerPowerOnDelay) {
        timerState = TIMER_ON;
      }
      break;
    case TIMER_ON:
      start = 1;
      if (++onCounter >= timerOnDelay) {
        onCounter = 0;
        timerState = TIMER_OFF;
      }
      break;
    case TIMER_OFF:
      start = 0;
      if (++offCounter >= timerOffDelay) {
        offCounter = 0;
        timerState = TIMER_ON;
      }
      break;
    default:
      timerState = TIMER_OFF;
  }

  return start;
}

void pumpControl(int start, pump_state_s &state) {
  switch (state.state) {
    case STATE_OFF:
      state.nSleepOut = LOW;
      state.pwmOut = 0;
      if (start == 1) {
        state.nSleepOut = HIGH;
        state.state = STATE_ACCEL;
      }
      break;

    case STATE_ACCEL:
      state.pwmOut += pwmAccelStep;
      if (state.pwmOut >= pwmMaxValue) {
        state.pwmOut = pwmMaxValue;
        state.state = STATE_ON;
      }
      break;

    case STATE_ON:
      if (start == 0) {
        state.state = STATE_DECCEL;
      }
      break;

    case STATE_DECCEL:
      state.pwmOut -= pwmAccelStep;
      if (state.pwmOut <= 0) {
        state.pwmOut = 0;
        state.state = STATE_OFF;
      }
      break;

    default:
      state.state = STATE_OFF;
  }
}

void loop() {
  start = pumpTimer();

  if (start == 0) {
    start = pumpReadButton();
  }

  pumpControl(start, pump_state);

  digitalWrite(nSleepPin, pump_state.nSleepOut);
  analogWrite(pwmPin, pump_state.pwmOut);

  delay(cycleTime);
}