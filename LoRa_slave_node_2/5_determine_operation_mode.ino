
#define TOTAL_RX_REQUIRED_TO_DETERMINE_OPERATION_MODE 10


uint8_t counter_RS485_RX = 0;
uint8_t counter_LoRa_RX = 0;


void determine_operation_mode() {
  if (!package_RS485.analyzed_by_determine_operation_mode) {
    package_RS485.analyzed_by_determine_operation_mode = true;
    counter_RS485_RX += 1;
  }
  if (!package_LoRa.analyzed_by_determine_operation_mode) {
    package_LoRa.analyzed_by_determine_operation_mode = true;
    counter_LoRa_RX += 1;
  }

  if (counter_RS485_RX + counter_LoRa_RX > TOTAL_RX_REQUIRED_TO_DETERMINE_OPERATION_MODE) {
    if (counter_RS485_RX > counter_LoRa_RX * 2) {
      operate_as_LoRa_slave = false;
    } else {
      operate_as_LoRa_slave = true;
    }
    counter_RS485_RX = 0;
    counter_LoRa_RX = 0;

  }
}
