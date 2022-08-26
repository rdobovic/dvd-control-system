// Include global header files
#include <Arduino.h>
// Include local header files
#include "modem.hpp"
#include "sms_pdu.hpp"
#include "system.hpp"
#include "storage.hpp"
#include "relays.hpp"
#include "panel.hpp"

// Create modem variable
modem_manipulation modem;

// Create commands variables
startup_cmd startup_modem;
check_cmd check_modem;
sms_cmd sms_modem;
delay_cmd delay_modem;
config_cmd config_modem;

// Create response handler variables
delivery_res delivery_modem;
ring_res ring_modem;
ring_end_res ring_end_modem;

/********************************************************************
 * Template class for creating unsolitited resposes functions       *
 ********************************************************************/
unsolicited_response::unsolicited_response() {
    res_starts_with[0] = '\0';
    is_done = 1;
}

void unsolicited_response::set_start(const char start[]) {
    strcopy(start, res_starts_with, 40);
}

int unsolicited_response::test(const char response[]) {
    return strstartswith(response, res_starts_with);
}

int unsolicited_response::done() {
    return is_done;
}

/********************************************************************
 * Template class for creating AT commands functions                *
 ********************************************************************/
at_command::at_command() {
    is_done = 1;
    execution_start = 0UL;
}

int at_command::done() {
    if (execution_start > millis())
        execution_start = millis();
    if (millis() - execution_start > MODEM_RESPONSE_WAIT) {
        system_control.set_error(ERROR_MODEM_TIMEOUT);
        is_done = 1;
    }

    return is_done;
}

void at_command::execute() {
    is_done = 0;
    execution_start = millis();
    send_to_serial();
}

/********************************************************************
 * Command to initiate modem on startup                             *
 ********************************************************************/
startup_cmd::startup_cmd() {
    start_counter = 0;
}

void startup_cmd::send_to_serial() {
    Serial3.println("AT");
    res_wait = millis();
    current_stage = WAITING_FOR_CMD_RES;
}

void startup_cmd::push_line(const char line[]) {
    // If response is OK finish
    if (strcompare(line, "OK")) {
        start_counter = 0;
        current_stage = EVERYTHING_IS_DONE;
        is_done = 1;
    // If response is AT keep waiting
    } else if (strcompare(line, "AT")) {
        current_stage = WAITING_FOR_CMD_RES;
        res_wait = millis();
    // If response is something else send AT command again
    } else {
        send_to_serial();
    }
}

void startup_cmd::update() {
    // Act based on current stage
    switch (current_stage) {
        case WAITING_FOR_CMD_RES:
            // If there is no anwser to test AT command in 2s
            // Initiate power on procedure
            if (millis() - res_wait > 2000) {
                digitalWrite(MODEM_POWER_PIN, HIGH);
                res_wait = millis();
                current_stage = WAITING_TO_POWER;
            }
            break;
        case WAITING_TO_TURN_ON:
            // Give modem 5s to turn on, after that start test again
            if (millis() - res_wait > 5000) {
                // If modem refuses to turn on set system error
                if (start_counter > 2) {
                    system_control.set_error(ERROR_MODEM_TURN_ON);
                    current_stage = EVERYTHING_IS_DONE;
                    is_done = 1;
                // Test if modem is finally on
                } else {
                    ++start_counter;
                    send_to_serial();
                }
            }
            break;
        case WAITING_TO_POWER:
            // After 1s of waiting set modem power pin back to LOW
            if (millis() - res_wait > 1000) {
                digitalWrite(MODEM_POWER_PIN, LOW);
                res_wait = millis();
                current_stage = WAITING_TO_TURN_ON;
            }
            break;
        case EVERYTHING_IS_DONE:
            /* Do nothing */
            break;
    }
}

/********************************************************************
 * Modem manipulation functions                                     *
 ********************************************************************/
modem_manipulation::modem_manipulation() {
    buffer[0] = '\0';
    current_ch = 0;
    cmd_getter = -1;
    cmd_setter = 0;
    handler_count = 0;
    check_start = 0;
    // Add handlers
    add_handler(delivery_modem);
    add_handler(ring_modem);
    add_handler(ring_end_modem);
}

void modem_manipulation::init() {
    Serial3.begin(9600);
    pinMode(MODEM_POWER_PIN, OUTPUT);
    digitalWrite(MODEM_POWER_PIN, LOW);
}

