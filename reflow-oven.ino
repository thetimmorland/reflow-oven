/*
 * Important Info:
 * To compile and program with this code you will require the ATTinyCore found at
 * https://github.com/SpenceKonde/ATTinyCore. You will also need to have the board
 * set to ATTiny25/48/85, the chip set to ATTiny85, and the clock to 1MHz internal.
 * 
 * The only thing you should tweak is the first block of define statements.
 * For more info on how these can be adjusted read
 * https://en.wikipedia.org/wiki/Reflow_soldering.
 * 
 * DO NOT set the REFLOWTEMP greater than 235! The oven will shut itself off at this
 * tempature so the chip will never know that the cycle if finsihed.
 */

#define STARTUPTEMP 50 // tempature at which the oven should detect a cycle starting
#define SOAKTEMP 150 // tempature of oven at soak stage
#define SOAKTIME 90000 // time in ms oven should spend at soak stage
#define REFLOWTEMP 220 // max tempature of oven reflow

#define CS PB1
#define SCK PB2
#define RELAY PB3
#define SO PB4

float ovenTemp;
float targetTemp;
long unsigned int soakStartTime;
bool soakStarted;

void readThermocouple();
void updateTargetTemp();
void updateRelayState();

void setup() {
  DDRB |= (1 << CS) | (1 << SCK) | (1 << RELAY);
  PORTB |= (1 << CS);
}

void loop() {
    readThermocouple();
    if (targetTemp != 0 || ovenTemp < STARTUPTEMP) {
      if (ovenTemp > SOAKTEMP && !soakStarted) {
        soakStartTime = millis();
        soakStarted = true;
      }
      updateTargetTemp();
    }
    updateRelayState();
}

void readThermocouple() {
  int16_t thermocoupleData = 0;

  PORTB &= ~(1 << CS);
  delayMicroseconds(0.4);

  for (int8_t n = 13; n > -1; n--) {
    thermocoupleData |= ((PINB & (1 << SO)) >> SO) << n;
    delayMicroseconds(0.05);

    PORTB |= 1 << SCK;
    delayMicroseconds(0.1);

    PORTB &= ~(1 << SCK);
    delayMicroseconds(0.05);
  }

  PORTB |= 1 << CS;

  ovenTemp = thermocoupleData / 4;
}

void updateTargetTemp() {
  if (millis() - soakStartTime < SOAKTIME) targetTemp = SOAKTEMP;
  else if (ovenTemp < REFLOWTEMP) targetTemp = REFLOWTEMP;
  else {
    targetTemp = 0;
    soakStarted = false;
  }
}

void updateRelayState() {
  if (ovenTemp < STARTUPTEMP || ovenTemp < targetTemp) PORTB |= 1 << RELAY;
  else PORTB &= ~(1 << RELAY);
}

