/**
* Модуль функций работы со строками
* @file
* @version 0.0.2.1
*/

#if !defined( __STRFUNC_H )
#define __STRFUNC_H

#include "ext_types.h"

#define     MAX_STR     1024
#define     STR_NULL    ((char) 0)

#define     SLOSH       '\\'
#define     SLASH       '/'
#define     DEFQUOTES   "\"'"   /**< default quote characters */
#define     CNULL       0
#define     CPNULL      0

// ---------------------- Функции управления регистром строк ----------------------------

/**
*   Привести строку к верхнему регистру
*/
char *str_upper(char* str);

/**
*   Привести строку к нижнему регистру
*/
char *str_lower(char* str);

/**
*   Перевод латинских символов строки к верхнему регистру.
*/
char *str_upper_lat(char* str);

/**
*   Перевод латинских символов строки к нижнему регистру.
*/
char *str_lower_lat(char* str);

// --------------- Функции удаления символов из строк ---------------------------

/**
*   Удаление начальных и завершающих пробелов из строки
*/
char *str_trim(char *str);

/**
*   Удаление начальных пробелов из строки
*/
char *str_trim_begin(char *str);
/**
*   Удаление завершающих пробелов из строки
*/
char *str_trim_end(char *str);

// ------------------ Функции замен в строках ---------------------------
typedef struct
{
    char *search;
    char *replace;
} nix_search_replace_t;

/**
*   Заменить символ с номером char_index на new_char в строке
*/
char *str_replace_char_idx(char *str, unsigned int char_index, char new_char);

/**
*   Произвести замену в строке
*/
char *create_str_replace(char *str, const char *from, const char *to);

/**
*   Произвести все замены в строке
*/

char *create_str_replace_all(char *src, nix_search_replace_t *replaces);

// ------------------ Функции работы с кодировками ---------------------------
#define     DEFAULT_CODEPAGE    "UTF-8"     /* Кодовая страница, используемая по умолчанию */

/**
*   Перевод строки в UTF-8 кодировку
*/
int to_utf8(char *from, char *to, const char *codepage);

/**
*   Создать строку в конировке UTF-8 из строки в кодировки CP1251
*/
char *create_utf8_from_cp1251(char *from);

/**
*   Создать строку в конировке UTF-8 из строки в кодировки CP866
*/
char *create_utf8_from_cp866(char *from);

/**
*   Создать строку, перекодированную из одной кодировки в другую
*/
char *create_str_encoding(char *src, char *src_codepage, char *dst_codepage);


// ---------------------- Функции удаления строк из памяти ----------------------------

/**
*   Удалить строку из памяти и присвоить NULL
*/
BOOL destroy_str_and_null(char *str);

/**
* Проверка на пустую строку
*/
BOOL is_str_empty(char *str);

// ---------------------- Функции форматированного преобразования строк ----------------------------

/**
*   Форматированный вывод в строку
*/
char *str_printf(char *str, char *fmt,...);

/**
*   Форматированный вывод в новую строку
*/
char *create_str_printf(char *fmt,...);

// ---------------------- Функции работы с подстроками ----------------------------
/**
*   It returns a pointer to the substring
*/
char *create_substr(char *str, unsigned int position, unsigned int length);

/**
*   Взять length символов с начала строки.
*/
char *create_str_begin(char *str, int length);

/**
*   Взять length символов с конца строки.
*/
char *create_str_end(char *str, int length);

/**
*   Создать копию строки. Аналог strdup. 
*/
char *create_str_clone(const char *str);


#endif /*__STRFUNC_H*/
