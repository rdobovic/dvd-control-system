/********************************************************************
 * Connections of RTC pins to Arduino Mega pins:                    *
 *     DS1302 CLK/SCLK   --> 5                                      *
 *     DS1302 DAT/IO     --> 4                                      *
 *     DS1302 RST/CE     --> 3                                      *
 ********************************************************************/
#include <SPI.h>
#include <SD.h>

#include "storage.hpp"
#include "system.hpp"
#include "helper_functions.hpp"

#define DATA_DIR "DATA"

#define SETTINGS_FILE   DATA_DIR "/SETTINGS.BIN"
#define LOG_FILE        DATA_DIR "/LOGS.BIN"
#define USERS_FILE      DATA_DIR "/USERS.BIN"

ThreeWire myWire(4,5,3);
RtcDS1302<ThreeWire> rtc(myWire);

storage_class storage;

/********************************************************************
 * Functions for initialization of necessary components             *
 ********************************************************************/

void init_rtc() {
    rtc.Begin();

    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);

    if (!rtc.IsDateTimeValid()) {
        rtc.SetDateTime(compiled);
    }
    if (rtc.GetIsWriteProtected()) {
        rtc.SetIsWriteProtected(false);
    }
    if (!rtc.GetIsRunning()) {
        rtc.SetIsRunning(true);
    }

    RtcDateTime now = rtc.GetDateTime();

    if (now < compiled) {
        rtc.SetDateTime(compiled);
    }
}

void init_sd() {
    if (!SD.begin()) {
        system_control.set_error(ERROR_SD_INIT);
    }
    pinMode(53, OUTPUT);
    digitalWrite(53, LOW);
}

void storage_class::init() {
    // Initilize dependencies
    init_sd();
    init_rtc();

    // Check if data dir exist
    if (!system_control.test_error(ERROR_SD) && !SD.exists(DATA_DIR)) {
        if (!SD.mkdir(DATA_DIR)) {
            system_control.set_error(ERROR_SD_WRITE);
        }
    }

    // Check if settings file exist
    if (!system_control.test_error(ERROR_SD) && !SD.exists(SETTINGS_FILE)) {
        File settings_file = SD.open(SETTINGS_FILE, (O_CREAT | O_READ | O_WRITE));
        if (!settings_file) {
            system_control.set_error(ERROR_SD_WRITE);
            return;
        }
        settings_file.close();

        set_setting(SETTING_PASSWORD, "0000");
        set_setting(SETTING_SIRENE_AUTH, 0);
        set_setting(SETTING_SETTINGS_AUTH, 0);
        set_setting(SETTING_MOTD, "");
        set_setting(SETTING_NEXT_USER_ID, USER_LAST_RESEVED + 1);
    }
}

/********************************************************************
 * Functions for system settings                                    *
 ********************************************************************/

struct setting_record storage_class::get_setting(setting_ids setting_id) {
    if (system_control.test_error(ERROR_SD_INIT | ERROR_SD_READ | ERROR_SD_UNKNOWN))
        return setting_record {};

    unsigned long i;
    struct setting_record setting;

    File settings_file = SD.open(SETTINGS_FILE, (O_READ | O_WRITE | O_CREAT));
    if (!settings_file) {
        system_control.set_error(ERROR_SD_READ);
        return setting_record {};
    }
    settings_file.seek(0);

    for (i = 0; i < settings_file.size(); i += sizeof(setting)) {
        settings_file.read((byte*)&setting, sizeof(setting));

        if (setting.id == setting_id) {
            settings_file.close();
            return setting;
        }
    }
    settings_file.close();
    return setting_record {};
}

