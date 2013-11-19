// SoftwareSerial must be included because the library depends on it
// If you use the Library in Wiegand Mode, then the compiler
// will optimize the SoftwareSerial away
#include <SoftwareSerial.h>
#include "RFID.h"

// Creates an RFID instance in Wiegand Mode
// DATA0 of the RFID Reader must be connected 
// to Pin 2 of your Arduino (INT0)
// DATA1 of the RFID Reader must be connected
// to Pin 3 of your Arduino (INT1)
RFID rfid(RFID_WIEGAND, W26BIT);

// Declares a struct to hold the data of the RFID card
// Available fields:
//  * id (3 Bytes) - card code
//  * valid - validity
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
        Serial.print("CC = ");
        Serial.println(tag.id, HEX);
        Serial.print("The ID is ");
        if (tag.valid) Serial.println("valid");
        else Serial.println("invalid");
    }
}
