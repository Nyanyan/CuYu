#include<avr/io.h>
#include<avr/wdt.h>
#include <avr/interrupt.h>

ISR(TIM0_COMPA_vect) { //タイマ割り込み
  digitalWrite(6, !digitalRead(6));
}

void setup() {
  pinMode(6, OUTPUT);
  DDRB     = 0b00000001;//PORTB出力設定
  PORTB    = 0b00000000;//PORTB_LOW
  TCCR0A   = 0b00000010;//CTC
  TCCR0B   = 0b00000101;//1024分周
  OCR0A    = 117;//0.0998sec
  TIMSK0  |= 0b00000100;//比較A割り込み
  sei();
}

void loop() {
  wdt_reset();
}
