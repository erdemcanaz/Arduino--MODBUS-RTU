

void setup() {
  Serial.begin(9600);
}

void loop() {

  
  if(Serial.available()){
    delay(350);
    while(Serial.available()) Serial.write( (uint8_t) Serial.read());
  }
  
 
  



 
}
