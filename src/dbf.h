/**
* Модуль основных функций взаимодействия с DBF файлом
* @file
* @version 0.0.0.1
*/

#if !defined( __DBF_H )
#define __DBF_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/** Коды ошибок */
#define NOT_ERROR  0    /** Нет ошибки */
#define ERR_OPEN   1    /** файл не возможно открыть */
#define ERR_GET    2    /** ошибка чтения байта из файла */
#define ERR_PUT    3    /** ошибка записи байта в файл */
#define ERR_SEEK   4    /** ошибка при позиционировании файла */
#define ERR_TELL   5    /** ошибка при определении позиции файла */
#define ERR_FIELD  6    /** ошибка поиска имени поля (поле с таким именем не найдено) */
#define ERR_TYPE   7    /** неверно указанный тип поля при создании базы */
#define ERR_MEMORY 8    /** ошибка выделения памяти (не хватает памяти) */
#define ERR_ALLOC  9    /** память не выделена */
#define ERR_LIBRARY 10  /** библиотека dll_dbf.dll не может быть загружена */
#define ERR_DEL    11   /** не могу удалить файл ( возможно где-то он используется) */
#define ERR_RENAME 12   /** не могу переименовать временный файл */

// #include "db_error.h"
#define MEDIA 0.5                   /** медиана для быстрого поиска */
#define TMP_NAME "________.tmp"     /** имя для временного файла */
#pragma hdrstop

int create3_dbf(char*);         /** создать базу, по подобию открытой (0-норма) */
int create1_dbf(void);          /** очистить список полей */
int create2_dbf(char *db_name, char db_type, unsigned int lengt,
               unsigned int decimal);  /** добавить в список полей */
int open_dbf(char*);                    /** открыть базу, (указать имя) возврат 0-норма; не ноль - ошибка */
int close_dbf(void);                    /** Закрыть базу,  0-норма */
int get_field_count_dbf(void);                 /** получить кол-во полей */
char* get_value_by_num_dbf(const int field_num);    /** получить значение поля в виде стринга по его номеру */
char* get_value_by_name_dbf(char *name);            /** получить значение поля в виде стринга по его имени */
int skip_dbf(long kol);                 /** движение по записям (1- без парам- +1; n- вперед на n; -n назад на n) */
int prev_dbf(void);                     /** Переход на предыдущую запись */
int next_dbf(void);                     /** Переход на следующую запись */
int goto_dbf(long rec);                 /** позиционировать по номеру записи */
int eof_dbf(void);                      /** Проверка на конец файла (1-попытка выйти за конец;0-норма) */
int bof_dbf(void);                      /** Проверка на начало файла (1-попытка выйти за начало;0-норма) */
int delete_dbf(int flag);               /** Удаление(99-проверка; 1-уст.удаление; 0-снять) */
int set_delete_dbf(int flag);              /** сброс/установка/проверка  фильтрации удаленных записей */
int append_dbf(void);                   /** добавить запись (пустую) в конец файла */
int set_value_by_num_dbf(char *value, int pole);    /** записать значение поля в виде стринга по его номеру */
int set_value_by_name_dbf(char *value, char *name); /** записать значение поля в виде стринга по его имени */
int get_error_dbf(void);                    /** получить ошибку (0- нет ошибки) см. файл db_error.h */
long int get_lastrec_dbf(void);             /** получить кол-во записей в базе */
int get_field_idx_dbf(char *name);            /** поиск номера поля по его имени */
int find_sort_dbf(char *pole, char *isk_stroka);   /** быстрый ??? поиск в упорядоченной базе */
long get_recno_dbf(void);                   /** получить текущий номер записи (1...lastrec) */
char is_used_dbf(void);                    /** проверить есть ли открытая база */
int zap_dbf(void);                     /** Запание базы ;) */
int pack_dbf(void);                    /** Упаковка базы от удаленных записей */
char* get_field_name_dbf(int num);
char get_field_type_dbf(int num);
int get_field_lengt_dbf(int num);
int get_field_decimal_dbf(int num);
void print_error_dbf(int err_code);

//******************************************************************************************
typedef struct
{
    int     da_year;    /** Year - 1980 */
    char    da_day;     /** Day of the month */
    char    da_mon;     /** Month (1 = Jan) */
} nix_dbf_date_t;

typedef struct  
{   /** подзапись поля заголовка */
    char name[11];        /** имя поля */
    char type;            /** тип поля */
    unsigned int offset;  /** смещения поля относительно записи (из заголовка) */
    unsigned int length;  /** длина поля */
    unsigned int decimal; /** позиция десятич. точки */
    unsigned char flag;   /** системный флаг */
    unsigned int sm;      /** расчитанное смещения поля относительно записи */
} nix_dbf_field_t;
//******************************************************************************************

#endif
