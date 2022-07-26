#include <SoftwareSerial.h>
#define RX_PIN 2
#define TX_PIN 3
#define OUT_ENABLE_PIN 4
#define SOFTWARE_SERIAL_BAUD_RATE 9600
#define WAIT_RESPONSE_TIME_ms 250
SoftwareSerial mySerial(RX_PIN, TX_PIN);//Rx,Tx

struct dataPackage {
  uint8_t B_0 = 0;
  uint8_t B_1 = 0;
  uint8_t B_2 = 0;
  uint8_t B_3 = 0;
  uint8_t B_4 = 0;
  uint8_t B_5 = 0;
  uint8_t CRC_LEAST = 0;
  uint8_t CRC_SIGNIFICANT = 0;
};

dataPackage dataPackageSent;
dataPackage dataPackageReceived;

void configure_master() {
  pinMode(OUT_ENABLE_PIN, OUTPUT);
  mySerial.begin(SOFTWARE_SERIAL_BAUD_RATE);
}


//SEND AND RECEIVE DATA
void master_write(uint8_t B_0, uint8_t B_1, uint8_t B_2, uint8_t B_3, uint8_t B_4, uint8_t B_5) {
  //send data.
  uint16_t CRC = generate_CRC_16_bit_for_6BYTE(B_0, B_1, B_2, B_3, B_4, B_5);
  uint8_t CRC_LEAST = CRC % 256;
  uint8_t CRC_SIGNIFICANT = CRC >> 8;

  dataPackageSent.B_0 = B_0;
  dataPackageSent.B_1 = B_1;
  dataPackageSent.B_2 = B_2;
  dataPackageSent.B_3 = B_3;
  dataPackageSent.B_4 = B_4;
  dataPackageSent.B_5 = B_5;
  dataPackageSent.CRC_LEAST = CRC_LEAST;
  dataPackageSent.CRC_SIGNIFICANT = CRC_SIGNIFICANT;

  digitalWrite(OUT_ENABLE_PIN, HIGH);
  mySerial.write(B_0);//ID
  mySerial.write(B_1);//FUNC CODE
  mySerial.write(B_2);//REGISTER ADDRESS
  mySerial.write(B_3);//REGISTER ADDRESS
  mySerial.write(B_4);//REGISTER VALUE
  mySerial.write(B_5);//REGISTER VALUE
  mySerial.write(CRC % 256); //CRC
  mySerial.write(CRC >> 8); //CRC
  digitalWrite(OUT_ENABLE_PIN, LOW);

  delay(WAIT_RESPONSE_TIME_ms);
  
  dataPackageReceived.B_0 = mySerial.read();
  dataPackageReceived.B_1 = mySerial.read();
  dataPackageReceived.B_2 = mySerial.read();
  dataPackageReceived.B_3 = mySerial.read();
  dataPackageReceived.B_4 = mySerial.read();
  dataPackageReceived.B_5 = mySerial.read();
  dataPackageReceived.CRC_LEAST = mySerial.read();
  dataPackageReceived.CRC_SIGNIFICANT = mySerial.read();

  while (mySerial.available())Serial.println(mySerial.read());

  boolean condition_for_succes1 = (dataPackageReceived.B_1 == 3 || dataPackageReceived.B_1 == 4 || dataPackageReceived.B_1 == 6) ;
  boolean condition_for_succes2 = dataPackageReceived.CRC_LEAST == dataPackageSent.CRC_LEAST && dataPackageReceived.CRC_SIGNIFICANT == dataPackageSent.CRC_SIGNIFICANT  ;
  if (condition_for_succes1 && condition_for_succes2) {
    Serial.println("Succes");
  } else {
    Serial.println("Error");
  }


}


//MAGICAL CRC_16 MODBUS code.
uint16_t generate_CRC_16_bit_for_6BYTE(uint8_t B_0, uint8_t B_1, uint8_t B_2, uint8_t B_3, uint8_t B_4, uint8_t B_5) {
  uint16_t remainder = CRC_16_bit_for_1BYTE(B_0, 65535);
  remainder = CRC_16_bit_for_1BYTE(B_1, remainder);
  remainder = CRC_16_bit_for_1BYTE(B_2, remainder);
  remainder = CRC_16_bit_for_1BYTE(B_3, remainder);
  remainder = CRC_16_bit_for_1BYTE(B_4, remainder);
  remainder = CRC_16_bit_for_1BYTE(B_5, remainder);
  return remainder;
  //Serial.print(remainder % 256);
  //Serial.print(" ");
  // Serial.println(remainder >> 8);
}
uint16_t generate_CRC_16_bit_for_3BYTE(uint8_t B_0, uint8_t B_1, uint8_t B_2) {
  uint16_t remainder = CRC_16_bit_for_1BYTE(B_0, 65535);
  remainder = CRC_16_bit_for_1BYTE(B_1, remainder);
  remainder = CRC_16_bit_for_1BYTE(B_2, remainder);
  return remainder;
  //Serial.print(remainder % 256);
  //Serial.print(" ");
  //Serial.println(remainder >> 8);
}
uint16_t CRC_16_bit_for_1BYTE(uint16_t data, uint16_t last_data) {
  //if this is first data (i.e LAST_DATA==null), LAST_DATA= 65535 = FFFF
  uint16_t key = 40961; //1010 0000 0000 0001
  data = data ^ last_data;
  for (int i = 0; i < 8; i++) {
    boolean should_XOR = false;
    if (data % 2 == 1)should_XOR = true;
    data = data >> 1;
    if (should_XOR)data = data ^ key;
  }
  return data;
}
