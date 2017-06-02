/**
* Модуль главных структур программы и основных запускающих функций
* @file
*/

#include "main.h"

/**
* Режим отладки
*/
BOOL DBG_MODE = TRUE;


/**
*   Вывести на экран все записи DBF файла.
*   @param dbf_filename Полное наименование DBF файла
*   @param src_codepage Кодовая страница DBF файла.
*   @param dst_codepage Кодовая страница для вывода.
*/
void print_all_records(char *dbf_filename, char *src_codepage, char *dst_codepage)
{
    int err = db_open(dbf_filename);
    int count_field = field_count();
    
    if ( err == 0)
    {
        while (!db_eof())
        {
            for (int i = 0; i < count_field; i++)
            {
                char *value = get_num(i);
                value = str_encoding(value, src_codepage, dst_codepage, FALSE);
                printf(value);
                // За последним значением разделитель ставить не надо
                if ( i != (count_field - 1) )
                    printf("|");
            }
            printf("\n");
            db_next();            
        }
    }
    else
        print_db_error(err);
    
    db_close();
}


/**
*   Вывести на экран диапазон записей DBF файла.
*   @param dbf_filename Полное наименование DBF файла
*   @param src_codepage Кодовая страница DBF файла.
*   @param dst_codepage Кодовая страница для вывода.
*   @param start_rec Номер первой обрабатываемой строки.
*   @param limit Ограничение выводимых записей. Если -1, то ограничения нет.
*/
void print_record_range(char *dbf_filename, char *src_codepage, char *dst_codepage,
                        long start_rec, int limit)
{
    int err = db_open(dbf_filename);
    int count_field = field_count();
    
  
    if ( err == 0)
    {
        if (start_rec > 0)
            err = db_goto(start_rec);
        
        if (err == 0 )
        {
            unsigned int i_rec = 0;
            while (!db_eof())
            {
                for (int i = 0; i < count_field; i++)
                {
                    char *value = get_num(i);
                    value = str_encoding(value, src_codepage, dst_codepage, FALSE);
                    printf(value);
                    // За последним значением разделитель ставить не надо
                    if ( i != (count_field - 1) )
                        printf("|");
                }
                printf("\n");
                db_next();
                
                i_rec++;
                if ( i_rec >= limit)
                    break;
            }
        }
    }
    
    if ( err != 0 )
        print_db_error(err);
    
    db_close();
}


/**
*   Вывести на экран описание полей DBF файла.
*   @param dbf_filename Полное наименование DBF файла
*/
void print_fields(char *dbf_filename)
{
    int err = db_open(dbf_filename);
    int count_field = field_count();
    
    if ( err == 0)
    {
        for (int i = 0; i < count_field; i++)
        {
            char *field_name = db_getname(i);
            char field_type = db_gettype(i);
            int field_length = db_getlengt(i);
            int field_decimal = db_getdecimal(i);
            printf("%s|%c|%d|%d\n", field_name, field_type, field_length, field_decimal);
        }
    }
    else
        print_db_error(err);
    
    db_close();
}


/**
*   Вывести на экран количество записей DBF файла.
*   @param dbf_filename Полное наименование DBF файла
*/
void print_record_count(char *dbf_filename)
{
    int err = db_open(dbf_filename);
    long int rec_count = db_lastrec();
    
    if ( err == 0)
        printf("%lu\n", rec_count);
    else
        print_db_error(err);
    
    db_close();
}

/**
*   Печатать выбранные записи DBF файла.
*   @param dbf_filename Полное наименование DBF файла
*   @param fields Список имен отображаемых полей. Если NULL отображаются все поля.
*   @param filter_field Поле фильтра. Если NULL фильтрация не производится.
*   @param filter_value Значение фильтра.
*   @param order_by Поле сортировки. Если NULL сортировка не производится.
*   @param is_reverse Вкл. обратная сортировка.
*   @param start_rec Номер первой обрабатываемой строки.
*   @param limit Ограничение выводимых записей. Если -1, то ограничения нет.
*   @param src_codepage Кодовая страница DBF файла.
*   @param dst_codepage Кодовая страница для вывода.
*/
void print_select(char *dbf_filename, char **fields, 
                  char *filter_field, char *filter_value,
                  char *order_by, BOOL is_reverse, 
                  long start_rec, int limit,
                  char *src_codepage, char *dst_codepage)
{
    if ( (fields == NULL) && (filter_field == NULL) && (order_by == NULL) && (start_rec == 0) && (limit == -1) )
        // Тупо выводим все записи 
        return print_all_records(dbf_filename, src_codepage, dst_codepage);
    else if ( (fields == NULL) && (filter_field == NULL) && (order_by == NULL) && (start_rec >= 0) && (limit > 0) )
        // Вывести диапазон строк
        return print_record_range(dbf_filename, src_codepage, dst_codepage, start_rec, limit);
}
                  

