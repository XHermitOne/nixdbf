/**
* Модуль сервисных функций
* @file
* @version 0.0.1.1
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <locale.h>
#include <malloc.h>
#include <iconv.h>
#include <ctype.h>

#include "strfunc.h"
#include "log.h"


/**
*   Слияние строк
*/
char *concatenate(char *str1, char *str2)
{
    int i = 0;
    int len = 0;
    int len2 = 0;

    len = strlen(str1);
    len2 = strlen(str2);
    for(i=0; i < len2; i++)
    {
        str1[len+i]=str2[i];
    }
    str1[len+i]='\x00';
    return (str1);
}


/**
* Усечение строки
*/
char *trim_space(char *str)
{
    int len = 0;
    char *ret = NULL;

    len = strlen(str);
    ret = (char*) calloc(len+1, sizeof(char));

    ret[0]='\x00';

    concatenate(ret, str);

    // str = strfree(str);
    strfree(str);
    return ret;
}


/**
*   Удаление начальных и завершающих пробелов из строки
*   do_free: Освободить автоматически память после использования?
*   По умолчанию do_free=FALSE.
*   В языке <C> нет возможности задать значение по умолчанию аргумента функции.
*/
char *strtrim(char *str, BOOL do_free)
{
    char *ret = NULL;
    ret = strtrim_left(str, FALSE);
    ret = strtrim_right(ret, TRUE);

    if (do_free)
        // str = strfree(str);
        strfree(str);
    return ret;
}


char *strtrim_left(char *str, BOOL do_free)
{
    char *ret = NULL;
    char *pointer = str;

    if (str)
        ret = strcopy(str);
    else
        ret = strgen_empty();

    while (isspace(*pointer))
        ++pointer;

    memmove(ret, pointer, strlen(pointer) + 1);

    if (do_free)
        //str = strfree(str);
        strfree(str);
    return ret;
}


char *strtrim_right(char *str, BOOL do_free)
{
    char *ret = (char *) calloc(strlen(str) + 1, sizeof(char));
    char *end = str + strlen(str);

    while ((end != str) && isspace(*(end-1)))
        --end;

    memmove(ret, str, end - str);
    ret[end - str] = '\0';

    if (do_free)
        //str = strfree(str);
        strfree(str);

    return ret;
}


static int to_utf8(char *from, char *to, const char *codepage)
{
    size_t Lfrom, Lto;
    int ret = 0;
    iconv_t d = iconv_open("UTF-8", codepage);

    Lfrom = strlen(from);
    Lto = 2 * Lfrom;
    ret = iconv(d, &from, &Lfrom, &to, &Lto);
    iconv_close(d);
    return ret;
}


char *cp1251_to_utf8(char *from, BOOL do_free)
{
    char *result = NULL;

    if (strempty(from))
        result = strgen_empty();
    else
    {
        result=(char *)calloc(strlen(from) * 2 + 1, sizeof(char));
        to_utf8(from, result, "CP1251");
    }

    if (do_free)
        //from = strfree(from);
        strfree(from);
    return result;
}


char *cp866_to_utf8(char *from, BOOL do_free)
{
    char *result = NULL;

    if (strempty(from))
        result = strgen_empty();
    else
    {
        result = (char *) calloc(strlen(from) * 2 + 1, sizeof(char));
        to_utf8(from, result, "CP866");
    }

    if (do_free)
        // from = strfree(from);
        strfree(from);
    return result;
}


/**
*   Перекодировка строки из одной кодировки в другую.
*/
char *str_encoding(char *src, char *src_codepage, char *dst_codepage, BOOL bFree)
{
    // Привести к верхнему регистру для корректной проверки
    src_codepage = str_upper_lat(src_codepage);
    dst_codepage = str_upper_lat(dst_codepage);
    
    if ( src_codepage == NULL )
        log_warning("Не определена исходная кодировка");
    else if ( dst_codepage == NULL )
        log_warning("Не определена результирующая кодировка");
    else if ( (strcmp(src_codepage, "CP866") == 0) && (strcmp(dst_codepage, "UTF-8") == 0) )
        return cp866_to_utf8(src, bFree);
    else if ( (strcmp(src_codepage, "CP1251") == 0) && (strcmp(dst_codepage, "UTF-8") == 0) )
        return cp1251_to_utf8(src, bFree);
    else
        log_warning("Не обрабатываемая перекодировка <%s> -> <%s>", src_codepage, dst_codepage);
    return src;
}


