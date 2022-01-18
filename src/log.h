/**
* Модуль функций записи в лог
* @file
* @version 0.0.2.1
*/

#if !defined( __LOG_H )
#define __LOG_H

#include "ext_types.h"

#define MAX_LOG_MSG 1024

/**
* Менеджер лога
*/
typedef struct
{
    FILE *out;
    BOOL isNew;
} nix_log_t;


nix_log_t* open_log(char *log_name);
BOOL close_log();

void add_log_line(char *fmt, ...);
void log_info(char *fmt, ...);
void log_err(char *fmt, ...);
void log_warning(char *fmt, ...);
void print_log_color_line(unsigned int color, char *fmt, ...);

#endif
