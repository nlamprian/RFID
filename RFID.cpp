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
 * Filename: RFID.cpp
 * File description: Implementation of methods for the RFID library
 */

#include "RFID.h"

byte RFID::_dataLen;  // Only for Wiegand Mode
volatile unsigned long long RFID::_databits;  // Stores the data bits
volatile unsigned char RFID::_bitCount;  // Counts the number of bits received so far
volatile boolean RFID::_wiegandIdAvailable;

// Forwards control to the right instance initializer
RFID::RFID(RFIDMode mode, byte arg)
{
    switch (mode)
    {
        case RFID_UART:
            initUART(arg);
            break;
        case RFID_WIEGAND:
            initWiegand(arg);
    }
}

// Initializes a UART interface
void RFID::initUART(byte rxPin)
{
    // Initializes RFIDTag fields
    _tag.mfr = 0;
    _tag.id = 0;
    _tag.chk = 0;
    _tag.valid = false;

    // Sets protocol parameters
    _uartIdAvailable = false;
    _bytesRead = 0;
    _libMode = RFID_UART;

    // Defines txPin as rxPin+1, but the library will not utilize that pin
    // So, txPin is available for use
    _rfidIO = new SoftwareSerial(rxPin, rxPin+1);
    _rfidIO->begin(9600);
}

// Initializes a Wiegand Interface
// This can only be used on PIN2 & PIN3, INT0 and INT1, respectively
// The dataLen attribute specifies the number of bits to read 
// The library currently only understands 26 bit or 35 bit formats
// Only one instance of the Wiegand interface can be supported
void RFID::initWiegand(byte dataLen)
{
    // Initializes RFIDTag fields
    _tag.mfr = 0;
    _tag.id = 0;
    _tag.chk = 0;
    _tag.valid = false;

    // Sets protocol parameters
    _dataLen = dataLen;
    _libMode = RFID_WIEGAND;
    
    resetWiegand();

    #if defined(__AVR_ATmega32U4__) // For Leonardo
    
    attachInterrupt(0, DATA1, FALLING);
    attachInterrupt(1, DATA0, FALLING);
    
    #else  // For the rest of boards
    
    attachInterrupt(0, DATA0, FALLING);
    attachInterrupt(1, DATA1, FALLING);
    
    #endif
}

// Reports whether an new RFID tag is available for reading
boolean RFID::available()
{ 
    switch (_libMode)
    {
        case RFID_UART:
            return prepareRFIDTagUART();
        case RFID_WIEGAND:
            return _wiegandIdAvailable;
    }
}

// Returns an RFIDTag construct with newly
// received data, and resets the interface
// variables to be ready for a new reading
RFIDTag RFID::getTag()
{
    switch (_libMode)
    {
        case RFID_UART:
            _uartIdAvailable = false;
            return _tag;
        case RFID_WIEGAND:
            refreshWiegand();
            return _tag;
    }
}

