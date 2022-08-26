#ifndef _INCLUDE_STORAGE_HPP_
#define _INCLUDE_STORAGE_HPP_

// Include general stuff
#include "helper_functions.hpp"

// Include Global header files needed
#include <Arduino.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>

// Define names for files and directories on SD card
#define DATA_DIR "DATA"

#define SETTINGS_FILE   DATA_DIR "/SETTINGS.BIN"
#define LOG_FILE        DATA_DIR "/LOGS.BIN"
#define USERS_FILE      DATA_DIR "/USERS.BIN"

// Definitions of setting IDs
enum setting_ids {
    SETTING_PASSWORD,         // 0 - PIN needed for some actions on panel
    SETTING_SIRENE_AUTH,      // 1 - boolean, request PIN to access sirene
    SETTING_SETTINGS_AUTH,    // 2 - boolean, request PIN to access settings
    SETTING_MOTD,             // 3 - string to be displayed as custom MOTD
    SETTING_NEXT_USER_ID,     // 4 - smallest not used user ID
    SETTING_LAST_LIGHT_STATE  // 5 - Last known state of light
};

// Reserved user IDs
enum reserved_user_ids {
    USER_DELETED,             // 0 - User that does not exist
    USER_PANEL,               // 1 - Action performed by panel
    USER_SERIAL,              // 2 - Action requested by serial command
    USER_LAST_RESEVED         // 3 - All IDs after this will be used for regular users
};

// Global variable for RTC manipulation
extern RtcDS1302<ThreeWire> rtc;

// Data type for storing system settings on SD card
struct setting_record {
    setting_ids id;
    int int_value;
    char string_value[30];
};
// Data type for storing users on SD card
struct user_record {
    int id;
    int active;
    char number[16];
};
// Data type for storing system logs on SD card
struct log_record {
    int user_id;
    char action[4];
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint16_t year;
};

// Function returns pointer to string, string will contain formated
// information about this page of logs, there are 4 logs on each page
// if there are no logs for that page, string will be empty
const char * get_log_page(unsigned long page);

class storage_class {
    private:
        // something
    public:
        // Init storage class
        void init();

        // Get setting structure for given setting ID
        struct setting_record get_setting(setting_ids setting_id);
        // Set string part for setting of given ID
        void set_setting(setting_ids setting_id, const char setting_str[]);
        // Set int part for setting of given ID
        void set_setting(setting_ids setting_id, const int setting_int);

        // Log string for given user ID
        void log_this(int user_id, const char * log_string);
        // Clear all log records
        void clear_log();
        // Get number of log records
        unsigned long get_log_count();
        // Get number of log records for specific date
        unsigned long get_log_count(uint8_t day, uint8_t month, uint16_t year);
        // Get log record
        // position -- number of records to go into past, where 0 is last record
        struct log_record get_log(unsigned long position);
        // Get log record for specific date
        // position -- number of records to go into past, where 0 is last record
        struct log_record get_log(unsigned long position, uint8_t day, uint8_t month, uint16_t year);

        // Add user to user file
        int add_user(const char number[]);
        // Set number for user with given ID
        void edit_user(const int id, const char number[]);
        // Disable user with given id
        void dis_user(const int id);
        // Enable user with given id
        void enb_user(const int id);
        // Count how many users are in users file
        int get_user_count();
        // Delete all users from user file
        void clear_user_file();
        // Get user with given id
        struct user_record get_user_by_id(const int id);
        // Get user with given number
        struct user_record get_user_by_num(const char number[]);
        // Get user by position in the file, where 0 is first user
        struct user_record get_user_by_pos(const int position);
        // Delete user from file permanently
        void delete_user(int id);
};

extern storage_class storage;

#endif