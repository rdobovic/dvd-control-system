#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

#include "menu.hpp"
#include "storage.hpp"
#include "system.hpp"
#include "helper_functions.hpp"

/********************************************************************
 * Create LCD class and other LCD stuff                             *
 ********************************************************************/

static LiquidCrystal_I2C lcd(0x27, 20, 4);

// Create cursor character
byte cursor_icon[8] = {
    0b00000,
    0b01110,
    0b01110,
    0b01110,
    0b01110,
    0b01110,
    0b01110,
    0b00000
};

// Function to init display on startup
void init_display() {
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.createChar(0, cursor_icon);
}

// Function to print cursor character
#define cursor_char() write(0)

/********************************************************************
 * Create Keypad class and other Keypad stuff                       *
 ********************************************************************/

static const byte ROWS = 4;
static const byte COLS = 4;
static char keys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

static byte rowPins[ROWS] = {28, 26, 24, 22};
static byte colPins[COLS] = {36, 34, 32, 30};

static Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

/********************************************************************
 * Create global page variables                                     *
 ********************************************************************/

home_page_class home_page;
main_menu_page_class main_menu_page;
doors_menu_page_class doors_menu_page;
small_door_page_class small_door_page;
big_door_page_class big_door_page;
settings_page_class settings_page;
access_control_page_class access_control_page;
settings_pass_page_class settings_pass_page;
auth_settings_page_class auth_settings_page;
change_pin_page_class change_pin_page;
user_list_page_class user_list_page;
number_edit_page_class number_edit_page;
number_add_page_class number_add_page;
log_page_class log_page;
incorrect_pin_page_class incorrect_pin_page;
siren_pass_page_class siren_pass_page;
siren_page_class siren_page;
nadolazeca_siren_page_class nadolazeca_siren_page;
neposredna_siren_page_class neposredna_siren_page;
prestanak_siren_page_class prestanak_siren_page;
vatrogasna_siren_page_class vatrogasna_siren_page;

/********************************************************************
 * Create global panel variables and functions                      *
 ********************************************************************/

panel main_panel(home_page);

panel::panel(menu_page_class &home_page) {
    // Set home page
    home_page_ptr = &home_page;
    current_page_ptr = NULL;
    back_page_ptr = NULL;
}

void panel::set_page(menu_page_class &page) {
    back_page_ptr = current_page_ptr;
    current_page_ptr = &page;
    lcd.clear();
    page.print();
    page.update();
}

void panel::update() {
    char key = keypad.getKey();

    if (key) {
        current_page_ptr->key_press(key);
    }

    current_page_ptr->update();
}

void panel::go_home() {
    set_page(*home_page_ptr);
}

/********************************************************************
 * Functions for MOTD manipulations                                 *
 ********************************************************************/

void panel::set_motd(const int level, const char text[]) {
    // If given level do not exist, do nothing
    if (level > MAX_MOTD_LEVEL) return;
    // Else set level, and copy text
    motd_levels[level].is_set = TRUE;
    strcopy(text, motd_levels[level].text, MAX_MOTD_SIZE);
    // Update motd if page has it
    if (current_page_ptr != NULL)
        current_page_ptr->update_motd();
}

void panel::clear_motd(const int level) {
    // If given level do not exist, do nothing
    if (level > MAX_MOTD_LEVEL) return;
    // Else unset level
    motd_levels[level].is_set = FALSE;
    motd_levels[level].text[0] = '\0';
    // Update motd if page has it
    if (current_page_ptr != NULL)
        current_page_ptr->update_motd();
}

const char * panel::get_motd() {
    int lvl;   // Index counter

    // Find highest level which is set
    for (lvl = MAX_MOTD_LEVEL; lvl >= 0; lvl--)
        if (motd_levels[lvl].is_set == TRUE)
            return motd_levels[lvl].text;

    // If no motd is set display default
    return DEFAULT_MOTD;
}

/********************************************************************
 * Template menu page functions                                     *
 ********************************************************************/
menu_page_class::menu_page_class() {
    cursor = 0;
    cursor_start = 0;
    cursor_end = 0;
}

void menu_page_class::set_cursor(int start, int end) {
    cursor_start = start;
    cursor_end = end;
    cursor = start;
}

void menu_page_class::up_cursor() {
    if (cursor != -1 && cursor > cursor_start)
        --cursor;
}