// Checks for UART Data and prepares an RFIDTag construct
boolean RFID::prepareRFIDTagUART()
{
    // Method outline
    // 1) Read until an STX byte is received
    // 2) Store any new data characters
    // 3) Upon ETX byte reception finalize the ID

    byte hex;

    _rfidIO->listen();

    if ( _rfidIO->available() )
    {
        // Upon STX character reception, it initializes parameters
        if ( ( hex = _rfidIO->read() ) == 0x02 )
        {
            _bytesRead = 0; 
    
            _tag.mfr = 0;
            _tag.id  = 0;
            _tag.chk = 0;
            _tag.valid = false;
            for (byte i = 0; i < 13; ++i) _tag.raw[i] = 0;
        
            _uartIdAvailable = false;
            
            #ifdef DEBUG
            Serial.println("\nSTART");
            #endif      
        }  // Upon ETX character reception, it finalizes ID
        else if ( hex == 0x03 )
        {
            byte checksum = 0;
        
            _tag.mfr = hex2dec(_tag.raw, 0, 3);
            _tag.id  = hex2dec(_tag.raw, 4, 9);
            _tag.chk = hex2dec(_tag.raw, 10, 11);

            // Do checksum calculation
            byte b;
            for (byte i = 0; i < 5; ++i)
            {
                b = 2 * i;
                checksum ^= hex2dec(_tag.raw, b, b+1);
            }
        
            #ifdef DEBUG
            Serial.println("VERIFICATION");
            Serial.print("  ID      : ");
            Serial.println(_tag.raw);
            Serial.print("  CHECKSUM: ");
            Serial.println(checksum, HEX);
            #endif
        
            if ( checksum == _tag.chk )
            {
                _tag.valid = true;

                #ifdef DEBUG
                Serial.println("VALID tag");
                #endif
            }

            _uartIdAvailable = true;

            #ifdef DEBUG
            Serial.println("END\n");
            #endif

        }  // Stores (12) data characters
        else if ( _bytesRead < 12 )
        {
            _tag.raw[_bytesRead++] = hex;

            #ifdef DEBUG
            Serial.print("VALUE: ");
            Serial.println(_tag.raw);
            #endif      
        }
    }

    return _uartIdAvailable;
}

// Parses a char array of hex characters to a
// decimal value (up to 8 bytes (16 hex characters))
// "243FB87D" (char[]: 8 Bytes) --> 0x243FB87D (long long: 4 Bytes)
// No "0x" prefix is necessary in the char array
long long RFID::hex2dec(char *hexCode, byte start, byte end)
{
    byte hex;  // Holds an ASCII character
    long long dec = 0;  // It will contain the result

    for (byte i = start; i < end+1; ++i)
    {
        hex = hexCode[i];
        hex -= 48;  // Transforms ASCII 0-9 to numbers 0-9
        if (hex > 9) hex -= 7;  // Transforms ASCII A-F to hex numbers A-F
        dec = (dec << 4) | hex;
    }

    #ifdef DEBUG
    Serial.print("PARSING \"");
    Serial.print(hexCode);
    Serial.print("\"[");
    Serial.print(start);
    Serial.print(',');
    Serial.print(end);
    Serial.print("]: \t");
    Serial.print((long)(dec>>32));
    Serial.println((long)dec);
    #endif

    return dec;
}

// Prepares an RFIDTag construct with a new ID
// and resets the interface parameters
void RFID::refreshWiegand()
{
    // noInterrupts();
    _wiegandIdAvailable = false;
    prepareRFIDTagWiegand();
    resetWiegand();
    // interrupts();
    
    #ifdef DEBUG
    Serial.println("getTag()");
    Serial.print("  MFR:\t");
    Serial.println(_tag.mfr, HEX);
    Serial.print("  ID:\t");
    Serial.println(_tag.id, HEX);
    Serial.print("  CHK:\t");
    Serial.println(_tag.chk, HEX);
    Serial.println(' ');
    #endif
}

// Resets the storing variables
void RFID::resetWiegand()
{
    _databits = 0;
    _bitCount = 0;
    _facilityCode = 0;
    _cardCode = 0;
}

// Manipulates the _databits received from the reader
// and sets the RFIDTag construct
void RFID::prepareRFIDTagWiegand()
{
    if ( _dataLen == W26BIT )
    {
        // This is not really correct
        // With only 3 bytes, we have to limit ourselves
        // to the card code 26 bit format
        /*
        // facility code = bits 1 to 8
        _facilityCode = ( _databits >> 1 ) & 0xFF;
        */

        // card code = bits 1 to 24
        _cardCode = ( _databits >> 1 ) & 0xFFFFFF;

        _tag.mfr = _facilityCode;
        _tag.id  = _cardCode;
        _tag.chk = 0;
        _tag.valid = checkParity26();

        #ifdef DEBUG
        Serial.print("\nFC = ");
        Serial.print(_facilityCode);
        Serial.print("\nCC = ");
        Serial.println(_cardCode);
        #endif
    }
    else if ( _dataLen == W35BIT )
    {
        // 35 bit HID Corporate 1000 format
        // For more info: http://www.pagemac.com/azure/data_formats.php
        // facility code = bits 2 to 13
        _facilityCode = ( _databits >> 2 ) & 0xFFF;
      
        // card code = bits 14 to 33
        _cardCode = ( _databits >> 14 ) & 0xFFFFF;
   
        _tag.mfr = _facilityCode;
        _tag.id  = _cardCode;
        _tag.chk = 0;
        _tag.valid = checkParity35();

        #ifdef DEBUG
        Serial.print("\nFC = ");
        Serial.print(_facilityCode);
        Serial.print("\nCC = ");
        Serial.println(_cardCode);   
        #endif
    }
}

