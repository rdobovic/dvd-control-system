#include <Arduino.h>

#include "system.hpp"
#include "menu.hpp"

system_class system_control;

static char motd[30];

system_class::system_class() {
    error_flags = 0;
}

void system_class:: set_error(const int flag) {
    error_flags |= flag;

    if (error_flags != 0) {
        sprintf(motd, "ERROR CODE 0x%04x", error_flags);

        main_panel.set_motd(8, motd);
        main_panel.go_home();
    } else {
        main_panel.clear_motd(8);
    }
}

void system_class::unset_error(const int flag) {
    error_flags &= ~flag;

    if (error_flags != 0) {
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