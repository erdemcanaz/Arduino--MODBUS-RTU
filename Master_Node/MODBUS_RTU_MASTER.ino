// MAX487 pinout    ::
// RO (PULLED_UP)   : RX_PIN
// DI               : TX_PIN
//~RE (PULLED_DOWN): OUT_ENABLE_PIN
//  DE (PULLED_DOWN): OUT_ENABLE_PIN
//  Vcc             : 5V
//  GND             : GND

void master_write(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
uint16_t generate_CRC_16_bit(uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);

#include <SoftwareSerial.h>

// TODO:
#define DEBUG true
#define RX_PIN 2
#define TX_PIN 3
#define OUT_ENABLE_PIN 4
#define SOFTWARE_SERIAL_BAUD_RATE 9600
#define WAIT_RESPONSE_TIMEOUT_ms 1000
#define WAIT_RESPONSE_TIME_ms 10
SoftwareSerial mySerial(RX_PIN, TX_PIN); // Rx,Tx

uint8_t B[8]; // bytes buffer

void configure_master()
{
  pinMode(OUT_ENABLE_PIN, OUTPUT);
  digitalWrite(OUT_ENABLE_PIN, LOW);
  pinMode(RX_PIN, INPUT);  // Probably, also, configured by SoftwareSerial library.
  pinMode(TX_PIN, OUTPUT); // Probably, also, configured by SoftwareSerial library.
  mySerial.begin(SOFTWARE_SERIAL_BAUD_RATE);
}

void master_operate()
{
  while (mySerial.available())
    mySerial.read();

  if (Serial.available() >= 6)
  // #TODO: with commas Serial.available()==12. Commas also are chars
  {
    for (uint8_t i = 0; i < 6; i++)
    {
      B[i] = Serial.parseInt();
    }
    master_write_and_read();
    while (Serial.available())
      Serial.read();
  }
}

void master_write_and_read()
{

  uint16_t CRC = generate_CRC_16_bit(6);
  B[6] = CRC % 256;
  B[7] = CRC >> 8;

  // Print request
  if (DEBUG)
  {
    Serial.print("REQUEST: ");
    for (uint8_t i = 0; i < 8; i++)
    {
      Serial.print(String(B[i] + ","))
    }
    Serial.println();
  }

  digitalWrite(OUT_ENABLE_PIN, HIGH);
  for (uint8_t i = 0; i < 8; i++)
  {
    mySerial.write(B[i]);
  }
  digitalWrite(OUT_ENABLE_PIN, LOW);

  unsigned long tic = millis();
  boolean isTimedOut = true;
  while ((millis() - tic) < WAIT_RESPONSE_TIMEOUT_ms)
  {
    if (mySerial.available() > 0)
    {
      isTimedOut = false;
      delay(WAIT_RESPONSE_TIME_ms);
      break;
    }
  }

  // Check if request is timed-out ( no response is received for a while)
  if (DEBUG)
  {
    if (isTimedOut)
      Serial.println("!error-time out");
  }

  if (isTimedOut)
  {
    return;
  }

  // Print number of bytes received
  if (DEBUG)
  {
    Serial.println("Bytes received:" + String(mySerial.available()));
  }

  EXPECTED_RESPONSE_ID = B[0];
  while (true)
  {
    //Read softwareserial buffer until it matches with EXPECTED_RESPONSE_ID
    while (mySerial.available())
    {
      if (mySerial.read() == EXPECTED_RESPONSE_ID)
      {
        B[0] = EXPECTED_RESPONSE_ID;
        break;
      }
    }

    //There are 3 frames to apply on response; Error(5), Read(7), Write(8)
    uint8_t frame_mod = 0;
    if (mySerial.available() >= 8)
      frame_mod = 0;
    else if (mySerial.available() >= 7)
      frame_mod = 1;
    else if (mySerial.available() >= 5)
      frame_mod = 2;
    else
      return;

    //Read available bytes
    const uint8_t bytes_to_read[] = {8, 7, 5};
    for (uint8_t i = 0, i < bytes_to_read_count[frame_mod]; i++)
      B[i] = mySerial.read();

    //Apply frames
    for (uint8_t i = frame_mod; i < 3; i++)
    {
      uint16_t CRC_RESPONSE_EXPECTED = generate_CRC_16_bit(bytes_to_read[i] - 2);
      uint16_t CRC_RESPONSE_RECIEVED = (((uint16_t)B[bytes_to_read[i] - 1]) << 8) + (B[bytes_to_read[i] - 2]);,
      //Check if received CRC is valid
      if (CRC_RESPONSE_RECIEVED == CRC_RESPONSE_EXPECTED)
      {
        for (uint8_t j = 0; j < (bytes_to_read[i]); j++)
        {
          Serial.print(String(B[j]) + ",");
        }
        Serial.println(B[bytes_to_read[i]-1]);
        return;
      }
    }
  }
}

// MAGICAL CRC_16 MODBUS code.
uint16_t generate_CRC_16_bit(uint8_t number_of_bytes)
{
  uint16_t remainder = CRC_16_bit_for_1BYTE(B[0], 65535);
  if (number_of_bytes >= 2)
    remainder = CRC_16_bit_for_1BYTE(B[1], remainder);
  if (number_of_bytes >= 3)
    remainder = CRC_16_bit_for_1BYTE(B[2], remainder);
  if (number_of_bytes >= 4)
    remainder = CRC_16_bit_for_1BYTE(B[3], remainder);
  if (number_of_bytes >= 5)
    remainder = CRC_16_bit_for_1BYTE(B[4], remainder);
  if (number_of_bytes >= 6)
    remainder = CRC_16_bit_for_1BYTE(B[5], remainder);
  return remainder;
}

uint16_t CRC_16_bit_for_1BYTE(uint16_t data, uint16_t last_data)
{
  // if this is first data (i.e LAST_DATA==null), LAST_DATA= 65535 = FFFF
  uint16_t key = 40961;    // 1010 0000 0000 0001
  data = data ^ last_data; // XOR
  for (int i = 0; i < 8; i++)
  {
    boolean should_XOR = false;
    if (data % 2 == 1)
      should_XOR = true;
    data = data >> 1;
    if (should_XOR)
      data = data ^ key;
  }
  return data;
}