void modem_manipulation::update() {
    int i;  // Index counter

    // If there was SD error turn off ready indicator and do nothing
    if (system_control.test_error(ERROR_SD)) {
        system_control.ready(OFF);
        return;
    }
    // If FATAL modem error accured do nothing
    if (system_control.test_error(ERROR_MODEM_TURN_ON)) {
        system_control.ready(OFF);
        return;
    }
    // If modem command timeout error accured
    if (system_control.test_error(ERROR_MODEM_TIMEOUT)) {
        #ifdef MODEM_DEBUG
            Serial.println(F("## MODEM loop: TIMEOUT error found"));
            Serial.flush();
        #endif
        int i;    // Handler counter

        // Unset TIMEOUT error
        system_control.unset_error(ERROR_MODEM_TIMEOUT);

        // Set ready indicator to OFF
        system_control.ready(OFF);
        // Clear command buffer
        current_cmd = NULL;
        cmd_getter = -1;
        cmd_setter = 0;
        // Clear active unsilicited responses
        for (i = 0; i < handler_count; i++) {
            if (!handlers[i]->done()) {
                handlers[i]->is_done = 1;
            }
        }
        // Clear character buffer
        current_ch = 0;
        buffer[current_ch] = '\0';
        // Delay all commands for 5 seconds
        delay_modem.set_delay(5000);
        run_cmd(delay_modem);
    }

    // Test if modem is OK in interval of READY_CHECK_INTERVAL
    if ((millis() > FIRST_READY_CHECK_WAIT && check_start == 0) || millis() - check_start > READY_CHECK_INTERVAL) {
        #ifdef MODEM_DEBUG
            Serial.println(F("## MODEM loop: Running modem OK check"));
            Serial.flush();
        #endif
        check_start = millis();
        run_cmd(check_modem);
    }

    // Run dynamic actions of command if needed
    if (current_cmd != NULL) {
        current_cmd->update();
    }
    
    // If there is data waiting on Serial3 (modem serial)
    while (Serial3.available()) {
        // Read character from Serial3
        char ch = Serial3.read();
        // If character is \n skip it
        // Also if character value is less than zero there is somthing
        // very wrong, so skip that character
        if (ch == '\n' || ch < 0) continue;
        // If character is return pass buffer to commands
        if (ch == '\r') {
            // If buffer is empty skip it
            if (buffer[0] == '\0' && current_ch == 0) continue;
            #ifdef MODEM_DEBUG
                // Print to serial for testing
                Serial.print(F("## MODEM loop SERIAL3: "));
                Serial.println(buffer);
                Serial.flush();
            #endif
            // Search if there is handler for this modem output
            for (i = 0; i < handler_count; i++) {
                if (handlers[i]->test(buffer) || !handlers[i]->done()) {
                    #ifdef MODEM_DEBUG
                        Serial.println(F("## MODEM loop: handler found !"));
                        Serial.flush();
                    #endif
                    // If handler is found execute it
                    handlers[i]->execute(buffer);
                    // And empty buffer
                    current_ch = 0;
                    buffer[current_ch] = '\0';
                    break;
                }
            }

            // If handler was found skip following part
            if (buffer[0] == '\0' && current_ch == 0) continue;
            // If there is command listening for response pass string to
            // command, then empty buffer
            if (current_cmd != NULL && !current_cmd->done())
                current_cmd->push_line(buffer);
            current_ch = 0;
            buffer[current_ch] = '\0';
        }
        // If character is anything else store it in buffer array
        else {
            // Check for the buffer overflow
            if (current_ch < BUFFER_SIZE - 1) {
                buffer[current_ch] = ch;
                ++current_ch;
                buffer[current_ch] = '\0';
            }
        }
    }

    // If command is done move to next command from buffer
    // or set pointer to NULL
    if (current_cmd != NULL && current_cmd->done()) {
        // If queue is not empty
        if (cmd_getter != -1) {
            #ifdef MODEM_DEBUG
                Serial.println(F("## MODEM loop: Current cmd DONE, executing next !"));
                Serial.flush();
            #endif
            // Take next command from queue
            current_cmd = cmd_buffer[cmd_getter];
            // If this was last command set getter to -1
            if ((cmd_getter + 1) % CMD_BUFFER_SIZE == cmd_setter)
                cmd_getter = -1;
            // Else set getter to index of next command
            else 
                cmd_getter = (cmd_getter + 1) % CMD_BUFFER_SIZE;
            // Finally execute new command
            current_cmd->execute();
        } else {
            #ifdef MODEM_DEBUG
                Serial.println(F("## MODEM loop: Current cmd DONE, all done !"));
                Serial.flush();
            #endif
            current_cmd = NULL;
        }
    }
}

