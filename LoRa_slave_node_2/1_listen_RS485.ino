#include <SoftwareSerial.h>
#define SOFTWARE_RX_PIN 2
#define SOFTWARE_TX_PIN 3
#define OUT_ENABLE_PIN 4
#define SOFTWARE_SERIAL_BAUD_RATE 9600
#define WAIT_SERIAL_TIME_ms 10

SoftwareSerial serial_RS485(SOFTWARE_RX_PIN, SOFTWARE_TX_PIN);//Rx,Tx,
void configure_lora_node() {
  pinMode(OUT_ENABLE_PIN, OUTPUT);
  digitalWrite(OUT_ENABLE_PIN, LOW);
  serial_RS485.begin(SOFTWARE_SERIAL_BAUD_RATE);
}
struct serial_package {
  uint8_t B[8];
  uint8_t byte_count = 0;
  unsigned long time_received_ms = 0;

  boolean has_valid_CRC = false;
  boolean analyzed_by_RS485_to_query = true;
};
serial_package package_RS485, package_LoRa;

void listen_RS485() {
  if (serial_RS485.available() < 1)return; //No software serial data is received
  delay(WAIT_SERIAL_TIME_ms);



  uint8_t byte_count_dummy = serial_RS485.available();
  if (byte_count_dummy < 5 || byte_count_dummy > 8) {
    while (serial_RS485.available())serial_RS485.read();
    return;
  }
  package_RS485.time_received_ms = millis();

  package_RS485.byte_count = byte_count_dummy;
  for (uint8_t i = 0; i < byte_count_dummy; i++) {
    package_RS485.B[i] = serial_RS485.read();
  }


  uint16_t received_CRC = ((uint16_t)package_RS485.B[byte_count_dummy - 1] << 8) + package_RS485.B[byte_count_dummy - 2];
  uint16_t expected_CRC = generate_CRC_16_bit(byte_count_dummy - 2, package_RS485.B[0],  package_RS485.B[1],  package_RS485.B[2],  package_RS485.B[3],  package_RS485.B[4],  package_RS485.B[5]);

  if (received_CRC == expected_CRC)package_RS485.has_valid_CRC = true;
  else package_RS485.has_valid_CRC = false;

  package_RS485.analyzed_by_RS485_to_query = false;
}
