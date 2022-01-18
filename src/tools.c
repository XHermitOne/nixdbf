/**
* Модуль сервисных функций
* @file
* @version 0.0.1.3
*/

#include "tools.h"


/**
* checkParm
*
* Checks for the given parameter in the program's command line arguments
* @return the argument number (1 to argc-1) or 0 if not present
*/
int check_parameters(const char *check, int argc, char **argv)
{
    int i = 0;

    for (i = 1; i < argc; i++)
    {
        if (!strcasecmp(check, argv[i]))
            return i;
    }

    return 0;
}


/**
* Вывод ошибок в виде окна сообщений
* Я использую в основном для отладки
*/
void err_box(char *fmt,...)
{
    char buffer[128];
    va_list ap;

    va_start(ap, fmt);
    vsprintf(buffer, fmt, ap);

    print_log_color_line(RED_COLOR_TEXT, "Error: ");
    print_log_color_line(RED_COLOR_TEXT, buffer);

    va_end(ap);
}


char print_error(char *message)
{
    err_box(message);
    return 0;
}



char print_message(char *message, int options)
{
    return print_error(message);
}



char *get_home_path(void)
{
    return getenv("HOME");
}


/**
* Проверить существует ли директория
*/
BOOL exists_dir(char *path)
{
    BOOL result = FALSE;
    struct stat st;

    if (!is_str_empty(path))
        result = (BOOL) (stat(path, &st) == 0);
    return result;
}


static int do_mkdir(const char *path, mode_t mode)
{
    struct stat st;
    int status = 0;

    if (stat(path, &st) != 0)
    {
        /* Directory does not exist. EEXIST for race condition */
        if (mkdir(path, mode) != 0) // && errno != EEXIST)
            status = -1;
    }
    else if (!S_ISDIR(st.st_mode))
    {
        if (DebugMode) add_log_line("ERROR: %s is not directory", path);
        status = -1;
    }

    return (status);
}


/**
* mkpath - ensure all directories in path exist
* Algorithm takes the pessimistic view and works top-down to ensure
* each directory in path exists, rather than optimistically creating
* the last element and working backwards.
*/
int mkpath(const char *path, mode_t mode)
{
    char *pp = NULL;
    char *sp = NULL;
    int status = 0;
    char *copy_path = create_str_clone(path);

    status = 0;
    pp = copy_path;
    while (status == 0 && (sp = strchr(pp, '/')) != 0)
    {
        if (sp != pp)
        {
            /* Neither root nor double slash in path */
            *sp = '\0';
            status = do_mkdir(copy_path, mode);
            *sp = '/';
        }
        pp = sp + 1;
    }
    if (status == 0)
        status = do_mkdir(path, mode);
    destroy_str_and_null(copy_path);
    return (status);
}


/**
* Cуществует ли файл
* @return TRUE  - exist / FALSE - don't exist
*/
BOOL exists_file(char *filename)
{
    BOOL exists = FALSE;
    if (!is_str_empty(filename))
    {
        FILE *in = fopen(filename, "rb");
        if (in != NULL)
        {
            exists = TRUE;
            fclose( in );
        }
    }
    return exists;
}


/**
*   Удалить файл
*/
BOOL del_file(char *filename)
{
    if (unlink(filename) >= 0 )
    {
        if (DebugMode) print_color_txt(YELLOW_COLOR_TEXT, "Delete file <%s>\n", filename);
        return TRUE;
    }
    else
    {
        if (DebugMode) print_color_txt(RED_COLOR_TEXT, "ERROR. Delete file <%s>\n", filename);
        return FALSE;
    }
}


/**
*   Прочитать файл и вернуть массив символов
*/
unsigned int load_txt_file_size(char *filename, char *result)
{
    unsigned int size = 0;
    FILE *f = fopen(filename, "rb");
    if (f == NULL)
    {
        result = NULL;
        if (DebugMode) add_log_line("ERROR. load_txt_file_size. Open file <%s>", filename);
        return -1; // -1 means file opening fail
    }
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    result = (char *) malloc(size + 1);
    if (size != fread(result, sizeof(char), size, f))
    {
        destroy_str_and_null(result);
        if (DebugMode) add_log_line("ERROR. load_txt_file_size. Read file <%s>", filename);
        return -2; // -2 means file reading fail
    }
    fclose(f);
    result[size] = 0;
    if (DebugMode) add_log_line("INFO. load_txt_file_size. File <%s>\tSize <%d>", filename, size);
    return size;
}


