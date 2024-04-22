#include <avr/sleep.h>
#include <avr/interrupt.h>
const int ports[6] = {A0, A1, A2, A3, A4, A5};
const int speakers[2] = {6, 8};
//const float tones[6] = {554.366, 587.33, 739.988, 659.256, 830.61, 932.328};
const int threshold = 700;
const unsigned long nums[6] = {112, 105, 83, 94, 74, 66};
unsigned long inf = 10000000;
unsigned long tmp1, tmp2;
unsigned long sleep_cnt = 0;

void setupTimer(unsigned long tim1, unsigned long tim2) {
  cli();
  if (tim1 < inf) {
    TCCR0A = _BV(COM0A0) | _BV(WGM01) | _BV(WGM00);
    TCCR0B = _BV(CS01) | _BV(WGM02);
    OCR0A = tim1;
  } else {
    TCCR0A = 0b00000000;
    TCCR0B = 0b00000000;
  }
  if (tim2 < inf) {
    TCCR1A = _BV(COM1A0) | _BV(WGM11) | _BV(WGM10);
    TCCR1B = _BV(CS11) | _BV(WGM12) | _BV(WGM13);
    OCR1A = tim2;
  } else {
    TCCR1A = 0b00000000;
    TCCR1B = 0b00000000;
  }
  sei();
}

void wakeup() {
  ;
}

void slp() {
  cli();
  TCCR0A = 0b00000000;
  TCCR0B = 0b00000000;
  TCCR1A = 0b00000000;
  TCCR1B = 0b00000000;
  sleep_cnt = 0;
  for (int i = 0; i < 2; i++) digitalWrite(speakers[i], LOW);
  delay(1000);
  //sei();
  //attachInterrupt(0, wakeup, LOW);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  ADCSRA &= ~(1 << ADEN);
  MCUCR |= (1 << BODS) | (1 << BODSE);
  MCUCR = (MCUCR & ~(1 << BODSE)) | (1 << BODS);
  sleep_cpu();

  sleep_disable();
  ADCSRA |= (1 << ADEN);
  //detachInterrupt(0);
}

void setup() {
  for (int i = 0; i < 6; i++) pinMode(ports[i], INPUT);
  for (int i = 0; i < 2; i++) pinMode(speakers[i], OUTPUT);
  //setupTimer();
}

void loop() {
  tmp1 = inf;
  for (int i = 0; i < 3; i++) {
    if (analogRead(ports[i]) < threshold) {
      tmp1 = nums[i];
    }
  }
  tmp2 = inf;
  for (int i = 3; i < 6; i++) {
    if (analogRead(ports[i]) < threshold) {
      tmp2 = nums[i];
    }
  }
  setupTimer(tmp1, tmp2);
  if (tmp1 == inf && tmp2 == inf) sleep_cnt++;
  else sleep_cnt = 0;
  if (sleep_cnt > 7500) slp(); // shutdown in 1 minute
}
