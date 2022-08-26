// Include global header files
#include <Arduino.h>
// Include local header files
#include "sms_pdu.hpp"

/********************************************************************
 * Function prototypes                                              *
 ********************************************************************/
int ch_to_int(const char ch);
char int_to_ch(const char nm);
void encode(const char ascii[], char encoded[]);
void decode(const char encoded[], char result[]);
char ascii_to_gsm(char ascii_ch);
char gsm_to_ascii(char gsm_ch);

/********************************************************************
 * Functions for number/character conversion                        *
 ********************************************************************/

// Convert single HEX character to int value of it
int ch_to_int(const char ch) {
    if (ch >= '0' && ch <= '9') {
        return ch - '0';
    }
    if (ch >= 'A' && ch <= 'F') {
        return (ch - 'A') + 10;
    }
    return 0;
}
// Convert value in range of HEX digit to character
char int_to_ch(const char nm) {
    if (nm < 10) {
        return nm + '0';
    }
    if (nm < 16) {
        return (nm - 10) + 'A';
    }
    return '0';
}

/********************************************************************
 * 7-bit Encoding GSM character encoding functions                  *
 ********************************************************************/

// Encode ASCII zero terminated string to GSM 7-bit
//    ascii   -- string to be encoded
//    encoded -- array where to place result
void encode(const char ascii[], char encoded[]) {
    int ordinal;   // Ordinal number of GSM character, counts form 0 to 6
    int ch;        // encoded[] array index
    int current;   // Current octet
    int i;         // ascii[] array index

    ordinal = 0;   // Set ordinal to 0
    ch = 0;        // Start from first element of encoded array

    // For each ASCII character
    for (i = 0; i == 0 || ascii[i] != '\0'; i++) {
        // If ordinal is equal to 7 skip character and set ordinal to 0
        if (ordinal == 7) {
            ordinal = 0;
            continue;
        }

        // Convert ASCII character to GSM character
        // Take first (7 - ordinal) bin digits from left, and put them in current
        current = ascii_to_gsm(ascii[i]) >> ordinal;
        // Take first (ordinal + 1) digits from the right of next character and add them ti the left of current
        current = current | ( ascii_to_gsm(ascii[i + 1]) & ~(~0 << (ordinal + 1)) ) << (7 - ordinal);
        // Convert current octet to ASCII characters and save them to encoded array
        encoded[ch++] = int_to_ch(current >> 4);
        encoded[ch++] = int_to_ch(current & ~(~0 << 4));

        ++ordinal;  // Increase ordinal
    }
    // End encoded character array
    encoded[ch] = '\0';
}

// Decode SMS PDU encoded by GSM 7 bit
//    encoded -- string to be decoded
//    result  -- array where result is to be placed
void decode(const char encoded[], char result[]) {
    int ordinal;   // Ordinal number of current GMS character counts from 1 to 7
    int cr_ch;     // Current character in result array
    int current;   // Current octet
    int previous;  // Previous octet
    int i;         // Index counter of encoded character array

    cr_ch = 0;     // Set index to 0
    ordinal = 1;   // Start from 1 ordinal number
    current = 0;   // Set current to 0 to prevent problems

    // For each character of encoded array
    for (i = 0; encoded[i] != '\0'; i++) {
        // If this is first digit of octet
        if (i % 2 == 0) {
            // Set previous to current octet
            previous = current;
            // Place first digit to it's place in new octet
            current = ch_to_int(encoded[i]) << 4;
        // If this is second digit of octet
        } else {
            // Place second character to it's place
            current = current | ch_to_int(encoded[i]);

            // Remove binary digits from the left by the value of ordinal
            result[cr_ch] = current & ( ~((~0u) << (8 - ordinal)) );
            // Shift it to the left by the number of digits removed on last octet
            result[cr_ch] = result[cr_ch] << (ordinal - 1);
            // Take removed digits from previous octet and add it to the right of new one
            result[cr_ch] = result[cr_ch] | (previous >> (9 - ordinal));
            // Convert GSM character to ASCII
            result[cr_ch] = gsm_to_ascii(result[cr_ch]);

            ++cr_ch;    // Increase Index
            ++ordinal;  // Increase ordinal
            
            // If this is 7th octet
            if (ordinal == 8) {
                // Set new character to digits removed from current octet
                result[cr_ch] = current >> 1;
                // Convert character to ASCII
                result[cr_ch] = gsm_to_ascii(result[cr_ch]);
                ordinal = 1;   // Start from 1 again
                ++cr_ch;       // Go to next position in result array
            }
        }
    }
    // End result character array
    result[cr_ch] = '\0';
}