void menu_page_class::down_cursor() {
    if (cursor != -1 && cursor < cursor_end)
        ++cursor;
}

void menu_page_class::zero_cursor() {
    if (cursor != -1)
        cursor = cursor_start;
}

void menu_page_class::print_cursor() {
    int i;    // Display line counter

    if (cursor == -1) return;
    
    for (i = cursor_start; i <= cursor_end; i++) {

        lcd.setCursor(0, i);
        if (cursor == i)
            lcd.cursor_char();
        else
            lcd.print(" ");

        lcd.setCursor(19, i);
        if (cursor == i)
            lcd.cursor_char();
        else
            lcd.print(" ");
    }
}

// If page do not have motd, do nothing
void menu_page_class::update_motd() {}

/********************************************************************
 * Template input page functions                                    *
 ********************************************************************/
input_page_class::input_page_class() {
    input_buffer[0] = '\0';
    position = 0;
    password = FALSE;
}

void input_page_class::is_password(int is_pass) {
    password = is_pass;
}

void input_page_class::print_input(int line) {
    int i;  // Index counter

    lcd.setCursor(0, line);
    lcd.print("[");
    if (password) {
        for (i = 0; input_buffer[i] != '\0'; i++) {
            lcd.print("*");
        }
    } else {
        lcd.print(input_buffer);
    }
    if (position < MAX_INPUT - 1) {
        lcd.print(
            (millis() / 1000 % 2) ? "|" : " "
        );
    }
    for (i = position + 1; i < MAX_INPUT - 1; i++) {
        lcd.print(" ");
    }
    lcd.print("]");
}

const char * input_page_class::get_buffer() {
    return input_buffer;
}

void input_page_class::clear_buffer() {
    input_buffer[0] = '\0';
    position = 0;
}

void input_page_class::key_input(const char key) {
    if (key == DELETE_KEY) {
        if (position != 0) {
            --position;
            input_buffer[position] = '\0';
        }
    } else if (position < MAX_INPUT - 1) {
        input_buffer[position] = key;
        ++position;
        input_buffer[position] = '\0';
    }
}

/********************************************************************
 * Home page functions                                              *
 ********************************************************************/
home_page_class::home_page_class() {
    set_cursor(-1, -1);
}

void home_page_class::print() {
    // Print motd in first line
    lcd.setCursor(0,0);
    lcd.print(main_panel.get_motd());
}

void home_page_class::update() {
    RtcDateTime now = rtc.GetDateTime();

    lcd.setCursor(1, 2);
    if (now.Hour() < 10) lcd.print("0");
    lcd.print(now.Hour());
    lcd.print(":");
    if (now.Minute() < 10) lcd.print("0");
    lcd.print(now.Minute());
    lcd.print(":");
    if (now.Second() < 10) lcd.print("0");
    lcd.print(now.Second());
}

void home_page_class::key_press(const char key) {
    switch(key) {
        case OK_KEY:
            main_panel.set_page(main_menu_page);
            break;
    }
}

void home_page_class::update_motd() {
    lcd.clear();
    print();
    update();
}

/********************************************************************
 * Main menu page functions                                         *
 ********************************************************************/
main_menu_page_class::main_menu_page_class() {
    set_cursor(0, 3);
}

void main_menu_page_class::print() {
    zero_cursor();

    lcd.setCursor(1, 0);
    lcd.print("Vrata");
    lcd.setCursor(1, 1);
    lcd.print("Sirene");
    lcd.setCursor(1, 2);
    lcd.print("Svijetlo");
    lcd.setCursor(1, 3);
    lcd.print("Postavke");
}

void main_menu_page_class::update() {
    print_cursor();

    // Light indicator code also goes here
}

