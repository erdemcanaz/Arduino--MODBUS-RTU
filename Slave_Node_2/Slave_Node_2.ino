

void setup() {
  Serial.begin(9600);
  
  configure_slave(14);
  configure_register(0, false); //read only register is generated
  configure_register(1, true); //read-write register is generated
  configure_register(2, true); //read
}

unsigned long asd = 0;
void loop() {  
  slave_operate();
  


}
