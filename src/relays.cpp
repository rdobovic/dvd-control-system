// Include global header files
#include <Arduino.h>
// Include local header files
#include "relays.hpp"
#include "storage.hpp"
#include "system.hpp"
#include "panel.hpp"

relay_control relay;

// >> General

relay_control::relay_control() {
    // Set default values for light variables
    current_light_state = -1;
    light_change_counter = 0;
    // Set default values for siren variables
    current_siren = SIREN_OFF;
    seconds_counter = 0;
    siren_start = 0;
    // Set default values for doors variables
    is_waiting_big = FALSE;
    waiting_start_big = 0;
    is_waiting_small = FALSE;
    waiting_start_small = 0;
}

void relay_control::init() {
    // Set mode for relay pins
    pinMode(LIGHT_PIN, OUTPUT);
    digitalWrite(LIGHT_PIN, LOW);
    pinMode(SIREN_PIN, OUTPUT);
    digitalWrite(SIREN_PIN, LOW);
    pinMode(BIG_DOOR_PIN, OUTPUT);
    digitalWrite(BIG_DOOR_PIN, LOW);
    pinMode(SMALL_DOOR_OPEN_PIN, OUTPUT);
    digitalWrite(SMALL_DOOR_OPEN_PIN, LOW);
    pinMode(SMALL_DOOR_CLOSE_PIN, OUTPUT);
    digitalWrite(SMALL_DOOR_CLOSE_PIN, LOW);
    // Set mode for sensor pins
    pinMode(LIGHT_S, INPUT);
    digitalWrite(LIGHT_S, LOW);
    pinMode(BIG_DOOR_S_OPEN, INPUT);
    digitalWrite(BIG_DOOR_S_OPEN, LOW);
    pinMode(BIG_DOOR_S_CLOSE, INPUT);
    digitalWrite(BIG_DOOR_S_CLOSE, LOW);
    pinMode(SMALL_DOOR_S_OPEN, INPUT);
    digitalWrite(SMALL_DOOR_S_OPEN, LOW);
    pinMode(SMALL_DOOR_S_CLOSE, INPUT);
    digitalWrite(SMALL_DOOR_S_CLOSE, LOW);

    // Check last known state of light and set light to that state
    int state = storage.get_setting(SETTING_LAST_LIGHT_STATE).int_value;
    // If there is problem with SD card leave light as it is
    if (system_control.test_error(ERROR_SD)) {
        current_light_state = -1;
    // Else set light to state from memory
    } else {
        if (state)
            current_light_state = ON;
        else
            current_light_state = OFF;
    }
    light_time = millis();
}