void main_menu_page_class::key_press(const char key) {
    switch(key) {
        case UP_KEY:
            up_cursor();
            break;
        case DOWN_KEY:
            down_cursor();
            break;
        case BACK_KEY:
            main_panel.set_page(home_page);
            break;
        case OK_KEY:
            switch(cursor) {
                case 0: // Line 0: Vrata
                    main_panel.set_page(doors_menu_page);
                    break;
                case 1: // Line 1: Sirene
                    // Check if there are any SD card errors
                    if (system_control.test_error(ERROR_SD)) {
                        main_panel.set_page(siren_page);
                    } else {
                        int siren_auth = storage.get_setting(SETTING_SIRENE_AUTH).int_value;
                        // If there was storage problem abort
                        if (system_control.test_error(ERROR_SD)) return;
                        if (siren_auth) {
                            main_panel.set_page(siren_pass_page);
                        // Else go to settings
                        } else {
                            main_panel.set_page(siren_page);
                        }
                    }
                    break;
                case 2: // Line 2: Svijetlo
                    break;
                case 3: // Line 3: Postavke
                    // Check if there are any SD card errors
                    if (system_control.test_error(ERROR_SD)) {
                        main_panel.go_home();
                    } else {
                        int settings_auth = storage.get_setting(SETTING_SETTINGS_AUTH).int_value;
                        // If there was storage problem abort
                        if (system_control.test_error(ERROR_SD)) return;
                        // If settings auth is enabled ask for PIN
                        if (settings_auth) {
                            main_panel.set_page(settings_pass_page);
                        // Else go to settings
                        } else {
                            main_panel.set_page(settings_page);
                        }
                    }
                    break;
            }
            break;
    }
}

/********************************************************************
 * Doors menu page functions                                        *
 ********************************************************************/
doors_menu_page_class::doors_menu_page_class() {
    set_cursor(1, 2);
}

void doors_menu_page_class::print() {
    zero_cursor();
    lcd.setCursor(0, 0);
    lcd.print("Izaberi vrata");
    lcd.setCursor(1, 1);
    lcd.print("Vrata Mala");
    lcd.setCursor(1, 2);
    lcd.print("Vrata Velika");
}

void doors_menu_page_class::update() {
    print_cursor();

    // Doors status indicator code also goes here
}

void doors_menu_page_class::key_press(const char key) {
    switch(key) {
        case UP_KEY:
            up_cursor();
            break;
        case DOWN_KEY:
            down_cursor();
            break;
        case BACK_KEY:
            main_panel.set_page(main_menu_page);
            break;
        case OK_KEY:
            switch(cursor) {
                case 1: // Line 0: Mala vrata
                    main_panel.set_page(small_door_page);
                    break;
                case 2: // Line 1: Velika vrata
                    main_panel.set_page(big_door_page);
                    break;
            }
            break;
    }
}

/********************************************************************
 * Small door page functions                                        *
 ********************************************************************/
small_door_page_class::small_door_page_class() {
    set_cursor(1, 2);
}

void small_door_page_class::print() {
    zero_cursor();
    lcd.setCursor(0, 0);
    lcd.print("Mala Vrata");
    lcd.setCursor(1, 1);
    lcd.print("Otvori");
    lcd.setCursor(1, 2);
    lcd.print("Zatvori");
}

void small_door_page_class::update() {
    print_cursor();
}

void small_door_page_class::key_press(const char key) {
    switch(key) {
        case UP_KEY:
            up_cursor();
            break;
        case DOWN_KEY:
            down_cursor();
            break;
        case BACK_KEY:
            main_panel.set_page(doors_menu_page);
            break;
        case OK_KEY:
            switch(cursor) {
                case 1: // Line 0: Otvori
                    break;
                case 2: // Line 1: Zatvori
                    break;
            }
            break;
    }
}

/********************************************************************
 * Big door page functions                                          *
 ********************************************************************/
big_door_page_class::big_door_page_class() {
    set_cursor(1, 2);
}

void big_door_page_class::print() {
    zero_cursor();
    lcd.setCursor(0, 0);
    lcd.print("Velika vrata");
    lcd.setCursor(1, 1);
    lcd.print("Otvori");
    lcd.setCursor(1, 2);
    lcd.print("Zatvori");
}

void big_door_page_class::update() {
    print_cursor();
}

void big_door_page_class::key_press(const char key) {
    switch(key) {
        case UP_KEY:
            up_cursor();
            break;
        case DOWN_KEY:
            down_cursor();
            break;
        case BACK_KEY:
            main_panel.set_page(doors_menu_page);
            break;
        case OK_KEY:
            switch(cursor) {
                case 1: // Line 0: Otvori
                    break;
                case 2: // Line 1: Zatvori
                    break;
            }
            break;
    }
}

/********************************************************************
 * Settings page functions                                          *
 ********************************************************************/
settings_page_class::settings_page_class() {
    set_cursor(1, 3);
}

