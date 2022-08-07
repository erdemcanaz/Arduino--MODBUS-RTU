#include <SoftwareSerial.h>
#define RX_PIN 10
#define TX_PIN 11

SoftwareSerial mySerial = SoftwareSerial(RX_PIN, TX_PIN);
void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);

}


uint8_t num = 0;
void loop() {  
 if_available_read_Lora_Buffer();

  
}


void if_available_read_Lora_Buffer(){
  if (mySerial.available() > 0) {
    Serial.println("Aldığım mesaj");
    while (mySerial.available()) {     
      uint8_t c = mySerial.read();
      //mySerial.write(c);
      //Serial.println(String((char) c) +" " +String((uint8_t ) c));     
      Serial.println((uint8_t ) c); 
    }
    Serial.println();
  }
  
}
