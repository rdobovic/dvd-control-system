// Include global header files
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
// Include local header files
#include "panel.hpp"
#include "storage.hpp"
#include "system.hpp"
#include "relays.hpp"

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
pass_page_class pass_page;
auth_settings_page_class auth_settings_page;
change_pin_page_class change_pin_page;
user_list_page_class user_list_page;
number_edit_page_class number_edit_page;
number_add_page_class number_add_page;
log_page_class log_page;
incorrect_pin_page_class incorrect_pin_page;
siren_page_class siren_page;
nadolazeca_siren_page_class nadolazeca_siren_page;
neposredna_siren_page_class neposredna_siren_page;
prestanak_siren_page_class prestanak_siren_page;
vatrogasna_siren_page_class vatrogasna_siren_page;
unknown_door_state_page_class unknown_door_state_page;

/********************************************************************
 * Create global panel variables and functions                      *
 ********************************************************************/

panel main_panel(home_page);

panel::panel(menu_page_class &home_page) {
    // Set home page
    home_page_ptr = &home_page;
    current_page_ptr = NULL;
    back_page_ptr = NULL;
    // Button is not pressed
    button_pressed = FALSE;
    // Last interaction was never
    last_interaction = 0;
}

void panel::init() {
    setting_record motd;   // Get motd from permanent storage
    // Run display init
    init_display();
    // Set button 1
    pinMode(BUTTON1_PIN, INPUT);
    digitalWrite(BUTTON1_PIN, LOW);
    // Set button 2
    pinMode(BUTTON2_PIN, INPUT);
    digitalWrite(BUTTON2_PIN, LOW);
    // Set button 3
    pinMode(BUTTON3_PIN, INPUT);
    digitalWrite(BUTTON3_PIN, LOW);
    // Set button 4
    pinMode(BUTTON4_PIN, INPUT);
    digitalWrite(BUTTON4_PIN, LOW);
    // Check if custom motd has been set
    motd = storage.get_setting(SETTING_MOTD);
    if (system_control.test_error(ERROR_SD)) return;
    // If custom motd is set and there were no SD card errors
    if (!strcompare("", motd.string_value)) {
        set_motd(1, motd.string_value);        // Set that motd at level 1
    }
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

    if (current_page_ptr != home_page_ptr && millis() - last_interaction > GO_HOME_TIME) {
        go_home();
    }

    if (key) {
        last_interaction = millis();
        system_control.beep(1);
        current_page_ptr->key_press(key);
    }

    current_page_ptr->update();

    if (digitalRead(BUTTON1_PIN)) {
        if (!button_pressed) {
            last_interaction = millis();
            button_pressed = 1;
            system_control.beep(1);
            
            int siren_auth = storage.get_setting(SETTING_SIRENE_AUTH).int_value;
            // If there was storage problem skip PIN
            if (system_control.test_error(ERROR_SD)) {
                set_page(neposredna_siren_page);
            // Else if pin is needed ask for pin
            } else if (siren_auth) {
                pass_page.back_page(home_page);
                pass_page.redirect_page(neposredna_siren_page);
                set_page(pass_page);
            // Else just go to requested page
            } else {
                set_page(neposredna_siren_page);
            }
        }
    } else if (digitalRead(BUTTON2_PIN)) {
        if (!button_pressed) {
            last_interaction = millis();
            button_pressed = 1;
            system_control.beep(1);
            
            int siren_auth = storage.get_setting(SETTING_SIRENE_AUTH).int_value;
            // If there was storage problem skip PIN
            if (system_control.test_error(ERROR_SD)) {
                set_page(prestanak_siren_page);
            // Else if pin is needed ask for pin
            } else if (siren_auth) {
                pass_page.back_page(home_page);
                pass_page.redirect_page(prestanak_siren_page);
                set_page(pass_page);
            // Else just go to requested page
            } else {
                set_page(prestanak_siren_page);
            }
        }
    } else if (digitalRead(BUTTON3_PIN)) {
        if (!button_pressed) {
            last_interaction = millis();
            button_pressed = 1;
            system_control.beep(1);
            
            int siren_auth = storage.get_setting(SETTING_SIRENE_AUTH).int_value;
            // If there was storage problem skip PIN
            if (system_control.test_error(ERROR_SD)) {
                set_page(vatrogasna_siren_page);
            // Else if pin is needed ask for pin
            } else if (siren_auth) {
                pass_page.back_page(home_page);
                pass_page.redirect_page(vatrogasna_siren_page);
                set_page(pass_page);
            // Else just go to requested page
            } else {
                set_page(vatrogasna_siren_page);
            }
        }
    } else if (digitalRead(BUTTON4_PIN)) {
        if (!button_pressed) {
            last_interaction = millis();
            button_pressed = 1;
            system_control.beep(2);
            // Stop any running sirens
            relay.siren_stop();
            storage.log_this(USER_PANEL, "UST");
            set_page(home_page);
        }
    } else if (button_pressed) {
        button_pressed = 0;
    }
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
    char time_formated[16];               // Formated time string
    RtcDateTime now = rtc.GetDateTime();  // Current time
    // Get time to formated string
    sprintf(
        time_formated, "%02hhu:%02hhu:%02hhu    ",  // Add few extra spaces at the end because piece of shit
        now.Hour(), now.Minute(), now.Second()      // somethimes glitches and prints one extra digit at the end
    );
    // Print formated string to display
    lcd.setCursor(1, 2);
    lcd.print(time_formated);
}

