//#define WAIT_SERIAL_TIME_ms 10

//package_LoRa;


void listen_LoRa() {
  if (Serial.available() < 1)return; //No hardware serial data is received
  delay(WAIT_SERIAL_TIME_ms);

  uint8_t byte_count_dummy = Serial.available();

  if (byte_count_dummy < 5 || byte_count_dummy > 8 || byte_count_dummy == 6) {
    while (Serial.available())Serial.print( String((uint8_t)Serial.read()) + " " );
    return;
  }
  
  
  package_LoRa.time_received_ms = millis();

  package_LoRa.byte_count = byte_count_dummy;
  for (uint8_t i = 0; i < byte_count_dummy; i++) {
    package_LoRa.B[i] = Serial.read();
  }


  uint16_t received_CRC = ((uint16_t)package_LoRa.B[byte_count_dummy - 1] << 8) + package_LoRa.B[byte_count_dummy - 2];
  uint16_t expected_CRC = generate_CRC_16_bit(byte_count_dummy - 2, package_LoRa.B[0],  package_LoRa.B[1],  package_LoRa.B[2],  package_LoRa.B[3],  package_LoRa.B[4],  package_LoRa.B[5]);
    
  if (received_CRC == expected_CRC)package_LoRa.has_valid_CRC = true;
  else package_LoRa.has_valid_CRC = false;

  
  
  
}
