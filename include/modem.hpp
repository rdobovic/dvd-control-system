#ifndef _INCLUDE_MODEM_HPP_
#define _INCLUDE_MODEM_HPP_

// Include general stuff
#include "helper_functions.hpp"

// Define MODEM_DEBUG to turn on debug messages on Serial
#undef MODEM_DEBUG

// Pin which turns on modem
#define MODEM_POWER_PIN 9
// How long to wait for command anwser before reporting error (in ms)
#define MODEM_RESPONSE_WAIT 90000
// Max number of characters to fit in input buffer
#define BUFFER_SIZE 300
// Interval on which system should check if modem is OK (in ms)
#define READY_CHECK_INTERVAL 60000
// How long to wait before first ready check (in ms)
#define FIRST_READY_CHECK_WAIT 30000
// How many messages should fit into SMS buffer before sending
#define SMS_BUFFER_SIZE 3
// Max number of commands that can be put in command queue
#define CMD_BUFFER_SIZE 10
// If set to 1 modem will reply with sms message when executing sms command
#define SMS_REPLY 1
// Number of logs on log page send in sms message
#define SMS_LOG 3

// Template for indicators send by modem
class unsolicited_response {
    private:
        // Array to store string with which respons starts
        char res_starts_with[40];
    protected:
        // Set string with which response starts
        void set_start(const char start[]);
    public:
        // 1 if handler is done, 0 if it's not
        int is_done;
        // Default constructor
        unsolicited_response();
        // Test if given response belongs to this class
        int test(const char response[]);
        // Check if this handler is done taking input
        int done();
        // Execute code to handle response
        virtual void execute(const char response[]) {}
};

// Template for commands send to modem to perform an action or request data
class at_command {
    private:
        unsigned long execution_start;  // millis() when command execution started
    protected:
        int is_done;                    // 1 if command execution is done 0 if not
        // Send command to serial
        virtual void send_to_serial() {}
    public:
        // Default constructor
        at_command();
        // Returns 1 if command is done executing, or 0 if not
        int done();
        // Start command execution
        void execute();
        // Push command result to the command so ot can be handled
        virtual void push_line(const char line[]) {}
        // If command needs to update in each cicle update it, else skip update
        virtual void update() {}
};

// Main class used for executing commands and manipulationg with modem
class modem_manipulation {
    private:
        char buffer[BUFFER_SIZE];                // Array to store characters arrived over serial
        int current_ch;                          // Current index of buffer array
        int cmd_getter;                          // Index where next cmd should be taken from cmd_buffer
        int cmd_setter;                          // Index where next cmd should be stored in cmd_buffer
        at_command *current_cmd;                 // Pointer to current command
        at_command *cmd_buffer[CMD_BUFFER_SIZE]; // Pointers to commands waiting to be executed
        int handler_count;                       // Counts how many handlers has been added
        unsolicited_response *handlers[10];      // Array of handlers for unsolicited responses
        unsigned long check_start;               // When was last modem OK check performed
    public:
        // Default constructor
        modem_manipulation();
        // Initilize all nececery components
        void init();
        // Initiate modem startup
        void start();
        // Run dynamic actions
        void update();
        // Run specific command
        void run_cmd(at_command &cmd);
        // Add unsolicited response handler
        void add_handler(unsolicited_response &handler);
};

extern modem_manipulation modem;

// Modem command used to check if modem is on and start it if it's not
class startup_cmd : public at_command {
    private:
        enum stages {
            WAITING_FOR_CMD_RES,  // Wait for res after sending AT to see if modem is ON
            WAITING_TO_TURN_ON,   // Wait for modem to turn on
            WAITING_TO_POWER,     // Wait before turning off modem power switch
            EVERYTHING_IS_DONE    // Everything went OK and modem is ON
        } current_stage;

        int start_counter;        // How many times functions tried power up modem
        unsigned long res_wait;   // millis() when waiting started
        void send_to_serial();    // Send command to the serial and initiate waiting
    public:
        // Default constructor
        startup_cmd();
        // Handle response from serial
        void push_line(const char line[]);
        // Update waiting status, and do nececery actions
        void update();
};

// Modem command used to check status of modem
// basicly checks: SIM card, mobile signal, and if modem is connected to network
class check_cmd : public at_command {
    private:
        enum responses {
            COMMAND_ECHO,        // Waiting to receive command echo
            SIM_CARD_ID,         // Waiting to receive SIM card ID, or error
            SIM_CARD_PIN,        // Waiting to receive SIM card PIN status
            SIGNAL_QUALITY,      // Waiting to receive signal quality report
            REGISTRATION_STATUS, // Waiting to receive registration status
            FINAL_OK             // Waiting to receive final OK
        } currently_waiting;

        void send_to_serial();   // Send command to serial
    public:
        // Default constructor
        check_cmd();
        // Handle response from serial
        void push_line(const char line[]);
};

// Modem command used to send sms messages
// message is send by adding message and recipient to queue using add_message()
// function, and then executing command using run_cmd() of modem class
class sms_cmd : public at_command {
    private:
        enum responses {
            COMMAND_ECHO,    // Waiting to receive command echo
            PDU_ECHO,        // Waiting to receive PDU echo
            MSG_INFO,        // Information about message send
            FINAL_OK         // Waiting to receive final OK
        } currently_waiting;

        char pdu[SMS_BUFFER_SIZE][BUFFER_SIZE]; // SMS PDU buffer waiting to be send
        int tpdu_length[SMS_BUFFER_SIZE];       // Size of TPDU for each PDU in the queue
        int getter;                             // Index where is next pdu to read or -1 if there is nothing to read
        int setter;                             // Index where next pdu is to be placed

        void send_to_serial();                  // Send command to serial
    public:
        // Default constructor
        sms_cmd();
        // Add new message to queue
        void add_message(const char number[], const char message[]);
        // Add new message to queue, but message is stored in flash
        void add_message(const char number[], const __FlashStringHelper *message);
        // Clear message queue
        void clear();
        // Handle response from serial
        void push_line(const char line[]);
};

extern sms_cmd sms_modem;

// Modem command used to pause execution of any command executed after it
// to specify number of ms execution should be paused use delay() function
class delay_cmd : public at_command {
    private:
        unsigned long delay_start;        // millis() when execution has started
        unsigned long delay_duritation;   // how many ms execution should be paused

        void send_to_serial();            // Start delay
    public:
        // Default constructor
        delay_cmd();
        // Set how many ms pause should take
        void set_delay(unsigned long delay_dur);
        // Handle response from serial (There should not be any)
        void push_line(const char line[]);
        // Check if enough time has passed if thats true finish
        void update();
};

// Modem command used to perform initial configuration
class config_cmd : public at_command {
    private:
        enum responses {
            COMMAND_ECHO,          // Waiting to receive command echo
            FINAL_OK               // Waiting to receive final OK
        } currently_waiting;

        void send_to_serial();     // Send command to serial
    public:
        // Default constructor
        config_cmd();
        // Handle response from serial
        void push_line(const char line[]);
};

// Unsolicited response handler, activated when new sms message arrives
class delivery_res : public unsolicited_response {
    public:
        // Default constructor
        delivery_res();
        // Execute code to handle response
        void execute(const char response[]);
};

// Unsolicited response handler, activated when modem rings
class ring_res : public unsolicited_response {
    public:
        // Default constructor
        ring_res();
        // Execute code to handle response
        void execute(const char response[]);
};

// Unsolicited response handler, activated when modem ended ringing
class ring_end_res : public unsolicited_response {
    public:
        // Default constructor
        ring_end_res();
        // Execute code to handle response
        void execute(const char response[]);
};

#endif