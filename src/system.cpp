// Include global header files
#include <Arduino.h>
// Include local header files
#include "system.hpp"
#include "panel.hpp"
#include "storage.hpp"
#include "relays.hpp"

// Create system control variable
system_class system_control;

/********************************************************************
 * Class for hadling commands over serial                           *
 ********************************************************************/
commands::commands() {
    buffer[0] = '\0';    // Set buffer to empty string
    counter = 0;         // Set counter to first character of buffer
    line_ready = 0;      // Line is not ready
}

void commands::init() {
    // Start serial
    Serial.begin(9600);
    // Print Welcome message on startup
    Serial.print("\r\n-------------------------------------------------");
    Serial.print("\r\n>> DVDCS Admin console <<\r\n");
    Serial.print("\r\n- Welcome, type help for list of commands\r\n");
    Serial.print("\r\n> ");
}

void commands::update() {
    // Dok ima neš na serialu čitaj
    while (Serial.available() > 0) {
        // Read character from serial
        char character = Serial.read();
        // If character is backspace
        if (character == '\b' || character == '\177') {
            // And if we are not at the line beggining
            // go one character back
            if (counter != 0) {
                --counter;
                Serial.write('\b');
                Serial.write(' ');
                Serial.write('\b');
            }
        // If character is enter set line as ready
        } else if (character == '\r') {
            line_ready = 1;
        // If buffer is not full
        // And if character is not Line feed write it to the buffer
        } else if (character != '\n' && counter < CONSOLE_BUFFER_SIZE) {
            buffer[counter] = character;
            ++counter;
            Serial.write(character);
        }
        buffer[counter] = '\0';
    }
}

void commands::clear() {
    // Clear buffer and set line as not ready
    counter = 0;
    line_ready = 0;
    buffer[counter] = '\0';
}

int commands::ready() {
    // Check if line is ready to be processed
    return line_ready;
}

char * commands::get() {
    // Return pointer to buffer
    return buffer;
}

/********************************************************************
 * Class for interactions with system                               *
 ********************************************************************/
system_class::system_class() {
    error_flags = 0;
}

void system_class::init() {
    // Initilize command handling
    command.init();
    // Set mode for needed pins
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
    pinMode(READY_LED_PIN, OUTPUT);
    digitalWrite(READY_LED_PIN, LOW);
}

void system_class::set_error(const int flag) {
    // Set given error flag
    error_flags |= flag;
    // Set motd to print current error code
    if (error_flags != 0) {
        char motd[30];
        sprintf(motd, "ERROR CODE 0x%04x", error_flags);

        main_panel.set_motd(8, motd);
        main_panel.go_home();
    } else {
        main_panel.clear_motd(8);
    }
}

void system_class::unset_error(const int flag) {
    // Unset given error flag
    error_flags &= ~flag;
    // If any error flag is set, set motd to print current error code
    if (error_flags != 0) {
        char motd[30];
        sprintf(motd, "ERROR CODE 0x%04x", error_flags);

        main_panel.set_motd(8, motd);
        main_panel.go_home();
    } else {
        main_panel.clear_motd(8);
    }
}

int system_class::test_error(const int flag) {
    return (error_flags & flag) > 0;
}

int system_class::get_error_flags() {
    return error_flags;
}

void system_class::beep(int how_many_times) {
    beep_start = millis();
    beep_counter = how_many_times;
    update();
}

void system_class::beep(unsigned int beep_delay, int how_many_times) {
    beep_wait_start = millis();
    beep_waiting_time = beep_delay;
    beep_after_wait_counter = how_many_times;
}