void storage_class::set_setting(setting_ids setting_id, const char setting_str[]) {
    if (system_control.test_error(ERROR_SD)) return;

    unsigned long i;
    struct setting_record setting;

    File settings_file = SD.open(SETTINGS_FILE, (O_READ | O_WRITE | O_CREAT));
    if (!settings_file) {
        system_control.set_error(ERROR_SD_WRITE);
        return;
    }
    settings_file.seek(0);

    for (i = 0; i < settings_file.size(); i += sizeof(setting)) {
        settings_file.read((byte*)&setting, sizeof(setting));

        if (setting.id == setting_id){
            strcopy(setting_str, setting.string_value, 30);
            settings_file.seek(i);
            settings_file.write((byte*)&setting, sizeof(setting));
            settings_file.close();
            return;
        }
    }
    settings_file.seek(settings_file.size());
    setting.id = setting_id;
    strcopy(setting_str, setting.string_value, 30);
    settings_file.write((byte*)&setting, sizeof(setting));
    settings_file.close();
}

void storage_class::set_setting(setting_ids setting_id, const int setting_int) {
    if (system_control.test_error(ERROR_SD)) return;

    unsigned long i;
    struct setting_record setting;

    File settings_file = SD.open(SETTINGS_FILE, (O_READ | O_WRITE | O_CREAT));
    if (!settings_file) {
        system_control.set_error(ERROR_SD_WRITE);
        return;
    }
    settings_file.seek(0);

    for (i = 0; i < settings_file.size(); i += sizeof(setting)) {
        settings_file.read((byte*)&setting, sizeof(setting));

        if (setting.id == setting_id){
            setting.int_value = setting_int;
            settings_file.seek(i);
            settings_file.write((byte*)&setting, sizeof(setting));
            settings_file.close();
            return;
        }
    }

    settings_file.seek(settings_file.size());
    setting.id = setting_id;
    setting.int_value = setting_int;
    settings_file.write((byte*)&setting, sizeof(setting));
    settings_file.close();
}

/********************************************************************
 * Functions for Logging                                            *
 ********************************************************************/

void storage_class::log_this(int user_id, const char * log_string) {
    if (system_control.test_error(ERROR_SD)) return;

    struct log_record log;
    File log_file = SD.open(LOG_FILE, (O_READ | O_WRITE | O_CREAT | O_APPEND));
    if (!log_file) {
        system_control.set_error(ERROR_SD_WRITE);
        return;
    }
    log.user_id = user_id;
    strcopy(log_string, log.action, 4);
    
    RtcDateTime now = rtc.GetDateTime();
    log.second = now.Second();
    log.minute = now.Minute();
    log.hour = now.Hour();
    log.day = now.Day();
    log.month = now.Month();
    log.year = now.Year();

    log_file.write((byte*)&log, sizeof(log));
    log_file.close();
}

void storage_class::clear_log() {
    if (system_control.test_error(ERROR_SD)) return;

    if (!SD.remove(LOG_FILE)) {
        system_control.set_error(ERROR_SD_WRITE);
        return;
    }
    File log_file = SD.open(LOG_FILE, O_CREAT | O_WRITE);
    if (!log_file) {
        system_control.set_error(ERROR_SD_WRITE);
        return;
    }
    log_file.close();
}

unsigned long storage_class::get_log_count() {
    if (system_control.test_error(ERROR_SD_INIT | ERROR_SD_READ | ERROR_SD_UNKNOWN))
        return 0u;

    unsigned long file_size;
    File log_file = SD.open(LOG_FILE, (O_READ | O_WRITE | O_CREAT));
    if (!log_file) {
        system_control.set_error(ERROR_SD_READ);
        Serial.println("FILE READ ERROR !!");
        return 0u;
    }
    file_size = log_file.size();
    log_file.close();

    return file_size / sizeof(log_record);
}

unsigned long storage_class::get_log_count(uint8_t day, uint8_t month, uint16_t year) {
    if (system_control.test_error(ERROR_SD_INIT | ERROR_SD_READ | ERROR_SD_UNKNOWN))
        return 0u;

    int counter = 0;
    unsigned long i;
    struct log_record log;
    File log_file = SD.open(LOG_FILE, (O_READ | O_WRITE | O_CREAT));
    if (!log_file) {
        system_control.set_error(ERROR_SD_READ);
        return 0u;
    }
    log_file.seek(0);

    for (i = 0; i < log_file.size(); i += sizeof(log_record)) {
        log_file.read((byte*)&log, sizeof(log_record));

        if (log.day == day && log.month == month && log.year == year) {
            ++counter;
        }
    }
    log_file.close();
    return counter;
}

