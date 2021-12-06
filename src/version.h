/**
* Модуль функций всё что связано с версией...
* @file
* @version 0.0.0.1
*/

#if !defined( __VERSION_H )
#define __VERSION_H

#include <stdio.h>
#include <string.h>
#include <sys/sysinfo.h>

char *get_version(void);
void print_version(void);
void print_help(void);
void print_system_info(void);


#endif /*__VERSION_H*/
