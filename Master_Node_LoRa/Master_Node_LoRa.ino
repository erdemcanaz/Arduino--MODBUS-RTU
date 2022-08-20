
void setup() {
  Serial.begin(9600);
  configure_master();
}

void loop() {
  master_operate();
}