void modem_manipulation::run_cmd(at_command &cmd) {
    // If there is not command currently executing execute command now
    if (current_cmd == NULL) {
        current_cmd = &cmd;
        current_cmd->execute();
    // Else add command to queue
    } else {
        // If queue is full discard command
        if (cmd_setter == cmd_getter) return;
        // If queue is empty set getter to setter value
        if (cmd_getter == -1)
            cmd_getter = cmd_setter;
        // Add command to queue and go to next index
        cmd_buffer[cmd_setter] = &cmd;
        cmd_setter = (cmd_setter + 1) % CMD_BUFFER_SIZE;
    }
}

void modem_manipulation::start() {
    // Start modem
    run_cmd(startup_modem);
    // Set delay to 4s
    delay_modem.set_delay(4000);
    // Run delay and then configure modem
    run_cmd(delay_modem);
    run_cmd(config_modem);
}

void modem_manipulation::add_handler(unsolicited_response &handler) {
    // Add handler to handlers array
    handlers[handler_count] = &handler;
    // Increase handlers count
    ++handler_count;
}

/********************************************************************
 * Command to check if modem is ready                               *
 ********************************************************************/
check_cmd::check_cmd() {
    currently_waiting = COMMAND_ECHO;
}

void check_cmd::send_to_serial() {
    Serial3.println("AT+CCID;+CPIN?;+CSQ;+CREG?");
    currently_waiting = COMMAND_ECHO;
}

void check_cmd::push_line(const char line[]) {
    switch (currently_waiting) {
        case COMMAND_ECHO:
            if (strcompare(line, "AT+CCID;+CPIN?;+CSQ;+CREG?")) {
                currently_waiting = SIM_CARD_ID;
            } else {
                system_control.ready(OFF);
                system_control.set_error(ERROR_MODEM_UNKNOWN);
                is_done = 1;
            }
            break;
        case SIM_CARD_ID:
            if (strcompare(line, "ERROR")) {
                system_control.ready(OFF);
                system_control.set_error(ERROR_MODEM_SIM);
                is_done = 1;
            } else {
                currently_waiting = SIM_CARD_PIN;
            }
            break;
        case SIM_CARD_PIN:
            if (strstartswith(line, "+CPIN: ")) {
                char sim_card_code[20];  // Status code returned by +CPIN 

                sscanf(line, "+CPIN: %s", sim_card_code);
                if (strcompare(sim_card_code, "READY")) {
                    currently_waiting = SIGNAL_QUALITY;
                } else {
                    system_control.ready(OFF);
                    system_control.set_error(ERROR_MODEM_SIM);
                    is_done = 1;
                }
            } else {
                system_control.ready(OFF);
                system_control.set_error(ERROR_MODEM_UNKNOWN);
                is_done = 1;
            }
            break;
        case SIGNAL_QUALITY:
            if (strstartswith(line, "+CSQ: ")) {
                int code1, code2;  // Numbers before and after , in result of command

                sscanf(line, "+CSQ: %d,%d", &code1, &code2);
                if (code1 > 0) {
                    currently_waiting = REGISTRATION_STATUS;
                } else {
                    system_control.ready(OFF);
                    system_control.set_error(ERROR_MODEM_SIGNAL);
                    is_done = 1;
                }
            } else {
                system_control.ready(OFF);
                system_control.set_error(ERROR_MODEM_UNKNOWN);
                is_done = 1;
            }
            break;
        case REGISTRATION_STATUS:
            if (strstartswith(line, "+CREG: ")) {
                int code1, code2;  // Numbers before and after , in result of command

                sscanf(line, "+CREG: %d,%d", &code1, &code2);
                if (code2 == 1 || code2 == 5) {
                    currently_waiting = FINAL_OK;
                } else {
                    system_control.ready(OFF);
                    system_control.set_error(ERROR_MODEM_REGISTER);
                    is_done = 1;
                }
            } else {
                system_control.ready(OFF);
                system_control.set_error(ERROR_MODEM_UNKNOWN);
                is_done = 1;
            }
            break;
        case FINAL_OK:
            if (strcompare(line, "OK")) {
                system_control.ready(ON);
                if (system_control.test_error(ERROR_MODEM))
                    system_control.unset_error(ERROR_MODEM);
                is_done = 1;
            } else {
                system_control.ready(OFF);
                system_control.set_error(ERROR_MODEM_UNKNOWN);
                is_done = 1;
            }
    }
}

