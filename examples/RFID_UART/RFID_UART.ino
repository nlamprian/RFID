#include <SoftwareSerial.h>
#include "RFID.h"

// Connect your RFID reader's output line to the RX Pin
// Read here: http://arduino.cc/en/Reference/SoftwareSerial
// for the available pin on your board
#define RFID_RX_PIN 4

// Creates an RFID instance in UART Mode
RFID rfid(RFID_UART, RFID_RX_PIN);

// Declares a struct to hold the data of the RFID card
// Available fields:
//  * mfr (2 Bytes) - manufacture's code
//  * id (3 Bytes) - card code
//  * chk (1 Byte) - checksum
//  * valid - validity
//  * raw (12 Bytes) - received data
RFIDTag tag;

void setup()
{
  Serial.begin(9600);
}

void loop()
{  
    if(rfid.available())
    {
      tag = rfid.getTag();
      Serial.print("FC      : ");
      Serial.println(tag.mfr, HEX);
      Serial.print("CC      : ");
      Serial.println(tag.id, HEX);
      Serial.print("CHECKSUM: ");
      Serial.println(tag.chk, HEX);
      Serial.print("RAW DATA: ");
      Serial.println(tag.raw);
      Serial.print("The ID is ");
      if (tag.valid) Serial.println("valid");
      else Serial.println("invalid");
    }
}

