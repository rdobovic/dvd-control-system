#ifndef _INCLUDE_HELPER_FUNCTIONS_HPP_
#define _INCLUDE_HELPER_FUNCTIONS_HPP_

// Used to choose on/off state of device
#define ON 1
#define OFF 0
// Used instead instead of boolean variables
#define TRUE 1
#define FALSE 0
// Used to choose state of door
#define DOPEN 1
#define DCLOSE 0

/********************************************************************
 * strcopy -- Function to copy one zero terminated string to other  *
 *                                                                  *
 * Arguments                                                        *
 *     from_this   -- string to copy                                *
 *     to_this     -- place where to copy given string              *
 *     to_size     -- how many characters can fit in to_this array  *
 ********************************************************************/
void strcopy(const char from_this[], char to_this[], const int to_size);

/********************************************************************
 * strcompare -- Function to compare two zero terminated strings    *
 *                                                                  *
 * Arguments                                                        *
 *     string1   -- first string to compare                         *
 *     string2   -- second string to compare                        *
 *                                                                  *
 * Returns                                                          *
 *     True (1) if strings are equal and False (0) if they are not  *
 ********************************************************************/
int strcompare(const char string1[], const char string2[]);

/********************************************************************
 * is_digit -- Function to check if given character is a decimal    *
 *             digit                                                *
 *                                                                  *
 * Arguments                                                        *
 *     _ch   -- character to check if it's a digit                  *
 *                                                                  *
 * Returns                                                          *
 *     True (1) if character is a digit and False (0) if not        *
********************************************************************/
int is_digit(const char _ch);

/********************************************************************
 * strlength -- Function counts how many characters are in the      *
 *              string                                              *
 *                                                                  *
 * Arguments                                                        *
 *     string   -- string which length should be counted            *
 *                                                                  *
 * Returns                                                          *
 *     Number of characters in the string (npr. returns 2 for "tt") *
********************************************************************/
int strlength(const char string[]);

/********************************************************************
 * strstartswith -- Function checks if string1 starts with string2  *
 *                                                                  *
 * Arguments                                                        *
 *     string1  -- string to be checked                             *
 *     string2  -- string with which is checked                     *
 *                                                                  *
 * Returns                                                          *
 *     True (1) if string1 starts with string2 or False (0) if not  *
********************************************************************/
int strstartswith(const char string1[], const char string2[]);

#endif