/********************************************************************
 * Command to send sms messages                                     *
 ********************************************************************/
sms_cmd::sms_cmd() {
    getter = -1;
    setter = 0;
}

void sms_cmd::send_to_serial() {
    // If there is nothing in queue do nothing
    if (getter == -1) {
        is_done = 1;
        return;
    }
    // Send sms message
    Serial3.print("AT+CMGS=");
    Serial3.println(tpdu_length[getter]);
    // Modem needs 120ms to start listening for PDU start
    delay(150);
    // Send PDU
    Serial3.print(pdu[getter]);
    Serial3.println("\x1A");
    // Start listening for modem response
    currently_waiting = COMMAND_ECHO;
}

void sms_cmd::clear() {
    // Set getter and setter like there is nothing in the queue
    getter = -1;
    setter = 0;
}

void sms_cmd::add_message(const char number[], const char message[]) {
    #ifdef MODEM_DEBUG
        Serial.println(F("MODEM sms: Adding new message to queue (from RAM)"));
        Serial.flush();
    #endif
    // If queue is full discard message
    if (setter == getter) return;
    // If getter is -1 (queue is empty) set it to current field
    if (getter == -1)
        getter = setter;
    // Initiate builder and calculate PDU
    builder new_pdu;
    new_pdu.set_number(number);
    new_pdu.set_message(message);
    new_pdu.calculate();
    // Store calculated PDU to the queue
    strcopy(new_pdu.get_pdu(), pdu[setter], BUFFER_SIZE);
    // Store TPDU length to the queue
    tpdu_length[setter] = new_pdu.get_tpdu_length();
    // Go to next field in the queue
    setter = (setter + 1) % SMS_BUFFER_SIZE;
}

void sms_cmd::add_message(const char number[], const __FlashStringHelper *message) {
    #ifdef MODEM_DEBUG
        Serial.println(F("MODEM sms: Adding new message to queue (from FLASH)"));
        Serial.flush();
    #endif
    // If queue is full discard message
    if (setter == getter) return;
    // If getter is -1 (queue is empty) set it to current field
    if (getter == -1)
        getter = setter;
    // Initiate builder and calculate PDU
    builder new_pdu;
    new_pdu.set_number(number);
    new_pdu.set_message(message);
    new_pdu.calculate();
    // Store calculated PDU to the queue
    strcopy(new_pdu.get_pdu(), pdu[setter], BUFFER_SIZE);
    // Store TPDU length to the queue
    tpdu_length[setter] = new_pdu.get_tpdu_length();
    // Go to next field in the queue
    setter = (setter + 1) % SMS_BUFFER_SIZE;
}

void sms_cmd::push_line(const char line[]) {
    switch (currently_waiting) {
        case COMMAND_ECHO:
            // Once command echo arrived
            if (strstartswith(line, "AT+CMGS=")) {
                // Move to waiting for PDU echo
                currently_waiting = PDU_ECHO;
                // Move to next message in the queue
                if ((getter + 1) % SMS_BUFFER_SIZE == setter) {
                    getter = -1;
                } else {
                    getter = (getter + 1) % SMS_BUFFER_SIZE;
                }
            }
            break;
        case PDU_ECHO:
            // On PDU echo continue waiting for message reference
            if (strstartswith(line, ">")) {
                currently_waiting = MSG_INFO;
            } else if (strcompare(line, "ERROR")) {
                system_control.ready(OFF);
                system_control.set_error(ERROR_MODEM_SMS_SEND);
                is_done = 1;
            }
            break;
        case MSG_INFO:
            // And finally after message reference wait for final OK
            if (strstartswith(line, "+CMGS: ")) {
                currently_waiting = FINAL_OK;
            } else if (strcompare(line, "ERROR")) {
                system_control.ready(OFF);
                system_control.set_error(ERROR_MODEM_SMS_SEND);
                is_done = 1;
            }
            break;
        case FINAL_OK:
            // Then move to next message or finish
            if (strcompare(line, "OK")) {
                if (getter == -1) {
                    is_done = 1;
                } else {
                    send_to_serial();
                }
            } else if (strcompare(line, "ERROR")) {
                system_control.ready(OFF);
                system_control.set_error(ERROR_MODEM_SMS_SEND);
                is_done = 1;
            }
            break;
    }
}