void settings_page_class::print() {
    zero_cursor();

    lcd.setCursor(0, 0);
    lcd.print("Izmjena postavki");
    lcd.setCursor(1, 1);
    lcd.print("Kontrola pristupa");
    lcd.setCursor(1, 2);
    lcd.print("Auth postavke");
    lcd.setCursor(1, 3);
    lcd.print("Pregled loga");
}

void settings_page_class::update() {
    print_cursor();
}

void settings_page_class::key_press(const char key) {
    switch(key) {
        case UP_KEY:
            up_cursor();
            break;
        case DOWN_KEY:
            down_cursor();
            break;
        case BACK_KEY:
            main_panel.set_page(main_menu_page);
            break;
        case OK_KEY:
            switch(cursor) {
                case 1: // Line 1: Kontrola pristupa
                    main_panel.set_page(access_control_page);
                    break;
                case 2: // Line 2: Auth postavke
                    main_panel.set_page(auth_settings_page);
                    break;
                case 3: // Line 3: Pregled loga
                    main_panel.set_page(log_page);
                    break;
            }
            break;
    }
}

/********************************************************************
 * Access control page functions                                    *
 ********************************************************************/
access_control_page_class::access_control_page_class() {
    set_cursor(1, 3);
}

void access_control_page_class::print() {
    zero_cursor();

    lcd.setCursor(0, 0);
    lcd.print("Kontrola pristupa");
    lcd.setCursor(1, 1);
    lcd.print("Korisnici");
    lcd.setCursor(1, 2);
    lcd.print("Dodaj korisnika");
}

void access_control_page_class::update() {
    print_cursor();
}

void access_control_page_class::key_press(const char key) {
    switch(key) {
        case UP_KEY:
            up_cursor();
            break;
        case DOWN_KEY:
            down_cursor();
            break;
        case BACK_KEY:
            main_panel.set_page(settings_page);
            break;
        case OK_KEY:
            switch(cursor) {
                case 1: // Line 1: Korisnici
                    main_panel.set_page(user_list_page);
                    break;
                case 2: // Line 2: Dodaj korisnika
                    main_panel.set_page(number_add_page);
                    break;
            }
            break;
    }
}

/********************************************************************
 * Settings enter password page functions                           *
 ********************************************************************/
settings_pass_page_class::settings_pass_page_class() {
    set_cursor(-1, -1);
    is_password(TRUE);
}

void settings_pass_page_class::print() {
    clear_buffer();
    lcd.setCursor(0,0);
    lcd.print("Unesite PIN:");
}

void settings_pass_page_class::update() {
    print_input(2);
}

void settings_pass_page_class::key_press(const char key) {

    if (is_digit(key) || key == DELETE_KEY) {
        key_input(key);
    } else {
        switch(key) {
            case BACK_KEY:
                main_panel.set_page(main_menu_page);
                break;
            case OK_KEY:
                // Get password
                char *pin = storage.get_setting(SETTING_PASSWORD).string_value;
                // If there was storage problem abort
                if (system_control.test_error(ERROR_SD)) return;
                // Check if entered pin is correct
                if (strcompare(get_buffer(), pin)) {
                    main_panel.set_page(settings_page);
                } else {
                    main_panel.set_page(incorrect_pin_page);
                }
                break;
        }
    }
}

/********************************************************************
 * Auth settings page functions                                     *
 ********************************************************************/
auth_settings_page_class::auth_settings_page_class() {
    set_cursor(1, 3);
}

void auth_settings_page_class::print() {
    zero_cursor();
    lcd.setCursor(0, 0);
    lcd.print("Auth postavke");
    lcd.setCursor(1, 1);
    lcd.print("Sirene Auth");
    lcd.setCursor(1, 2);
    lcd.print("Postavke Auth");
    lcd.setCursor(1, 3);
    lcd.print("Promjeni PIN");

    lcd.setCursor(16, 1);
    lcd.print(
        (storage.get_setting(SETTING_SIRENE_AUTH).int_value) ? "DA" : "NE"
    );
    // If there was storage problem abort
    if (system_control.test_error(ERROR_SD)) return;
    lcd.setCursor(16, 2);
    lcd.print(
        (storage.get_setting(SETTING_SETTINGS_AUTH).int_value) ? "DA" : "NE"
    );
    // If there was storage problem abort
    if (system_control.test_error(ERROR_SD)) return;
}

void auth_settings_page_class::update() {
    print_cursor();
}