void home_page_class::key_press(const char key) {
    switch(key) {
        case OK_KEY:
            main_panel.set_page(main_menu_page);
            break;
    }
}

void home_page_class::update_motd() {
    int i;                                           // Counter
    int length = strlength(main_panel.get_motd());   // Get length of motd
    // Print motd to first line
    lcd.setCursor(0,0);
    lcd.print(main_panel.get_motd());
    // Fill rest of the line with spaces
    for (i = 0; i < 20 - length; i++) {
        lcd.print(' ');
    }
    
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
    lcd.print(F("Vrata"));
    lcd.setCursor(1, 1);
    lcd.print(F("Sirene"));
    lcd.setCursor(1, 2);
    lcd.print(F("Svijetlo"));
    lcd.setCursor(1, 3);
    lcd.print(F("Postavke"));
}

void main_menu_page_class::update() {
    print_cursor();

    lcd.setCursor(15, 2);
    lcd.print(
        (relay.get_light()) ? F("ON ") : F("OFF")
    );
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
                            pass_page.back_page(main_menu_page);
                            pass_page.redirect_page(siren_page);
                            main_panel.set_page(pass_page);
                        // Else go to settings
                        } else {
                            main_panel.set_page(siren_page);
                        }
                    }
                    break;
                case 2: // Line 2: Svijetlo
                    // Change state of light to oposit of current state
                    storage.log_this(USER_PANEL, (!relay.get_light()) ? "SON" : "SOF");
                    relay.light(!relay.get_light());
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
                            pass_page.back_page(main_menu_page);
                            pass_page.redirect_page(settings_page);
                            main_panel.set_page(pass_page);
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
    lcd.print(F("Izaberi vrata"));
    lcd.setCursor(1, 1);
    lcd.print(F("Vrata Mala"));
    lcd.setCursor(1, 2);
    lcd.print(F("Vrata Velika"));
}

