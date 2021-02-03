void setup() {
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  //Serial.begin(115200);
}

int i = 0;
void loop() {
  //Serial.println(i);
  if (i % 3 == 0){
    if ((i / 3) % 2)digitalWrite(12, HIGH);
    else digitalWrite(12, LOW);
  }
  if (i % 5 == 0){
    if ((i / 5) % 2)digitalWrite(13, HIGH);
    else digitalWrite(13, LOW);
  }
  i += 1;
  i %= 30;
  delay(1);
}