void auth_settings_page_class::key_press(const char key) {
    switch(key) {
        case UP_KEY:
            up_cursor();
            break;
        case DOWN_KEY:
            down_cursor();
            break;
        case BACK_KEY:
            main_panel.set_page(settings_page);
            break;
        case OK_KEY:
            switch(cursor) {
                case 1: // Line 1: Sirene Auth
                    if (storage.get_setting(SETTING_SIRENE_AUTH).int_value == 0) {
                        storage.set_setting(SETTING_SIRENE_AUTH, 1);
                    } else {
                        storage.set_setting(SETTING_SIRENE_AUTH, 0);
                    }
                    // If there was storage problem abort
                    if (system_control.test_error(ERROR_SD)) return;

                    print();
                    break;
                case 2: // Line 2: Postavke Auth
                    if (storage.get_setting(SETTING_SETTINGS_AUTH).int_value == 0) {
                        storage.set_setting(SETTING_SETTINGS_AUTH, 1);
                    } else {
                        storage.set_setting(SETTING_SETTINGS_AUTH, 0);
                    }
                    // If there was storage problem abort
                    if (system_control.test_error(ERROR_SD)) return;

                    print();
                    break;
                case 3: // Line 3: Promjeni PIN
                    main_panel.set_page(change_pin_page);
                    break;
            }
            break;
    }
}

/********************************************************************
 * Change pin page functions                                        *
 ********************************************************************/
change_pin_page_class::change_pin_page_class() {
    set_cursor(-1, -1);
    is_password(FALSE);
}

void change_pin_page_class::print() {
    clear_buffer();
    lcd.setCursor(0, 0);
    lcd.print("Unesite novi PIN:");
}

void change_pin_page_class::update() {
    print_input(2);
}

void change_pin_page_class::key_press(const char key) {

    if (is_digit(key) || key == DELETE_KEY) {
        key_input(key);
    } else {
        switch(key) {
            case BACK_KEY:
                main_panel.set_page(auth_settings_page);
                break;
            case OK_KEY:
                storage.set_setting(SETTING_PASSWORD, get_buffer());
                // If there was storage problem abort
                if (system_control.test_error(ERROR_SD)) return;
                main_panel.set_page(auth_settings_page);
                break;
        }
    }
}

/********************************************************************
 * User list page functions                                         *
 ********************************************************************/
user_list_page_class::user_list_page_class() {
    set_cursor(1,2);
}

void user_list_page_class::private_print() {
    int user_count = storage.get_user_count();
    // If there was storage problem abort
    if (system_control.test_error(ERROR_SD)) return;

    if (user_count == 0) {
        set_cursor(-1, -1);
        lcd.setCursor(0,0);
        lcd.print("Datoteka korisnika");
        lcd.setCursor(0,1);
        lcd.print("prazna");
    } else {
        set_cursor(1,2);
        zero_cursor();
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Broj: +");

        user_record user = storage.get_user_by_pos(list_page);
        // If there was storage problem abort
        if (system_control.test_error(ERROR_SD)) return;
        current_user_id = user.id;

        lcd.print(user.number);
        lcd.setCursor(1, 1);
        lcd.print("Aktivan");
        lcd.setCursor(16, 1);
        lcd.print(
            (user.active) ? "DA" : "NE"
        );
        lcd.setCursor(1, 2);
        lcd.print("Izmjena broja");
        lcd.setCursor(0, 3);
        lcd.print("Str [");
        lcd.print(list_page + 1);
        lcd.print("/");
        lcd.print(user_count);
        lcd.print("]");
    }
}

void user_list_page_class::print() {
    list_page = 0;
    private_print();
}

void user_list_page_class::update() {
    print_cursor();
}

void user_list_page_class::key_press(const char key) {
    switch (key) {
        case UP_KEY:
            up_cursor();
            break;
        case DOWN_KEY:
            down_cursor();
            break;
        case NEXT_LIST_KEY:
            if (list_page < storage.get_user_count() - 1) {
                // If there was storage problem abort
                if (system_control.test_error(ERROR_SD)) return;

                ++list_page;
                private_print();
            }
            break;
        case PREVIUS_LIST_KEY:
            if (list_page > 0) {
                --list_page;
                private_print();
            }
            break;
        case BACK_KEY:
            main_panel.set_page(access_control_page);
            break;
        case OK_KEY:
            switch (cursor) {
                case 1: // Line 1: Aktivan DA/NE
                    if (storage.get_user_by_id(current_user_id).active) {
                        storage.dis_user(current_user_id);
                    } else {
                        storage.enb_user(current_user_id);
                    }
                    lcd.setCursor(16, 1);
                    lcd.print(
                        (storage.get_user_by_id(current_user_id).active) ? "DA" : "NE"
                    );
                    // If there was storage problem abort
                    if (system_control.test_error(ERROR_SD)) return;
                    break;
                case 2: // Line 2: Izmjena broja
                    number_edit_page.user_id = current_user_id;
                    main_panel.set_page(number_edit_page);
                    break;
            }
            break;
    }
}