void relay_control::update() {
    // Get current state of light
    int light_state = digitalRead(LIGHT_S);
    // If state of light changed store current state to storage
    if (last_light_state != light_state) {
        last_light_state = light_state;
        storage.set_setting(SETTING_LAST_LIGHT_STATE, light_state);
    }
    // If light state is scheduled to be changed, try to change light state each 500ms
    if (current_light_state != -1 && (millis() - light_time) / 500u >= (unsigned long)light_change_counter) {
        // If system tried to change light state 10 times without success set error
        if (light_change_counter > 10) {
            current_light_state = -1;
            light_change_counter = 0;
            system_control.set_error(ERROR_LIGHT_UNKNOWN);
        } else {
            ++light_change_counter;
            // If light is not in requested state change relay state
            if (digitalRead(LIGHT_S) != current_light_state) {
                digitalWrite(LIGHT_PIN, !digitalRead(LIGHT_PIN));
            // Else finish
            } else {
                current_light_state = -1;
                light_change_counter = 0;
            }
        }
    }

    // Handle sirens based on selected siren
    unsigned long current_millis = millis();
    switch (current_siren) {
        case SIREN_NADOLAZECA:
            // If second passed reset motd (counter to the end is on motd)
            if ((current_millis - siren_start) / 1000 >= (unsigned int)seconds_counter) {
                char new_motd[21];
                sprintf(new_motd, "Nadolazeca    % 4ds ", 100 - seconds_counter);
                main_panel.set_motd(4, new_motd);
                ++seconds_counter;
            }
            // Turn siren relay ON and OFF depending on siren graph
            if ((current_millis - siren_start) > 100000u) {
                digitalWrite(SIREN_PIN, LOW);
                current_siren = SIREN_OFF;
                main_panel.clear_motd(4);
            } else if ((current_millis - siren_start) / 20000u % 2u == 0u) {
                digitalWrite(SIREN_PIN, HIGH);
            } else {
                if ((current_millis - siren_start) % 20000u / 3000u % 2u == 0u) {
                    digitalWrite(SIREN_PIN, LOW);
                } else {
                    digitalWrite(SIREN_PIN, HIGH);
                }
            }
            break;
        case SIREN_NEPOSREDNA:
            // If second passed reset motd (counter to the end is on motd)
            if ((current_millis - siren_start) / 1000 >= (unsigned int)seconds_counter) {
                char new_motd[21];
                sprintf(new_motd, "Neposredna    % 4ds ", 60 - seconds_counter);
                main_panel.set_motd(4, new_motd);
                ++seconds_counter;
            }
            // Turn siren relay ON and OFF depending on siren graph
            if ((current_millis - siren_start) > 60000u) {
                digitalWrite(SIREN_PIN, LOW);
                current_siren = SIREN_OFF;
                main_panel.clear_motd(4);
            } else if ((current_millis - siren_start) / 3000u % 2u == 0u) {
                digitalWrite(SIREN_PIN, HIGH);
            } else {
                digitalWrite(SIREN_PIN, LOW);
            }
            break;
        case SIREN_VATROGASNA:
            // If second passed reset motd (counter to the end is on motd)
            if ((current_millis - siren_start) / 1000 >= (unsigned int)seconds_counter) {
                char new_motd[21];
                sprintf(new_motd, "Vatrogasna    % 4ds ", 90 - seconds_counter);
                main_panel.set_motd(4, new_motd);
                ++seconds_counter;
            }
            // Turn siren relay ON and OFF depending on siren graph
            if ((current_millis - siren_start) > 90000u) {
                digitalWrite(SIREN_PIN, LOW);
                current_siren = SIREN_OFF;
                main_panel.clear_motd(4);
            } else if ((current_millis - siren_start) % 35000u < 20000u) {
                digitalWrite(SIREN_PIN, HIGH);
            } else {
                digitalWrite(SIREN_PIN, LOW);
            }
            break;
        case SIREN_PRESTANAK:
            // If second passed reset motd (counter to the end is on motd)
            if ((current_millis - siren_start) / 1000 >= (unsigned int)seconds_counter) {
                char new_motd[21];
                sprintf(new_motd, "Prestanak     % 4ds ", 60 - seconds_counter);
                main_panel.set_motd(4, new_motd);
                ++seconds_counter;
            }
            // Turn siren relay ON and OFF depending on siren graph
            if ((current_millis - siren_start) > 60000u) {
                digitalWrite(SIREN_PIN, LOW);
                current_siren = SIREN_OFF;
                main_panel.clear_motd(4);
            } else {
                digitalWrite(SIREN_PIN, HIGH);
            }
            break;
        case SIREN_OFF:
            /* Do nothing */
            break;
    }

    // Turn off control relay of big door if 1s has passed
    if (is_waiting_big && millis() - waiting_start_big > 1000u) {
        is_waiting_big = FALSE;
        digitalWrite(BIG_DOOR_PIN, LOW);
    }
    // Turn off control relays of small door if 1s has passed
    if (is_waiting_small && millis() - waiting_start_small > 1000u) {
        is_waiting_small = FALSE;
        digitalWrite(SMALL_DOOR_OPEN_PIN, LOW);
        digitalWrite(SMALL_DOOR_CLOSE_PIN, LOW);
    }
}

// >> Lights

void relay_control::light(int new_state) {
    current_light_state = new_state;
    light_change_counter = 0;
    light_time = millis();
}

