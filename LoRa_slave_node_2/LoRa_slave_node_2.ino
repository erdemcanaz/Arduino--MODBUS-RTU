uint16_t generate_CRC_16_bit(uint8_t , uint8_t , uint8_t , uint8_t , uint8_t , uint8_t , uint8_t );

boolean operate_as_LoRa_slave = true;

void setup() {
  configure_lora_node();
}

void loop() {

  listen_RS485();
  listen_LoRa();
  determine_operation_mode();
  
  if (operate_as_LoRa_slave) {
    //LoRa_to_RS485
  } else {
    RS485_to_query();
    query_to_LoRa();
  }



  //DEBUG
  //print_queries();
}
