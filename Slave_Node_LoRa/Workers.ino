
uint8_t operation_mode = 0;
//1: broadcast next message and wait response.
//2: write lora broadcast to rs485, wait for line-> if listen a reply broadcast it.

unsigned long last_time = 0;
void test() {
  if(package_RS485.time_received_ms == last_time)return;
  last_time = millis();
  
  Serial.println();
  
  for(uint8_t i = 0; i< package_RS485.byte_count; i++){
    Serial.print(String(package_RS485.B[i])+ " ");
  }Serial.println();
    
  Serial.println("Byte count: " + String(package_RS485.byte_count));
  Serial.println("CRC:" + String(package_RS485.has_valid_CRC));
  Serial.println("Received when:" + String(package_RS485.time_received_ms));
}