/********************************************************************
 * Number edit page functions                                       *
 ********************************************************************/
number_edit_page_class::number_edit_page_class() {
    set_cursor(-1, -1);
    is_password(FALSE);
}

void number_edit_page_class::print() {
    clear_buffer();
    lcd.setCursor(0, 0);
    lcd.print("Unesite novi broj:");
}

void number_edit_page_class::update() {
    print_input(2);
}

void number_edit_page_class::key_press(const char key) {
    if (is_digit(key) || key == DELETE_KEY) {
        key_input(key);
    } else {
        switch (key) {
            case BACK_KEY:
                main_panel.set_page(user_list_page);
                break;
            case OK_KEY:
                if (strcompare(get_buffer(), "")) {
                    storage.delete_user(user_id);
                } else {
                    storage.edit_user(user_id, get_buffer());
                }
                // If there was storage problem abort
                if (system_control.test_error(ERROR_SD)) return;
                main_panel.set_page(user_list_page);
                break;
        }
    }
}

/********************************************************************
 * Number add page functions                                        *
 ********************************************************************/
number_add_page_class::number_add_page_class() {
    set_cursor(-1, -1);
    is_password(FALSE);
}

void number_add_page_class::print() {
    clear_buffer();
    lcd.setCursor(0, 0);
    lcd.print("Unesi novi broj: ");
}

void number_add_page_class::update() {
    print_input(2);
}

void number_add_page_class::key_press(const char key) {
    if (is_digit(key) || key == DELETE_KEY) {
        key_input(key);
    } else {
        switch (key) {
            case BACK_KEY:
                main_panel.set_page(access_control_page);
                break;
            case OK_KEY:
                if (!strcompare(get_buffer(), ""))
                    storage.add_user(get_buffer());
                // If there was storage problem abort
                if (system_control.test_error(ERROR_SD)) return;
                main_panel.set_page(access_control_page);
                break;
        }
    }
}

/********************************************************************
 * Log page functions                                               *
 ********************************************************************/
log_page_class::log_page_class() {
    set_cursor(-1, -1);
}

void log_page_class::private_print() {
    int log_count = storage.get_log_count();
    // If there was storage problem abort
    if (system_control.test_error(ERROR_SD)) return;
    // If there are no logs print message
    if (log_count == 0) {
        lcd.setCursor(0, 0);
        lcd.print("Log datoteka prazna");
    // Else print log list
    } else {
        // Get log and user
        log_record log = storage.get_log(record_num);
        user_record user = storage.get_user_by_id(log.user_id);
        // If there was storage problem abort
        if (system_control.test_error(ERROR_SD)) return;
        // Print date and time
        lcd.setCursor(0, 0);
        if (log.hour < 10) lcd.print("0");
        lcd.print(log.hour);
        lcd.print(":");
        if (log.minute < 10) lcd.print("0");
        lcd.print(log.minute);
        lcd.print(":");
        if (log.second < 10) lcd.print("0");
        lcd.print(log.second);
        lcd.print(" ");
        if (log.day < 10) lcd.print("0");
        lcd.print(log.day);
        lcd.print("-");
        if (log.month < 10) lcd.print("0");
        lcd.print(log.month);
        lcd.print("-");
        if (log.year < 1000) lcd.print("0");
        if (log.year < 100) lcd.print("0");
        if (log.year < 10) lcd.print("0");
        lcd.print(log.year);
        // Print number of user
        lcd.setCursor(0, 1);
        lcd.print("Kor: ");
        // If user is pseudo user do not print + before name
        if (user.id > USER_LAST_RESEVED)
            lcd.print("+");
        lcd.print(user.number);
        // Print code for action
        lcd.setCursor(0, 2);
        lcd.print("Log kod: ");
        lcd.print(log.action);
        // Print number of record
        lcd.setCursor(0, 3);
        lcd.print("Zapis: ");
        lcd.print(log_count - record_num);
    }
}

