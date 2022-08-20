
uint8_t operation_mode = 0;
//1: operate as broadcast master
//2: operate as broadcast slave


void operate() {
  set_operation_mode();
  if (operation_mode == 1)operate_as_broadcast_master();
}

void set_operation_mode() {
  //!operation mode belli bir süre sonra 0 lansın (timeout)
  if (operation_mode != 0)return;

  //1: operate as broadcast master
  
  if (package_RS485.is_new && package_RS485.has_valid_CRC) {
    if (package_RS485.B[0] == LORA_ID) {
      //! tamam diye cevap ver master'a software_serial_RS485.write()
      operation_mode = 1;    
      package_LoRa.is_new = false;
    }
    package_RS485.is_new = false;
  }
  //2: operate as broadcast slave

}


void operate_as_broadcast_master() {
  if (package_RS485.byte_count != 8) {
    operation_mode = 0;
    return;
  }
  if(!package_RS485.is_new)return;

  uint16_t CRC = generate_CRC_16_bit(7, 254, package_RS485.B[0],  package_RS485.B[1],  package_RS485.B[2],  package_RS485.B[3],  package_RS485.B[4],  package_RS485.B[5]);
  uint8_t CRC_lst = CRC%256;
  uint8_t CRC_sig = CRC >>8;

  Serial.write(254);
  for(uint8_t i = 0; i<6; i++)Serial.write(package_RS485.B[i]);
  Serial.write(CRC_lst);
  Serial.write(CRC_sig);

  unsigned long tic = millis();
  while ( (millis()-tic) < BROADCAST_MASTER_TIMEOUT_ms){
    //Serial dan rx aldın mı onu takip et.
  } 

  //END
  while(software_serial_RS485.available())software_serial_RS485.read();
  while(Serial.available())Serial.read();
  package_LoRa.is_new = false;
  package_RS485.is_new = false;
  operation_mode = 0;
}






















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
