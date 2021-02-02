const int ports[1] = {A0};
const int speaker = A2;
const float tones[6] = {261.626, 293.665, 329.628, 369.994, 415.305, 466.164};

void setup() {
  for(int i = 0;i < 1;i++) pinMode(ports[i], INPUT);
  pinMode(speaker, OUTPUT);
}

void loop() {
  for (int i = 0;i < 1;i++){
    if (analogRead(ports[i]) < threshold) tone(speaker, tones[i]);
    else noTone();
  }
}
