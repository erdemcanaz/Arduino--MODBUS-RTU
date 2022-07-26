
void master_write(uint8_t , uint8_t , uint8_t , uint8_t , uint8_t , uint8_t );
uint16_t generate_CRC_16_bit_for_6BYTE(uint8_t , uint8_t , uint8_t , uint8_t , uint8_t , uint8_t );
uint16_t generate_CRC_16_bit_for_3BYTE(uint8_t , uint8_t , uint8_t);
void configure_master();

void setup() {
  Serial.begin(9600);
  configure_master();
}

//0F 06 20 00 00 01 42 E4 |HEX , start motor
//0F 06 20 00 00 05 43 27 |HEX , stop motor

void loop() {
  master_write(15, 6, 32, 0, 0, 1);
  delay(10000);
  master_write(15, 6, 32, 0, 0, 5);
  delay(10000);

}
