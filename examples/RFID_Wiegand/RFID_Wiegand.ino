#include <SoftwareSerial.h>  // SoftwareSerial must be included because the library depends on it
#include "RFID.h"

// Creates an RFID instance in Wiegand Mode
// DATA0 of the RFID Reader must be connected 
// to Pin 2 of your Arduino (INT0 on most boards, INT1 on Leonardo)
// DATA1 of the RFID Reader must be connected
// to Pin 3 of your Arduino (INT1 on most boards, INT0 on Leonrado)
RFID rfid(RFID_WIEGAND, W26BIT);

// Declares a struct to hold the data of the RFID tag
// Available fields:
//  * id (3 Bytes) - card code
//  * valid - validity
RFIDTag tag;

void setup() 
{
    Serial.begin(9600);  // Initializes serial port
    // Waits for serial port to connect. Needed for Leonardo only
    while ( !Serial ) ;
}

void loop()
{
    if( rfid.available() )  // Checks if there is available an RFID tag
    {
        tag = rfid.getTag();  // Retrieves the information of the tag
        Serial.print("CC = ");  // and prints that info on the serial port
        Serial.println(tag.id, HEX);
        Serial.print("The ID is ");
        if (tag.valid) Serial.println("valid");
        else Serial.println("invalid");
    }
}
