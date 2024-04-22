const int ports[6] = {A0, A1, A3, A2, A4, A5};
const int speakers[2] = {6, 8};
//const float tones[6] = {130.813, 146.832, 164.814, 184.997, 207.652, 233.082};
const float tones[6] = {130.813, 146.832, 184.997, 164.814, 207.652, 233.082};
const int threshold = 700;
volatile unsigned long tim = 0;
const unsigned long nums[6] = {224, 212, 189, 168, 149, 133};
//const unsigned long half_nums[6] = {1, 1, 1, 1, 1, 1}; // base_freq / freq / 2
//const unsigned long lcm = 4; // lcm(nums)
//bool sound[6];
//bool vol;
//bool flag;
//int idx;
unsigned long inf = 10000000000;
unsigned long tmp1, tmp2;

// freq = 8000000 / (X + 1) / 8 * 25

ISR(TIM0_COMPA_vect) {
  /*
    for (int i = 0; i < 2; i++) {
      vol = false;
      for (int j = 0; j < 3; j++) {
        vol |= ((tim % nums[i * 3 + j]) < half_nums[i * 3 + j]); //sound[i * 3 + j] &&
      }
      digitalWrite(speakers[i], vol);
    }
    tim += 1;
    tim %= lcm;
  */
  //digitalWrite(speakers[0], !digitalRead(speakers[0]));
}

ISR(TIM1_COMPA_vect) {
  /*
    for (int i = 0; i < 2; i++) {
      vol = false;
      for (int j = 0; j < 3; j++) {
        vol |= ((tim % nums[i * 3 + j]) < half_nums[i * 3 + j]); //sound[i * 3 + j] &&
      }
      digitalWrite(speakers[i], vol);
    }
    tim += 1;
    tim %= lcm;
  */
  //digitalWrite(speakers[1], !digitalRead(speakers[1]));
}

void setupTimer() {
  /*
    TCCR0A = 0b01000011;
    TCCR0B = 0b00001001;
    TIMSK0 = 0b00000010;
    OCR0A = 24;
  */
  /*
    TCCR1A = 0b01000011;
    TCCR1B = 0b00011001;
    TIMSK1 = 0b00000010;
    OCR1A = 48;
  */
  TCCR0A = _BV(COM0A0) | _BV(WGM01) | _BV(WGM00); // COM : Toggle OC1A on Match
  TCCR0B = _BV(CS01) | _BV(WGM02);   // CS  : no prescale
  //OCR0A = nums[0];
  TCCR1A = _BV(COM1A0) | _BV(WGM11) | _BV(WGM10); // COM : Toggle OC1A on Match
  TCCR1B = _BV(CS11) | _BV(WGM12) | _BV(WGM13);   // CS  : no prescale
  //OCR1A = nums[5];
  sei();
}

void setup() {
  //for (int i = 0; i < 6; i++) sound[i] = true;
  for (int i = 0; i < 6; i++) pinMode(ports[i], INPUT);
  for (int i = 0; i < 2; i++) pinMode(speakers[i], OUTPUT);
  for (int i = 0; i < 2; i++) digitalWrite(speakers[i], LOW);
  setupTimer();
}

void loop() {
  tmp1 = inf;
  for (int i = 0; i < 3; i++) {
    if (analogRead(ports[i]) < threshold){
      tmp1 = nums[i];
    }
  }
  tmp2 = inf;
  for (int i = 3; i < 6; i++) {
    if (analogRead(ports[i]) < threshold){
      tmp2 = nums[i];
    }
  }
  OCR0A = tmp1;
  OCR1A = tmp2;
}
