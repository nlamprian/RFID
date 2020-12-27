RFID
====

An Arduino library for interfacing RFID readers with either a UART or a Wiegand protocol of communication.

![rfid](https://raw.githubusercontent.com/wiki/nlamprian/RFID/assets/rfid.png)


Interface
---------
The library provides three methods:
* `available()` - Returns true if a new RFID tag is available for reading
* `getTag()` - Returns the newly available RFID tag
* `hex2dec()` - Parses a char array of hex (ASCII) characters (up to 16) to a long long number

The library separates the individual data fields of the RFID reader datastream, and makes the necessary checks for validity of the received data, based on the checksum in UART Mode and the parity bits in Wiegand Mode.

The library provides a struct to accommodate the information of an RFID Card
```cpp
struct RFIDTag
{
    int mfr;        // Manufacturer (?) Code (2 Bytes), only useful in UART Mode
    long id;        // Tag ID (3 Bytes)
    byte chk;       // Checksum (1 Byte), only useful in UART Mode
    boolean valid;  // Validity of the Tag, based on the Checksum (UART Mode) or the parity bits (Wiegand Mode)
    char raw[13];   // The whole tag as a raw string, only useful in UART Mode
};
```


Use
---
To make use of the interface, an instance of the RFID class will have to be created.
* For an RFID reader with a UART protocol, the library makes use of the SoftwareSerial library. The constructor of the class should be called with arguments the mode of operation, `RFID_UART`, and the number of the pin that will be used as an RX pin.
* For an RFID reader with a Wiegand protocol, the library makes use of the interrupts INT0 and INT1. The constructor of the class should be called with arguments the mode of operation, `RFID_WIEGAND`, and the number of bits of the particular format of the protocol, `W26BIT` or `W35BIT`.

Attribution
-----------
The library was based on the work of Johann Richard with the [SeeedRFIDLib](https://github.com/johannrichard/SeeedRFIDLib).
