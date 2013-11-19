/**
 * Name: RFID
 * Author: Nick Lamprianidis (lampnick67@yahoo.com)
 * Version: 1.0
 * Description: A library for interfacing RFID readers
                with either a UART or a Wiegand protocol
 * Attribution: The library was based on the work of Johann Richard
 *              https://github.com/johannrichard/SeeedRFIDLib
 * License: Copyright (c) 2013 Nick Lamprianidis 
 *          This library is licensed under the MIT license
 *          http://www.opensource.org/licenses/mit-license.php
 *
 * Filename: RFID.h
 * File description: Definitions and methods for the RFID library
 */

#ifndef RFID_h
#define RFID_h

// #define DEBUG true // Use this if you want to debug your RFID stuff

#include <SoftwareSerial.h>
#include <Arduino.h>

// Struct for storing an RFID tag
struct RFIDTag
{
    int mfr;        // Manufacturer (?) Code (2 Bytes), only useful in UART Mode
    long id;        // Tag ID (3 Bytes)
    byte chk;       // Checksum (1 Byte), only useful in UART Mode
    boolean valid;  // Validity of the Tag, based on the Checksum (UART Mode) or the parity bits (Wiegand Mode)
    char raw[13];   // The whole tag as a raw string, only useful in UART Mode
};

enum RFIDMode
{
    RFID_UART,
    RFID_WIEGAND
};

#define W26BIT 26
#define W35BIT 35
#define AVAILABLE 1

// Class for reading and checking RFID tags
class RFID
{
public:
    RFID(RFIDMode mode, byte arg);
    boolean available();
    RFIDTag getTag();
    static long long hex2dec(char *hexCode, byte start, byte end);

private:
    RFIDTag _tag;
    RFIDMode _libMode;  // Signifies the mode of the interface
    // UART Mode
    void initUART(byte rxPin);
    boolean prepareRFIDTagUART();
    SoftwareSerial *_rfidIO;
    byte _bytesRead;  // Counts the number of bytes received
    boolean _uartIdAvailable;
    // Wiegand Mode
    void initWiegand(byte dataLen);
    void refreshWiegand();
    void resetWiegand();
    void prepareRFIDTagWiegand();
    boolean checkParity26();
    boolean checkParity35();
    static void DATA0();
    static void DATA1();
    static byte _dataLen;  // #bits of the protocol
    volatile static unsigned long long _databits;  // stores all of the data bits
    volatile static unsigned char _bitCount;  // Counts the number of bits received
    volatile static boolean _wiegandIdAvailable;
    unsigned long _facilityCode;
    unsigned long _cardCode;
};

#endif  // RFID