struct log_record storage_class::get_log(unsigned long position) {
    if (system_control.test_error(ERROR_SD_INIT | ERROR_SD_READ | ERROR_SD_UNKNOWN))
        return log_record {0, "", 0, 0, 0, 0, 0, 0};

    log_record log = {
        0, "", 0, 0, 0, 0, 0, 0
    };

    File log_file = SD.open(LOG_FILE, (O_READ | O_WRITE | O_CREAT));
    if (!log_file) {
        system_control.set_error(ERROR_SD_READ);
        return log;
    }
    if (log_file.size() == 0) {
        log_file.close();
        return log;
    }

    log_file.seek(
        log_file.size() - sizeof(log_record) - (sizeof(log_record) * position)
    );
    log_file.read((byte*)&log, sizeof(log_record));
    log_file.close();

    return log;
}

struct log_record storage_class::get_log(unsigned long position, uint8_t day, uint8_t month, uint16_t year) {
    if (system_control.test_error(ERROR_SD_INIT | ERROR_SD_READ | ERROR_SD_UNKNOWN))
        return log_record {0, "", 0, 0, 0, 0, 0, 0};
    
    unsigned long i;
    log_record log;
    File log_file = SD.open(LOG_FILE, (O_READ | O_WRITE | O_CREAT));
    if (!log_file) {
        system_control.set_error(ERROR_SD_READ);
        return log_record {0, "", 0, 0, 0, 0, 0, 0};
    }

    for (i = log_file.size() - sizeof(log_record); i >= 0; i -= sizeof(log_record)) {
        log_file.seek(i);
        log_file.read((byte*)&log, sizeof(log_record));

        if (log.day == day && log.month == month && log.year == year) {
            log_file.seek(i - sizeof(log_record) * position);
            log_file.read((byte*)&log, sizeof(log_record));
            log_file.close();
            return log;
        }
    }

    log_record no_log = {
        0, "", 0, 0, 0, 0, 0, 0
    };
    log_file.close();
    return no_log;
}

/********************************************************************
 * Functions for user manipulation                                  *
 ********************************************************************/

int storage_class::add_user(const char number[]) {
    if (system_control.test_error(ERROR_SD)) return 0;

    // Create variables
    File user_file;       // File variable
    user_record user;     // Record for new user
    unsigned long i;      // Counter to count bytes while reading file

    // Get id for new user from settings file
    user.id = get_setting(SETTING_NEXT_USER_ID).int_value;
    // Open users file
    user_file = SD.open(USERS_FILE, (O_READ | O_CREAT | O_WRITE));
    if (!user_file) {
        system_control.set_error(ERROR_SD_WRITE);
        return 0;
    }
    // Copy number to new user record and set it to active
    strcopy(number, user.number, 16);
    user.active = 1;

    // If there is nothing in users file add first record
    if (user_file.size() < sizeof(user_record)) {
        user_file.seek(0);
        user_file.write((byte*)&user, sizeof(user_record));
        set_setting(SETTING_NEXT_USER_ID, user.id + 1);
    } else {
        int done_flag = 0;         // Indicator if new user already exist
        user_record search_user;   // Used to read users while searching
        // Start from file beggining
        user_file.seek(0);

        for (i = 0; i < user_file.size(); i += sizeof(user_record)) {
            user_file.read((byte*)&search_user, sizeof(user_record));

            if (strcompare(number, search_user.number)) {
                user.id = search_user.id;
                user_file.seek(i);
                user_file.write((byte*)&user, sizeof(user_record));
                done_flag = 1;
                break;
            }
        }

        if (!done_flag) {
            user_file.write((byte*)&user, sizeof(user_record));
            set_setting(SETTING_NEXT_USER_ID, user.id + 1);
        }
    }

    user_file.close();
    return user.id;
}

