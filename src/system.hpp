#ifndef _INCLUDE_SYSTEM_HPP_
#define _INCLUDE_SYSTEM_HPP_

// Specific Modem error masks (used for setting)
const int ERROR_MODEM_TURN_ON    = 1 << 0;
const int ERROR_MODEM_SIM_EXIST  = 1 << 1;
const int ERROR_MODEM_SIM_PIN    = 1 << 2;
const int ERROR_MODEM_SIGNAL     = 1 << 3;
const int ERROR_MODEM_REGISTER   = 1 << 4;
const int ERROR_MODEM_SMS_SEND   = 1 << 5;
const int ERROR_MODEM_UNKNOWN    = 1 << 6;
// All Modem errors mask (used for testing)
const int ERROR_MODEM = ERROR_MODEM_TURN_ON | ERROR_MODEM_SIM_EXIST | ERROR_MODEM_SIM_PIN | ERROR_MODEM_SIGNAL | ERROR_MODEM_REGISTER | ERROR_MODEM_SMS_SEND | ERROR_MODEM_UNKNOWN;

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


class system_class {
    private:
        int error_flags;
    public:
        system_class();
        void set_error(const int flag);
        void unset_error(const int flag);
        int test_error(const int flag);
        int get_error_flags();
};

extern system_class system_control;

#endif