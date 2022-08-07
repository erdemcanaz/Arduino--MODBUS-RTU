#define DEBUG false
#include <SoftwareSerial.h>

boolean OPERATE_AS_LORA_SLAVE = true;
#define ID_LORA 254
#define RX_PIN 2
#define TX_PIN 3

#define LORA_RESPONSE_TIME_ms 700

#define OUT_ENABLE_PIN 4
#define SOFTWARE_SERIAL_BAUD_RATE 9600
#define WAIT_RESPONSE_TIME_ms 10

#define NUMBER_OF_QUERIES 8


struct struct_query {
  unsigned long time_created_ms = 0;
  boolean is_active = false;
  boolean is_responded = false;
  uint8_t B_query[8];
  uint8_t B_response[9];
  uint8_t response_count = 0;
};

struct_query queries[NUMBER_OF_QUERIES];
uint8_t query_counter = 0;

uint8_t ALLOWED_ID[32];//0-255
void set_all_ids_allowed() {
  for (uint8_t i; i < 32; i++)ALLOWED_ID[i] = 255;
}
boolean set_or_read_allowed_id(boolean should_set, uint8_t id, boolean is_allowed) {
  uint8_t reg = id / 8;
  if (reg > 31) {
    //####
    if (DEBUG)Serial.println("ALLOWED_ID FUNCTION(): \"id:+" + String(id) + " is out of range");
    //####
    return false;
  }
  uint8_t bit_no = id % 8;

  uint8_t bit_mask = 0b00000001;
  bit_mask = bit_mask << bit_no;

  if (should_set) {
    if (is_allowed)ALLOWED_ID[reg] =  ALLOWED_ID[reg] | bit_mask;
    else ALLOWED_ID[reg] = ALLOWED_ID[reg] & (~bit_mask);
    return true;
  } else {
    uint8_t state = ALLOWED_ID[reg] & (bit_mask);
    if (state)return true;
    else return false;
  }
}


SoftwareSerial RS485_Serial(RX_PIN, TX_PIN);//Rx,Tx,


void configure_lora_node() {
  pinMode(OUT_ENABLE_PIN, OUTPUT);
  RS485_Serial.begin(SOFTWARE_SERIAL_BAUD_RATE);
  set_all_ids_allowed();
}

uint8_t B_RS485[9];//B_RS485[8] is reserved for number of bytes

void operate_LoRa_node() {
  //both
  listen_RS485();

  //master
  should_I_append_to_the_query_or_have_response_for_it();
  listen_and_write_LoRa();
  //slave
}

boolean new_rs485 = false;
void listen_RS485() {
  RS485_Serial.listen();
  delay(1);
  if (RS485_Serial.available() >= 1)delay(WAIT_RESPONSE_TIME_ms);
  else return;

  uint8_t number_of_bytes_received = RS485_Serial.available();
  if (number_of_bytes_received == 8 || number_of_bytes_received == 7 || number_of_bytes_received == 5) {//8: write-read request or write response, 7: read response,5: error response
    for (int i = 0; i < number_of_bytes_received; i++) {
      B_RS485[i] = RS485_Serial.read();
    }
  } else {
    while (RS485_Serial.available())RS485_Serial.read();
    return;
  }
  ////
  uint16_t received_CRC = (((uint16_t)B_RS485[number_of_bytes_received - 1]) << 8) + B_RS485[number_of_bytes_received - 2];
  uint16_t expected_CRC = generate_CRC_16_bit(number_of_bytes_received - 2, B_RS485[0], B_RS485[1], B_RS485[2], B_RS485[3], B_RS485[4], B_RS485[5]);
  if (received_CRC != expected_CRC) {
    while (RS485_Serial.available())RS485_Serial.read();
    return;
  }

  new_rs485 = true;
  B_RS485[8] = number_of_bytes_received;

  if (DEBUG) {
    for (uint8_t i = 0; i < number_of_bytes_received; i++)Serial.print(String(B_RS485[i]) + (" "));
    Serial.println();
  }

}

void should_I_append_to_the_query_or_have_response_for_it() {
  if (!new_rs485)return;
  
  if (B_RS485[0] == ID_LORA) {
    for (uint8_t i = 0; i < NUMBER_OF_QUERIES; i++)queries[i].is_active = false;
    set_all_ids_allowed();
    new_rs485 = false;
    return;
  }
  if (B_RS485[8] != 8) { // is read or write reques by master
    new_rs485 = false;
    return;
  }



  if ( !set_or_read_allowed_id(false, B_RS485[0], true) ) { //is an allowed id
    new_rs485 = false;
    return;
  }

  boolean should_append_query = true;

  //-------------------------------------
  for (uint8_t i = 0; i < NUMBER_OF_QUERIES; i++) {
    boolean does_query_match = false;
    //1
    if (queries[i].is_active) {
      does_query_match = true;
      for (uint8_t j = 0; j < 4; j++) { //0-id, 1-func, 2-reg ad, 3-reg ad
        if (queries[i].B_query[j] != B_RS485[j]) {
          does_query_match = false;
        }
      }
      if (B_RS485[1] == 6) { //4- reg val, 5-reg val
        if (queries[i].B_query[4] != B_RS485[4] || queries[i].B_query[5] != B_RS485[5]) {
          does_query_match = false;
        }
      }
    }
    if (!does_query_match) continue;
    //2
    if (queries[i].is_responded) {
      //! rs485 hattına elimde olan responsu yazdır
    } else {
      //! Belki ilerde bende cevabı olur ama şuan yok, HİÇBİR ŞEY YAPMA master 500 ms sonra timeout yer zaten
    }
    should_append_query = false;
    break;
  }
  //

  //--------------------------------------
  if (should_append_query) {
    boolean is_fail = true;
    for (uint8_t i = 0; i < NUMBER_OF_QUERIES; i++) {
      if (queries[i].is_active == false) {
        for (int j = 0; j < 8; j++) {
          queries[i].B_query[j] = B_RS485[j];
        }
        queries[i].time_created_ms = millis();
        queries[i].is_active = true;
        queries[i].is_responded = false;
        is_fail = false;
        break;
      }
    }

    if (is_fail) {
      uint8_t overwrite_query_index = 0;
      unsigned long time_dif = 0;
      for (int i = 0; i < NUMBER_OF_QUERIES; i++) {
        if (millis() - queries[i].time_created_ms > time_dif && i != query_counter) {
          time_dif = millis() - queries[i].time_created_ms;
          overwrite_query_index = i;
        }
      }
      for (int j = 0; j < 8; j++) {
        queries[overwrite_query_index].B_query[j] = B_RS485[j];
      }
      queries[overwrite_query_index].is_active = true;
    }

  }

  new_rs485 = false;
}


