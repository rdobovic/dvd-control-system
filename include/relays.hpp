#ifndef _INCLUDE_RELAYS_HPP_
#define _INCLUDE_RELAYS_HPP_

// Define pins where relays are connected
#define LIGHT_PIN 31             /* Relay 5 */
#define SIREN_PIN 29             /* Relay 4 */
#define BIG_DOOR_PIN 27          /* Relay 3 */
#define SMALL_DOOR_OPEN_PIN 25   /* Relay 2 */
#define SMALL_DOOR_CLOSE_PIN 23  /* Relay 1 */

// Define pins where sensors are connected
#define LIGHT_S 6                /* 220V sensor */
#define BIG_DOOR_S_OPEN 47       /* Magnet SW 4 */
#define BIG_DOOR_S_CLOSE 45      /* Magnet SW 3 */
#define SMALL_DOOR_S_OPEN 43     /* Magnet SW 2 */
#define SMALL_DOOR_S_CLOSE 41    /* Magnet SW 1 */

// Include general stuff
#include "helper_functions.hpp"

// Possible door states, that are actualy bitmasks for sensor states
// Lower digit indicates state of lower sensor, and higher of upper sensor
const int DOOR_MIDDLE = 0b00;
const int DOOR_CLOSED = 0b01;
const int DOOR_OPENED = 0b10;
const int DOOR_ERROR  = 0b11;

// All possible states of siren
enum sirens {
    SIREN_OFF,             // No siren is emitating at the moment
    SIREN_NADOLAZECA,      // Siren Nadolazeca opasnost is emitating
    SIREN_NEPOSREDNA,      // Siren Neposredna opasnost is emitating
    SIREN_VATROGASNA,      // Siren Vatrogasna uzbuna is emitating
    SIREN_PRESTANAK        // Siren Prestanak opasnosti is emitating
};

// Class used for controling relays and reading sensors asociated
// with functions that relays perform
class relay_control {
    private:
        // Light
        int current_light_state;   // Requested state of light ON/OFF, -1 if nothing is requested
        int last_light_state;      // Last known light state, also stored in storage
        int light_change_counter;  // Count how many times system attempted to change light state
        unsigned long light_time;  // millis() when light state was changed
        // Siren
        enum sirens current_siren; // Siren currently emitating
        int seconds_counter;       // Used so motd setter is fired up every 1s to display second left
        unsigned long siren_start; // millis() when siren started emitating
        // Doors
        int is_waiting_big;                // Indication if big door relay is waiting to be turned off
        unsigned long waiting_start_big;   // millis() when this waiting started
        int is_waiting_small;              // Indication if one of small door relays is waiting to be turned off
        unsigned long waiting_start_small; // millis() when this waiting started
    public:
        // Default constructor
        relay_control();
        // Initilize all nececery stuff
        void init();
        // Set state of light ON/OFF
        void light(int new_state);
        // Get current state of light
        int get_light();
        // Start siren nadolazeca opasnost
        void siren_nadolazeca();
        // Start siren neposredna opasnost
        void siren_neposredna();
        // Start siren vatrogasna uzbuna
        void siren_vatrogasna();
        // Start siren prestanak opasnosti
        void siren_prestanak();
        // Immediately stop any siren emitating
        void siren_stop();
        // Get current state of siren
        sirens get_siren();
        // Set state of big door DOPEN/DCLOSE
        void door_big(int new_state);
        // Override, turn on requested relay regardless of current door state
        void override_door_big(int new_state);
        // Get current state of big door
        int get_door_big();
        // Set state of small door DOPEN/DCLOSE
        void door_small(int new_state);
        // Override, turn on requested relay regardless of current door state
        void override_door_small(int new_state);
        // Get current state of small door
        int get_door_small();
        // Update dynamic stuff
        void update();
};

extern relay_control relay;

#endif