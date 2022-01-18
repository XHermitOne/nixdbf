/**
* Модуль сервисных функций
* @file
* @version 0.0.1.3
*/

#if !defined( __TOOLS_H )
#define __TOOLS_H

#include <stdio.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <iconv.h>
#include <math.h>
#include <locale.h>
#include <malloc.h>
#include <time.h>

#include <fcntl.h>              /* O_RDONLY */
#include <sys/types.h>
#include <sys/statvfs.h>
#include <sys/param.h>          /* MAXPATHLEN */

#include "ext_types.h"
#include "log.h"
#include "strfunc.h"
#include "main.h"

#define MAX_NUM_LEN 20

int check_parameters(const char *check, int argc, char **argv);

void err_box(char *fmt,...);

char print_error(char *msg);

char print_message(char *message, int options);

char *get_home_path(void);

BOOL exists_dir(char *path);
int mkpath(const char *path, mode_t mode);

BOOL exists_file(char *filename);
BOOL del_file(char *filename);

/**
*   Прочитать файл и вернуть массив символов
*/
unsigned int load_txt_file_size(char *filename, char *result);
char *load_txt_file(char *filename);

/**
*   Выбрать максимальное/минимальное значение из 2-х
*/
double max(double a, double b);
double min(double a, double b);

char *itoa(int value, int base);
char * dtoa(double n);

/**
*   Конвертация строки в число double
*/
double str_to_double(char *value);

int bytes_to_int(BYTE *bytes);
double bytes_to_double(BYTE *bytes);

long get_now_time(void);
long time_to_long(char *buffer);
char *long_to_time(long time);

/**
*   Нормализация пути
*/
char *create_norm_path(char *src, size_t src_len);

/**
*   Конвертация представления пути из dos(C:\\path\\) в unix(C:/path/)
*/
char *create_dos_to_unix_path(char *src);

/**
*   Поменять расширение в имени файла
*/
char *create_change_filename_ext(char *filename, const char *new_ext);

/**
*   Проверка на то же самый файл
*/
BOOL is_same_file(const char *filename1, const char *filename2);


/**
*   Цвета
*/
enum nix_print_color_t {
    NORMAL_COLOR_TEXT = 0,      // normal
    RED_COLOR_TEXT,             // red
    GREEN_COLOR_TEXT,           // green
    YELLOW_COLOR_TEXT,          // yellow
    BLUE_COLOR_TEXT,            // blue
    PURPLE_COLOR_TEXT,          // purple
    CYAN_COLOR_TEXT,            // cyan
    WHITE_COLOR_TEXT,           // white
};
/**
*   Печать цветного текста
*/
void print_color_txt(unsigned int color, char *str, ...);

#endif /*__TOOLS_H*/
