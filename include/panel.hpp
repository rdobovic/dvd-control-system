#ifndef _INCLUDE_PANEL_HPP_
#define _INCLUDE_PANEL_HPP_

// Include general stuff
#include "helper_functions.hpp"

// Define global buttons
#define UP_KEY 'A'
#define DOWN_KEY 'B'
#define OK_KEY '#'
#define BACK_KEY '*'
#define DELETE_KEY 'D'
#define NEXT_LIST_KEY 'D'
#define PREVIUS_LIST_KEY 'C'

// Define button pins
#define BUTTON1_PIN 39
#define BUTTON2_PIN 37
#define BUTTON3_PIN 35
#define BUTTON4_PIN 33

// If none of buttons is pressed for given number of ms panel will return to home page
#define GO_HOME_TIME 120000

// Max number of characters in motd
#define MAX_MOTD_SIZE 21
// Max number of motd levels
#define MAX_MOTD_LEVEL 9
// Default motd if no level is set
const char DEFAULT_MOTD[] = "DVDCS - By BBT";

// Max number of characters enterd through keyboard
#define MAX_INPUT 19

// Generic class that describes menu page
class menu_page_class {
    protected:
        // Line on which cursor is
        int cursor;
        // Cursor borders, set them to -1 to disable cursor
        int cursor_start, cursor_end;
        // Set cursor borders
        void set_cursor(int start, int end);
        // Move cursor up
        void up_cursor();
        // Move cursor down
        void down_cursor();
        // Set cursor to first position
        void zero_cursor();
        // Set cursor to first position
        void print_cursor();
    public:
        // Default constructor
        menu_page_class();
        // Print menu to display
        virtual void print() {}
        // Update dynamic parts of page (npr. cursor)
        virtual void update() {}
        // Do action based on key press
        virtual void key_press(const char key) {}
        // Update motd for page if page uses motd
        virtual void update_motd() {}
};

// Generic class that describes page with numeric input field
class input_page_class : public menu_page_class {
    private:
        // Array to store typed characters
        char input_buffer[MAX_INPUT];
        // Current position in array
        int position;
        // 1 if this is a password
        int password;
    protected:
        // Display * instead of characters
        void is_password(int is_pass);
        // Print input line to display
        void print_input(int line);
        // Get pointer to input
        const char * get_buffer();
        // Clear input buffer
        void clear_buffer();
        // Input a key
        void key_input(const char key);
    public:
        // Default constructor
        input_page_class();
        // Print menu to display
        virtual void print() {}
        // Update dynamic parts of page (npr. cursor)
        virtual void update() {}
        // Do action based on key press
        virtual void key_press(const char key) {}
};

// Class for in charge for handling interaction phisical interactions
// with system using system main panel (buttons, and display)
class panel {
    private:
        // Structure to hold motd for each level
        struct motd_element {
            int is_set = FALSE;             // 1 if motd is set for this level
            char text[MAX_MOTD_SIZE] = "";  // Text for given motd
        
        } motd_levels[MAX_MOTD_LEVEL + 1];  // Array of motd levels

        // Pointer to current menu page
        class menu_page_class *current_page_ptr;
        // Pointer to go back page
        class menu_page_class *back_page_ptr;
        // Pointer to home menu page
        class menu_page_class *home_page_ptr;
        // 1 if button was pressed in last update
        int button_pressed;
        // millis when last user interaction happend
        unsigned long last_interaction;

    public:
        // Constructor sets home page
        panel(menu_page_class &home_page);
        // Initilize panel
        void init();

        // Update panel readings and output
        void update();

        // Set motd for given level
        void set_motd(const int level, const char text[]);
        // Clear motd for given level
        void clear_motd(const int level);
        // Get pointer to text of highest level motd
        const char * get_motd();

        // Go to the home page
        void go_home();
        // Set page on display
        void set_page(menu_page_class &page);
        // Pass pressed key to current page
        void key(const char key);
};

extern panel main_panel;

// Home page
class home_page_class : public menu_page_class {
    public:
        home_page_class();
        void print();
        void update();
        void key_press(const char key);
        void update_motd();
};

extern home_page_class home_page;

// Main menu page
class main_menu_page_class : public menu_page_class {
    public:
        main_menu_page_class();
        void print();
        void update();
        void key_press(const char key);
};

extern main_menu_page_class main_menu_page;

// Doors menu page
class doors_menu_page_class : public menu_page_class {
    public:
        doors_menu_page_class();
        void print();
        void update();
        void key_press(const char key);
};

extern doors_menu_page_class doors_menu_page;

// Small door page
class small_door_page_class : public menu_page_class {
    public:
        small_door_page_class();
        void print();
        void update();
        void key_press(const char key);
};

extern small_door_page_class small_door_page;