void log_page_class::print() {
    record_num = 0;
    private_print();
}

void log_page_class::update() {
    // Nothing to do
}

void log_page_class::key_press(const char key) {
    switch (key) {
        case NEXT_LIST_KEY:
            if (record_num + 1 < storage.get_log_count()) {
                // If there was storage problem abort
                if (system_control.test_error(ERROR_SD)) return;
                record_num += 1;
                lcd.clear();
                private_print();
            }
            break;
        case PREVIUS_LIST_KEY:
            if (record_num > 0) {
                record_num -= 1;
                lcd.clear();
                private_print();
            }
            break;
        case BACK_KEY:
            main_panel.set_page(settings_page);
            break;
    }
}

/********************************************************************
 * Incorrect PIN page functions                                     *
 ********************************************************************/
incorrect_pin_page_class::incorrect_pin_page_class() {
    set_cursor(-1, -1);
}

void incorrect_pin_page_class::print() {
    lcd.setCursor(3, 1);
    lcd.print("PIN netocan !!");
}

void incorrect_pin_page_class::update() {
    // Nothing to do
}

void incorrect_pin_page_class::key_press(const char key) {
    switch (key) {
        case OK_KEY:
            // Fall to next
        case BACK_KEY:
            main_panel.set_page(main_menu_page);
    }
}

/********************************************************************
 * Siren enter password page functions                              *
 ********************************************************************/
siren_pass_page_class::siren_pass_page_class() {
    set_cursor(-1, -1);
    is_password(TRUE);
}

void siren_pass_page_class::print() {
    clear_buffer();
    lcd.setCursor(0, 0);
    lcd.print("Unesite PIN:");
}

void siren_pass_page_class::update() {
    print_input(2);
}

void siren_pass_page_class::key_press(const char key) {

    if (is_digit(key) || key == DELETE_KEY) {
        key_input(key);
    } else {
        switch(key) {
            case BACK_KEY:
                main_panel.set_page(main_menu_page);
                break;
            case OK_KEY:
                char *pin = storage.get_setting(SETTING_PASSWORD).string_value;
                // If there was storage problem abort
                if (system_control.test_error(ERROR_SD)) return;
                if (strcompare(get_buffer(), pin)) {
                    main_panel.set_page(siren_page);
                } else {
                    main_panel.set_page(incorrect_pin_page);
                }
                break;
        }
    }
}

/********************************************************************
 * Siren page functions                                             *
 ********************************************************************/
siren_page_class::siren_page_class() {
    set_cursor(0, 3);
}

void siren_page_class::print() {
    zero_cursor();
    lcd.setCursor(1, 0);
    lcd.print("Nadolazeca");
    lcd.setCursor(1, 1);
    lcd.print("Neposredna");
    lcd.setCursor(1, 2);
    lcd.print("Prestanak");
    lcd.setCursor(1, 3);
    lcd.print("Vatrogasna");
}

void siren_page_class::update() {
    print_cursor();
}

void siren_page_class::key_press(const char key) {
    switch (key) {
        case UP_KEY:
            up_cursor();
            break;
        case DOWN_KEY:
            down_cursor();
            break;
        case BACK_KEY:
            main_panel.set_page(main_menu_page);
            break;
        case OK_KEY:
            switch (cursor) {
                case 0: // Line 0: Nadolazeca opasnos
                    main_panel.set_page(nadolazeca_siren_page);
                    break;
                case 1: // Line 1: Neposredna opasnos
                    main_panel.set_page(neposredna_siren_page);
                    break;
                case 2: // Line 2: Prestanak opasnost
                    main_panel.set_page(prestanak_siren_page);
                    break;
                case 3: // Line 3: Vatrogasna uzbuna
                    main_panel.set_page(vatrogasna_siren_page);
                    break;
            }
            break;
    }
}

/********************************************************************
 * Nadolazeca opasnost siren page functions                         *
 ********************************************************************/
nadolazeca_siren_page_class::nadolazeca_siren_page_class() {
    set_cursor(2, 3);
}

void nadolazeca_siren_page_class::print() {
    zero_cursor();
    lcd.setCursor(0, 0);
    lcd.print("Nadolazeca opasnost");
    lcd.setCursor(0, 1);
    lcd.print("pokretanje uzbune");
    lcd.setCursor(1, 2);
    lcd.print("Pokreni");
    lcd.setCursor(1, 3);
    lcd.print("Odustani");
}