// Convert ASCII character value to GSM character
char ascii_to_gsm(char ascii_ch) {
    int i;
    char ch;

    if (ascii_ch == ' ')   // If it's space return space
        return 0x20;

    // Else search for right character
    for (i = 0; i <= 0x7F; i++) {
        ch = gsm_to_ascii(i);
        // If this is the one return it
        if (ch == ascii_ch)
            return i;
    }

    return 0x00;  // If character is not found return 0 (@)
}

// Convert GSM character value to ASCII
char gsm_to_ascii(char gsm_ch) {
    switch (gsm_ch) {
        case 0x00 : return '@'  ;
        case 0x01 : return ' '  ; // £
        case 0x02 : return '$'  ;
        case 0x03 : return ' '  ; // ¥
        case 0x04 : return ' '  ; // è
        case 0x05 : return ' '  ; // é
        case 0x06 : return ' '  ; // ù
        case 0x07 : return ' '  ; // ì
        case 0x08 : return ' '  ; // ò
        case 0x09 : return ' '  ; // Ç
        case 0x0A : return '\n' ;
        case 0x0B : return ' '  ; // Ø
        case 0x0C : return ' '  ; // ø
        case 0x0D : return '\r' ;
        case 0x0E : return ' '  ; // Å
        case 0x0F : return ' '  ; // å
        case 0x10 : return ' '  ; // Δ
        case 0x11 : return '_'  ;
        case 0x12 : return ' '  ; // Φ
        case 0x13 : return ' '  ; // Γ
        case 0x14 : return ' '  ; // Λ
        case 0x15 : return ' '  ; // Ω
        case 0x16 : return ' '  ; // Π
        case 0x17 : return ' '  ; // Ψ
        case 0x18 : return ' '  ; // Σ
        case 0x19 : return ' '  ; // Θ
        case 0x1A : return ' '  ; // Ξ
        case 0x1B : return '\e' ;
        case 0x1C : return ' '  ; // Æ
        case 0x1D : return ' '  ; // æ
        case 0x1E : return ' '  ; // ß
        case 0x1F : return ' '  ; // É
        case 0x20 : return ' '  ;
        case 0x21 : return '!'  ;
        case 0x22 : return '"'  ;
        case 0x23 : return '#'  ;
        case 0x24 : return ' '  ; // ¤
        case 0x25 : return '%'  ;
        case 0x26 : return '&'  ;
        case 0x27 : return '\'' ;
        case 0x28 : return '('  ;
        case 0x29 : return ')'  ;
        case 0x2A : return '*'  ;
        case 0x2B : return '+'  ;
        case 0x2C : return ','  ;
        case 0x2D : return '-'  ;
        case 0x2E : return '.'  ;
        case 0x2F : return '/'  ;
        case 0x30 : return '0'  ;
        case 0x31 : return '1'  ;
        case 0x32 : return '2'  ;
        case 0x33 : return '3'  ;
        case 0x34 : return '4'  ;
        case 0x35 : return '5'  ;
        case 0x36 : return '6'  ;
        case 0x37 : return '7'  ;
        case 0x38 : return '8'  ;
        case 0x39 : return '9'  ;
        case 0x3A : return ':'  ;
        case 0x3B : return ';'  ;
        case 0x3C : return '<'  ;
        case 0x3D : return '='  ;
        case 0x3E : return '>'  ;
        case 0x3F : return '?'  ;
        case 0x40 : return ' '  ; // ¡
        case 0x41 : return 'A'  ;
        case 0x42 : return 'B'  ;
        case 0x43 : return 'C'  ;
        case 0x44 : return 'D'  ;
        case 0x45 : return 'E'  ;
        case 0x46 : return 'F'  ;
        case 0x47 : return 'G'  ;
        case 0x48 : return 'H'  ;
        case 0x49 : return 'I'  ;
        case 0x4A : return 'J'  ;
        case 0x4B : return 'K'  ;
        case 0x4C : return 'L'  ;
        case 0x4D : return 'M'  ;
        case 0x4E : return 'N'  ;
        case 0x4F : return 'O'  ;
        case 0x50 : return 'P'  ;
        case 0x51 : return 'Q'  ;
        case 0x52 : return 'R'  ;
        case 0x53 : return 'S'  ;
        case 0x54 : return 'T'  ;
        case 0x55 : return 'U'  ;
        case 0x56 : return 'V'  ;
        case 0x57 : return 'W'  ;
        case 0x58 : return 'X'  ;
        case 0x59 : return 'Y'  ;
        case 0x5A : return 'Z'  ;
        case 0x5B : return ' '  ; // Ä
        case 0x5C : return ' '  ; // Ö
        case 0x5D : return ' '  ; // Ñ
        case 0x5E : return ' '  ; // Ü
        case 0x5F : return ' '  ; // §
        case 0x60 : return ' '  ; // ¿
        case 0x61 : return 'a'  ;
        case 0x62 : return 'b'  ;
        case 0x63 : return 'c'  ;
        case 0x64 : return 'd'  ;
        case 0x65 : return 'e'  ;
        case 0x66 : return 'f'  ;
        case 0x67 : return 'g'  ;
        case 0x68 : return 'h'  ;
        case 0x69 : return 'i'  ;
        case 0x6A : return 'j'  ;
        case 0x6B : return 'k'  ;
        case 0x6C : return 'l'  ;
        case 0x6D : return 'm'  ;
        case 0x6E : return 'n'  ;
        case 0x6F : return 'o'  ;
        case 0x70 : return 'p'  ;
        case 0x71 : return 'q'  ;
        case 0x72 : return 'r'  ;
        case 0x73 : return 's'  ;
        case 0x74 : return 't'  ;
        case 0x75 : return 'u'  ;
        case 0x76 : return 'v'  ;
        case 0x77 : return 'w'  ;
        case 0x78 : return 'x'  ;
        case 0x79 : return 'y'  ;
        case 0x7A : return 'z'  ;
        case 0x7B : return ' '  ; // ä
        case 0x7C : return ' '  ; // ö
        case 0x7D : return ' '  ; // ñ
        case 0x7E : return ' '  ; // ü
        case 0x7F : return ' '  ; // à

        default   : return ' '  ;
    }
}

