uint16_t generate_CRC_16_bit(uint8_t , uint8_t , uint8_t , uint8_t , uint8_t , uint8_t , uint8_t );

boolean led_state = false;
uint8_t LoRa_query_index = 0;

void setup() {
  //false -> master
  //true -> slave
  configure_lora_node(); 
  pinMode(13,OUTPUT);
}

void loop() {
  
  listen_RS485();
  listen_LoRa();
  
  RS485_to_query();
  query_to_LoRa();
  



  //DEBUG
  //print_queries();
}