char *strreplace_old(char *src, char *from, char *to, BOOL do_free)
{
    size_t size    = strlen(src) + 1;
    size_t fromlen = strlen(from);
    size_t tolen   = strlen(to);
    char *value = (char *) malloc(size);
    char *dst = value;
    char *psrc = src;

    if (value != NULL)
    {
        for ( ;; )
        {
            const char *match = strstr(psrc, from);
            if (match != NULL)
            {
                size_t count = match-src;
                char *temp = NULL;
                size += tolen-fromlen;
                temp = (char *)realloc(value, size);
                if (temp == NULL)
                {
                    value = strfree(value);
                    return NULL;
                }
                dst = temp+(dst-value);
                value = temp;
                memmove(dst, psrc, count);
                psrc += count;
                dst += count;
                memmove(dst, to, tolen);
                psrc += fromlen;
                dst += tolen;
            }
            else /* No match found. */
            {
                strcpy(dst, psrc);
                break;
            }
        }
    }

    if (do_free)
        // src = strfree(src);
        strfree(src);
    return value;
}


char *strreplace(char *str, const char *from, const char *to, BOOL do_free)
{
    char *ret = NULL;
    char *r = NULL;
    const char *p = NULL;
    const char *q = NULL;
    size_t oldlen = strlen(from);
    size_t count, retlen, newlen = strlen(to);

    if (oldlen != newlen)
    {
        for (count = 0, p = str; (q = strstr(p, from)) != NULL; p = q + oldlen)
            count++;
        /* this is undefined if p - str > PTRDIFF_MAX */
        retlen = p - str + strlen(p) + count * (newlen - oldlen);
    }
    else
        retlen = strlen(str);

    if ((ret = (char *) malloc(retlen + 1)) == NULL)
    {
        printf("Memory allocation error\n");
        return NULL;
    }

    for (r = ret, p = str; (q = strstr(p, from)) != NULL; p = q + oldlen)
    {
        /* this is undefined if q - p > PTRDIFF_MAX */
        ptrdiff_t l = q - p;
        memcpy(r, p, l);
        r += l;
        memcpy(r, to, newlen);
        r += newlen;
    }
    strcpy(r, p);

    if (do_free)
        // str = strfree(str);
        strfree(str);

    return ret;
}


char *strreplace_all(char *src, nix_search_replace_t *replaces)
{
    char *ret = src;
    int i = 0;

    for(i=0; replaces[i].search; i++)
        ret = strreplace(ret, replaces[i].search, replaces[i].replace, FALSE);

    //ret = strreplace(ret, "\\", "/");
    return ret;
}


static  char    *buff = NULL;       /**< buffer for strings */
static  char    **ptrs = NULL;      /**< buffer for pointers to strings */

static  char    *bufp = NULL;       /**< current buffer pointer */
static  char    **ptrp = NULL;      /**< next word address */

static  int words = NULL;           /**< number of words in string */
static  int space = NULL;           /**< number of characters to store */


static const char *skip(const char *str, const char *white)
{
    while (*str && strchr(white,*str) != CNULL)
        str++;
    return str;
}


static void copy(char ch)
{
    if (buff != CNULL)
        *bufp ++= ch;
    else
        space++;
}


static void newword(char *cp)
{
    if (buff != CNULL)
        *ptrp++ = cp;
    else
        words++;
}


