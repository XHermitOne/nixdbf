/**
* Модуль функций записи в лог
* @file
* @version 0.0.2.3
*/

#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "tools.h"
#include "log.h"
#include "strfunc.h"
#include "config.h"

static nix_log_t Log;

char DefaultLogFileName[] = "/nixdbf.log";     /**< Имя файла лога по умолчанию*/

/**
* Текущее время-дата
*/
char CurrentDateTime[128];

char *get_current_datetime()
{
    CurrentDateTime[0] = 0;
    time_t loc_time;
    time(&loc_time);

    struct tm *today = localtime(&loc_time);
    strftime(CurrentDateTime, 128, "%d/%m/%y (%H:%M:%S)", today);

    return CurrentDateTime;
}

/**
*   Открыть лог для записи
*       LogName - Имя файла лога
*           Если NULL, то берется имя по умолчанию
*/
nix_log_t* open_log(char *log_name)
{
    if (log_name == NULL)
        log_name = DefaultLogFileName;

    Log.out = NULL;
    Log.isNew = FALSE;
    char *cfg_path = get_config_path();

    char *full_log_filename = (char*) calloc(strlen(cfg_path) + strlen(log_name) + 1, sizeof(char));
    strcpy(full_log_filename, cfg_path);
    strcat(full_log_filename, log_name);

    Log.out = fopen(full_log_filename, "a");
    fprintf(Log.out, "[START LOG] %s - - - - - - - - - - - - - - - - - - - - -\n", get_current_datetime());

    destroy_str_and_null(full_log_filename);

    // Освобождаем память
    free(cfg_path);

    return &Log;
}


BOOL close_log()
{
    if (Log.out)
    {
        fprintf(Log.out, "[STOP LOG] %s - - - - - - - - - - - - - - - - - - - - -\n", get_current_datetime());
        fclose(Log.out);
        Log.out = NULL;
        return TRUE;
    }
    return FALSE;
}

/**
* Добавить строчку в лог
*/
void add_log_line(char *fmt, ...)
{
    char buffer[MAX_LOG_MSG];
    va_list ap;

    va_start(ap, fmt);
    vsprintf(buffer, fmt, ap);

    print_log_color_line(CYAN_COLOR_TEXT, buffer);

    va_end(ap);
}

/**
* Вывести информационную строку
*/
void log_info(char *fmt, ...)
{
    char buffer[MAX_LOG_MSG];
    va_list ap;

    va_start(ap, fmt);
    vsprintf(buffer, fmt, ap);

    print_log_color_line(GREEN_COLOR_TEXT, buffer);

    va_end(ap);
}

/**
* Сообщение об ошибке
*/
void log_err(char *fmt, ...)
{
    char buffer[MAX_LOG_MSG];
    va_list ap;

    va_start(ap, fmt);
    vsprintf(buffer, fmt, ap);

    print_log_color_line(RED_COLOR_TEXT, buffer);

    va_end(ap);
}


/**
* Предупреждение
*/
void log_warning(char *fmt, ...)
{
    char buffer[MAX_LOG_MSG];
    va_list ap;

    va_start(ap, fmt);
    vsprintf(buffer, fmt, ap);

    print_log_color_line(YELLOW_COLOR_TEXT, buffer);

    va_end(ap);
}


void print_log_color_line(unsigned int color, char *fmt, ...)
{
    if (Log.out)
    {
        va_list argptr;
        va_start(argptr, fmt);

        char msg[MAX_LOG_MSG];
        vsnprintf(msg, sizeof(msg), fmt, argptr);
        va_end(argptr);

        // Сигнатуру сообщения определяем по цвету
        char signature[10];
        if (color == CYAN_COLOR_TEXT)
            strcpy(signature, "DEBUG:");
        else if (color == RED_COLOR_TEXT)
            strcpy(signature, "ERROR:");
        else if (color == YELLOW_COLOR_TEXT)
            strcpy(signature, "WARNING:");
        else
            strcpy(signature, "");

        fprintf(Log.out, "    %s %s %s\n", get_current_datetime(), signature, msg);
        print_color_txt(color, "%s %s %s\n", get_current_datetime(), signature, msg);
        print_color_txt(NORMAL_COLOR_TEXT, "");
        fflush(Log.out);
    }
}