// Checks the tag ID parities (W26BIT)
boolean RFID::checkParity26()
{
    // Isolates odd/even parity parts of the card code
    int oddCode = ( _databits >> 13 ) & 0x1FFF;
    int evenCode = _databits & 0x1FFF;

    byte oddCount = 0;
    byte evenCount = 0;
      
    for (byte i = 0; i < 13; ++i)
    {
        if ( oddCode & 0x01 ) oddCount++;
        oddCode >>= 1;
    }

    for (byte i = 0; i < 13; ++i)
    {
        if ( evenCode & 0x01 ) evenCount++;
        evenCode >>= 1;
    }

    // As far as I know, these parities (I receive from the reader) are reversed
    // But my hands are tied, I don't have the resources to test it further
    if ( oddCount % 2 == 0 && evenCount % 2 == 1 ) return true;
    else return false;
}

// Checks the tag ID parities (W35BIT). NOT TESTED!
boolean RFID::checkParity35()
{
    // Isolates parts of the card code
    long long parity1 = _databits;
    int parity2 = 0;
    int parity35 = _databits >> 34;
    byte base2 = 2, base35 = 1;
    for (byte i = 0; i < 11; ++i)
    {
        parity2 = ( parity2 << 2 ) | ( ( _databits >> base2 ) & 0x11 );
        parity35 = ( parity35 << 2 ) | ( ( _databits >> base35 ) & 0x11 );
        base2 += 3;
        base35 += 3;
    }
    parity2 = ( parity2 << 1 ) | ( _databits & 0x01 );

    byte p1OddCount = 0;
    byte p2EvenCount = 0;
    byte p35OddCount = 0;

    for (byte i = 0; i < 35; ++i)
    {
        if ( parity1 & 0x01 ) p1OddCount++;
        parity1 >>= 1;
    }

    for (byte i = 0; i < 23; ++i)
    {
        if ( parity2 & 0x01 ) p2EvenCount++;
        parity2 >>= 1;
    }

    for (byte i = 0; i < 23; ++i)
    {
        if ( parity35 & 0x01 ) p35OddCount++;
        parity35 >>= 1;
    }

    if ( p1OddCount % 2 == 0 && p2EvenCount % 2 == 1 && p35OddCount % 2 == 0 ) return true;
    else return false;
}

// INT0 callback function (bit 0)
void RFID::DATA0()
{
    _databits <<= 1;
    _bitCount++;

    if ( _bitCount == W26BIT && _dataLen == W26BIT )
        _wiegandIdAvailable = AVAILABLE;
    else if ( _bitCount == W35BIT && _dataLen == W35BIT )
        _wiegandIdAvailable = AVAILABLE;

    #ifdef DEBUG
    Serial.print('0');
    #endif
}

// INT1 callback function (bit 1)
void RFID::DATA1()
{
    _databits <<= 1;
    _databits |= 0x01;
    _bitCount++;

    if ( _bitCount == W26BIT && _dataLen == W26BIT )
        _wiegandIdAvailable = AVAILABLE;
    else if ( _bitCount == W35BIT && _dataLen == W35BIT )
        _wiegandIdAvailable = AVAILABLE;

    #ifdef DEBUG
    Serial.print('1');
    #endif
}
