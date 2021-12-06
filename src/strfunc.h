/**
* Модуль функций работы со строками
* @file
* @version 0.0.1.1
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
#define     vnew(object,n)  ((object *)malloc((size_t)((n)*sizeof(object))))

static  const char *skip(const char *str, const char *white);
static  void copy(char ch);
static  void newword(char *cp);
static  void subsplit(const char *str, const char *delim,const char *quotes);

char **strsplit(char *str, char *delim);

/**
*   Определить количество строк в списке
*/
unsigned int get_string_count(char **strings);


typedef struct
{
    char *search;
    char *replace;
} nix_search_replace_t;

char *strreplace(char *src, const char *from, const char *to, BOOL do_free);
char *strreplace_all(char *src, nix_search_replace_t *replaces);

char *concatenate(char *a1, char *a2);
char *trim_space(char *str);
char *strtrim(char *str, BOOL do_free);
char *strtrim_left(char *str, BOOL do_free);
char *strtrim_right(char *str, BOOL do_free);
char *cp1251_to_utf8(char *from, BOOL do_free);
char *cp866_to_utf8(char *from, BOOL do_free);
/**
*   Перекодировка строки из одной кодировки в другую.
*/
char *str_encoding(char *src, char *src_codepage, char *dst_codepage, BOOL bFree);

char *strlwr_lat(char *pstr);
char *strupr_lat(char *pstr);
char *strnset(char *str, int ch, size_t n);
BOOL strempty(char *str);

/**
* Найти слово в строке
*/
BOOL find_word(char *source, char *search);

/**
*   C substring function: It returns a pointer to the substring
*/
char *substr(char *str, unsigned int position, unsigned int length);

char *strright_pos(char *str, int position);


/**
*   Взять length символов с начала строки.
*/
char *strleft(char *str, int length, BOOL do_free);

/**
*   Взять length символов с конца строки.
*/
char *strright(char *str, int length, BOOL do_free);

/**
*   detecting whether base is starts with str
*/
BOOL startswith(char *base, char *str);

/**
*   detecting whether base is ends with str
*/
BOOL endswith(char *base, char *str);

/**
*   getting the first index of str in base
*/
int strfind(char *base, char *str);

int strfind_offset(char *base, char *str, int start_index);

/**
*   Найти символ в строке начиная с startIndex
*/
int strfind_char(char *str, char c, int start_index);

/**
*   use two index to search in two part to prevent the worst case
 *  (assume search 'aaa' in 'aaaaaaaa', you cannot skip three char each time)
 */
int strfind_last(char *base, char *str);

/**
*   Количество подстрок
*/
unsigned int strfind_count(char *base, char *str);
unsigned int strfind_count_offset(char *base, char *str, int start_index);

/**
*   Проверка на равество двух строк
*/
BOOL strequal(char *str1, char *str2);

/**
*   Строка наоборот
*/
char *strreverse(char *str);

/**
*   Установить все символы строки в c
*/
char *strset(char *str, char c, int length);

/**
*   Создать строку и становить все символы строки в c
*/
char *strgen(char c, unsigned int len);

/**
*   Генерация пустой строки.
*/
char *strgen_empty(void);

/**
*   Функция "съедает" n символов в строке
*/
char *str_eat_chars(char *str, size_t n);

/**
*   Освободить строку.
*/
char *strfree(char *str);

/**
*   Форматированный вывод в строку с выделением памяти.
*/
char *strprintf(char *str, char *fmt,...);

/**
*   Форматированное объединение строк с выделением памяти.
*/
char *strconcatenate(char *str1, char *str2, BOOL do_free);

/**
*   Заменить символ с номером char_index на new_char в строке
*/
char *strreplacechar(char *str, unsigned int char_index, char new_char);

/**
*   Заменить символ с номером char_index на строку new_str в строке
*/
char *strreplace_pos(char *str, unsigned int pos, char *new_str, BOOL do_free);

/**
*   Взять подстроку слева до указанного символа
*/
char *strleft_to(char *str, char symb, BOOL do_free);

/**
*   Взять подстрку справа до указанного символа
*/
char *strright_to(char *str, char symb, BOOL do_free);

/**
*   Функция определяет является ли строка числом
*/
BOOL isnumeric(const char *str);

/**
*   Функция определяет количество десятичных цифр после запятой
*/
int decimal_point(const char *number);

char *strinit(char *str, char *init);

/**
*   Аналог strdup
*/
char *strcopy(const char *str);


/**
*   Перевод латинских символов строки к верхнему регистру.
*/
char *str_upper_lat(char* str);

/**
*   Перевод латинских символов строки к нижнему регистру.
*/
char *str_lower_lat(char* str);

#endif /*__STRFUNC_H*/