void doors_menu_page_class::update() {
    print_cursor();

    // Set door status for small door
    lcd.setCursor(15, 1);
    switch (relay.get_door_small()) {
        case DOOR_MIDDLE:
            lcd.print(F("NEP"));
            break;
        case DOOR_CLOSED:
            lcd.print(F("ZAT"));
            break;
        case DOOR_OPENED:
            lcd.print(F("OTV"));
            break;
        case DOOR_ERROR:
            lcd.print(F("ERR"));
            break;
    }
    // Set status for big door
    lcd.setCursor(15, 2);
    switch (relay.get_door_big()) {
        case DOOR_MIDDLE:
            lcd.print(F("NEP"));
            break;
        case DOOR_CLOSED:
            lcd.print(F("ZAT"));
            break;
        case DOOR_OPENED:
            lcd.print(F("OTV"));
            break;
        case DOOR_ERROR:
            lcd.print(F("ERR"));
            break;
    }
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
    lcd.print(F("Mala Vrata"));
    lcd.setCursor(1, 1);
    lcd.print(F("Otvori"));
    lcd.setCursor(1, 2);
    lcd.print(F("Zatvori"));
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
                    // If door is in the middle or there is an error with switches report error
                    if (relay.get_door_small() == DOOR_MIDDLE || relay.get_door_small() == DOOR_ERROR) {
                        main_panel.set_page(unknown_door_state_page);
                    // Try to open the door
                    } else {
                        relay.door_small(DOPEN);
                        storage.log_this(USER_PANEL, "VMO");
                        main_panel.set_page(doors_menu_page);
                    }
                    break;
                case 2: // Line 1: Zatvori
                    // If door is in the middle or there is an error with switches report error
                    if (relay.get_door_small() == DOOR_MIDDLE || relay.get_door_small() == DOOR_ERROR) {
                        main_panel.set_page(unknown_door_state_page);
                    // Try to close the door
                    } else {
                        relay.door_small(DCLOSE);
                        storage.log_this(USER_PANEL, "VMZ");
                        main_panel.set_page(doors_menu_page);
                    }
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
    lcd.print(F("Velika vrata"));
    lcd.setCursor(1, 1);
    lcd.print(F("Otvori"));
    lcd.setCursor(1, 2);
    lcd.print(F("Zatvori"));
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
                    // If door is in the middle report error
                    if (relay.get_door_big() == DOOR_MIDDLE || relay.get_door_big() == DOOR_ERROR) {
                        main_panel.set_page(unknown_door_state_page);
                    // Try to open the door
                    } else {
                        relay.door_big(DOPEN);
                        storage.log_this(USER_PANEL, "VVO");
                        main_panel.set_page(doors_menu_page);
                    }
                    break;
                case 2: // Line 1: Zatvori
                    // If door is in the middle report error
                    if (relay.get_door_big() == DOOR_MIDDLE || relay.get_door_big() == DOOR_ERROR) {
                        main_panel.set_page(unknown_door_state_page);
                    // Try to close the door
                    } else {
                        relay.door_big(DCLOSE);
                        storage.log_this(USER_PANEL, "VVZ");
                        main_panel.set_page(doors_menu_page);
                    }
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
    lcd.print(F("Izmjena postavki"));
    lcd.setCursor(1, 1);
    lcd.print(F("Kontrola pristupa"));
    lcd.setCursor(1, 2);
    lcd.print(F("Auth postavke"));
    lcd.setCursor(1, 3);
    lcd.print(F("Pregled loga"));
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
    lcd.print(F("Kontrola pristupa"));
    lcd.setCursor(1, 1);
    lcd.print(F("Korisnici"));
    lcd.setCursor(1, 2);
    lcd.print(F("Dodaj korisnika"));
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
 * Enter password page functions                                    *
 ********************************************************************/
pass_page_class::pass_page_class() {
    set_cursor(-1, -1);
    is_password(TRUE);
    redirect_page_ptr = NULL;
    back_page_ptr = NULL;
}

void pass_page_class::print() {
    clear_buffer();
    lcd.setCursor(0,0);
    lcd.print(F("Unesite PIN:"));
}

void pass_page_class::update() {
    print_input(2);
}

void pass_page_class::redirect_page(menu_page_class &page) {
    redirect_page_ptr = &page;
}

void pass_page_class::back_page(menu_page_class &page) {
    back_page_ptr = &page;
}

void pass_page_class::key_press(const char key) {

    if (is_digit(key) || key == DELETE_KEY) {
        key_input(key);
    } else {
        switch(key) {
            case BACK_KEY:
                main_panel.set_page(*back_page_ptr);
                break;
            case OK_KEY:
                // Get password
                char *pin = storage.get_setting(SETTING_PASSWORD).string_value;
                // If there was storage problem abort
                if (system_control.test_error(ERROR_SD)) return;
                // Check if entered pin is correct
                if (strcompare(get_buffer(), pin)) {
                    main_panel.set_page(*redirect_page_ptr);
                } else {
                    incorrect_pin_page.next_page(*back_page_ptr);
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
    lcd.print(F("Auth postavke"));
    lcd.setCursor(1, 1);
    lcd.print(F("Sirene Auth"));
    lcd.setCursor(1, 2);
    lcd.print(F("Postavke Auth"));
    lcd.setCursor(1, 3);
    lcd.print(F("Promjeni PIN"));

    lcd.setCursor(16, 1);
    lcd.print(
        (storage.get_setting(SETTING_SIRENE_AUTH).int_value) ? F("DA") : F("NE")
    );
    // If there was storage problem abort
    if (system_control.test_error(ERROR_SD)) return;
    lcd.setCursor(16, 2);
    lcd.print(
        (storage.get_setting(SETTING_SETTINGS_AUTH).int_value) ? F("DA") : F("NE")
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
    lcd.print(F("Unesite novi PIN:"));
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
        lcd.print(F("Datoteka korisnika"));
        lcd.setCursor(0,1);
        lcd.print(F("prazna"));
    } else {
        set_cursor(1,2);
        zero_cursor();
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(F("Broj: +"));

        user_record user = storage.get_user_by_pos(list_page);
        // If there was storage problem abort
        if (system_control.test_error(ERROR_SD)) return;
        current_user_id = user.id;

        lcd.print(user.number);
        lcd.setCursor(1, 1);
        lcd.print(F("Aktivan"));
        lcd.setCursor(16, 1);
        lcd.print(
            (user.active) ? F("DA") : F("NE")
        );
        lcd.setCursor(1, 2);
        lcd.print(F("Izmjena broja"));
        lcd.setCursor(0, 3);
        lcd.print(F("Str ["));
        lcd.print(list_page + 1);
        lcd.print(F("/"));
        lcd.print(user_count);
        lcd.print(F("]"));
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
                        (storage.get_user_by_id(current_user_id).active) ? F("DA") : F("NE")
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
    lcd.print(F("Unesite novi broj:"));
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
    lcd.print(F("Unesi novi broj: "));
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
        lcd.print(F("Log datoteka prazna"));
    // Else print log list
    } else {
        // Get log and user
        log_record log = storage.get_log(record_num);
        user_record user = storage.get_user_by_id(log.user_id);
        // If there was storage problem abort
        if (system_control.test_error(ERROR_SD)) return;
        // Print date and time
        lcd.setCursor(0, 0);
        if (log.hour < 10) lcd.print(F("0"));
        lcd.print(log.hour);
        lcd.print(F(":"));
        if (log.minute < 10) lcd.print(F("0"));
        lcd.print(log.minute);
        lcd.print(F(":"));
        if (log.second < 10) lcd.print(F("0"));
        lcd.print(log.second);
        lcd.print(F(" "));
        if (log.day < 10) lcd.print(F("0"));
        lcd.print(log.day);
        lcd.print(F("-"));
        if (log.month < 10) lcd.print(F("0"));
        lcd.print(log.month);
        lcd.print(F("-"));
        if (log.year < 1000) lcd.print(F("0"));
        if (log.year < 100) lcd.print(F("0"));
        if (log.year < 10) lcd.print(F("0"));
        lcd.print(log.year);
        // Print number of user
        lcd.setCursor(0, 1);
        lcd.print(F("Kor: "));
        // If user is pseudo user do not print + before name
        if (user.id > USER_LAST_RESEVED)
            lcd.print(F("+"));
        lcd.print(user.number);
        // Print code for action
        lcd.setCursor(0, 2);
        lcd.print(F("Log kod: "));
        lcd.print(log.action);
        // Print number of record
        lcd.setCursor(0, 3);
        lcd.print(F("Zapis: "));
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
    next_page_ptr = NULL;
}

void incorrect_pin_page_class::print() {
    lcd.setCursor(3, 1);
    lcd.print(F("PIN netocan !!"));
}

void incorrect_pin_page_class::update() {
    // Nothing to do
}

void incorrect_pin_page_class::next_page(menu_page_class &page) {
    next_page_ptr = &page;
}

void incorrect_pin_page_class::key_press(const char key) {
    switch (key) {
        case OK_KEY:
            // Fall to next
        case BACK_KEY:
            main_panel.set_page(*next_page_ptr);
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
    lcd.print(F("Nadolazeca"));
    lcd.setCursor(1, 1);
    lcd.print(F("Neposredna"));
    lcd.setCursor(1, 2);
    lcd.print(F("Prestanak"));
    lcd.setCursor(1, 3);
    lcd.print(F("Vatrogasna"));
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
    lcd.print(F("Nadolazeca opasnost"));
    lcd.setCursor(0, 1);
    lcd.print(F("pokretanje uzbune"));
    lcd.setCursor(1, 2);
    lcd.print(F("Pokreni"));
    lcd.setCursor(1, 3);
    lcd.print(F("Odustani"));
    system_control.beep(BEEP_DURITATION * 2, 3);
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
                    relay.siren_nadolazeca();
                    storage.log_this(USER_PANEL, "UNA");
                    main_panel.go_home();
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
    lcd.print(F("Neposredna opasnost"));
    lcd.setCursor(0, 1);
    lcd.print(F("pokretanje uzbune"));
    lcd.setCursor(1, 2);
    lcd.print(F("Pokreni"));
    lcd.setCursor(1, 3);
    lcd.print(F("Odustani"));
    system_control.beep(BEEP_DURITATION * 2, 3);
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
                    relay.siren_neposredna();
                    storage.log_this(USER_PANEL, "UNE");
                    main_panel.go_home();
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
    lcd.print(F("Prestanak opasnosti"));
    lcd.setCursor(0, 1);
    lcd.print(F("pokretanje uzbune"));
    lcd.setCursor(1, 2);
    lcd.print(F("Pokreni"));
    lcd.setCursor(1, 3);
    lcd.print(F("Odustani"));
    system_control.beep(BEEP_DURITATION * 2, 3);
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
                    relay.siren_prestanak();
                    storage.log_this(USER_PANEL, "UPR");
                    main_panel.go_home();
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
    lcd.print(F("Vatrogasna uzbuna"));
    lcd.setCursor(0, 1);
    lcd.print(F("pokretanje uzbune"));
    lcd.setCursor(1, 2);
    lcd.print(F("Pokreni"));
    lcd.setCursor(1, 3);
    lcd.print(F("Odustani"));
    system_control.beep(BEEP_DURITATION * 2, 3);
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
                    relay.siren_vatrogasna();
                    storage.log_this(USER_PANEL, "UVT");
                    main_panel.go_home();
                    break;
                case 3: // Line 3: Odustani
                    main_panel.set_page(siren_page);
                    break;
            }
            break;
    }
}

/********************************************************************
 * Unknown door state page functions                                *
 ********************************************************************/

unknown_door_state_page_class::unknown_door_state_page_class() {
    set_cursor(-1, -1);
}

void unknown_door_state_page_class::print() {
    lcd.setCursor(0, 0);
    lcd.print(F("Greska !!"));
    lcd.setCursor(0, 1);
    lcd.print(F("nepoznato stanje"));
    lcd.setCursor(0, 2);
    lcd.print(F("vrata"));
}

void unknown_door_state_page_class::update() {
    /* Do nothing */
}

void unknown_door_state_page_class::key_press(const char key) {
    switch (key) {
        case OK_KEY:
            // Fall to next
        case BACK_KEY:
            main_panel.set_page(doors_menu_page);
    }
}