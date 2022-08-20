
void setup() {
  Serial.begin(9600);
  configure_slave();
}

void loop() {
  listen_RS485();
  operate();
  
}