char *load_txt_file(char *filename)
{
    char *result = NULL;
    unsigned int size = 0;
    FILE *f = fopen(filename, "rb");
    unsigned int i = 0;

    if (f == NULL)
    {
        if (DebugMode) print_color_txt(RED_COLOR_TEXT, "ERROR. load_txt_file. Open file <%s>\n", filename);
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    result = (char *) calloc(size + 1, sizeof(char));

    if (size != fread(result, sizeof(char), size, f))
    {
        destroy_str_and_null(result);
        if (DebugMode) print_color_txt(RED_COLOR_TEXT, "ERROR. load_txt_file. Read file <%s>\n", filename);
        return NULL;
    }
    fclose(f);

    //Удалить все 0 и заменить их на пробелы
    for (i = 0; i < size; i++)
        if (result[i] == '\0')
            result[i] = ' ';
    result[size] = '\0';

    if (DebugMode) print_color_txt(WHITE_COLOR_TEXT, "INFO. load_txt_file. File <%s>\tSize <%d>\n", filename, size);
    return result;
}


double max(double a, double b)
{
    return (((a) > (b)) ? (a) : (b));
}


double min(double a, double b)
{
    return (((a) < (b)) ? (a) : (b));
}


/**
* C++ version char *style "itoa".  It would appear that
* itoa() isn't ANSI C standard and doesn't work with GCC on Linux
* @param value value to convert to ascii
* @param base base for conversion
* @return value converted to ascii
*/
char *itoa(int value, int base)
{
    char *result = (char *) calloc(MAX_NUM_LEN + 1, sizeof(char));

    // check that the base if valid
    if (base < 2 || base > 16)
         return NULL;

    char *out = result;
    int quotient = value;

    do
    {
        *out = "0123456789abcdef"[ abs( quotient % base ) ];
        ++out;
        quotient /= base;
    } while (quotient);

    // Only apply negative sign for base 10
    if (value < 0 && base == 10)
        *out++ = '-';

    *out = 0;

    return result;
}


/**
 * Double to ASCII
 */
static double PRECISION = 0.00000000000001;
static int MAX_NUMBER_STRING_SIZE = 32;

char *dtoa(double n)
{
    char *s = (char *) calloc(80, sizeof(char));

    if (isnan(n))
        strcpy(s, "nan");
    else if (isinf(n))
        strcpy(s, "inf");
    else if (n == 0.0)
        strcpy(s, "0");
    else
    {
        int digit = 0;
        int m = 0;
        int m1 = 0;
        char *c = s;
        int neg = (n < 0);
        if (neg)
            n = -n;

        m = log10(n);
        int use_exp = (m >= 14 || (neg && m >= 9) || m <= -9);
        if (neg)
            *(c++) = '-';

        if (use_exp)
        {
            if (m < 0)
                m -= 1.0;
            n = n / pow(10.0, m);
            m1 = m;
            m = 0;
        }
        if (m < 1.0)
            m = 0;

        while (n > PRECISION || m >= 0)
        {
            double weight = pow(10.0, m);
            if (weight > 0 && !isinf(weight))
            {
                digit = floor(n / weight);
                n -= (digit * weight);
                *(c++) = '0' + digit;
            }
            if (m == 0 && n > 0)
                *(c++) = '.';
            m--;
        }
        if (use_exp)
        {
            int i = 0;
            int j = 0;
            *(c++) = 'e';
            if (m1 > 0)
                *(c++) = '+';
            else
            {
                *(c++) = '-';
                m1 = -m1;
            }
            m = 0;
            while (m1 > 0)
            {
                *(c++) = '0' + m1 % 10;
                m1 /= 10;
                m++;
            }
            c -= m;
            for (i = 0, j = m-1; i<j; i++, j--)
            {
                c[i] ^= c[j];
                c[j] ^= c[i];
                c[i] ^= c[j];
            }
            c += m;
        }
        *(c) = '\0';
    }
    return s;
}

/**
*   Конвертация строки в число double
*/
double str_to_double(char *value)
{
    return strtod(value, NULL);
}

int bytes_to_int(BYTE *bytes)
{
    if (bytes == NULL)
        return 0;

    int result = (bytes[1] << 8) | bytes[0];
    return result;
}


double bytes_to_double(BYTE *bytes)
{
    if (bytes == NULL)
        return 0.0;

    double result = 0.0;
    memcpy(&result, bytes, sizeof(double));
    return result;
}


long time_to_long(char *buffer)
{
    long _time = 0L;
    int i = 0;
    int j = 0;
    int k = 0;

    for(_time = i = j = 0; i < 3; i++)
    {
        sscanf(&buffer[j], "%d", &k);
        _time = _time * 60 + k;
        for(; buffer[j] != ':' && buffer[j]; j++);
        if (!buffer[j])
            break;
        else
            j++;
    }
    return _time;
}


char *long_to_time(long ltime)
{
    char *buffer = (char *) calloc(9, sizeof(char));
    if (ltime >= 0 && ltime != 1000000000)
        sprintf(buffer, "%02ld:%02ld:%02ld", (ltime/3600)%24, (ltime/60)%60, ltime%60);
    else
        strcpy(buffer,"##:##:##");
    if (ltime != 1000000000 && ltime >= 86400)
        sprintf(&buffer[9], "%06ld", ltime/86400);
    return buffer;
}


long get_now_time(void)
{
    time_t sys_time;
    return (long) time(&sys_time);
}


/**
*   Нормализация пути
*   По умолчанию:
*       src_len=-1
*/
char *create_norm_path(char *src, size_t src_len)
{
    char * res = NULL;
    size_t res_len;

    if (src_len < 0)
        src_len = strlen(src)-1;

    const char *ptr = src;
    const char *end = &src[src_len];
    const char *next = NULL;

    if (src_len == 0 || src[0] != '/')
    {
        // relative path

        char pwd[PATH_MAX];
        size_t pwd_len;

        if (getcwd(pwd, sizeof(pwd)) == NULL)
        {
            return NULL;
        }

        pwd_len = strlen(pwd);
        res = (char *) malloc(pwd_len + 1 + src_len + 1);
        memcpy(res, pwd, pwd_len);
        res_len = pwd_len;
    }
    else
    {
        res = (char *) malloc((src_len > 0 ? src_len : 1) + 1);
        res_len = 0;
    }

    for (ptr = src; ptr < end; ptr=next+1)
    {
        size_t len;
        next = (char *) memchr(ptr, '/', end-ptr);
        if (next == NULL)
        {
            next = end;
        }
        len = next-ptr;
        switch(len)
        {
            case 2:
                if (ptr[0] == '.' && ptr[1] == '.')
                {
                    const char * slash = (char *) memrchr(res, '/', res_len);
                    if (slash != NULL)
                    {
                        res_len = slash - res;
                    }
                    continue;
                }
                break;
            case 1:
                if (ptr[0] == '.')
                {
                    continue;

                }
                break;
            case 0:
                continue;
        }
        res[res_len++] = '/';
        memcpy(&res[res_len], ptr, len);
        res_len += len;
    }

    if (res_len == 0)
    {
        res[res_len++] = '/';
    }
    res[res_len] = '\0';

    return res;
}


/**
*   Конвертация представления пути из dos(C:\\path\\) в unix(C:/path/)
*/
char *create_dos_to_unix_path(char *src)
{
    // Подразумевается что в unix все папки в нижнем регистре
    return str_lower_lat(create_str_replace(src, "\\", "/"));
}


/**
*   Поменять расширение в имени файла
*/
char *create_change_filename_ext(char *filename, const char *new_ext)
{
    char *ext_filename = strrchr(filename, '.');
    char *result_filename = NULL;

    if (ext_filename != NULL)
    {
        char *base = create_str_begin(filename, ext_filename - filename);
        result_filename = create_str_printf("%s%s", base, new_ext);
        destroy_str_and_null(base);
    }
    else
        result_filename = create_str_printf("%s%s", filename, new_ext);

    return result_filename;
}

/**
*   Проверка на то же самый файл
*/
BOOL is_same_file(const char *filename1, const char *filename2)
{
    struct stat st1, st2;

    if (stat(filename1, &st1) != 0 || stat(filename2, &st2) != 0)
        return FALSE;

    if (st1.st_dev != st2.st_dev || st1.st_ino != st2.st_ino)
        return FALSE;

    return TRUE;
}

/**
*   Печать цветного текста
*/
void print_color_txt(unsigned int color, char *fmt, ...)
{
    va_list argptr;
    va_start(argptr, fmt);

    char msg[4096];
    char str_color[10];
    vsnprintf(msg, sizeof(msg), fmt, argptr);
    va_end(argptr);

    if (color ==  RED_COLOR_TEXT)
        strcpy(str_color, "\x1b[31;1m");
    else if (color ==  GREEN_COLOR_TEXT)
        strcpy(str_color, "\x1b[32m");
    else if (color ==  YELLOW_COLOR_TEXT)
        strcpy(str_color, "\x1b[33m");
    else if (color ==  BLUE_COLOR_TEXT)
        strcpy(str_color, "\x1b[34m");
    else if (color ==  PURPLE_COLOR_TEXT)
        strcpy(str_color, "\x1b[35m");
    else if (color ==  CYAN_COLOR_TEXT)
        strcpy(str_color, "\x1b[36m");
    else if (color ==  WHITE_COLOR_TEXT)
        strcpy(str_color, "\x1b[37m");
    else if (color ==  NORMAL_COLOR_TEXT)
        strcpy(str_color, "\x1b[0m");

    printf("%s %s\x1b[0m", str_color, msg);
}
