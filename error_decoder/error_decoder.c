#include <stdio.h>

int main() {
    int error_code;   // Variable to hold value of error code

    // Print heading
    printf("\nDVD Control system -- error code decoder\n\n");

    // Repeat until zero is entered
    while (1) {
        // Print message to the user
        printf("-----------------------------------------------------------\n\n");
        printf("Enter value of error code or 0 to exit: 0x");
        // Get error code from user
        scanf("%x", &error_code);

        // If error code is zero stop
        if (error_code == 0) {
            printf("\nStopping...\n");
            break;
        // Else calculate values of all error flags and display them to the user
        } else {
            printf("\nList of error flags for given error code: \n\n");

            printf("ERROR MODEM TURN ON      -- %d\n", !!(error_code & (1 << 0)));
            printf("ERROR MODEM TIMEOUT      -- %d\n", !!(error_code & (1 << 1)));
            printf("ERROR MODEM SIM          -- %d\n", !!(error_code & (1 << 2)));
            printf("ERROR MODEM SIGNAL       -- %d\n", !!(error_code & (1 << 3)));
            printf("ERROR MODEM REGISTER     -- %d\n", !!(error_code & (1 << 4)));
            printf("ERROR MODEM SMS SEND     -- %d\n", !!(error_code & (1 << 5)));
            printf("ERROR MODEM UNKNOWN      -- %d\n", !!(error_code & (1 << 6)));
            printf("ERROR SD INIT            -- %d\n", !!(error_code & (1 << 7)));
            printf("ERROR SD READ            -- %d\n", !!(error_code & (1 << 8)));
            printf("ERROR SD WRITE           -- %d\n", !!(error_code & (1 << 9)));
            printf("ERROR SD UNKNOWN         -- %d\n", !!(error_code & (1 << 10)));
            printf("ERROR RTC CONFIDENCE     -- %d\n", !!(error_code & (1 << 11)));
            printf("ERROR RTC UNKNOWN        -- %d\n", !!(error_code & (1 << 12)));
            printf("ERROR LIGHT UNKNOWN      -- %d\n", !!(error_code & (1 << 13)));

            printf("\n");
        }
    }
    return 0;
}