static void subsplit(const char *str,const char *delim,const char *quotes)
{
    int sloshed = 0;
    char quotec = 0;

    words = 0;
    space = 0;
    str = skip(str, delim);
    if (*str)
        newword(bufp);

    for (quotec='\0', sloshed=FALSE; *str; str++)
    {
        if (quotec != '\0')
            /* in quotes */
            if (sloshed)
            /* in quotes after a slosh */
            {
                if (*str != quotec && *str != SLOSH)
                    /* not something that's escaped */
                    copy(SLOSH);

                copy(*str);

                /* forget slosh */
                sloshed = FALSE;
            }
            else
                /* in quotes not after a slosh */
                if (*str == quotec)
                    /* leave quotes */
                    quotec='\0';
                else
                    if ((sloshed=(*str == SLOSH)))
                        /* notice slosh */
                        ;
                    else
                        copy(*str);
        else
            /* not in quotes */
            if (sloshed)
            {
                /* not in quotes after a slosh */
                if (*str != SLOSH
                    && strchr(quotes,*str) == CNULL
                    && strchr(delim,*str) == CNULL)
                    copy(SLOSH);

                copy(*str);

                /* forget slosh */
                sloshed=FALSE;
            }
            else
            {
                /* not in quotes not after a slosh */

                if (strchr(quotes,*str) != CNULL)
                    /* enter quotes */
                    quotec = *str;
                else
                    if (strchr(delim,*str) != CNULL)
                    {
                        /* find next word */
                        str=skip(str,delim);
                        if (*str)
                        {
                            copy('\0');
                            newword(bufp);
                        }

                        str--;
                    }
                    else
                        if ((sloshed=(*str == SLOSH)))
                            ;
                        else
                            copy(*str);
            }
        }

    /* catch trailing sloshes */
    if (sloshed)
        copy(SLOSH);

    copy('\0');
    newword(CNULL);
}


char **qstrsplit(const char *str,const char *delim,const char *quotes)
{
    /* default delimiters */
    if (delim == CNULL)
        delim=" \t\n";

    /* mark pass one */
    buff=CNULL;

    /* count words and characters */
    subsplit(str,delim,quotes);

    /* allocate room for characters */
    if ((buff=vnew(char, space)) == CNULL)
        return CPNULL;

    /* allocate room for words */
    if ((ptrs=vnew(char *,words+1)) == CPNULL)
    {
        buff = strfree(buff);
        return CPNULL;
    }

    /* initialise pointers */
    bufp=buff;
    ptrp=ptrs;

    /* point to allocated space */
    *ptrp++ = buff;

    /* copy words into buffer */
    subsplit(str,delim,quotes);

    return &ptrs[1];    /* return pointer to words */
}


/**
*   Разбить строку с разделителем на отдельные строки
*   str - Исходная строка
*   delim - Подстрока-разделитель
*/
char **strsplit(char *str, char *delim)
{
    return qstrsplit(str, delim, DEFQUOTES);
}

/**
*   Определить количество строк в списке
*/
unsigned int get_string_count(char **strings)
{
    unsigned int i = 0;

    if (!strings)
        // Строки вообще не определены
        return 0;

    while (strings[i])
        i++;

    return i;
}

/**
*   Привести строку к нижнему регистру
*/
char *strlwr_lat(char *pstr)
{
    char *p = pstr;
    while (*p)
    {
        if(*p >= 'A' && *p <= 'Z')
            *p = tolower(*p);
        p++;
    }
    return pstr;
}


/**
*   Привести строку к верхнему регистру
*/
char *strupr_lat(char *pstr)
{
    char *p = pstr;
    while (*p)
    {
        if(*p >= 'A' && *p <= 'Z')
            *p = toupper(*p);
        p++;
    }
    return pstr;
}


char *strnset(char *str, int ch, size_t n)
{
    int i = 0;
    for(i = 0; i < (int) n; i++)
    {
        if(str[i] == STR_NULL )
            return (str); // return when find null
        str[i] = ch;
    }
    return  (str);
}


/**
* Проверка на пустую строку
*/
BOOL strempty(char *str)
{
    int len = 0;

    if (str == NULL)
        return TRUE;

    len = strlen(str);
    if (len > 0)
        return FALSE;
    else
        return TRUE;
}


/**
*   Найти слово в строке
*/
BOOL find_word(char *source, char *search)
{
    int i = 0;                //  Position inside source
    int search_length = 0;    //  Length of string search
    int source_length = 0;    //  Length of string source
    BOOL found = FALSE;       //  Flag 1-> found the value

    search_length = strlen(search);
    source_length = strlen(source);

    /*  While we haven't found it and we haven't readched the      */
    /*  point where our current position plus the length of the    */
    /*  string we are looking for is longer than the string itself,*/
    /*  check the next search_length characters for a match        */
    while (!found && ((i+search_length) <= (source_length)))
    {
        found = (strncasecmp(source+i, search, search_length) == 0);
        i++;
    }

    return (found);
}