void nadolazeca_siren_page_class::update() {
    print_cursor();
}

void nadolazeca_siren_page_class::key_press(const char key) {
    switch (key) {
        case UP_KEY:
            up_cursor();
            break;
        case DOWN_KEY:
            down_cursor();
            break;
        case BACK_KEY:
            main_panel.set_page(siren_page);
            break;
        case OK_KEY:
            switch (cursor) {
                case 2: // Line 2: Pokreni
                    // >> Siren shit <<
                    break;
                case 3: // Line 3: Odustani
                    main_panel.set_page(siren_page);
                    break;
            }
            break;
    }
}

/********************************************************************
 * Neposredna opasnost siren page functions                         *
 ********************************************************************/
neposredna_siren_page_class::neposredna_siren_page_class() {
    set_cursor(2, 3);
}

void neposredna_siren_page_class::print() {
    zero_cursor();
    lcd.setCursor(0, 0);
    lcd.print("Neposredna opasnost");
    lcd.setCursor(0, 1);
    lcd.print("pokretanje uzbune");
    lcd.setCursor(1, 2);
    lcd.print("Pokreni");
    lcd.setCursor(1, 3);
    lcd.print("Odustani");
}

void neposredna_siren_page_class::update() {
    print_cursor();
}

void neposredna_siren_page_class::key_press(const char key) {
    switch (key) {
        case UP_KEY:
            up_cursor();
            break;
        case DOWN_KEY:
            down_cursor();
            break;
        case BACK_KEY:
            main_panel.set_page(siren_page);
            break;
        case OK_KEY:
            switch (cursor) {
                case 2: // Line 2: Pokreni
                    // >> Siren shit <<
                    break;
                case 3: // Line 3: Odustani
                    main_panel.set_page(siren_page);
                    break;
            }
            break;
    }
}

/********************************************************************
 * Prestanak opasnosti siren page functions                         *
 ********************************************************************/
prestanak_siren_page_class::prestanak_siren_page_class() {
    set_cursor(2, 3);
}

void prestanak_siren_page_class::print() {
    zero_cursor();
    lcd.setCursor(0, 0);
    lcd.print("Prestanak opasnosti");
    lcd.setCursor(0, 1);
    lcd.print("pokretanje uzbune");
    lcd.setCursor(1, 2);
    lcd.print("Pokreni");
    lcd.setCursor(1, 3);
    lcd.print("Odustani");
}

void prestanak_siren_page_class::update() {
    print_cursor();
}

void prestanak_siren_page_class::key_press(const char key) {
    switch (key) {
        case UP_KEY:
            up_cursor();
            break;
        case DOWN_KEY:
            down_cursor();
            break;
        case BACK_KEY:
            main_panel.set_page(siren_page);
            break;
        case OK_KEY:
            switch (cursor) {
                case 2: // Line 2: Pokreni
                    // >> Siren shit <<
                    break;
                case 3: // Line 3: Odustani
                    main_panel.set_page(siren_page);
                    break;
            }
            break;
    }
}

/********************************************************************
 * Vatrogasna uzbuna siren page functions                           *
 ********************************************************************/
vatrogasna_siren_page_class::vatrogasna_siren_page_class() {
    set_cursor(2, 3);
}

void vatrogasna_siren_page_class::print() {
    zero_cursor();
    lcd.setCursor(0, 0);
    lcd.print("Vatrogasna uzbuna");
    lcd.setCursor(0, 1);
    lcd.print("pokretanje uzbune");
    lcd.setCursor(1, 2);
    lcd.print("Pokreni");
    lcd.setCursor(1, 3);
    lcd.print("Odustani");
}

void vatrogasna_siren_page_class::update() {
    print_cursor();
}

void vatrogasna_siren_page_class::key_press(const char key) {
    switch (key) {
        case UP_KEY:
            up_cursor();
            break;
        case DOWN_KEY:
            down_cursor();
            break;
        case BACK_KEY:
            main_panel.set_page(siren_page);
            break;
        case OK_KEY:
            switch (cursor) {
                case 2: // Line 2: Pokreni
                    // >> Siren shit <<
                    break;
                case 3: // Line 3: Odustani
                    main_panel.set_page(siren_page);
                    break;
            }
            break;
    }
}