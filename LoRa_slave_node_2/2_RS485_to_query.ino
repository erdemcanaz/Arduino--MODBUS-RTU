#define NUMBER_OF_QUERIES 4 //keep less than 255
struct struct_query {
  boolean is_active = false;

  unsigned long time_created_ms = 0;
  unsigned long time_queried_ms = 0;
  unsigned long time_responded_ms = 0; //eğer 0 ise cevap yok demek

  uint8_t B_query[8];
  uint8_t B_response[8];
  uint8_t response_byte_count = 0;
};

struct_query query_list[NUMBER_OF_QUERIES];


void RS485_to_query() {
  if (package_RS485.analyzed_by_RS485_to_query == true)return;
  package_RS485.analyzed_by_RS485_to_query = true;

  if (package_RS485.has_valid_CRC == false)return;

  /*!banlı id'lerden biriyse ignore'la

  */

  //PAKETİN İÇERDİĞİ BYTE SAYISI 8 DEĞİLSE İGNORELA
  if ( package_RS485.byte_count != 8)return; //çünkü master tarafından gönderilen read,write isteği veya slave tarafından gönderilen write responsu olamaz


  //PAKET HALİHAZIRDA QUERY'DE Mİ KONTROL ET
  uint8_t id = package_RS485.B[0];
  if(id == 254){
    /*!do lora slave commands
     * 
     */
  }
  uint8_t function_code = package_RS485.B[1];
  uint8_t reg_add_sig = package_RS485.B[2];
  uint8_t reg_add_lst = package_RS485.B[3];
  uint8_t reg_val_or_quantity_sig = package_RS485.B[4];
  uint8_t reg_val_or_quantity_lst = package_RS485.B[5];
  uint8_t CRC_lst = package_RS485.B[6];
  uint8_t CRC_sig = package_RS485.B[7];

  for (uint8_t query_index = 0; query_index < NUMBER_OF_QUERIES; query_index ++) {
    if (!query_list[query_index].is_active)continue;

    if (id == query_list[query_index].B_query[0]) {
      if (function_code == query_list[query_index].B_query[1]) {
        if (reg_add_sig == query_list[query_index].B_query[2] && reg_add_lst == query_list[query_index].B_query[3]) {

          if (function_code == 6) {
            if (reg_val_or_quantity_sig == query_list[query_index].B_query[4] && reg_val_or_quantity_lst == query_list[query_index].B_query[5]) {
              if (query_list[query_index].time_responded_ms != 0) {
                //!!send_response(query_index)
              }
              query_list[query_index].time_queried_ms = millis();
              return;
            } else {
              query_list[query_index].is_active = true;
              query_list[query_index].time_created_ms = millis();
              query_list[query_index].time_queried_ms = millis();
              query_list[query_index].time_responded_ms = 0;
              query_list[query_index].B_query[4] = reg_val_or_quantity_sig;
              query_list[query_index].B_query[5] = reg_val_or_quantity_lst;
              query_list[query_index].B_query[6] = CRC_lst;
              query_list[query_index].B_query[7] = CRC_sig;
              return;
            }
          } else if (function_code == 3 || function_code == 4) {
            if (query_list[query_index].time_responded_ms != 0) {
              //!!send_response(query_index)
            }
            query_list[query_index].time_queried_ms = millis();
            return;
          } else {
            return;
          }


        }//reg addres check
      }//function code check
    }//id check
  }

  //PAKETİ QUERY YE EKLE
  uint8_t unused_query_index = (LoRa_query_index + 1) % NUMBER_OF_QUERIES;
  unsigned long unused_query_query_time_ms = query_list[unused_query_index].time_queried_ms;
  for (uint8_t  index_now = 0; index_now < NUMBER_OF_QUERIES; index_now ++) {
    if (index_now == LoRa_query_index)continue;

    if (query_list[index_now].is_active == false) {
      query_list[index_now].is_active = true;
      query_list[index_now].time_created_ms = millis();
      query_list[index_now].time_queried_ms = millis();
      query_list[index_now].time_responded_ms = 0;
      query_list[index_now].B_query[0] = id;
      query_list[index_now].B_query[1] = function_code;
      query_list[index_now].B_query[2] = reg_add_sig;
      query_list[index_now].B_query[3] = reg_add_lst;
      query_list[index_now].B_query[4] = reg_val_or_quantity_sig;
      query_list[index_now].B_query[5] = reg_val_or_quantity_lst;
      query_list[index_now].B_query[6] = CRC_lst;
      query_list[index_now].B_query[7] = CRC_sig;
      return;
    }

    if (query_list[index_now].time_queried_ms < unused_query_query_time_ms) {
      unused_query_index = index_now;
      unused_query_query_time_ms = query_list[index_now].time_queried_ms;
    }
  }

  //overwrite an existing query
  query_list[unused_query_index].is_active = true;
  query_list[unused_query_index].time_created_ms = millis();
  query_list[unused_query_index].time_queried_ms = millis();
  query_list[unused_query_index].time_responded_ms = 0;
  query_list[unused_query_index].B_query[0] = id;
  query_list[unused_query_index].B_query[1] = function_code;
  query_list[unused_query_index].B_query[2] = reg_add_sig;
  query_list[unused_query_index].B_query[3] = reg_add_lst;
  query_list[unused_query_index].B_query[4] = reg_val_or_quantity_sig;
  query_list[unused_query_index].B_query[5] = reg_val_or_quantity_lst;
  query_list[unused_query_index].B_query[6] = CRC_lst;
  query_list[unused_query_index].B_query[7] = CRC_sig;
}


//DEBUG
unsigned long last_print = 0;
void print_queries() {
  if (millis() - last_print < 30000)return;
  last_print = millis();
  Serial.println("-----------------------------");
  for (int i = 0; i < NUMBER_OF_QUERIES; i++) {
    Serial.print(String(i) + ":" + String(query_list[i].is_active) + " " + String(query_list[i].time_created_ms) + " " + String(query_list[i].time_queried_ms) + " ");
    for (int j = 0; j < 8; j++)Serial.print(String(query_list[i].B_query[j]) + " ");
      Serial.println();
  }
  Serial.println("-----------------------------");
}