unsigned long last_query_broadcast_time = 0;
uint8_t B_LORA[8];//B_RS485[8] is reserved for number of bytes

void listen_and_write_LoRa() {

  if (millis() - last_query_broadcast_time < LORA_RESPONSE_TIME_ms)return;


  delay(WAIT_RESPONSE_TIME_ms);

  boolean is_response_received = false;
  uint8_t number_of_bytes_received = Serial.available();
  if (number_of_bytes_received == 8) {//8: write-read request
    for (int i = 0; i < number_of_bytes_received; i++) {
      B_LORA[i] = Serial.read();
    }
    is_response_received = true;
  } else {
    while (Serial.available())Serial.read();
  }
  ////
  if (is_response_received) { //cevap geldiyse ve active ise query ye eşitle
    if (queries[query_counter].is_active) {
      for (int i = 0; i < number_of_bytes_received; i++) {
        queries[query_counter].B_response[i] = B_LORA[i];
      }
      queries[query_counter].B_response[8] = number_of_bytes_received;
      queries[query_counter].is_responded = true;
    }
  } else { //cevap geldi mi önceki mesaja, gelmediyse önceki mesaja ait tüm id'li query leri sil(pasif hale getir) ve önceki mesajın id sini banla
    uint8_t id_to_deactivate =  queries[query_counter].B_query[0];
    set_or_read_allowed_id(true, id_to_deactivate , false);
    for (int i = 0; i < NUMBER_OF_QUERIES; i++) {
      if ( queries[i].B_query[0] == id_to_deactivate) {
        queries[i].is_active = false;
      }
    }

  }

  ////
  query_counter = (query_counter + 1) % NUMBER_OF_QUERIES;
  if (queries[query_counter].is_active == true) {
    last_query_broadcast_time = millis();
    write_Serial( 6, queries[query_counter].B_query[0], queries[query_counter].B_query[1], queries[query_counter].B_query[2], queries[query_counter].B_query[3], queries[query_counter].B_query[4], queries[query_counter].B_query[5]);
  }


}


void print_queries() {
  Serial.println("----------------");
  for (int i = 0; i < NUMBER_OF_QUERIES; i++) {
    Serial.print(String(i) + ":" + String(queries[i].is_active) + " | " );
    for (int j = 0; j < 8; j++) {
      Serial.print(String(queries[i].B_query[j]) + " ");
    }
    Serial.println();
  }
  Serial.println("----------------");
}


















void write_Serial( uint8_t number_of_bytes, uint8_t B_0, uint8_t B_1, uint8_t B_2, uint8_t B_3, uint8_t B_4, uint8_t B_5) {
  uint16_t CRC = generate_CRC_16_bit(number_of_bytes, B_0,  B_1,  B_2,  B_3,  B_4,  B_5);
  uint8_t CRC_LEAST = CRC % 256;
  uint8_t CRC_SIGNIFICANT = CRC >> 8;

  Serial.write(B_0);
  if (number_of_bytes >= 2 )  Serial.write(B_1);
  if (number_of_bytes >= 3 )  Serial.write(B_2);
  if (number_of_bytes >= 4 )  Serial.write(B_3);
  if (number_of_bytes >= 5 )  Serial.write(B_4);
  if (number_of_bytes >= 6 )  Serial.write(B_5);
  Serial.write(CRC_LEAST); //CRC (LST)
  Serial.write(CRC_SIGNIFICANT); //CRC (SIG)

}


void write_RS485( uint8_t number_of_bytes, uint8_t B_0, uint8_t B_1, uint8_t B_2, uint8_t B_3, uint8_t B_4, uint8_t B_5) {
  uint16_t CRC = generate_CRC_16_bit(number_of_bytes, B_0,  B_1,  B_2,  B_3,  B_4,  B_5);
  uint8_t CRC_LEAST = CRC % 256;
  uint8_t CRC_SIGNIFICANT = CRC >> 8;
  delay(WAIT_RESPONSE_TIME_ms);

  digitalWrite(OUT_ENABLE_PIN, HIGH);
  RS485_Serial.write(B_0);
  if (number_of_bytes >= 2 )  RS485_Serial.write(B_1);
  if (number_of_bytes >= 3 )  RS485_Serial.write(B_2);
  if (number_of_bytes >= 4 )  RS485_Serial.write(B_3);
  if (number_of_bytes >= 5 )  RS485_Serial.write(B_4);
  if (number_of_bytes >= 6 )  RS485_Serial.write(B_5);
  RS485_Serial.write(CRC_LEAST); //CRC (LST)
  RS485_Serial.write(CRC_SIGNIFICANT); //CRC (SIG)
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