int relay_control::get_light() {
    return digitalRead(LIGHT_S);
}

// >> Sirens

void relay_control::siren_nadolazeca() {
    if (current_siren == SIREN_OFF) {
        siren_start = millis();
        current_siren = SIREN_NADOLAZECA;
        seconds_counter = 0;
    }
}

void relay_control::siren_neposredna() {
    if (current_siren == SIREN_OFF) {
        siren_start = millis();
        current_siren = SIREN_NEPOSREDNA;
        seconds_counter = 0;
    }
}

void relay_control::siren_vatrogasna() {
    if (current_siren == SIREN_OFF) {
        siren_start = millis();
        current_siren = SIREN_VATROGASNA;
        seconds_counter = 0;
    }
}

void relay_control::siren_prestanak() {
    if (current_siren == SIREN_OFF) {
        siren_start = millis();
        current_siren = SIREN_PRESTANAK;
        seconds_counter = 0;
    }
}

void relay_control::siren_stop() {
    digitalWrite(SIREN_PIN, LOW);
    current_siren = SIREN_OFF;
    main_panel.clear_motd(4);
}

enum sirens relay_control::get_siren() {
    return current_siren;
}

// >> Doors

void relay_control::door_big(int new_state) {
    switch (new_state) {
        case DOPEN:
            if (digitalRead(BIG_DOOR_S_CLOSE)) {
                is_waiting_big = TRUE;
                waiting_start_big = millis();
                digitalWrite(BIG_DOOR_PIN, HIGH);
            }
            break;
        case DCLOSE:
            if (digitalRead(BIG_DOOR_S_OPEN)) {
                is_waiting_big = TRUE;
                waiting_start_big = millis();
                digitalWrite(BIG_DOOR_PIN, HIGH);
            }
            break;
    }
}

void relay_control::door_small(int new_state) {
    switch (new_state) {
        case DOPEN:
            if (digitalRead(SMALL_DOOR_S_CLOSE)) {
                is_waiting_small = TRUE;
                waiting_start_small = millis();
                digitalWrite(SMALL_DOOR_OPEN_PIN, HIGH);
            }
            break;
        case DCLOSE:
            if (digitalRead(SMALL_DOOR_S_OPEN)) {
                is_waiting_small = TRUE;
                waiting_start_small = millis();
                digitalWrite(SMALL_DOOR_CLOSE_PIN, HIGH);
            }
            break;
    }
}

void relay_control::override_door_big(int new_state) {
    switch (new_state) {
        case DOPEN:
            // Fall through
            // Since big door has only one control relay, if door is stuck in
            // unknown position, only solution is to turn relay on and wait to
            // see what will happen
        case DCLOSE:
            // Force relay to turn on, regardless of door state
            is_waiting_big = TRUE;
            waiting_start_big = millis();
            digitalWrite(BIG_DOOR_PIN, HIGH);
            break;
    }
}

void relay_control::override_door_small(int new_state) {
    switch (new_state) {
        // Force open relay to turn on, regardless of door state
        case DOPEN:
            is_waiting_small = TRUE;
            waiting_start_small = millis();
            digitalWrite(SMALL_DOOR_OPEN_PIN, HIGH);
            break;
        // Force close relay to turn on, regardless of door state
        case DCLOSE:
            is_waiting_small = TRUE;
            waiting_start_small = millis();
            digitalWrite(SMALL_DOOR_CLOSE_PIN, HIGH);
            break;
    }
}

int relay_control::get_door_big() {
    // Form bitmask to indicate current state of sensors
    return (digitalRead(BIG_DOOR_S_OPEN) << 1) | (digitalRead(BIG_DOOR_S_CLOSE) << 0);
}

int relay_control::get_door_small() {
    // Form bitmask to indicate current state of sensors
    return (digitalRead(SMALL_DOOR_S_OPEN) << 1) | (digitalRead(SMALL_DOOR_S_CLOSE) << 0);
}

