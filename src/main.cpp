#include <Arduino.h>

#include "menu.hpp"
#include "storage.hpp"
#include "system.hpp"

char ch;

void setup() {
    unsigned long li;
    int i;
     
    Serial.begin(9600);
    init_display();
    //init_rtc();
    //init_sd();
    storage.init();

    Serial.println("-- Start settings test --");

    Serial.println("-- End settings test --");

    Serial.println("-- Start log test --");

    /*//storage.log_this(USER_PANEL, "VMO");
    storage.log_this(USER_SERIAL, "SUG");
    storage.log_this(5, "SUP");

    Serial.print("Num of records: ");
    Serial.println(storage.get_log_count());

    for (li = 0; li < storage.get_log_count(); li++) {
        log_record log = storage.get_log(li);

        Serial.print(log.user_id);
        Serial.print(" -- ");
        Serial.print(log.hour);
        Serial.print(":");
        Serial.print(log.minute);
        Serial.print(":");
        Serial.print(log.second);
        Serial.print(" ");
        Serial.print(log.day);
        Serial.print("-");
        Serial.print(log.month);
        Serial.print("-");
        Serial.print(log.year);
        Serial.print(" -- ");
        Serial.println(log.action);
    }

    Serial.println("-- End log test --");

    Serial.println("-- Start users test --");

    //storage.clear_user_file();
    //storage.delete_user(6);
    //storage.delete_user(8);

    //storage.dis_user(2);
    //storage.dis_user(4);
    //storage.enb_user(2);

    for (i = 0; i < storage.get_user_count(); i++) {
        user_record user = storage.get_user_by_pos(i);
        Serial.print("User ");
        Serial.print(user.id);
        Serial.print(" -- ");
        Serial.print(user.active);
        Serial.print(" -- ");
        Serial.println(user.number);
    }

    Serial.println();
    //Serial.println(storage.add_user("+385976644521"));
    //Serial.println(storage.add_user("+385955254311"));
    //Serial.println(storage.add_user("+385919748651"));
    //Serial.println(storage.add_user("+385937263561"));
    Serial.println();

    for (i = 0; i < storage.get_user_count(); i++) {
        user_record user = storage.get_user_by_pos(i);
        Serial.print("User ");
        Serial.print(user.id);
        Serial.print(" -- ");
        Serial.print(user.active);
        Serial.print(" -- ");
        Serial.println(user.number);
    }

    Serial.println();

    user_record user;

    user = storage.get_user_by_num("+385976644526");

    Serial.print("User ");
    Serial.print(user.id);
    Serial.print(" -- ");
    Serial.print(user.active);
    Serial.print(" -- ");
    Serial.println(user.number);

    user = storage.get_user_by_id(2);

    Serial.print("User ");
    Serial.print(user.id);
    Serial.print(" -- ");
    Serial.print(user.active);
    Serial.print(" -- ");
    Serial.println(user.number);

    Serial.println("-- End users test --");*/

    main_panel.set_motd(1, "DVD Kontrolni Sustav");
    main_panel.set_page(home_page);

    
}

void loop() {
    while (Serial.available()) {
        ch = Serial.read();
        if (ch == 'a') {
            Serial.print("Password: ");
            Serial.println(storage.get_setting(SETTING_PASSWORD).string_value);
            Serial.print("Sirene auth: ");
            Serial.println(storage.get_setting(SETTING_SIRENE_AUTH).int_value);
            Serial.print("Settings auth: ");
            Serial.println(storage.get_setting(SETTING_SETTINGS_AUTH).int_value);
        } else if (ch == 's') {
            system_control.set_error(ERROR_MODEM_TURN_ON);
            system_control.set_error(ERROR_LIGHT_UNKNOWN);
            system_control.set_error(ERROR_SD_WRITE);
        } else if (ch == 'u') {
            system_control.unset_error(ERROR_MODEM_TURN_ON);
            system_control.unset_error(ERROR_LIGHT_UNKNOWN);
            system_control.unset_error(ERROR_SD_WRITE);
        }
    }

    main_panel.update();
}