void system_class::update() {
    // Handle current beeping
    if (beep_counter > 0 && beep_start <= millis()) {
        if ((millis() - beep_start) / BEEP_DURITATION / 2 >= (unsigned int)beep_counter) {
            beep_counter = 0;
        }

        if (beep_counter > 0 && (millis() - beep_start) / BEEP_DURITATION % 2 == 0) {
            digitalWrite(BUZZER_PIN, HIGH);
        } else {
            digitalWrite(BUZZER_PIN, LOW);
        }
    }
    // Handle delayed beeping
    if (beep_after_wait_counter > 0) {
        if (beep_wait_start + beep_waiting_time <= millis()) {
            int temp_counter = beep_after_wait_counter;  // Store counter in temo variable
            beep_after_wait_counter = 0;                 // Set it to 0 to stop recursion
            beep(temp_counter);                          // Start beep
        }
    }
    // Serial communication
    command.update();

    if (command.ready()) {
        Serial.println();
        // Command echo -- print stuff back to console
        if (strstartswith(command.get(), "echo ")) {
            Serial.print(F("echo: "));
            Serial.println(command.get() + 5);
        }
        // Command echo (no parametars) -- print nothing
        else if (strcompare(command.get(), "echo")) {
            Serial.println(F("echo: "));
        }
        // Command help -- print list of commands
        else if (strcompare(command.get(), "help")) {
            Serial.println(F("-------------------------------------------------"));
            Serial.print(F("   DVD Control System -- Ver "));
            Serial.println(F(FIRMWARE_VERSION));
            Serial.println(F("-------------------------------------------------"));
            Serial.println();
            Serial.println(F("help                         -- Print this help message"));
            Serial.println(F("echo <text>                  -- Print text back to console (useless)"));
            Serial.println(F("motd                         -- Print current Message Of The Day"));
            Serial.println(F("setmotd <text>               -- Set custom motd to be displayed on home screen"));
            Serial.println(F("clearmotd                    -- Remove custom motd if it's set"));
            Serial.println(F("dispass                      -- Disable password autentification for everything"));
            Serial.println(F("settings                     -- List values of all system settings"));
            Serial.println(F("users                        -- List all SMS users"));
            Serial.println(F("errors                       -- Show values of all error flags"));
            Serial.println(F("log <number>                 -- Print specified number of log records"));
            Serial.println(F("clearusers                   -- Permanently delete all users from users file"));
            Serial.println(F("clearlog                     -- Permanently delete all log records from log file"));
            Serial.println(F("unseterrors                  -- Unset all error flags (DON'T DO THIS)"));
            Serial.println(F("date                         -- Display current date and time"));
            Serial.println(F("setdate DD-MM-YYYY hh-mm-ss  -- Set new date and time"));
            Serial.println(F("sensors                      -- Read state of all sensors"));
            Serial.println();
        }
        // Command errors -- print error flags
        else if (strcompare(command.get(), "errors")) {
            Serial.print(F("Error -- MODEM TURN ON     -- "));
            Serial.println(test_error(ERROR_MODEM_TURN_ON));
            Serial.print(F("Error -- MODEM TIMEOUT     -- "));
            Serial.println(test_error(ERROR_MODEM_TIMEOUT));
            Serial.print(F("Error -- MODEM SIM         -- "));
            Serial.println(test_error(ERROR_MODEM_SIM));
            Serial.print(F("Error -- MODEM SIGNAL      -- "));
            Serial.println(test_error(ERROR_MODEM_SIGNAL));
            Serial.print(F("Error -- MODEM REGISTER    -- "));
            Serial.println(test_error(ERROR_MODEM_REGISTER));
            Serial.print(F("Error -- MODEM SMS SEND    -- "));
            Serial.println(test_error(ERROR_MODEM_SMS_SEND));
            Serial.print(F("Error -- MODEM UNKNOWN     -- "));
            Serial.println(test_error(ERROR_MODEM_UNKNOWN));
            Serial.print(F("Error -- SD INIT           -- "));
            Serial.println(test_error(ERROR_SD_INIT));
            Serial.print(F("Error -- SD READ           -- "));
            Serial.println(test_error(ERROR_SD_READ));
            Serial.print(F("Error -- SD WRITE          -- "));
            Serial.println(test_error(ERROR_SD_WRITE));
            Serial.print(F("Error -- SD UNKNOWN        -- "));
            Serial.println(test_error(ERROR_SD_UNKNOWN));
            Serial.print(F("Error -- RTC CONFIDENCE    -- "));
            Serial.println(test_error(ERROR_RTC_CONFIDENCE));
            Serial.print(F("Error -- RTC UNKNOWN       -- "));
            Serial.println(test_error(ERROR_RTC_UNKNOWN));
            Serial.print(F("Error -- LIGHT UNKNOWN     -- "));
            Serial.println(test_error(ERROR_LIGHT_UNKNOWN));
        }
        // Command settings -- print values of each setting
        else if (strcompare(command.get(), "settings")) {
            if (test_error(ERROR_SD)) {
                Serial.println(F("DVDCS: SD card error"));
            } else {
                setting_record setting;

                Serial.print(F("Setting -- PASSWORD (PIN)     -- "));
                setting = storage.get_setting(SETTING_PASSWORD);
                Serial.print(setting.int_value);
                Serial.print(F(", \""));
                Serial.print(setting.string_value);
                Serial.println(F("\""));

                Serial.print(F("Setting -- SIRENE AUTH        -- "));
                setting = storage.get_setting(SETTING_SIRENE_AUTH);
                Serial.print(setting.int_value);
                Serial.print(F(", \""));
                Serial.print(setting.string_value);
                Serial.println(F("\""));

                Serial.print(F("Setting -- SETTINGS AUTH      -- "));
                setting = storage.get_setting(SETTING_SETTINGS_AUTH);
                Serial.print(setting.int_value);
                Serial.print(F(", \""));
                Serial.print(setting.string_value);
                Serial.println(F("\""));

                Serial.print(F("Setting -- MOTD               -- "));
                setting = storage.get_setting(SETTING_MOTD);
                Serial.print(setting.int_value);
                Serial.print(F(", \""));
                Serial.print(setting.string_value);
                Serial.println(F("\""));

                Serial.print(F("Setting -- NEXT USER ID       -- "));
                setting = storage.get_setting(SETTING_NEXT_USER_ID);
                Serial.print(setting.int_value);
                Serial.print(F(", \""));
                Serial.print(setting.string_value);
                Serial.println(F("\""));
                
                Serial.print(F("Setting -- LAST LIGHT STATE   -- "));
                setting = storage.get_setting(SETTING_LAST_LIGHT_STATE);
                Serial.print(setting.int_value);
                Serial.print(F(", \""));
                Serial.print(setting.string_value);
                Serial.println(F("\""));
            }
        }
        // Command users -- print list of users
        else if (strcompare(command.get(), "users")) {
            if (test_error(ERROR_SD)) {
                Serial.println(F("DVDCS: SD card error"));
            } else {
                int user_count = storage.get_user_count();

                if (user_count == 0) {
                    Serial.println(F("users: No users found in users file"));
                } else {
                    int i;

                    Serial.print(F("Number of user records: "));
                    Serial.println(user_count);
                    Serial.println();

                    for (i = 0; i < user_count; i++) {
                        user_record user = storage.get_user_by_pos(i);

                        Serial.print(F("User -- "));
                        Serial.print(
                            (user.active) ? F("Enabled  -- ") : F("Disabled -- ")
                        );
                        Serial.print(F("Number +"));
                        Serial.print(user.number);
                        Serial.print(F(" -- ID "));
                        Serial.println(user.id);
                    }
                }
            }
        }
        // Command motd -- print current motd
        else if (strcompare(command.get(), "motd")) {
            Serial.println(F("-------------------------------------------------"));
            Serial.print(F("   MOTD: "));
            Serial.println(main_panel.get_motd());
            Serial.println(F("-------------------------------------------------"));
        }
        // Command setmotd -- set custom motd
        else if (strcompare(command.get(), "setmotd")) {
            Serial.println(F("setmotd: Syntax of command is setmotd <text>"));
        }
        else if (strstartswith(command.get(), "setmotd ")) {
            if (test_error(ERROR_SD)) {
                Serial.println(F("DVDCS: SD card error"));
            } else {
                if (strlength(command.get() + 8) > 20) {
                    Serial.println(F("setmotd: Max length of motd is 20 ASCII characters"));
                } else {
                    storage.set_setting(SETTING_MOTD, command.get() + 8);
                    main_panel.set_motd(1, command.get() + 8);
                    Serial.print(F("setmotd: Motd is set to \""));
                    Serial.print(command.get() + 8);
                    Serial.println(F("\""));
                }
            }
        }
        // Command clearmotd -- remove custom motd
        else if (strcompare(command.get(), "clearmotd")) {
            if (test_error(ERROR_SD)) {
                Serial.println(F("DVDCS: SD card error"));
            } else {
                storage.set_setting(SETTING_MOTD, "");
                main_panel.clear_motd(1);
                Serial.println(F("clearmotd: Custom motd is cleared, default motd will be displayed instead"));
            }
        }
        // Command dispass -- Disable password for settings and sirens, so they can be accessed without one
        else if (strcompare(command.get(), "dispass")) {
            if (test_error(ERROR_SD)) {
                Serial.println(F("DVDCS: SD card error"));
            } else {
                storage.set_setting(SETTING_SIRENE_AUTH, FALSE);
                storage.set_setting(SETTING_SETTINGS_AUTH, FALSE);
                Serial.println(F("dispass: Authetication is disabled, you now have access to all features of DVDCS without PIN"));
            }
        }
        // Command unseterrors -- Unset all error system flags
        else if (strcompare(command.get(), "unseterrors")) {
            unset_error(ERROR);
            Serial.println(F("unseterrors: All error flags are now set to 0, I hope you know what you're doing"));
        }
        // Command clearusers -- clear users file
        else if (strcompare(command.get(), "clearusers")) {
            if (test_error(ERROR_SD)) {
                Serial.println(F("DVDCS: SD card error"));
            } else {
                storage.clear_user_file();
                Serial.println(F("clearusers: Users file has been cleared, you might also want clear log file since it's mostly useless now"));
            }
        }
        // Command clearlog -- clear log file
        else if (strcompare(command.get(), "clearlog")) {
            if (test_error(ERROR_SD)) {
                Serial.println(F("DVDCS: SD card error"));
            } else {
                storage.clear_log();
                Serial.println(F("clearlog: Log file has been cleared"));
            }
        }
        // Command log <number> -- display specified number of logs
        else if (strcompare(command.get(), "log")) {
            Serial.println(F("log: Syntax of command is log <number>"));
        }
        else if (strstartswith(command.get(), "log ")) {
            if (test_error(ERROR_SD)) {
                Serial.println(F("DVDCS: SD card error"));
            } else {
                unsigned long num;                                      // Number of records to print
                // If command is correctly formated
                if (sscanf(command.get(), "log %lu", &num) == 1) {
                    unsigned long log_count = storage.get_log_count();  // Get log count
                    unsigned long current;                              // Current log
                    char log_formated[50];                              // String with current log information to print to console

                    for (current = 0; current < log_count && current < num; current++) {
                        log_record logr = storage.get_log(current);                 // Log record
                        user_record userr = storage.get_user_by_id(logr.user_id);   // User for current record
                        // Format log
                        sprintf(
                            log_formated, "Log -- %02u-%02u-%04u %02u:%02u:%02u -- %s -- %20s",
                            logr.day, logr.month, logr.year, logr.hour, logr.minute, logr.second, logr.action, userr.number
                        );
                        // Print formated log
                        Serial.println(log_formated);
                    }
                    // Print log count at the end
                    Serial.println();
                    Serial.print("Log records ");
                    // If requested number is greater than log count print log count
                    if (log_count < num)
                        Serial.print(log_count);
                    else
                        Serial.print(num);
                    Serial.print(" / ");
                    Serial.println(log_count);
                } else {
                    Serial.println(F("log: Syntax of command is log <number>"));
                }
            }
        }
        // Command setdate DD-MM-YYYY hh-mm-ss -- set time for RTC
        else if (strcompare(command.get(), "setdate")) {
            Serial.println(F("setdate: Syntax of command is setdate DD-MM-YYYY hh-mm-ss"));
        }
        else if (strstartswith(command.get(), "setdate ")) {
            uint8_t mon, day, hour, min, sec;
            uint16_t year;
            if (sscanf(command.get(), "setdate %02hhu-%02hhu-%04u %02hhu:%02hhu:%02hhu", &day, &mon, &year, &hour, &min, &sec) == 6) {
                Serial.println(day);
                RtcDateTime time_to_set(year, mon, day, hour, min, sec);
                rtc.SetDateTime(time_to_set);

                Serial.println(F("setdate: Task completed, new time is set"));
            } else {
                Serial.println(F("setdate: Syntax of command is setdate DD-MM-YYYY hh-mm-ss"));
            }
        }
        // Command date -- display current time
        else if (strcompare(command.get(), "date")) {
            if (test_error(ERROR_RTC)) {
                Serial.println(F("date: RTC error accured, unable to read time"));
            } else {
                char time_formated[30];
                RtcDateTime now = rtc.GetDateTime();

                sprintf(
                    time_formated, "%02u-%02u-%04u %02u:%02u:%02u",
                    now.Day(), now.Month(), now.Year(), now.Hour(), now.Minute(), now.Second()
                );

                Serial.print(F("date: "));
                Serial.println(time_formated);
            }
        }
        // Command sensors -- display current state of all sensors
        else if (strcompare(command.get(), "sensors")) {
            Serial.print("Sensor -- Door big opened    -- ");
            Serial.println(digitalRead(BIG_DOOR_S_OPEN));
            Serial.print("Sensor -- Door big closed    -- ");
            Serial.println(digitalRead(BIG_DOOR_S_CLOSE));
            Serial.print("Sensor -- Door small opened  -- ");
            Serial.println(digitalRead(SMALL_DOOR_S_OPEN));
            Serial.print("Sensor -- Door small closed  -- ");
            Serial.println(digitalRead(SMALL_DOOR_S_CLOSE));
            Serial.print("Sensor -- Light              -- ");
            Serial.println(digitalRead(LIGHT_S));
        }
        // If command is not found, print command not found
        else if (command.get()[0] != '\0') {
            Serial.println(F("DVDCS: Command not found"));
        }
        // If no command is entered just print prompt
        Serial.print(F("> "));

        command.clear();
    }
}

void system_class::ready(int state) {
    digitalWrite(READY_LED_PIN, state);
}