#include <SoftwareSerial.h>
#define RX_PIN 10
#define TX_PIN 11

SoftwareSerial mySerial = SoftwareSerial(RX_PIN, TX_PIN);
void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);

}


uint8_t num = 0;
int counter = 0;
void loop() {

  
  num = random(255);
  Serial.println("Mesaj gönderdim: "+String(num));
  mySerial.write(14);
  mySerial.write(6);
  mySerial.write((uint8_t)0);
  mySerial.write(1);
  mySerial.write((uint8_t)0);
  mySerial.write(num);
  delay(500);
  if_available_read_Lora_Buffer();





}


void if_available_read_Lora_Buffer() {
  if (mySerial.available() > 0) {
    delay(10);
    Serial.println("Aldığım mesaj");
    while (mySerial.available()) {
      uint8_t c = mySerial.read();
      //mySerial.write(c);
      Serial.println((uint8_t ) c);
    }
    Serial.println();
  }

}
