uint16_t generate_CRC_16_bit(uint8_t , uint8_t , uint8_t , uint8_t , uint8_t , uint8_t , uint8_t );

void setup(){
  Serial.begin(9600);
  configure_lora_node();
}

void loop(){
  listen_RS485();
  RS485_to_query();
  //print_queries();
}
