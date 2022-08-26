/*
 *    ______     ______     ____            _             _   ____            _                 
 *   |  _ \ \   / /  _ \   / ___|___  _ __ | |_ _ __ ___ | | / ___| _   _ ___| |_ ___ _ __ ___  
 *   | | | \ \ / /| | | | | |   / _ \| '_ \| __| '__/ _ \| | \___ \| | | / __| __/ _ \ '_ ` _ \ 
 *   | |_| |\ V / | |_| | | |__| (_) | | | | |_| | | (_) | |  ___) | |_| \__ \ ||  __/ | | | | |
 *   |____/  \_/  |____/   \____\___/|_| |_|\__|_|  \___/|_| |____/ \__, |___/\__\___|_| |_| |_|
 *                                                               |___/                       
 * 
 *   Author: rdobovic
 * 
 *   Version: 1.0.0
 * 
 *   DVDCS is a device used by fire brigade to remotly and locally perform actions inside fire 
 *   station. This version of DVDCS supports control over SMS, and using buttons on front panel
 *   of device. Using this device user can control two station doors, air siren, and light within
 *   the station. Configuration of device can be done using device command line, which can be
 *   accessed using USB serial connection.
 * 
 */

// Include global header files
#include <Arduino.h>
// Include local header files
#include "panel.hpp"
#include "storage.hpp"
#include "system.hpp"
#include "modem.hpp"
#include "relays.hpp"

// Initialize stuff on startup
void setup() {
    // Initialize system
    system_control.init();
    // Initialize storage
    storage.init();
    // Initialize panel (LCD and buttons)
    main_panel.init();
    // Initialize relays and sensors
    relay.init();
    // Initialize modem control
    modem.init();
    
    // Start modem if it's not running yet
    modem.start();
    // Go to home LCD page on startup
    main_panel.go_home();
}

// Update stuff based on user interactions for
// each part of the system
void loop() {
    // Run dynamic actions for panel
    main_panel.update();
    // Run dynamic actions for system
    system_control.update();
    // Run dynamic actions for modem
    modem.update();
    // Run dynamic actions for relays
    relay.update();
}