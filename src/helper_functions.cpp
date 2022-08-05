// Function to copy one zero terminated string to another
void strcopy( const char from_this[], char to_this[], const int to_size) {
    int i;  // Index counter

    for (i = 0; from_this[i] != '\0' && i < to_size; i++) {
        to_this[i] = from_this[i];
    }

    to_this[i] = '\0';
}

// Function to compare two zero terminated strings
int strcompare(const char string1[], const char string2[]) {
    int i;  // Index counter

    while(1) {
        if (string1[i] == '\0')
            break;
        if (string2[i] == '\0')
            break;
        if (string1[i] != string2[i])
            return 0;
        ++i;
    }

    return (string1[i] == '\0' && string2[i] == '\0');
}

// Function to check if given character is a decimal digit
int is_digit(const char _ch) {
    return _ch >= '0' && _ch <= '9';
}

// Function counts how many characters are in the string
int strlength(char string[]) {
    int i;  // Index counter
    for (i = 0; string[i] != '\0'; i++);
    return i;
}

// Function checks if string1 starts with string2
int strstartswith(const char string1[], const char string2[]) {
    int i;  // Index counter

    while(1) {
        if (string2[i] == '\0')
            return 1;
        if (string1[i] == '\0')
            return 0;
        if (string1[i] != string2[i])
            return 0;
        ++i;
    }
}