/**
*   C substring function: It returns a pointer to the substring
*/
char *substr(char *str, unsigned int position, unsigned int length)
{
    char *ret = NULL;
    unsigned int c = 0;

    if ((position + length) > strlen(str))
        length = strlen(str) - position;

    ret = (char *) calloc(length + 1, sizeof(char));

    if (ret == NULL)
    {
        printf("FUNCTION [substr]. Unable to allocate memory.\n");
        return NULL;
    }

    for (c = 0 ; c < length ; c++)
        *(ret + c) = (char) str[position + c];

    *(ret + c) = '\0';

    return ret;
}


char *strright_pos(char *str, int position)
{
    return substr(str, position, strlen(str)-position);
}


/**
*   Взять length символов с начала строки.
*   do_free: Освободить автоматически память после использования?
*   По умолчанию do_free=FALSE.
*   В языке <C> нет возможности задать значение по умолчанию аргумента функции.
*/
char *strleft(char *str, int length, BOOL do_free)
{
    char *ret = substr(str, 0, length);
    if (do_free)
        // str = strfree(str);
        strfree(str);
    return ret;
}


/**
*   Взять length символов с конца строки.
*   do_free: Освободить автоматически память после использования?
*   По умолчанию do_free=FALSE.
*   В языке <C> нет возможности задать значение по умолчанию аргумента функции.
*/
char *strright(char *str, int length, BOOL do_free)
{
    int len = strlen(str);
    char *ret = substr(str, len-length, len);
    if (do_free)
        // str = strfree(str);
        strfree(str);
    return ret;
}


/**
*   detecting whether base is starts with str
*/
BOOL startswith(char *base, char *str)
{
    return (strstr(base, str) - base) == 0;
}


/**
*   detecting whether base is ends with str
*/
BOOL endswith(char *base, char *str)
{
    int blen = strlen(base);
    int slen = strlen(str);
    return (blen >= slen) && (0 == strcmp(base + blen - slen, str));
}


/**
*   getting the first index of str in base
*/
int strfind(char *base, char *str)
{
    return strfind_offset(base, str, 0);
}


int strfind_offset(char *base, char *str, int startIndex)
{
    int result = 0;
    int baselen = strlen(base);

    // str should not longer than base
    if (strlen(str) > baselen || startIndex > baselen)
    {
        result = -1;
    }
    else
    {
        if (startIndex < 0 )
        {
            startIndex = 0;
        }
        char *pos = strstr(base + startIndex, str);
        if (pos == NULL)
        {
            result = -1;
        }
        else
        {
            result = pos - base;
        }
    }
    return result;
}


/**
*   Найти символ в строке начиная с startIndex
*   По умолчанию startIndex=0
*   В языке <C> нет возможности задать значение по умолчанию аргумента функции
*/
int strfind_char(char *str, char c, int startIndex)
{
    unsigned int len = strlen(str);
    int ret = -1;
    unsigned int i = 0;

    for (i=startIndex; i < len; i++)
        if (str[i] == c)
        {
            ret = i;
            break;
        }

    return ret;
}


/**
*   Количество подстрок
*/
unsigned int strfind_count(char *base, char *str)
{
    return strfind_count_offset(base, str, 0);
}


unsigned int strfind_count_offset(char *base, char *str, int startIndex)
{
    unsigned int result = 0;
    unsigned int baselen = strlen(base);

    // str should not longer than base
    if (strlen(str) > baselen || startIndex >= baselen)
        result = 0;
    else
    {
        if (startIndex < 0 )
            startIndex = 0;
        char *pos = strstr(base + startIndex, str);
        if (pos == NULL)
            result = 0;
        else
            result = 1 + strfind_count_offset(base, str, startIndex + 1);
    }
    return result;
}


/**
*   use two index to search in two part to prevent the worst case
 *  (assume search 'aaa' in 'aaaaaaaa', you cannot skip three char each time)
 */
int strfind_last(char *base, char *str)
{
    int result = 0;

    // str should not longer than base
    if (strlen(str) > strlen(base))
    {
        result = -1;
    }
    else
    {
        int start = 0;
        int endinit = strlen(base) - strlen(str);
        int end = endinit;
        int endtmp = endinit;
        while(start != end)
        {
            start = strfind_offset(base, str, start);
            end = strfind_offset(base, str, end);

            // not found from start
            if (start == -1)
            {
                end = -1; // then break;
            }
            else
                if (end == -1)
                {
                    // found from start
                    // but not found from end
                    // move end to middle
                    if (endtmp == (start+1))
                    {
                        end = start; // then break;
                    }
                    else
                    {
                        end = endtmp - (endtmp - start) / 2;
                        if (end <= start)
                        {
                            end = start+1;
                        }
                        endtmp = end;
                    }
                }
                else
                {
                    // found from both start and end
                    // move start to end and
                    // move end to base - strlen(str)
                    start = end;
                    end = endinit;
                }
        }
        result = start;
    }
    return result;
}


