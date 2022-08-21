#define DEBUG true
#define LORA_ID 254

#include <SoftwareSerial.h>
#define SOFTWARE_RX_PIN 8
#define SOFTWARE_TX_PIN 9
#define OUT_ENABLE_PIN 4
#define SOFTWARE_SERIAL_BAUD_RATE 9600
#define TIMEOUT_ms  1500
#define WAIT_TIME_ms  10
SoftwareSerial software_serial_RS485(SOFTWARE_RX_PIN, SOFTWARE_TX_PIN);//Rx,Tx

#define BROADCAST_MASTER_TIMEOUT_ms 750 //takes about 500ms
#define BROADCAST_SLAVE_TIMEOUT_ms 350 //takes about 50ms

void configure_slave() {
  pinMode(OUT_ENABLE_PIN, OUTPUT);
  digitalWrite(OUT_ENABLE_PIN, LOW);
  pinMode(SOFTWARE_RX_PIN, INPUT); //Probably, also, configured by SoftwareSerial library.
  pinMode(SOFTWARE_TX_PIN, OUTPUT);//Probably, also, configured by SoftwareSerial library.
  software_serial_RS485.begin(SOFTWARE_SERIAL_BAUD_RATE);
}

struct dataPackage {
  boolean is_new = false;
  unsigned long time_received_ms;
  uint8_t B[9];
  uint8_t byte_count;
  boolean has_valid_CRC = false;
};

dataPackage package_RS485, package_LoRa;

void listen_RS485() {

  if (software_serial_RS485.available() < 1)return; //No software serial data is received
  delay(WAIT_TIME_ms);

  uint8_t byte_count_dummy = software_serial_RS485.available();

  if (byte_count_dummy < 5) {
    while (software_serial_RS485.available())software_serial_RS485.read();
    return;
  }

  if (byte_count_dummy > 8) byte_count_dummy = 8;

  package_RS485.is_new = true;
  package_RS485.time_received_ms = millis();


  for (uint8_t i = 0; i < byte_count_dummy; i++) {
    package_RS485.B[i] = software_serial_RS485.read();
  }
  package_RS485.byte_count = byte_count_dummy;

  uint16_t received_CRC = ((uint16_t)package_RS485.B[byte_count_dummy - 1] << 8) + package_RS485.B[byte_count_dummy - 2];
  uint16_t expected_CRC = generate_CRC_16_bit(byte_count_dummy - 2, package_RS485.B[0],  package_RS485.B[1],  package_RS485.B[2],  package_RS485.B[3],  package_RS485.B[4],  package_RS485.B[5], 0);

  if (received_CRC == expected_CRC)package_RS485.has_valid_CRC = true;
  else package_RS485.has_valid_CRC = false;



  while (software_serial_RS485.available())software_serial_RS485.read();

}

void listen_LoRa() {
  if (Serial.available() < 1)return; //No serial data is received
  delay(WAIT_TIME_ms);

  uint8_t byte_count_dummy = Serial.available();

  if (byte_count_dummy < 5 ) {
    while (Serial.available())Serial.read();
    return;
  }

  if (byte_count_dummy > 9) byte_count_dummy = 9;

  package_LoRa.is_new = true;
  package_LoRa.time_received_ms = millis();

  for (uint8_t i = 0; i < byte_count_dummy; i++) {
    package_LoRa.B[i] = Serial.read();
  }
  package_LoRa.byte_count = byte_count_dummy;

  uint16_t received_CRC = ((uint16_t)package_LoRa.B[byte_count_dummy - 1] << 8) + package_LoRa.B[byte_count_dummy - 2];
  uint16_t expected_CRC = generate_CRC_16_bit(byte_count_dummy - 2, package_LoRa.B[0],  package_LoRa.B[1],  package_LoRa.B[2],  package_LoRa.B[3],  package_LoRa.B[4],  package_LoRa.B[5], package_LoRa.B[6]);

  if (received_CRC == expected_CRC)package_LoRa.has_valid_CRC = true;
  else package_LoRa.has_valid_CRC = false;


  while (Serial.available())Serial.read();
}
