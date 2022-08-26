#ifndef _INCLUDE_SMS_PDU_HPP_
#define _INCLUDE_SMS_PDU_HPP_

// Include general stuff
#include "helper_functions.hpp"

// How many characters can fit on builder pdu buffer
#define BUILDER_BUFFER 300

// Builder class used to calculate SMS pdu for given number and message
// this pdu can than be send by modem in pdu mode
class builder {
    private:
        char number[20];               // Receiver number
        char message[170];             // Message to be encoded
        char encoded[BUILDER_BUFFER];  // Calculated PDU
    public:
        // Default constructor
        builder();
        // Set receiver number to put in PDU
        void set_number(const char num[]);
        // Set receiver number but number is stored in FLASH
        void set_number(const __FlashStringHelper *num);
        // Set message to be encoded in PDU
        void set_message(const char msg[]);
        // Set message to be encoded bit message is stored in FLASH
        void set_message(const __FlashStringHelper *msg);
        // Calculate PDU for given information
        void calculate();
        // Return pointer to calculated PDU
        const char * get_pdu();
        // Return length of TPDU (needed for send command)
        int get_tpdu_length();
};

// Parser class used to get number and message from SMS pdu which
// arrived from modem over Serial3
class parser {
    private:
        char number[20];           // Number parsed from PDU
        char message[170];         // Message decoded from PDU
        int type_of_address;       // Type of PDU sender address
        int data_coding_scheme;    // Data coding scheme of PDU data (message)
        int protocol_identifier;   // Protocol identifier of PDU
        int type_of_message;       // Type of SMS message
    public:
        // Default constructor
        parser();
        // Decode given PDU string
        void set_pdu(const char encoded[]);
        // Get sender number of given pdu
        const char * get_number();
        // Get message decoded from given PDU
        const char * get_message();
};

#endif