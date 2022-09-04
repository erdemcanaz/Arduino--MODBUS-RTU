

void setup() {
  Serial.begin(9600);  
  configure_slave(14);  
  pinMode(13,OUTPUT);
}

void loop() {  
  slave_operate();
  set_input_register(0,random(255));
  int state = get_holding_register(0);
  if(state)digitalWrite(13,HIGH);
  else digitalWrite(13,LOW);
}