/**
*   Проверка на равество двух строк
*/
BOOL strequal(char *str1, char *str2)
{
    if ((str1 == NULL) && str2 )
        return FALSE;
    if ((str2 == NULL) && str1 )
        return FALSE;
    return strcmp(str1, str2) == 0;
}


/**
*   Строка наоборот
*/
char *strreverse(char *str)
{
   int length = 0;
   int c = 0;
   char *begin = NULL;
   char *end = NULL;
   char temp = 0;

   length = strlen(str);
   begin  = str;
   end    = str;

   for (c = 0; c < length - 1; c++)
      end++;

   for (c = 0; c < length/2; c++)
   {
      temp   = *end;
      *end   = *begin;
      *begin = temp;

      begin++;
      end--;
   }
   return str;
}


/**
*   Установить все символы строки в c.
*   По умолчанию length=-1
*   В языке <C> нет возможности задать значение по умолчанию аргумента функции.
*/
char *strset(char *str, char c, int length)
{
    unsigned int len = 0;

    if (length < 0)
        len = strlen(str);
    else
        len = length;

    unsigned int i = 0;
    for (i = 0; i < len; i++)
        str[i] = c;
    return str;
};


/**
*   Создать строку и становить все символы строки в c
*/
char *strgen(char c, unsigned int len)
{
    char *s = (char *) calloc(len + 1, sizeof(char));
    char *ret = strset(s, c, len);
    ret[len] = '\0';
    return ret;
}


char *strgen_empty(void)
{
    char *s = (char *) calloc(1, sizeof(char));
    s[0] = '\0';
    return s;
}


/**
*   Функция "съедает" n символов в начале строки
*/
char *str_eat_chars(char *str, size_t n)
{
    char *result = NULL;
    char *emptystr = "";
    unsigned int len = strlen(str);

    n = MIN(n, len);
    len -= n;

    if (len)
    {
        result = (char *) calloc(len+1, sizeof(char));
        strcpy(result, str + n);
    }
    else
        return emptystr;

    return result;
}


char *strfree(char *str)
{
    if (str != NULL)
        {
            free(str);
            str = NULL;
        }
    return str;
}


char *strprintf(char *str, char *fmt,...)
{
    char buffer[MAX_STR];
    va_list ap;

    va_start(ap, fmt);
    vsprintf(buffer, fmt, ap);

    if (str == NULL)
        str = (char *) calloc(strlen(buffer)+1, sizeof(char));

    strcpy(str, buffer);

    va_end(ap);
    return str;
}


char *strconcatenate(char *str1, char *str2, BOOL do_free)
{
    char *ret = (char *) calloc((strlen(str1)+strlen(str2)+1), sizeof(char));

    ret = strcpy(ret, str1);
    if (do_free)
        // str1 = strfree(str1);
        strfree(str1);

    return concatenate(ret, str2);
}


/**
*   Заменить символ с номером char_index на new_char в строке
*/
char *strreplacechar(char *str, unsigned int char_index, char new_char)
{
    unsigned int len = strlen(str);
    unsigned int i = 0;

    if (char_index < 0 || char_index >= len)
        return str;

    for (i = 0; i < len; i++)
    {
        if (i == char_index)
        {
            *(str + i) = new_char;
            break;
        }
    }
    return str;
}


/**
*   Заменить символ с номером char_index на строку new_str в строке
*/
char *strreplace_pos(char *str, unsigned int pos, char *new_str, BOOL do_free)
{
    unsigned int len = strlen(str);
    unsigned int i = 0;

    if ((pos < 0) || (pos >= len))
        return str;

    char *result = (char *) calloc(len + strlen(new_str), sizeof(char));

    for (i = 0; i < len; i++)
    {
        if (i == pos)
        {
            char *v = strleft(str, i, FALSE);
            strcpy(result, v);
            strcat(result, new_str);
            strcat(result, str + i + 1);
            v = strfree(v);
            break;
        }
    }

    if (do_free)
        // str = strfree(str);
        strfree(str);
    return result;
}