/********************************************************************
 * SMS PDU builder functions                                        *
 ********************************************************************/

builder::builder() {
    number[0] = '\0';
    message[0] = '\0';
}

void builder::set_number(const char num[]) {
    int i;
    for (i = 0; num[i] != '\0' && i < 160; i++) {
        number[i] = num[i];
    }
    number[i] = '\0';
}

void builder::set_number(const __FlashStringHelper *num) {
    unsigned int address = (unsigned int)num;     // Get address in flash
    unsigned int i = 0;                           // Character counter

    while (1) {
        // Check for buffer overflow
        if (i >= 160) {
            message[i] = '\0';
            break;
        }
        // Get current character from flash
        char ch = pgm_read_byte_near(address + i);
        // Add read character to number array
        number[i] = ch;
        // If this is last character it's done
        if (ch == '\0') break;
        // Else go to next character
        ++i;
    }
}

void builder::set_message(const char msg[]) {
    int i;
    for (i = 0; msg[i] != '\0' && i < 160; i++) {
        message[i] = msg[i];
    }
    message[i] = '\0';
}

void builder::set_message(const __FlashStringHelper *msg) {
    unsigned int address = (unsigned int)msg;     // Get address in flash
    unsigned int i = 0;                           // Character counter

    while (1) {
        // Check for buffer overflow
        if (i >= 160) {
            message[i] = '\0';
            break;
        }
        // Get current character from flash
        char ch = pgm_read_byte_near(address + i);
        // Add read character to message array
        message[i] = ch;
        // If this is last character it's done
        if (ch == '\0') break;
        // Else go to next character
        ++i;
    }
}