/**
* Функция запуска основного алгоритма
*/
int run(int argc, char *argv[])
{
    // Разбор коммандной строки
    int opt = 0;
    BOOL is_select_cmd = TRUE;      // По умолчанию выборка записей из DBF
    BOOL is_delete_cmd = FALSE;     
    BOOL is_fields_cmd = FALSE;     
    BOOL is_length_cmd = FALSE;     
    char *dbf_filename = NULL;
    long start_rec = 0;
    int limit = -1;
    
    // Используемые кодовые страницы
    char *src_codepage = "CP866";
    char *dst_codepage = "UTF-8";
    
    const struct option long_opts[] = {
              { "debug", no_argument, NULL, 'd' },
              { "log", no_argument, NULL, 'l' },
              { "version", no_argument, NULL, 'v' },
              { "help", no_argument, NULL, 'h' },
              { "dbf", required_argument, NULL, 'f' },
              { "cmd", required_argument, NULL, 'C' },
              { "where", required_argument, NULL, 'W' },
              { "start_rec", required_argument, NULL, 'B' },
              { "limit", required_argument, NULL, 'L' },
              { "fields", required_argument, NULL, 'F' },
              { "order_by", required_argument, NULL, 'O' },
              { "reverse", no_argument, NULL, 'R' },
              { "src_codepage", required_argument, NULL, 'S' },
              { "dst_codepage", required_argument, NULL, 'D' },
              { NULL, 0, NULL, 0 }
       };
  
    if (DBG_MODE) logInfo("OPTIONS:");
    while ((opt = getopt_long(argc, argv, "dlvhfCWLFORSD:", long_opts, NULL)) != -1)
    {
        switch (opt) 
        {
            case 'd':
                DBG_MODE = TRUE;
                if (DBG_MODE) logInfo("\t--debug");
                break;

            case 'l':
                DBG_MODE = TRUE;
                if (DBG_MODE) logInfo("\t--log");
                break;
                
            case 'h':
                printHelp();
                if (DBG_MODE) logInfo("\t--help");
                break;
                
            case 'v':
                printVersion();
                if (DBG_MODE) logInfo("\t--version");
                break;
                
            case '?':
                printHelp();
                return TRUE;

            case 'f':
                dbf_filename = optarg;
                if (DBG_MODE) logInfo("\t--dbf = %s", dbf_filename);
                break;

            case 'B':
                start_rec = atol(optarg);
                if (DBG_MODE) logInfo("\t--start_rec = %s", optarg);
                break;
                                
            case 'L':
                limit = atoi(optarg);
                if (DBG_MODE) logInfo("\t--limit = %s", optarg);
                break;

            case 'S':
                src_codepage = optarg;
                if (DBG_MODE) logInfo("\t--src_codepage = %s", src_codepage);
                break;

            case 'D':
                dst_codepage = optarg;
                if (DBG_MODE) logInfo("\t--dst_codepage = %s", dst_codepage);
                break;
                
            case 'C':
                if (strcmp(optarg, "SELECT") == 0)
                {
                    is_select_cmd = TRUE;
                    is_delete_cmd = FALSE;
                    is_fields_cmd = FALSE;
                    is_length_cmd = FALSE;
                }
                else if (strcmp(optarg, "DELETE") == 0)
                {
                    is_select_cmd = FALSE;
                    is_delete_cmd = TRUE;
                    is_fields_cmd = FALSE;
                    is_length_cmd = FALSE;
                }                
                else if (strcmp(optarg, "FIELDS") == 0)
                {
                    is_select_cmd = FALSE;
                    is_delete_cmd = FALSE;
                    is_fields_cmd = TRUE;
                    is_length_cmd = FALSE;
                }
                else if (strcmp(optarg, "LENGTH") == 0)
                {
                    is_select_cmd = FALSE;
                    is_delete_cmd = FALSE;
                    is_fields_cmd = FALSE;
                    is_length_cmd = TRUE;
                }
                if (DBG_MODE) logInfo("\t--cmd = %s", optarg);
                break;
                
            default:
                fprintf(stderr, "Unknown parameter: \'%c\'", opt);
                return FALSE;
        }
    }

    if (is_select_cmd)    
        print_select(dbf_filename, NULL, NULL, NULL, NULL, FALSE, start_rec, limit, 
                     src_codepage, dst_codepage);
    else if (is_delete_cmd)    
        ;
    else if (is_fields_cmd)
        print_fields(dbf_filename);
    else if (is_length_cmd)
        print_record_count(dbf_filename);
                
    return 1;
}