/********************************************************************
 * Command to pause execution of commands                           *
 ********************************************************************/
delay_cmd::delay_cmd() {
    delay_start = 0;
    delay_duritation = 0;
}

void delay_cmd::send_to_serial() {
    #ifdef MODEM_DEBUG
        Serial.println(F("## MODEM delay: Pause started"));
        Serial.flush();
    #endif
    delay_start = millis();
    is_done = 0;
}

void delay_cmd::set_delay(unsigned long delay_dur) {
    delay_duritation = delay_dur;
}

void delay_cmd::push_line(const char line[]) {
    /* Do nothing, ignore everything that comes from modem during delay */
}

void delay_cmd::update() {
    // If millis() returned to 0, set delay_start to millis
    if (millis() < delay_start)
        delay_start = millis();
    // If enough time has passed finish
    if (millis() - delay_start > delay_duritation) {
        #ifdef MODEM_DEBUG
            Serial.println(F("## MODEM delay: Pause ended"));
            Serial.flush();
        #endif
        is_done = 1;
    }
    // Else continue doing nothing
}

/********************************************************************
 * Command to configure modem                                       *
 ********************************************************************/
config_cmd::config_cmd() {
    currently_waiting = COMMAND_ECHO;
}

void config_cmd::send_to_serial() {
    #ifdef MODEM_DEBUG
        Serial.println(F("## MODEM config: config started"));
        Serial.flush();
    #endif
    Serial3.println("AT+CMGF=0;+CNMI=2,2,0,0,0");
    currently_waiting = COMMAND_ECHO;
    is_done = 0;
}

void config_cmd::push_line(const char line[]) {
    switch (currently_waiting) {
        case COMMAND_ECHO:
            if (strcompare(line, "AT+CMGF=0;+CNMI=2,2,0,0,0")) {
                currently_waiting = FINAL_OK;
            }
            break;
        case FINAL_OK:
            if (strcompare(line, "OK")) {
                #ifdef MODEM_DEBUG
                    Serial.println(F("## MODEM config: config ended"));
                    Serial.flush();
                #endif
                is_done = 1;
            }
            break;
    }
}

/********************************************************************
 * Unsolicited response activated when new message arrives          *
 ********************************************************************/
delivery_res::delivery_res() {
    set_start("+CMT");
}