void builder::calculate() {
    int i = 0;    // Index counter of encoded array
    int j;        // Array index multiple uses
    int length;   // String length multiple uses
    char temp;    // Temp variable for character switching
    int max_length;
    
    // Get length of phone number
    length = strlength(number);
    // Add F to the and if number length is not even
    if (length % 2 != 0) {
        number[length++] = 'F';
        number[length] = '\0';
    }
    // Reverse digits in number
    for (j = 0; number[j] != '\0'; j += 2) {
        temp = number[j];
        number[j] = number[j + 1];
        number[j + 1] = temp;
    }

    // Set 1st octet to 0x00 - SMSC stored in phone is used
    encoded[i++] = '0';
    encoded[i++] = '0';
    // Set 2nd octet to 0x11 - SMS-SUBMIT message
    encoded[i++] = '1';
    encoded[i++] = '1';
    // Set 3rd octet to 0x00 - allow phone to set reference number
    encoded[i++] = '0';
    encoded[i++] = '0';
    // Set length of phone number
    encoded[i++] = int_to_ch(length >> 4);
    encoded[i++] = int_to_ch(length & ~(~0 << 4));
    // Set Type-of-Address to international format of phone number
    encoded[i++] = '9';
    encoded[i++] = '1';
    // Copy phone number octets
    for (j = 0; number[j] != '\0'; j++)
        encoded[i++] = number[j];
    // Set TP-PID protocol identifier
    encoded[i++] = '0';
    encoded[i++] = '0';
    // Set Data coding scheme to 7-bit alphabet
    encoded[i++] = '0';
    encoded[i++] = '0';
    // Set TP-Validity-Period to 4 days
    encoded[i++] = 'A';
    encoded[i++] = 'A';
    // Calculate max length of message that can fit in buffer
    encoded[i] = '\0';
    max_length = ((BUILDER_BUFFER - strlength(encoded)) / 2 / 7 * 8) + ((BUILDER_BUFFER - strlength(encoded)) / 2 % 7);
    // If message could be larger than max length, shrink message
    if (max_length < 170) {
        message[max_length - 8] = '\0';
    }
    // Set Length of message
    length = strlength(message);
    encoded[i++] = int_to_ch(length >> 4);
    encoded[i++] = int_to_ch(length & ~(~0 << 4));
    // Encode message
    encode(message, encoded + i);
}

const char * builder::get_pdu() {
    return encoded;
}

int builder::get_tpdu_length() {
    return (strlength(encoded) / 2) - 1;
}

/********************************************************************
 * SMS PDU parser functions                                         *
 ********************************************************************/

parser::parser() {
    number[0] = '\0';
    message[0] = '\0';
}

void parser::set_pdu(const char encoded[]) {
    int i;        // Index of encoded array
    int j;        // Index counter - multiple uses
    int length;   // Length of current information
    char temp;    // Temp variable for character switching

    i = 0;

    // Get length of SMSC information
    length = ch_to_int(encoded[i++]) << 4;
    length = length | ch_to_int(encoded[i++]);
    // Skip SMSC information (We don't need it)
    i += length * 2;
    // Store Type of PDU
    type_of_message = ch_to_int(encoded[i++]) << 4;
    type_of_message = type_of_message | ch_to_int(encoded[i++]);
    // Get length of sender number
    length = ch_to_int(encoded[i++]) << 4;
    length = length | ch_to_int(encoded[i++]);
    // Store type of sender number
    type_of_address = ch_to_int(encoded[i++]) << 4;
    type_of_address = type_of_address | ch_to_int(encoded[i++]);
    // Store sender number
    for (j = 0; j < length; j++)
        number[j] = encoded[i++];
    // End number array
    number[j] = '\0';
    // Reverse digits in right order
    for (j = 0; number[j] != '\0'; j += 2) {
        temp = number[j];
        number[j] = number[j + 1];
        number[j + 1] = temp;
    }
    // Remove F from the end of number if it exist
    if (number[length - 1] == 'F')
        number[length - 1] = '\0';
    // Store protocol identifier
    protocol_identifier = ch_to_int(encoded[i++]) << 4;
    protocol_identifier = protocol_identifier | ch_to_int(encoded[i++]);
    // Store data coding scheme
    data_coding_scheme = ch_to_int(encoded[i++]) << 4;
    data_coding_scheme = data_coding_scheme | ch_to_int(encoded[i++]);
    // Skip time stamp (We don't need it)
    i += 14;    // 14 is length of timestamp
    // Skip length of message (We don't need it)
    i += 2;     // 2 because we need to skip 1 octet (two characters)

    if (data_coding_scheme == 0x00) {           // If its 7-bit encoded
        decode(encoded + i, message);           // Decode message
    } else {
        for (j = 0; encoded[i] != '\0'; j++) {  // Else copy message as it is
            message[j] = encoded[i++];
        }
        message[j] = '\0';                      // End message array
    }
}

const char * parser::get_message() {
    return message;
}

const char * parser::get_number() {
    return number;
}