/**
*   Взять подстроку слева до указанного символа
*   do_free: Освободить автоматически память после использования?
*   По умолчанию do_free=FALSE.
*   В языке <C> нет возможности задать значение по умолчанию аргумента функции.
*/
char *strleft_to(char *str, char symb, BOOL do_free)
{
    char *p = str;
    char *result = NULL;

    while (*p)
    {
        if (*p == symb)
        {
            result = strleft(str, p-str, FALSE);
            if (do_free)
                // str = strfree(str);
                strfree(str);
            return result;
        }
        p++;
    }

    result = strcopy(str);
    if (do_free)
        // str = strfree(str);
        strfree(str);
    return result;
}


/**
*   Взять подстрку справа до указанного символа
*   do_free: Освободить автоматически память после использования?
*   По умолчанию do_free=FALSE.
*   В языке <C> нет возможности задать значение по умолчанию аргумента функции.
*/
char *strright_to(char *str, char symb, BOOL do_free)
{
    char *start = str + strlen(str) - 1;
    char *p = start;        //Перейти на последний элемент
    char *result = NULL;

    while (*p)
    {
        if (*p == symb)
        {
            result = strright(str, start - p, FALSE);
            if (do_free)
                // str = strfree(str);
                strfree(str);
            return result;
        }
        p--;
    }

    result = strcopy(str);
    if (do_free)
        // str = strfree(str);
        strfree(str);
    return result;
}


/**
*   Функция определяет является ли строка числом
*/
BOOL isnumeric(const char *str)
{
    if (str == NULL || *str == '\0' || isspace(*str))
    {
        // if (DebugMode) logAddLine("WARNING. Empty string <%s>", str);
        return FALSE;
    }

    char *p = NULL;
    strtod(str, &p);
    BOOL result = (*p == '\0');
    // if (DebugMode) logAddLine("Is NUM <%s> [%s] [%d]", str, p, result);

    // Число может быть ИНН
    // Обработка такого случая
    if (result)
    {
        // Если все символы строки цифровые и длина строки 10 или 12,
        // то считается что это ИНН
        BOOL is_digit = TRUE;
        int i = 0;
        while (i < strlen(str) && is_digit == TRUE)
        {
            if ((str[i] == '.') || (str[i] == ','))
            {
                // Нашли <.> или <,> Значит это просто большое число
                i = -1;
                break;
            }
            is_digit = isdigit(str[i]);
            i++;
        }
        result = (!((i == 10) || (i == 12)));
        //if (DebugMode) logAddLine("INN <%s> [%d] [%d]", str, i, result);
    }

    return result;
}


int decimal_point(const char *number)
{
    if (number == NULL || *number == '\0' || isspace(*number))
        return -1;

    // Указатель на конец строки
    int i = 0;
    int last = strlen(number);
    int count = 0;  // Количество цифр после десятичной точки

    for (i = last; i--; i >= 0)
    {
        // if (DebugMode) logAddLine("Find decimal point <%c>", *(number+i));
        if (*(number+i) == '.')
            break;
        else
            count++;
    }
    if (i < 0)
        // Десятичной точки вообще нет, значит нет и цифр после точки
        count = 0;
    return count;
}


char *strinit(char *str, char *init)
{
    // str = strfree(str);
    strfree(str);
    return init;
}


/**
*   Аналог strdup
*/
char *strcopy(const char *str)
{
    size_t len;
    char *copy = NULL;

    if (!str)
        return NULL;

    len = strlen(str) + 1;
    copy = (char *) calloc(len, sizeof(char));

    if (copy)
        memcpy(copy, str, len);
    else
        log_line("Memory allocation error");

    return copy;
}


/**
*   Перевод латинских символов строки к верхнему регистру.
*/
char *str_upper_lat(char* str)
{
    char *str_begin = str;
    while ((*str = (char) toupper(*str))) str++;
    return str_begin;
}


/**
*   Перевод латинских символов строки к нижнему регистру.
*/
char *str_lower_lat(char* str)
{
    char *str_begin = str;
    while ((*str = (char) tolower(*str))) str++;
    return str_begin;
}
