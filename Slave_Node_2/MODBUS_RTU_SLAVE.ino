#include <SoftwareSerial.h>
uint8_t ID = 0;
#define RX_PIN 2
#define TX_PIN 3
#define OUT_ENABLE_PIN 4
#define SOFTWARE_SERIAL_BAUD_RATE 9600
#define WAIT_RESPONSE_TIME_ms 10
#define NUMBER_OF_REGISTERS 128
SoftwareSerial mySerial(RX_PIN, TX_PIN);//Rx,Tx,

boolean doesRegisterExist[128];
boolean isWriteAllowed[128];
uint16_t registers[128];
uint8_t B[8];//received bytes buffer


void configure_slave(uint8_t id) {
  ID = id;
  pinMode(OUT_ENABLE_PIN, OUTPUT);
  pinMode(RX_PIN, INPUT); //Probably also configured by SoftwareSerial library.
  pinMode(TX_PIN, OUTPUT);//Probably also configured by SoftwareSerial library.
  mySerial.begin(SOFTWARE_SERIAL_BAUD_RATE);
  for (int i = 0; i < NUMBER_OF_REGISTERS; i ++) {
    doesRegisterExist[i] = false;
    isWriteAllowed[i] = false;
  }
}


void slave_operate() {
  slave_read();
}


void slave_read() {
  if (mySerial.available() >= 1)delay(WAIT_RESPONSE_TIME_ms);
  else return;


  uint8_t number_of_bytes_received = mySerial.available();
  if (number_of_bytes_received == 8) {//write or read request
    for (int i = 0; i < 8; i++) {
      B[i] = mySerial.read();
    }
  } else {
    while (mySerial.available())mySerial.read();
    return;
  }
  //---------------ERROR CHECK
  //1-ID CHECK
  if (B[0] != ID) {
    while (mySerial.available())mySerial.read();
    slave_write(3, ID, 128, 7, 0, 0, 0);
    return;
  }
  //2-CRC CHECK
  uint16_t received_CRC = (((uint16_t)B[7]) << 8) + B[6];
  uint16_t expected_CRC = generate_CRC_16_bit(6, B[0], B[1], B[2], B[3], B[4], B[5]);

  if (received_CRC != expected_CRC) {
    while (mySerial.available())mySerial.read();
    slave_write(3, ID, 128, 7, 0, 0, 0);
    return;
  }

  //------------------

  if (B[1] == 3) {
    //QUERY: 0-ID, 1-FUNC_CODE, 2-REG_ADDR(SIG), 3-REG_ADDR(LST), 4-NUMBER_OF_REG(SIG), 5-NUMBER_OF_REG(LST), 6-CRC(LST), 7-CRC(SIG)
    uint16_t register_addres = ((uint16_t)B[2] << 8) + B[3];

    //3-ILLEGAL DATA ADDRESS CHECK
    if (!doesRegisterExist[register_addres]) {
      while (mySerial.available())mySerial.read();
      slave_write(3, ID, 128, 7, 0, 0, 0);
      return;
    }
    uint8_t register_val_sig = registers[register_addres] >> 8;
    uint8_t register_val_lst = registers[register_addres] % 256;

    //RESPONSE: 0-ID, 1-FUNC_CODE, 2-BYTE_COUNT, 3-REG_VAL(SIG), 4-REG_VAL(LST), 5-CRC(LST), 6-CRC(SIG);
    slave_write(5, B[0], B[1], 2, register_val_sig, register_val_lst, 0);
  }
  else if (B[1] == 6) {
    //QUERY: 0-ID, 1-FUNC_CODE, 2-REG_ADDR(SIG), 3-REG_ADDR(LST), 4-REG_VAL(SIG), 5-REG_VAL(LST), 6-CRC(LST), 7-CRC(SIG)
    uint16_t register_addres = ((uint16_t)B[2] << 8) + B[3];

    //3-ILLEGAL DATA ADDRESS CHECK
    if (!doesRegisterExist[register_addres]) {
      while (mySerial.available())mySerial.read();
      slave_write(3, ID, 128, 7, 0, 0, 0);
      return;
    }
    //4-WRITE PERMISSION CHECK
    if (!isWriteAllowed[register_addres]) {
      while (mySerial.available())mySerial.read();
      slave_write(3, ID, 128, 7, 0, 0, 0);
      return;
    }
    uint16_t register_val = ((uint16_t)B[4] << 8) + B[5]; //or number of register to read which is assuemd to be 1

    registers[register_addres] = register_val;

    //RESPONSE: 0-ID, 1-FUNC_CODE, 2-REG_ADDR(SIG), 3-REG_ADDR(LST), 4-REG_VAL(SIG), 5-REG_VAL(LST), 6-CRC(LST), 7-CRC(SIG)
    slave_write(6, B[0], B[1], B[2], B[3], B[4], B[5]);
  }

}

void slave_write( uint8_t number_of_bytes, uint8_t B_0, uint8_t B_1, uint8_t B_2, uint8_t B_3, uint8_t B_4, uint8_t B_5) {
  uint16_t CRC = generate_CRC_16_bit(number_of_bytes, B_0,  B_1,  B_2,  B_3,  B_4,  B_5);
  uint8_t CRC_LEAST = CRC % 256;
  uint8_t CRC_SIGNIFICANT = CRC >> 8;
  delay(WAIT_RESPONSE_TIME_ms);

  digitalWrite(OUT_ENABLE_PIN, HIGH);
  mySerial.write(B_0);
  if (number_of_bytes >= 2 )  mySerial.write(B_1);
  if (number_of_bytes >= 3 )  mySerial.write(B_2);
  if (number_of_bytes >= 4 )  mySerial.write(B_3);
  if (number_of_bytes >= 5 )  mySerial.write(B_4);
  if (number_of_bytes >= 6 )  mySerial.write(B_5);
  mySerial.write(CRC_LEAST); //CRC (LST)
  mySerial.write(CRC_SIGNIFICANT); //CRC (SIG)
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


void  configure_register(uint8_t reg_addrs, boolean state) {
  if (!(reg_addrs >= 0 && reg_addrs < NUMBER_OF_REGISTERS))return;
  doesRegisterExist[reg_addrs] = true;
  isWriteAllowed[reg_addrs] = state;
}
uint16_t get_register(uint8_t reg_addrs) {
  if (!(reg_addrs >= 0 && reg_addrs < NUMBER_OF_REGISTERS) || !doesRegisterExist[reg_addrs])return 0;
  return registers[reg_addrs];
}
boolean write_register(uint8_t reg_addrs, uint16_t reg_val, boolean is_device_itself) {
  if (isWriteAllowed[reg_addrs] || is_device_itself) {
    registers[reg_addrs] = reg_val;
    return true;
  } else {
    return false;
  }
}
