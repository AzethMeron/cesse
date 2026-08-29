#ifndef CESSE_BOOL_H
#define CESSE_BOOL_H

/**
* @file cesse/bool.h
* @author Jakub Grzana
* @date August 2026
* @brief Boolean type abstraction
* 
* This headerfile includes bool type, as well as keyword true and false
* For C23 all that's needed is #include <stdbool.h>, but it's kept for the sake of compability:
* if sooner or later there's need to port library for earlier C versions, here those symbols shall be implemented.
*/

#include <stdbool.h>

//typedef char bool;
//#define true (1)
//#define false (0)

#endif