void storage_class::edit_user(const int id, const char number[]) {
    if (system_control.test_error(ERROR_SD)) return;

    unsigned long i;
    user_record user;
    File user_file = SD.open(USERS_FILE, (O_READ | O_CREAT | O_WRITE));
    if (!user_file) {
        system_control.set_error(ERROR_SD_WRITE);
        return;
    }
    user_file.seek(0);

    for (i = 0; i < user_file.size(); i += sizeof(user_record)) {
        user_file.read((byte*)&user, sizeof(user_record));

        if (user.id == id) {
            user_file.seek(i);
            strcopy(number, user.number, 16);
            user_file.write((byte*)&user, sizeof(user_record));
            break;
        }
    }

    user_file.close();
}

void storage_class::dis_user(const int id) {
    if (system_control.test_error(ERROR_SD)) return;

    unsigned long i;
    user_record user;
    File user_file = SD.open(USERS_FILE, (O_READ | O_CREAT | O_WRITE));
    if (!user_file) {
        system_control.set_error(ERROR_SD_WRITE);
        return;
    }
    user_file.seek(0);

    for (i = 0; i < user_file.size(); i += sizeof(user_record)) {
        user_file.read((byte*)&user, sizeof(user_record));

        if (user.id == id) {
            user_file.seek(i);
            user.active = 0;
            user_file.write((byte*)&user, sizeof(user_record));
            break;
        }
    }

    user_file.close();
}

void storage_class::enb_user(const int id) {
    if (system_control.test_error(ERROR_SD)) return;

    unsigned long i;
    user_record user;
    File user_file = SD.open(USERS_FILE, (O_READ | O_CREAT | O_WRITE));
    if (!user_file) {
        system_control.set_error(ERROR_SD_WRITE);
        return;
    }
    user_file.seek(0);

    for (i = 0; i < user_file.size(); i += sizeof(user_record)) {
        user_file.read((byte*)&user, sizeof(user_record));

        if (user.id == id) {
            user_file.seek(i);
            user.active = 1;
            user_file.write((byte*)&user, sizeof(user_record));
            break;
        }
    }

    user_file.close();
}

int storage_class::get_user_count() {
    if (system_control.test_error(ERROR_SD_INIT | ERROR_SD_READ | ERROR_SD_UNKNOWN))
        return 0;

    unsigned long size;
    File user_file = SD.open(USERS_FILE, (O_READ | O_WRITE | O_CREAT));
    if (!user_file) {
        system_control.set_error(ERROR_SD_READ);
        return 0;
    }

    size = user_file.size();
    user_file.close();

    return size / sizeof(user_record);
}

void storage_class::clear_user_file() {
    if (system_control.test_error(ERROR_SD)) return;

    if (!SD.remove(USERS_FILE)) {
        system_control.set_error(ERROR_SD_WRITE);
        return;
    }
    File user_file = SD.open(USERS_FILE, O_CREAT | O_WRITE);
    if (!user_file) {
        system_control.set_error(ERROR_SD_WRITE);
        return;
    }
    user_file.close();
}

struct user_record storage_class::get_user_by_id(const int id) {
    if (system_control.test_error(ERROR_SD_INIT | ERROR_SD_READ | ERROR_SD_UNKNOWN))
        return user_record {USER_DELETED, 0, "OBRISAN"};
    
    unsigned long i;
    user_record user;
    File user_file = SD.open(USERS_FILE, (O_READ | O_WRITE | O_CREAT));
    if (!user_file) {
        system_control.set_error(ERROR_SD_READ);
        return user_record {USER_DELETED, 0, "OBRISAN"};
    }
    user_file.seek(0);

    for (i = 0; i < user_file.size(); i += sizeof(user_record)) {
        user_file.read((byte*)&user, sizeof(user_record));

        if (user.id == id) {
            return user;
        }
    }

