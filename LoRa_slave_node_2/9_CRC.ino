void slave_write( uint8_t number_of_bytes, uint8_t B_0, uint8_t B_1, uint8_t B_2, uint8_t B_3, uint8_t B_4, uint8_t B_5) {
  uint16_t CRC = generate_CRC_16_bit(number_of_bytes, B_0,  B_1,  B_2,  B_3,  B_4,  B_5);
  uint8_t CRC_LEAST = CRC % 256;
  uint8_t CRC_SIGNIFICANT = CRC >> 8;
  delay(WAIT_SERIAL_TIME_ms);

  digitalWrite(OUT_ENABLE_PIN, HIGH);
  serial_RS485.write(B_0);
  if (number_of_bytes >= 2 )  serial_RS485.write(B_1);
  if (number_of_bytes >= 3 )  serial_RS485.write(B_2);
  if (number_of_bytes >= 4 )  serial_RS485.write(B_3);
  if (number_of_bytes >= 5 )  serial_RS485.write(B_4);
  if (number_of_bytes >= 6 )  serial_RS485.write(B_5);
  serial_RS485.write(CRC_LEAST); //CRC (LST)
  serial_RS485.write(CRC_SIGNIFICANT); //CRC (SIG)
  digitalWrite(OUT_ENABLE_PIN, LOW);

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
