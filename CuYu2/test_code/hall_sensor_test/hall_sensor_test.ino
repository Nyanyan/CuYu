void setup() {
  Serial.begin(115200);
  pinMode(D10, INPUT);
  pinMode(D0, OUTPUT);
  Serial.println("start");

}

void loop() {
  digitalWrite(D0, digitalRead(D10));
  //Serial.println(digitalRead(D10));
  //delay(100);
}
