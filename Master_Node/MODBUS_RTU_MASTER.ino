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

    // error-> ID+ (2Byte) + (2Byte CRC)
    // read -> ID+ (4Byte) + (2Byte CRC)
    // write-> ID + (5Byte) + (2Byte CRC)

    while (mySerial.available())
    {
      if (mySerial.read() == EXPECTED_RESPONSE_ID)
      {
        B[0] = EXPECTED_RESPONSE_ID;
        break;
      }
    }

    if (mySerial.available() >= 4)
    {
      for (uint8_t i = 1; i < 5; i++)
      {
        B[i] = mySerial.read();
      }
    }
    else
      return;

    uint16_t CRC_RESPONSE_EXPECTED = generate_CRC_16_bit(3);
    if (B[3] == (CRC_RESPONSE_EXPECTED % 256) && B[4] == (CRC_RESPONSE_EXPECTED >> 8))
    {
      Serial.println(String(B[0]) + "," + String(B[2]) + "," + String(B[3]) + "," + String(B[4]) + "," + String(B[5]));
      return;
    }

    if (mySerial.available() >= 2)
    {
      for (uint8_t i = 5; i < 7; i++)
      {
        B[i] = mySerial.read();
      }
    }
    else
    {
      return;
    }

    uint16_t CRC_RESPONSE_EXPECTED = generate_CRC_16_bit(5);
    if (B[5] == (CRC_RESPONSE_EXPECTED % 256) && B[6] == (CRC_RESPONSE_EXPECTED >> 8))
    {
      Serial.println(String(B[0]) + "," + String(B[2]) + "," + String(B[3]) + "," + String(B[4]) + "," + String(B[5]) + "," + String(B[6]));
      return;
    }

    if (mySerial.available() >= 1)
    {
      for (uint8_t i = 7; i < 8; i++)
      {
        B[i] = mySerial.read();
      }
    }
    else
    {
      return;
    }

    uint16_t CRC_RESPONSE_EXPECTED = generate_CRC_16_bit(6);
    if (B[6] == (CRC_RESPONSE_EXPECTED % 256) && B[7] == (CRC_RESPONSE_EXPECTED >> 8))
    {
      Serial.println(String(B[0]) + "," + String(B[2]) + "," + String(B[3]) + "," + String(B[4]) + "," + String(B[5]) + "," + String(B[6]) + "," + String(B[7]));
      return;
    }

    if (mySerial.available() == 0)
    {

      break;
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
