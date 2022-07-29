#include <SoftwareSerial.h>
#define RX_PIN 2
#define TX_PIN 3
#define OUT_ENABLE_PIN 4
#define SOFTWARE_SERIAL_BAUD_RATE 9600
#define WAIT_RESPONSE_TIME_ms 50
SoftwareSerial mySerial(RX_PIN, TX_PIN);//Rx,Tx



uint8_t B[8];//received bytes buffer

void configure_master() {
  pinMode(OUT_ENABLE_PIN, OUTPUT);
  pinMode(RX_PIN, INPUT); //Probably also configured by SoftwareSerial library.
  pinMode(TX_PIN, OUTPUT);//Probably also configured by SoftwareSerial library.
  mySerial.begin(SOFTWARE_SERIAL_BAUD_RATE);
}

void master_operate() {
  if (Serial.available() >= 6) {
    for (uint8_t i = 0; i < 6; i++)B[i] = Serial.parseInt() ;
    master_write_and_read(B[0], B[1], B[2], B[3], B[4], B[5]);
    while (Serial.available())Serial.read();
  }
}
//SEND AND RECEIVE DATA
void master_write_and_read(uint8_t B_0, uint8_t B_1, uint8_t B_2, uint8_t B_3, uint8_t B_4, uint8_t B_5) {

  uint16_t CRC = generate_CRC_16_bit(6, B_0,  B_1,  B_2,  B_3,  B_4,  B_5);
  uint8_t CRC_LEAST = CRC % 256;
  uint8_t CRC_SIGNIFICANT = CRC >> 8;

  digitalWrite(OUT_ENABLE_PIN, HIGH);
  mySerial.write(B_0);//ID
  mySerial.write(B_1);//FUNC CODE
  mySerial.write(B_2);//REGISTER ADDRESS (SIG)
  mySerial.write(B_3);//REGISTER ADDRESS (LST)
  mySerial.write(B_4);//REGISTER VALUE (SIG)
  mySerial.write(B_5);//REGISTER VALUE (LST)
  mySerial.write(CRC_LEAST); //CRC (LST)
  mySerial.write(CRC_SIGNIFICANT); //CRC (SIG)
  digitalWrite(OUT_ENABLE_PIN, LOW);

  for (int i = 0; i < 100; i++) {
    if (mySerial.available() >= 1) {
      delay(WAIT_RESPONSE_TIME_ms);
      break;
    }
    delay(WAIT_RESPONSE_TIME_ms);
  }

  uint8_t number_of_bytes_received = mySerial.available();
  if (number_of_bytes_received == 5) { //Exception

    //ID, EXCEPTION RESPONSE FUNCTION CODE, EXCEPTION CODE, CRC (LST), CRC (SIG)
    for (int i = 0; i < 5; i++)B[i] = mySerial.read();

    uint16_t received_CRC = (((uint16_t)B[4]) << 8) + B[3];
    uint16_t expected_CRC = generate_CRC_16_bit(3, B[0], B[1], B[2], 0, 0, 0);

    if (received_CRC !=  expected_CRC ) {//CRC CHECK
      //return;
    } else if (B_0 == B[0]) {//ID check
      Serial.println(String(B[0]) + "," + String(B[1]) + "," + String(B[2]));
    } else {
      //return;
    }
  }
  else if (number_of_bytes_received == 7) { //Reply for function code 3 with single register requested
    //0-ID, 1-FUNCTION CODE, 2-BYTE COUNT, 3-REGISTER VALUE (SIG), 4-REGISTER VALUE (LST), 5-CRC (LST), 6-CRC(SIG)
    for (int i = 0; i < 7; i++)B[i] = mySerial.read();

    uint16_t received_CRC = (((uint16_t)B[6]) << 8) + B[5];
    uint16_t expected_CRC = generate_CRC_16_bit(5, B[0], B[1], B[2], B[3], B[4], 0);

    if (received_CRC !=  expected_CRC ) {//CRC CHECK
      //return;
    } else if (B_0 == B[0]) {//ID check
      Serial.println(String(B[0]) + "," + String(B[1]) + "," + String(B[2]) + "," + String(B[3]) + "," + String(B[4]));
    } else {
      //return;
    }
    while (mySerial.available())mySerial.read();
  }
  else if (number_of_bytes_received == 8) {//Reply for function code 6
    //0-ID, 1-FUNCTION CODE, 2-REGISTER ADDRESS(SIG), 3-REGISTER ADDRESS (LST), 4-REGISTER VALUE (SIG), 5-REGISTER VALUE (LST), 6-CRC (LST), 7-CRC(SIG)
    for (int i = 0; i < 8; i++)B[i] = mySerial.read();

    uint16_t received_CRC = (((uint16_t)B[7]) << 8) + B[6];
    uint16_t expected_CRC = generate_CRC_16_bit_for_6BYTE(B[0], B[1], B[2], B[3], B[4], B[5]);

    if (received_CRC !=  expected_CRC ) {//CRC CHECK
      //return;
    } else if (B_0 == B[0]) {//ID check
      Serial.println(String(B[0]) + "," + String(B[1]) + "," + String(B[2]) + "," + String(B[3]) + "," + String(B[4]) + "," + String(B[5]));
    } else {
      //return;
    }
  } else {
    //return;
  }
  while (mySerial.available())mySerial.read();
}