void delivery_res::execute(const char response[]) {
    if (is_done == 1) {
        is_done = 0;
    } else {
        is_done = 1;

        parser message;
        user_record user;
        message.set_pdu(response);
        
        #ifdef MODEM_DEBUG
            Serial.print(F("## MODEM new msg: "));
            Serial.print(message.get_number());
            Serial.print(F(" -- "));
            Serial.println(message.get_message());
            Serial.flush();
        #endif

        // If SD card is not functional abort
        if (system_control.test_error(ERROR_SD)) return;
        // Check if user exist in users file
        user = storage.get_user_by_num(message.get_number());
        // If user do not exist, or is disabled do nothing
        if (user.id == USER_DELETED || user.active == 0) return;

        const char *txt = message.get_message();  // Pointer to message text, just to make things more readable
        const char *num = message.get_number();   // Pointer to sender number, just to make things more readable

        Serial.flush();
        
        // If it's a door command
        if (strcompare(txt, "vmo") || strcompare(txt, "vmz") || strcompare(txt, "vvo") || strcompare(txt, "vvz")) {
            // For small door
            if (txt[1] == 'm') {
                // Get current door state
                int state = relay.get_door_small();
                // If door is in unknown position send error message
                if (state == DOOR_MIDDLE || state == DOOR_ERROR) {
                    sms_modem.add_message(num, F("Greska, mala vrata su u nepoznatom polozaju"));
                }
                // If door opening is requested
                else if (txt[2] == 'o') {
                    // Open the door if door is closed
                    if (state == DOOR_CLOSED) {
                        sms_modem.add_message(num, F("Pokrecem postupak otvaranja malih vrata"));
                        storage.log_this(user.id, "VMO");
                        relay.door_small(DOPEN);
                    }
                    else if (state == DOOR_OPENED) {
                        sms_modem.add_message(num, F("Nemoguce otvoriti mala vrata, vrata su vec otvorena"));
                    }
                }
                // If door closing is requested
                else if (txt[2] == 'z') {
                    // Close the door if door is opened
                    if (state == DOOR_OPENED) {
                        sms_modem.add_message(num, F("Pokrecem postupak zatvaranja malih vrata"));
                        storage.log_this(user.id, "VMZ");
                        relay.door_small(DCLOSE);
                    }
                    else if (state == DOOR_CLOSED) {
                        sms_modem.add_message(num, F("Nemoguce zatvoriti mala vrata, vrata su vec zatvorena"));
                    }
                }
            }
            // For big door
            else if (txt[1] == 'v') {
                // Get current door state
                int state = relay.get_door_big();
                // If door is in unknown position send error message
                if (state == DOOR_MIDDLE || state == DOOR_ERROR) {
                    sms_modem.add_message(num, F("Greska, velika vrata su u nepoznatom polozaju"));
                }
                // If door opening is requested
                else if (txt[2] == 'o') {
                    if (state == DOOR_CLOSED) {
                        sms_modem.add_message(num, F("Pokrecem postupak otvaranja velikih vrata"));
                        storage.log_this(user.id, "VVO");
                        relay.door_big(DOPEN);
                    }
                    else if (state == DOOR_OPENED) {
                        sms_modem.add_message(num, F("Nemoguce otvoriti velika vrata, vrata su vec otvorena"));
                    }
                }
                // If door closing is requested
                else if (txt[2] == 'z') {
                    if (state == DOOR_OPENED) {
                        sms_modem.add_message(num, F("Pokrecem postupak zatvaranja velikih vrata"));
                        storage.log_this(user.id, "VVZ");
                        relay.door_big(DCLOSE);
                    }
                    else if (state == DOOR_CLOSED) {
                        sms_modem.add_message(num, F("Nemoguce zatvoriti velika vrata, vrata su vec zatvorena"));
                    }
                }
            }
        }
        // If it's siren command
        else if (strcompare(txt, "una") || strcompare(txt, "une") || strcompare(txt, "upr") || strcompare(txt, "uva") || strcompare(txt, "ust")) {
            // Get siren state
            int state = relay.get_siren();
            // If siren stop is requested stop siren
            if (txt[1] == 's' && txt[2] == 't') {
                sms_modem.add_message(num, F("Pokrecem postupak zaustavljanja sirene ukoliko je aktivna"));
                storage.log_this(user.id, "UST");
                relay.siren_stop();
            }
            // If siren is running and other siren is requested send error
            else if (state != SIREN_OFF) {
                sms_modem.add_message(num, F("Sirena je trenutno aktivna, kako biste pokrenuli sirenu zaustavite je, te ponovo pokrenite"));
            }
            // Start siren nadolazeca opasnost
            else if (txt[1] == 'n' && txt[2] == 'a') {
                sms_modem.add_message(num, F("Pokrecem uzbunu Nadolazeca opasnost"));
                storage.log_this(user.id, "UNA");
                relay.siren_nadolazeca();
            }
            // Start siren neposredna opasnost
            else if (txt[1] == 'n' && txt[2] == 'e') {
                sms_modem.add_message(num, F("Pokrecem uzbunu Neposredna opasnost"));
                storage.log_this(user.id, "UNE");
                relay.siren_neposredna();
            }
            // Start siren prestanak opasnosti
            else if (txt[1] == 'p' && txt[2] == 'r') {
                sms_modem.add_message(num, F("Pokrecem uzbunu Prestanak opasnosti"));
                storage.log_this(user.id, "UPR");
                relay.siren_prestanak();
            }
            // Start siren vatrogasna uzbuna
            else if (txt[1] == 'v' && txt[2] == 'a') {
                sms_modem.add_message(num, F("Pokrecem uzbunu Vatrogasna uzbuna"));
                storage.log_this(user.id, "UVA");
                relay.siren_vatrogasna();
            }
        }
        // If it's light command
        else if (strcompare(txt, "son") || strcompare(txt, "sof")) {
            // Get current state of light
            int state = relay.get_light();
            // If light on is requested
            if (txt[2] == 'n') {
                if (state == OFF) {
                    sms_modem.add_message(num, F("Pokrecem postupak paljenja svjetla"));
                    storage.log_this(user.id, "SON");
                    relay.light(ON);
                }
                else if (state == ON) {
                    sms_modem.add_message(num, F("Nemoguce upaliti svjetlo, svjetlo je vec upaljeno"));
                }
            }
            // If light off is requested
            else if (txt[2] == 'f') {
                if (state == ON) {
                    sms_modem.add_message(num, F("Pokrecem postupak gasenja svjetla"));
                    storage.log_this(user.id, "SOF");
                    relay.light(OFF);
                }
                else if (state == OFF) {
                    sms_modem.add_message(num, F("Nemoguce ugasiti svjetlo, svjetlo je vec ugaseno"));
                }
            }
        }
        // If it's status command
        else if (strcompare(txt, "status")) {
            char anwser[100];
            int big = relay.get_door_big();      // Get state of big door
            int small = relay.get_door_small();  // Get state of small door
            int light = relay.get_light();       // Get state of light
            int siren = relay.get_siren();       // Get state of siren

            // Create anwser message
            sprintf(anwser, "%s\n\nMala Vrata: %s\nVelika Vrata: %s\nSvjetlo: %s\nSirena: %s",
                // Get current motd and set it as title
                main_panel.get_motd(),
                // Print current state of small door
                (small == DOOR_ERROR) ? "ERR" :                                    // If door state is error print ERR
                    (small == DOOR_MIDDLE) ? "NEP" :                               // If door state is unknown print NEP
                        (small == DOOR_OPENED) ? "OTV" :                           // If door state is opened print OTV
                            "ZAT",                                                 // Else door must be in closed state so print ZAT
                // Print current state of big door
                (big == DOOR_ERROR) ? "ERR" :                                      // If door state is error print ERR
                    (big == DOOR_MIDDLE) ? "NEP" :                                 // If door state is unknown print NEP
                        (big == DOOR_OPENED) ? "OTV" :                             // If door state is opened print OTV
                            "ZAT",                                                 // Else door must be in closed state so print ZAT
                // Print current state of light
                (light == ON) ? "ON" :                                             // If light is on print ON
                    "OFF",                                                         // If light is off print OFF
                // Get siren currently running
                (siren == SIREN_OFF) ? "OFF" :                                     // If no siren is running print OFF
                    (siren == SIREN_NADOLAZECA) ? "Nadolazeca opasnost" :          // If siren is Nadolazeca opasnost
                        (siren == SIREN_NEPOSREDNA) ? "Neposredna opasnost" :      // If siren is Neposredna opasnost
                            (siren == SIREN_PRESTANAK) ? "Prestanak opasnosti" :   // If siren is Prestanak opasnosti
                                "Vatrogasna uzbuna"                                // Else it must be Vatrogasna uzbuna
            );

            sms_modem.add_message(num, anwser);
        }
        // If it's log command
        else if (strstartswith(txt, "log")) {
            unsigned long page;
            if (sscanf(txt, "log %lu", &page) == 1) {
                if (storage.get_log_count() > (page - 1u) * SMS_LOG) {
                    unsigned long i;
                    char log_list[160];
                    unsigned long log_count = storage.get_log_count();

                    log_list[0] = '\0';

                    for (i = 0; i < SMS_LOG && log_count > (page - 1u) * SMS_LOG + i; i++) {
                        log_record logr = storage.get_log((page - 1u) * SMS_LOG + i);
                        user_record userr = storage.get_user_by_id(logr.user_id);

                        sprintf(
                            log_list + strlength(log_list), "%02u-%02u-%04u %02u:%02u:%02u %s %s\n",
                            logr.day, logr.month, logr.year, logr.hour, logr.minute, logr.second, logr.action, userr.number
                        );
                    }
                    sprintf(
                        log_list + strlength(log_list), "\nStr %lu/%lu",
                        page, (log_count / SMS_LOG) + !!(log_count % SMS_LOG)
                    );

                    sms_modem.add_message(num, log_list);
                } else {
                    sms_modem.add_message(num, F("Trazena stranica loga ne postoji"));
                }
            } else {
                sms_modem.add_message(num, F("Sintaksa naredbe log je:\nlog <stranica>"));
            }
        }
        // If it's premosti command
        else if (strstartswith(txt, "premosti")) {
            // Get subcommand
            const char *subcmd = txt + 8;
            // If override of big door is requested
            if (strcompare(subcmd, " vvo") || strcompare(subcmd, " vvz")) {
                storage.log_this(user.id, "PVV");
                relay.override_door_big(DOPEN);
                sms_modem.add_message(num, F("Zaobilazim sigurnosne provjere magnetskih senzora i pokrecem promjenu stanja velikih vrata bez obzira na trenutno stanje"));
            }
            // If override of small door opening is requested
            else if (strcompare(subcmd, " vmo")) {
                storage.log_this(user.id, "PMO");
                relay.override_door_small(DOPEN);
                sms_modem.add_message(num, F("Zaobilazim sigurnosne provjere magnetskih senzora i pokrecem otvaranje malih vrata bez obzira na trenutno stanje"));
            }
            // If override of small door closing is requested
            else if (strcompare(subcmd, " vmz")) {
                storage.log_this(user.id, "PMZ");
                relay.override_door_small(DCLOSE);
                sms_modem.add_message(num, F("Zaobilazim sigurnosne provjere magnetskih senzora i pokrecem zatvaranje malih vrata bez obzira na trenutno stanje"));
            }
            // If command syntax is incorrect send error and help
            else {
                sms_modem.add_message(num, F("Sintaksa naredbe premosti je:\npremosti <vvo/vvz/vmo/vmz>"));
            }
        }
        // If it's special command 1
        else if (strcompare(txt, "1")) {
            // Log requested actions
            storage.log_this(user.id, "VVO");
            storage.log_this(user.id, "VMO");
            storage.log_this(user.id, "SON");
            // Perform requested actions
            relay.door_big(DOPEN);
            relay.door_small(DOPEN);
            relay.light(ON);
            // Send report
            sms_modem.add_message(num, F("Pokrecem grupno izvrsavanje naredbi:\n- Otvori mala vrata\n- Otvori velika vrata\n- Upali svjetlo"));
        }
        // If it's special command 2
        else if (strcompare(txt, "2")) {
            // Log requested actions
            storage.log_this(user.id, "VVO");
            // Perform requested actions
            relay.door_big(DOPEN);
            // Send report
            sms_modem.add_message(num, F("Pokrecem pokusaj otvaranja velikih vrata"));
        }
        // If it's special command 3
        else if (strcompare(txt, "3")) {
            // Log requested actions
            storage.log_this(user.id, "VMO");
            // Perform requested actions
            relay.door_small(DOPEN);
            // Send report
            sms_modem.add_message(num, F("Pokrecem pokusaj otvaranja malih vrata"));
        }
        // If it's special command 4
        else if (strcompare(txt, "4")) {
            // Log requested actions
            storage.log_this(user.id, "VVO");
            storage.log_this(user.id, "VMO");
            storage.log_this(user.id, "SON");
            storage.log_this(user.id, "UVA");
            // Perform requested actions
            relay.door_big(DOPEN);
            relay.door_small(DOPEN);
            relay.light(ON);
            relay.siren_vatrogasna();
            // Send report
            sms_modem.add_message(num, F("Pokrecem grupno izvrsavanje naredbi:\n- Otvori mala vrata\n- Otvori velika vrata\n- Upali svjetlo\n- Pokreni Vatrogasnu uzbunu"));
        }
        // If it's spacial command 5
        else if (strcompare(txt, "5")) {
            // Log requested actions
            storage.log_this(user.id, "VVZ");
            storage.log_this(user.id, "VMZ");
            storage.log_this(user.id, "SOF");
            // Perform requested actions
            relay.door_big(DCLOSE);
            relay.door_small(DCLOSE);
            relay.light(OFF);
            // Send report
            sms_modem.add_message(num, F("Pokrecem grupno izvrsavanje naredbi:\n- Zatvori mala vrata\n- Zatvori velika vrata\n- Ugasi svjetlo"));
        }

        // If there is SMS ERROR or replies are disabled clear messages
        if (!SMS_REPLY || system_control.test_error(ERROR_MODEM_SMS_SEND)) {
            sms_modem.clear();
        // Else send reply
        } else {
            modem.run_cmd(sms_modem);
        }
    }
}

/********************************************************************
 * Unsolicited response activated when modem rings                  *
 ********************************************************************/
ring_res::ring_res() {
    set_start("RING");
}

void ring_res::execute(const char response[]) {
    /* Do nothing */
    #ifdef MODEM_DEBUG
        Serial.println(F("## MODEM ring: Someone is calling"));
        Serial.flush();
    #endif
}

/********************************************************************
 * Unsolicited response activated when modem ended ringing          *
 ********************************************************************/
ring_end_res::ring_end_res() {
    set_start("NO CARRIER");
}

void ring_end_res::execute(const char response[]) {
    /* Do nothing */
    #ifdef MODEM_DEBUG
        Serial.println(F("## MODEM ring stop: Ringing stopped, no anwser"));
        Serial.flush();
    #endif
}