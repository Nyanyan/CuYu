const int ports[6] = {A0, A1, A2, A3, A4, A5};
const int speaker = 3;
const float tones[6] = {130.813, 146.832, 164.814, 184.997, 207.652, 233.082};
const int threshold = 700;
bool flag;

void setup() {
  for(int i = 0;i < 6;i++) pinMode(ports[i], INPUT);
  pinMode(speaker, OUTPUT);
}

void loop() {
  flag = false;
  for (int i = 0;i < 6;i++){
    if (analogRead(ports[i]) < threshold) {
      tone(speaker, tones[i]);
      flag = true;
    }
  }
  if (!flag) noTone(speaker);
}