//MAGICAL CRC_16 MODBUS code.
uint16_t generate_CRC_16_bit(uint8_t number_of_bytes, uint8_t B_0, uint8_t B_1, uint8_t B_2, uint8_t B_3, uint8_t B_4, uint8_t B_5) {
  uint16_t remainder = CRC_16_bit_for_1BYTE(B_0, 65535);
  if (number_of_bytes >= 2 )  remainder = CRC_16_bit_for_1BYTE(B_1, remainder);
  if (number_of_bytes >= 3 )  remainder = CRC_16_bit_for_1BYTE(B_2, remainder);
  if (number_of_bytes >= 4 )  remainder = CRC_16_bit_for_1BYTE(B_3, remainder);
  if (number_of_bytes >= 5 )  remainder = CRC_16_bit_for_1BYTE(B_4, remainder);
  if (number_of_bytes >= 6 )  remainder = CRC_16_bit_for_1BYTE(B_5, remainder);
  return remainder;

}

uint16_t generate_CRC_16_bit_for_6BYTE(uint8_t B_0, uint8_t B_1, uint8_t B_2, uint8_t B_3, uint8_t B_4, uint8_t B_5) {
  uint16_t remainder = CRC_16_bit_for_1BYTE(B_0, 65535);
  remainder = CRC_16_bit_for_1BYTE(B_1, remainder);
  remainder = CRC_16_bit_for_1BYTE(B_2, remainder);
  remainder = CRC_16_bit_for_1BYTE(B_3, remainder);
  remainder = CRC_16_bit_for_1BYTE(B_4, remainder);
  remainder = CRC_16_bit_for_1BYTE(B_5, remainder);
  return remainder;

}
uint16_t generate_CRC_16_bit_for_5BYTE(uint8_t B_0, uint8_t B_1, uint8_t B_2, uint8_t B_3, uint8_t B_4) {
  uint16_t remainder = CRC_16_bit_for_1BYTE(B_0, 65535);
  remainder = CRC_16_bit_for_1BYTE(B_1, remainder);
  remainder = CRC_16_bit_for_1BYTE(B_2, remainder);
  remainder = CRC_16_bit_for_1BYTE(B_3, remainder);
  remainder = CRC_16_bit_for_1BYTE(B_4, remainder);
  return remainder;

}
uint16_t generate_CRC_16_bit_for_3BYTE(uint8_t B_0, uint8_t B_1, uint8_t B_2) {
  uint16_t remainder = CRC_16_bit_for_1BYTE(B_0, 65535);
  remainder = CRC_16_bit_for_1BYTE(B_1, remainder);
  remainder = CRC_16_bit_for_1BYTE(B_2, remainder);
  return remainder;
}
uint16_t CRC_16_bit_for_1BYTE(uint16_t data, uint16_t last_data) {
  //if this is first data (i.e LAST_DATA==null), LAST_DATA= 65535 = FFFF
  uint16_t key = 40961; //1010 0000 0000 0001
  data = data ^ last_data;//XOR
  for (int i = 0; i < 8; i++) {
    boolean should_XOR = false;
    if (data % 2 == 1)should_XOR = true;
    data = data >> 1;
    if (should_XOR)data = data ^ key;
  }
  return data;
}
