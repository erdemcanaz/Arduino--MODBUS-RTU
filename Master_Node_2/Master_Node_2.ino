
void master_write(uint8_t , uint8_t , uint8_t , uint8_t , uint8_t , uint8_t );
uint16_t generate_CRC_16_bit(uint8_t , uint8_t , uint8_t , uint8_t , uint8_t , uint8_t , uint8_t );
uint16_t generate_CRC_16_bit_for_6BYTE(uint8_t , uint8_t , uint8_t , uint8_t , uint8_t , uint8_t );
uint16_t generate_CRC_16_bit_for_3BYTE(uint8_t , uint8_t , uint8_t);
void configure_master();

void setup() {
  Serial.begin(9600);
  configure_master();
}

void loop() { 
  //master_operate(); 
  test_endian();
  delay(10);

}
