const int ports[6] = {A0, A1, A2, A3, A4, A5};
const int speakers[2] = {12, 13};
const float tones[6] = {130.813, 146.832, 164.814, 184.997, 207.652, 233.082};
const int threshold = 700;
volatile unsigned long tim = 0;
const float base_freq = 5000;
const unsigned long nums[6] = {19, 17, 15, 14, 12, 11}; // base_freq / freq
const unsigned long half_nums[6] = {10, 9, 8, 7, 6, 5}; // base_freq / freq / 2
const unsigned long lcm = 2 * 813960; // lcm(nums)
bool sound[6];
bool high[6];
bool vols[2];
bool flag;


ISR(TIMER0_COMPA_vect) {
  tim += 1;
  tim %= lcm;
  for (int i = 0; i < 2; i++) digitalWrite(speakers[i], vols[i]);
}

void setupTimer0() {
  noInterrupts();
  // Clear registers
  TCCR0A = 0;
  TCCR0B = 0;
  TCNT0 = 0;

  // (8000000/((X+1)*64)) Hz
  OCR0A = 49;
  // CTC
  TCCR0A |= (1 << WGM01);
  // Prescaler 64
  TCCR0B |= (1 << CS01) | (1 << CS00);
  // Output Compare Match A Interrupt Enable
  TIMSK0 |= (1 << OCIE0A);
  interrupts();
}

void setup() {
  for (int i = 0; i < 6; i++) sound[i] = false;
  for (int i = 0; i < 6; i++) high[i] = false;
  for (int i = 0; i < 6; i++) pinMode(ports[i], INPUT);
  for (int i = 0; i < 2; i++)pinMode(speakers[i], OUTPUT);
  setupTimer0();
  //Serial.begin(115200);
}

void loop() {
  for (int i = 0; i < 2; i++) {
    for (int j = i; j < 6; j += 2) {
      sound[j] = analogRead(ports[j]) < threshold;
      //Serial.print(sound[j]);
      //Serial.print('\t');
    }
  }
  for (int i = 0; i < 2; i++) {
    vols[i] = false;
    for (int j = i; j < 6; j += 2) {
      if (sound[j] && (tim % nums[j]) < half_nums[j]) {
        vols[i] = true;
      }
    }
  }
  //Serial.println(tim);
}
