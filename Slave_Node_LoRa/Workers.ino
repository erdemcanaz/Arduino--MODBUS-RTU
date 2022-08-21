
uint8_t operation_mode = 0;
//1: operate as broadcast master
//2: operate as broadcast slave


void operate() {
  set_operation_mode();
  if (operation_mode == 1)operate_as_broadcast_master();
  else if (operation_mode == 2)operate_as_broadcast_slave();
}


void set_operation_mode() {

  if (operation_mode != 0)return;

  //1: operate as broadcast master
  if (package_RS485.is_new) {
    if (package_RS485.B[0] == LORA_ID && package_RS485.has_valid_CRC) {

      delay(WAIT_TIME_ms);
      digitalWrite(OUT_ENABLE_PIN, HIGH);
      for (uint8_t i = 0 ; i < 8; i++)software_serial_RS485.write(package_RS485.B[i]);
      digitalWrite(OUT_ENABLE_PIN, LOW);

      operation_mode = 1;
      package_LoRa.is_new = false;
    }
    package_RS485.is_new = false;
  }

  //2: operate as broadcast slave
  if (package_LoRa.is_new) {
    if (package_LoRa.B[0] == LORA_ID && package_LoRa.has_valid_CRC && package_LoRa.byte_count == 9) {
      operation_mode = 2;
      package_RS485.is_new = false;
    }
    package_LoRa.is_new = false;
  }
}


void operate_as_broadcast_master() {
  if (!package_RS485.is_new)return;
  package_RS485.is_new = false;
  operation_mode = 0;

  if (package_RS485.byte_count != 8)return;


  uint16_t CRC = generate_CRC_16_bit(7, 254, package_RS485.B[0],  package_RS485.B[1],  package_RS485.B[2],  package_RS485.B[3],  package_RS485.B[4],  package_RS485.B[5]);
  uint8_t CRC_lst = CRC % 256;
  uint8_t CRC_sig = CRC >> 8;

  Serial.write(254);
  for (uint8_t i = 0; i < 6; i++)Serial.write(package_RS485.B[i]);
  Serial.write(CRC_lst);
  Serial.write(CRC_sig);

  unsigned long tic = millis();
  while ( (millis() - tic) < BROADCAST_MASTER_TIMEOUT_ms) {
    listen_LoRa();

    if (package_LoRa.is_new) {
      if (!package_LoRa.has_valid_CRC || package_LoRa.B[0] == 254 )break;
      if (package_LoRa.B[0] == 254 )break;

      delay(WAIT_TIME_ms);
      digitalWrite(OUT_ENABLE_PIN, HIGH);
      for (uint8_t i = 0 ; i < package_LoRa.byte_count; i++)software_serial_RS485.write(package_LoRa.B[i]);
      digitalWrite(OUT_ENABLE_PIN, LOW);
      break;
    }

  }


  //END
  while (software_serial_RS485.available())software_serial_RS485.read();
  while (Serial.available())Serial.read();
  package_LoRa.is_new = false;
  package_RS485.is_new = false;
  operation_mode = 0;
}

void operate_as_broadcast_slave() {

  delay(WAIT_TIME_ms);
  digitalWrite(OUT_ENABLE_PIN, HIGH);
  for (uint8_t i = 1; i < 9; i++)software_serial_RS485.write(package_LoRa.B[i]);
  digitalWrite(OUT_ENABLE_PIN, LOW);

  //! timeout available()- RS485 den gelcek cevabı bekle, cevap gelirse loradan yay
  
  
  if (package_LoRa.has_valid_CRC) {
    Serial.write(14);
    Serial.write(6);
    Serial.write(0);
    Serial.write(0);
    Serial.write(0);
    Serial.write(1);
    Serial.write(72);
    Serial.write(245);
  }



  //END
  while (software_serial_RS485.available())software_serial_RS485.read();
  while (Serial.available())Serial.read();
  package_LoRa.is_new = false;
  package_RS485.is_new = false;
  operation_mode = 0;
}







/*

*/



/*unsigned long tic = millis();
  while ( (millis() - tic) < BROADCAST_SLAVE_TIMEOUT_ms) {
  listen_RS485();
  if (package_RS485.is_new) {
    if (!package_RS485.has_valid_CRC)break;
    for (uint8_t i = 0; i < package_RS485.byte_count; i++)Serial.write(package_RS485.B[i]);
  }
  }*/










//------------------------------------------------------
unsigned long last_time__test = 0;
void test() {
  if (package_RS485.time_received_ms == last_time__test)return;
  last_time__test = millis();

  Serial.println();

  for (uint8_t i = 0; i < package_RS485.byte_count; i++) {
    Serial.print(String(package_RS485.B[i]) + " ");
  } Serial.println();

  Serial.println("Byte count: " + String(package_RS485.byte_count));
  Serial.println("CRC:" + String(package_RS485.has_valid_CRC));
  Serial.println("Received when:" + String(package_RS485.time_received_ms));
}
