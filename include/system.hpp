#ifndef _INCLUDE_SYSTEM_HPP_
#define _INCLUDE_SYSTEM_HPP_

// Include general stuff
#include "helper_functions.hpp"

// Current version of firmware
#define FIRMWARE_VERSION "1.0.0"

// How many ms should beep last
#define BEEP_DURITATION 100
// Pin Where buzzer is connected
#define BUZZER_PIN 7
// Pin where Ready indicator is connected
#define READY_LED_PIN 8
// Size of console buffer, how many characters can fit in console buffer
#define CONSOLE_BUFFER_SIZE 50

// Specific Modem error masks (used for setting)
const int ERROR_MODEM_TURN_ON    = 1 << 0;
const int ERROR_MODEM_TIMEOUT    = 1 << 1;
const int ERROR_MODEM_SIM        = 1 << 2;
const int ERROR_MODEM_SIGNAL     = 1 << 3;
const int ERROR_MODEM_REGISTER   = 1 << 4;
const int ERROR_MODEM_SMS_SEND   = 1 << 5;
const int ERROR_MODEM_UNKNOWN    = 1 << 6;
// All Modem errors mask (used for testing)
const int ERROR_MODEM = ERROR_MODEM_TURN_ON | ERROR_MODEM_TIMEOUT | ERROR_MODEM_SIM | ERROR_MODEM_SIGNAL | ERROR_MODEM_REGISTER | ERROR_MODEM_SMS_SEND | ERROR_MODEM_UNKNOWN;

// Specific SD card error masks (used for setting)
const int ERROR_SD_INIT          = 1 << 7;
const int ERROR_SD_READ          = 1 << 8;  
const int ERROR_SD_WRITE         = 1 << 9;
const int ERROR_SD_UNKNOWN       = 1 << 10;
// All SD card errors mask (used for testing)
const int ERROR_SD = ERROR_SD_INIT | ERROR_SD_READ | ERROR_SD_WRITE | ERROR_SD_UNKNOWN;

// Specific RTC error masks (used for setting)
const int ERROR_RTC_CONFIDENCE   = 1 << 11;
const int ERROR_RTC_UNKNOWN      = 1 << 12;
// All RTC errors mask (used for testing)
const int ERROR_RTC = ERROR_RTC_CONFIDENCE | ERROR_RTC_UNKNOWN;

// Specific Light error masks (used for setting)
const int ERROR_LIGHT_UNKNOWN    = 1 << 13;
// All Light errors mask (used for testing)
const int ERROR_LIGHT = ERROR_LIGHT_UNKNOWN;

// Mask to test for any error
const int ERROR = ~0 >> 1;

class commands {
    private:
        char buffer[CONSOLE_BUFFER_SIZE];  // Array of characters received since laste \r
        int counter;                       // Current character in the buffer
        int line_ready;                    // Is 1 if line is ready to be processed (Enter is pressed)
    public:
        // Default constructor
        commands();
        // Initilize serial
        void init();
        // Get new characters from serial if there are any
        void update();
        // Clear buffer so it can wait for new characters
        void clear();
        // Check if entered command is ready to be processed
        int ready();
        // Ger pointer to buffer
        char * get();
};

class system_class {
    private:
        // Error handling
        int error_flags;                // Variable to hold system error flags
        // Beep
        unsigned long beep_start;       // millis() when beep started
        unsigned long beep_wait_start;  // millis() when beep delay started
        unsigned int beep_waiting_time; // Number of ms to wait before first beep
        int beep_counter;               // How many beeps should be done
        int beep_after_wait_counter;    // How many beeps should be done after delay
        // Console
        commands command;
    public:
        // Default constructor
        system_class();
        // Init system stuff
        void init();
        // Set specific error flag
        void set_error(const int flag);
        // Unset specific error flag
        void unset_error(const int flag);
        // Test if error flag has been set using mask
        // Returns: 1 if flag is set, or 0 if not
        int test_error(const int flag);
        // Get variable containing error flags
        int get_error_flags();

        // Make beep sound
        void beep(int how_many_times);
        // Wait for beep_delay ms and then make beep sound
        void beep(unsigned int beep_delay, int how_many_times);

        // Run periodic stuff
        void update();

        // Set state of Ready indicator
        // state -- 1 to turn on indicator, or 0 to turn it off
        void ready(int state);
};

extern system_class system_control;

#endif