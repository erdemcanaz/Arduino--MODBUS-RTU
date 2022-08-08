#define LORA_BROADCAST_PERIOD_ms 1500
unsigned long last_lora_broadcast_ms = 0;


//data yollanacak 0
//data bekleniyor 1

boolean is_query_to_LoRa_waits_data = false;
void query_to_LoRa() {

  if (!is_query_to_LoRa_waits_data) {
  
    LoRa_query_index = ( LoRa_query_index + 1 ) % NUMBER_OF_QUERIES;
    if (LoRa_query_index == 1)digitalWrite(13, HIGH);
    else digitalWrite(13, LOW);

    if (!query_list[LoRa_query_index].is_active) return;
    //!banlananları kaldır



    uint8_t id = query_list[LoRa_query_index].B_query[0];
    uint8_t function_code = query_list[LoRa_query_index].B_query[1];
    uint8_t reg_add_sig = query_list[LoRa_query_index].B_query[2];
    uint8_t reg_add_lst = query_list[LoRa_query_index].B_query[3];
    uint8_t reg_val_or_quantity_sig = query_list[LoRa_query_index].B_query[4];
    uint8_t reg_val_or_quantity_lst = query_list[LoRa_query_index].B_query[5];
    uint8_t CRC_lst = query_list[LoRa_query_index].B_query[6];
    uint8_t CRC_sig = query_list[LoRa_query_index].B_query[7];

    Serial.write(id);
    Serial.write(function_code);
    Serial.write(reg_add_sig);
    Serial.write(reg_add_lst);
    Serial.write(reg_val_or_quantity_sig);
    Serial.write(reg_val_or_quantity_lst);
    Serial.write(CRC_lst);
    Serial.write(CRC_sig);

    last_lora_broadcast_ms = millis();
    is_query_to_LoRa_waits_data = true;

  } else {
    if ( (millis() - last_lora_broadcast_ms) < LORA_BROADCAST_PERIOD_ms)return;    
    is_query_to_LoRa_waits_data = false;

     
     
    //Serial.println(package_LoRa.time_received_ms);

    unsigned long time_diff = 0; 
    if(package_LoRa.time_received_ms>last_lora_broadcast_ms)time_diff = package_LoRa.time_received_ms-last_lora_broadcast_ms;
    else time_diff = LORA_BROADCAST_PERIOD_ms;
    
    if (time_diff >=  LORA_BROADCAST_PERIOD_ms) {
      query_list[LoRa_query_index].is_active = false;
      return;
    }
  
    

    uint8_t byte_count_dummy = package_LoRa.byte_count;
    uint16_t received_CRC = ((uint16_t)package_LoRa.B[byte_count_dummy - 1] << 8) + package_LoRa.B[byte_count_dummy - 2];
    uint16_t expected_CRC = generate_CRC_16_bit(byte_count_dummy - 2, package_LoRa.B[0],  package_LoRa.B[1],  package_LoRa.B[2],  package_LoRa.B[3],  package_LoRa.B[4],  package_LoRa.B[5]);

    if (received_CRC != expected_CRC) {
      return;
    }


    query_list[LoRa_query_index].time_responded_ms = millis();
    query_list[LoRa_query_index].response_byte_count = byte_count_dummy;

    for (uint8_t i; i < byte_count_dummy; i++) {
      query_list[LoRa_query_index].B_response[i] = package_LoRa.B[i];
    }
    slave_write( byte_count_dummy, package_LoRa.B[0], package_LoRa.B[1], package_LoRa.B[2], package_LoRa.B[3], package_LoRa.B[4], package_LoRa.B[5]);

  }
}
