

void setup() {
  Serial.begin(9600);
  configure_slave();
  pinMode(13, OUTPUT);

}

unsigned long tic = 0;

void loop() {
  slave_operate();

  uint8_t state= get_reg(5);
  if(state)digitalWrite(13,HIGH);
  else digitalWrite(13,LOW);
  
 
}
