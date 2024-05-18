void setup() {
  Serial.begin(115200);
  pinMode(D0, INPUT);
  Serial.println("start");

}

void loop() {
  //digitalWrite(D0, digitalRead(D10));
  Serial.println(digitalRead(D0));
  //delay(100);
}