    user_file.close();

    switch (id) {
        case USER_PANEL:
            return user_record {USER_PANEL, 1, "PANEL"};
        case USER_SERIAL:
            return user_record {USER_SERIAL, 1, "KONZOLA"};
        case USER_LAST_RESEVED:
            return user_record {USER_LAST_RESEVED, 0, "RESERVED"};
        default:
            return user_record {USER_DELETED, 0, "OBRISAN"};
    }
}

struct user_record storage_class::get_user_by_num(const char number[]) {
    if (system_control.test_error(ERROR_SD_INIT | ERROR_SD_READ | ERROR_SD_UNKNOWN))
        return user_record {0, 0, "OBRISAN"};

    unsigned long i;
    user_record user;
    File user_file = SD.open(USERS_FILE, (O_READ | O_WRITE | O_CREAT));
    if (!user_file) {
        system_control.set_error(ERROR_SD_READ);
        return user_record {USER_DELETED, 0, "OBRISAN"};
    }
    user_file.seek(0);

    for (i = 0; i < user_file.size(); i += sizeof(user_record)) {
        user_file.read((byte*)&user, sizeof(user_record));

        if (strcompare(number, user.number)) {
            return user;
        }
    }

    user_record no_user = {
        0, 0, "DELETED"
    };

    user_file.close();
    return no_user;
}

struct user_record storage_class::get_user_by_pos(const int position) {
    if (system_control.test_error(ERROR_SD_INIT | ERROR_SD_READ | ERROR_SD_UNKNOWN))
        return user_record {0, 0, "OBRISAN"};

    File user_file = SD.open(USERS_FILE, (O_READ | O_WRITE | O_CREAT));
    if (!user_file) {
        system_control.set_error(ERROR_SD_READ);
        return user_record {USER_DELETED, 0, "OBRISAN"};
    }
    user_record user = {
        0, 0, "DELETED"
    };

    if (position * sizeof(user_record) < user_file.size()) {
        user_file.seek(position * sizeof(user_record));
        user_file.read((byte*)&user, sizeof(user_record));
    }

    user_file.close();
    return user;
}

void storage_class::delete_user(int id) {
    if (system_control.test_error(ERROR_SD)) return;

    File user_file, temp_file;
    unsigned long i;
    user_record user;

    user_file = SD.open(USERS_FILE, (O_READ | O_WRITE | O_CREAT));
    if (!user_file) {
        system_control.set_error(ERROR_SD_WRITE);
        return;
    }
    temp_file = SD.open("UTEMP.BIN", (O_READ | O_WRITE | O_CREAT));
    if (!temp_file) {
        system_control.set_error(ERROR_SD_WRITE);
        return;
    }
    user_file.seek(0);
    temp_file.seek(0);

    for (i = 0; i < user_file.size(); i += sizeof(user_record)) {
        user_file.read((byte*)&user, sizeof(user_record));

        if (user.id != id) {
            temp_file.write((byte*)&user, sizeof(user_record));
        }
    }

    user_file.close();
    temp_file.close();

    if (!SD.remove(USERS_FILE)) {
        system_control.set_error(ERROR_SD_WRITE);
        return;
    }

    user_file = SD.open(USERS_FILE, (O_READ | O_WRITE | O_CREAT));
    if (!user_file) {
        system_control.set_error(ERROR_SD_WRITE);
        return;
    }
    temp_file = SD.open("TEMP.BIN", (O_READ | O_WRITE | O_CREAT));
    if (!temp_file) {
        system_control.set_error(ERROR_SD_WRITE);
        return;
    }

    for (i = 0; i < temp_file.size(); i += sizeof(user_record)) {
        temp_file.read((byte*)&user, sizeof(user_record));
        user_file.write((byte*)&user, sizeof(user_record));
    }
    
    user_file.close();
    temp_file.close();

    if (!SD.remove("TEMP.BIN")) {
        system_control.set_error(ERROR_SD_WRITE);
        return;
    }
}
