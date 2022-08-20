#define DEBUG true

#include <SoftwareSerial.h>
#define SOFTWARE_RX_PIN 2
#define SOFTWARE_TX_PIN 3
#define OUT_ENABLE_PIN 4
#define SOFTWARE_SERIAL_BAUD_RATE 9600
#define WAIT_RESPONSE_TIMEOUT_ms  1000
#define WAIT_RESPONSE_TIME_ms  10
SoftwareSerial softwareSerial(SOFTWARE_RX_PIN, SOFTWARE_TX_PIN);//Rx,Tx

void configure_master() {
  pinMode(OUT_ENABLE_PIN, OUTPUT);
  digitalWrite(OUT_ENABLE_PIN, LOW);
  pinMode(SOFTWARE_RX_PIN, INPUT); //Probably, also, configured by SoftwareSerial library.
  pinMode(SOFTWARE_TX_PIN, OUTPUT);//Probably, also, configured by SoftwareSerial library.
  softwareSerial.begin(SOFTWARE_SERIAL_BAUD_RATE);
}

void master_operate() {
  while (softwareSerial.available())softwareSerial.read();

  if (Serial.available() < 6)return;

  uint8_t B[8];
  for (uint8_t i = 0 ; i < 6 ; i++) B[i] = Serial.parseInt();

  uint16_t CRC = generate_CRC_16_bit(6, B[0],  B[1],  B[2],  B[3],  B[4],  B[5]);
  uint8_t CRC_LEAST = CRC % 256;
  uint8_t CRC_SIGNIFICANT = CRC >> 8;

  digitalWrite(OUT_ENABLE_PIN, HIGH);
  for (uint8_t i = 0 ; i < 6; i++)softwareSerial.write(B[i]);
  softwareSerial.write(CRC_LEAST);
  softwareSerial.write(CRC_SIGNIFICANT);
  digitalWrite(OUT_ENABLE_PIN, LOW);

  delay(1);
  while (softwareSerial.available())softwareSerial.read();

  unsigned long tic = millis();
  boolean isTimedOut = true;
  while ((millis() - tic) < WAIT_RESPONSE_TIMEOUT_ms) {
    if (softwareSerial.available() > 0) {
      isTimedOut = false;
      delay(WAIT_RESPONSE_TIME_ms);
      break;
    }
  }
  if (isTimedOut) {
    while (Serial.available())Serial.read();
    return;
  }


  uint8_t number_of_bytes_received = softwareSerial.available();
  for (int i = 0; i < number_of_bytes_received; i++)B[i] = softwareSerial.read();
  
  //Exception-response
  if (number_of_bytes_received == 5) {     
    uint16_t received_CRC = (((uint16_t)B[4]) << 8) + B[3];
    uint16_t expected_CRC = generate_CRC_16_bit(3, B[0], B[1], B[2], 0, 0, 0);
    if (received_CRC ==  expected_CRC ) {
      Serial.println(String(B[0]) + "," + String(B[1]) + "," + String(B[2]) + "," + String(B[3]) + "," + String(B[4]) );
    }    
  }
  //Read-response
  else if (number_of_bytes_received == 7) {
    uint16_t received_CRC = (((uint16_t)B[6]) << 8) + B[5];
    uint16_t expected_CRC = generate_CRC_16_bit(5, B[0], B[1], B[2], B[3], B[4], 0);
    if (received_CRC ==  expected_CRC ) {
      Serial.println(String(B[0]) + "," + String(B[1]) + "," + String(B[2]) + "," + String(B[3]) + "," + String(B[4]) + "," + String(B[5]) + "," + String(B[6]) );
    }
  }
  //Write-response
   else if (number_of_bytes_received == 8) {
    uint16_t received_CRC = (((uint16_t)B[7]) << 8) + B[6];
    uint16_t expected_CRC = generate_CRC_16_bit(6, B[0], B[1], B[2], B[3], B[4], B[5]);
    if (received_CRC ==  expected_CRC ) {
      Serial.println(String(B[0]) + "," + String(B[1]) + "," + String(B[2]) + "," + String(B[3]) + "," + String(B[4]) + "," + String(B[5]) + "," + String(B[6])+","+String(B[7]));
    }
  }

  while (Serial.available())Serial.read();
}
