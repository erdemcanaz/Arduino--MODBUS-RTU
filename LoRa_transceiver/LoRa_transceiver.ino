#include <SoftwareSerial.h>
#define RX_PIN 10
#define TX_PIN 11

SoftwareSerial mySerial = SoftwareSerial(RX_PIN, TX_PIN);
void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);

}

void loop() {  
  for(int i=0;i<5;i++){
    
  }  
  delay(1000);

}

void broad_byte(uint8_t i){
  mySerial.write(i);
}
void if_available_read_Lora_Buffer(){
  if (mySerial.available() > 0) {
    delay(10);
    while (mySerial.available()) {
      uint8_t c = mySerial.read();
      Serial.println((uint8_t ) c);     
    }
    Serial.println();
  }
  
}
