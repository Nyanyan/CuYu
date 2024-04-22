const int ports[6] = {A0, A1, A2, A3, A4, A5};
const int speaker = 3;
const float tones[6] = {130.813, 146.832, 164.814, 184.997, 207.652, 233.082};
const int threshold = 700;
volatile unsigned long tim = 0;
const float base_freq = 15625.0;
const int nums[6] = {60, 53, 47, 42, 38, 34}; // base_freq / freq
const unsigned long half_nums[6] = {30, 27, 24, 21, 19, 17}; // base_freq / freq / 2
const unsigned long lcm = 337929060; // lcm(nums)
const int pls = 255 / 6;
bool sound[6];
bool high[6];
int vol;
bool flag;


ISR(TIMER0_COMPA_vect) {
  vol = 0;
  for(int i = 0;i < 6;i++){
    if (sound[i] && tim % nums[i] < half_nums[i]) {
      vol += pls;
    }
  }
  digitalWrite(speaker, vol);
  tim %= lcm;
}

void setupTimer0() {
  noInterrupts();
  // Clear registers
  TCCR0A = 0;
  TCCR0B = 0;
  TCNT0 = 0;

  // 7812.5 Hz (8000000/((15+1)*64))
  OCR0A = 15;
  // CTC
  TCCR0A |= (1 << WGM01);
  // Prescaler 64
  TCCR0B |= (1 << CS01) | (1 << CS00);
  // Output Compare Match A Interrupt Enable
  TIMSK0 |= (1 << OCIE0A);
  interrupts();
}

void setup() {
  for (int i = 0;i < 6;i++) sound[i] = false;
  for (int i = 0;i < 6;i++) high[i] = false;
  for(int i = 0;i < 6;i++) pinMode(ports[i], INPUT);
  pinMode(speaker, OUTPUT);
  //setupTimer0();
  Serial.begin(115200);
}

void loop() {
  flag = false;
  for (int i = 0;i < 6;i++){
    if (analogRead(ports[i]) < threshold) {
      tone(speaker, tones[i]);
      flag = true;
    }
    sound[i] = analogRead(ports[i]) < threshold;
    Serial.print(sound[i]);
    Serial.print('\t');
  }
  if (!flag) noTone(speaker);
  Serial.println("");
}

/*

void setup() {
  for(int i = 0;i < 1;i++) pinMode(ports[i], INPUT);
  pinMode(speaker, OUTPUT);
}

void loop() {
  for (int i = 0;i < 1;i++){
    if (analogRead(ports[i]) < threshold) tone(speaker, tones[i]);
    else noTone(speaker);
  }
}
*/
