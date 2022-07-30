

void setup() {
  Serial.begin(9600);
  configure_slave(14);
  pinMode(13, OUTPUT);

}

void loop() {
  slave_operate();

  uint8_t state= get_reg(5);
  
  if(state)digitalWrite(13,HIGH);
  else digitalWrite(13,LOW);
  
 
}