// Big door page
class big_door_page_class : public menu_page_class {
    public:
        big_door_page_class();
        void print();
        void update();
        void key_press(const char key);
};

extern big_door_page_class big_door_page;

// Settings page
class settings_page_class : public menu_page_class {
    public:
        settings_page_class();
        void print();
        void update();
        void key_press(const char key);
};

extern settings_page_class settings_page;

// Access control page
class access_control_page_class : public menu_page_class {
    public:
        access_control_page_class();
        void print();
        void update();
        void key_press(const char key);
};

extern access_control_page_class access_control_page;

// Page to input pin before you can access settings menu
class pass_page_class : public input_page_class {
    private:
        menu_page_class *redirect_page_ptr;   // Pointer to success page
        menu_page_class *back_page_ptr;       // Pointer to back page
    public:
        // Default constructor
        pass_page_class();
        // Print static content of page
        void print();
        // Update dynamic content of page
        void update();
        // Set page to redirect to after if PIN is correct
        void redirect_page(menu_page_class &page);
        // Set page to go to if back key is pressed
        void back_page(menu_page_class &page);
        // Pass key to be handled by page
        void key_press(const char key);
};

extern pass_page_class pass_page;

// Auth settings page
class auth_settings_page_class : public menu_page_class {
    public:
        auth_settings_page_class();
        void print();
        void update();
        void key_press(const char key);
};

extern auth_settings_page_class auth_settings_page;

// Page to input new PIN when changing it
class change_pin_page_class : public input_page_class {
    public:
        change_pin_page_class();
        void print();
        void update();
        void key_press(const char key);
};

extern change_pin_page_class change_pin_page;

// User list page
class user_list_page_class : public menu_page_class {
    private:
        int list_page;        // Current page od list
        int current_user_id;  // ID of current user
        void private_print();
    public:
        user_list_page_class();
        void print();
        void update();
        void key_press(const char key);
};

extern user_list_page_class user_list_page;

// Edit number of existing user
class number_edit_page_class : public input_page_class {
    private:
        int user_id;    // User id to edit
    public:
        number_edit_page_class();
        void print();
        void update();
        void key_press(const char key);

    friend user_list_page_class;
};

extern number_edit_page_class number_edit_page;

// Add new user
class number_add_page_class : public input_page_class {
    public:
        number_add_page_class();
        void print();
        void update();
        void key_press(const char key);
};

extern number_add_page_class number_add_page;

// Page to lookup logs
class log_page_class : public menu_page_class {
    private:
        unsigned long record_num;   // How many records in the past
        void private_print();
    public:
        log_page_class();
        void print();
        void update();
        void key_press(const char key);
};

extern log_page_class log_page;

// Incorrect PIN page
class incorrect_pin_page_class : public menu_page_class {
    private:
        menu_page_class *next_page_ptr;  // Pointer to next page
    public:
        // Default constructor
        incorrect_pin_page_class();
        // Print static content of page
        void print();
        // Print dynamic content of page
        void update();
        // Set page to go to when any key is pressed
        void next_page(menu_page_class &page);
        // Pass key to the page to handle it
        void key_press(const char key);
};

extern incorrect_pin_page_class incorrect_pin_page;

// Sirens page
class siren_page_class : public menu_page_class {
    public:
        siren_page_class();
        void print();
        void update();
        void key_press(const char key);
};

extern siren_page_class siren_page;

// Confirm page for nadolazeca opasnost
class nadolazeca_siren_page_class : public menu_page_class {
    public:
        nadolazeca_siren_page_class();
        void print();
        void update();
        void key_press(const char key);
};

extern nadolazeca_siren_page_class nadolazeca_siren_page;

// Confirm page for neposredna opasnost
class neposredna_siren_page_class : public menu_page_class {
    public:
        neposredna_siren_page_class();
        void print();
        void update();
        void key_press(const char key);
};

extern neposredna_siren_page_class neposredna_siren_page;

// Confirm page for prestanak opasnosti
class prestanak_siren_page_class : public menu_page_class {
    public:
        prestanak_siren_page_class();
        void print();
        void update();
        void key_press(const char key);
};

extern prestanak_siren_page_class prestanak_siren_page;

// Confirm page for vatrogasna uzbuna
class vatrogasna_siren_page_class : public menu_page_class {
    public:
        vatrogasna_siren_page_class();
        void print();
        void update();
        void key_press(const char key);
};

extern vatrogasna_siren_page_class vatrogasna_siren_page;

// Doors are in unknown state error
class unknown_door_state_page_class : public menu_page_class {
    public:
        unknown_door_state_page_class();
        void print();
        void update();
        void key_press(const char key);
};

extern unknown_door_state_page_class unknown_door_state_page;


#endif