/**
* Модуль основных функций взаимодействия с DBF файлом
* @file
*/

#if !defined( __DBF_H )
#define __DBF_H

//#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <io.h>
//#include <dos.h>
#include <time.h>

/** Коды ошибок */
#define NOT_ERROR  0    /** Нет ошибки */
#define NOT_OPEN   1    /** файл не возможно открыть */
#define NOT_GET    2    /** ошибка чтения байта из файла */
#define NOT_PUT    3    /** ошибка записи байта в файл */
#define NOT_SEEK   4    /** ошибка при позиционировании файла */
#define NOT_TELL   5    /** ошибка при определении позиции файла */
#define NOT_FIELD  6    /** ошибка поиска имени поля (поле с таким именем не найдено) */
#define NOT_TYPE   7    /** неверно указанный тип поля при создании базы */
#define NOT_MEMORY 8    /** ошибка выделения памяти (не хватает памяти) */
#define NOT_ALLOC  9    /** память не выделена */
#define NOT_LIBRARY 10  /** библиотека dll_dbf.dll не может быть загружена */
#define NOT_DEL    11   /** не могу удалить файл ( возможно где-то он используется) */
#define NOT_RENAME 12   /** не могу переименовать временный файл */

// #include "db_error.h"
#define MEDIA 0.5                   /** медиана для быстрого поиска */
#define TMP_NAME "________.tmp"     /** имя для временного файла */
#pragma hdrstop

int db_create3(char*);         /** создать базу, по подобию открытой (0-норма) */
int db_create1(void);          /** очистить список полей */
int db_create2(char *db_name, char db_type, unsigned int lengt,
               unsigned int decimal);  /** добавить в список полей */
int db_open(char*);                    /** открыть базу, (указать имя) возврат 0-норма; не ноль - ошибка */
int db_close(void);                    /** Закрыть базу,  0-норма */
int field_count(void);                 /** получить кол-во полей */
char* get_num(const int field_num);    /** получить значение поля в виде стринга по его номеру */
char* get_name(char *name);            /** получить значение поля в виде стринга по его имени */
int db_skip(long kol);                 /** движение по записям (1- без парам- +1; n- вперед на n; -n назад на n) */
int db_prev(void);                     /** Переход на предыдущую запись */
int db_next(void);                     /** Переход на следующую запись */
int db_goto(long rec);                 /** позиционировать по номеру записи */
int db_eof(void);                      /** Проверка на конец файла (1-попытка выйти за конец;0-норма) */
int db_bof(void);                      /** Проверка на начало файла (1-попытка выйти за начало;0-норма) */
int db_delete(int flag);               /** Удаление(99-проверка; 1-уст.удаление; 0-снять) */
int set_delete(int flag);              /** сброс/установка/проверка  фильтрации удаленных записей */
int db_append(void);                   /** добавить запись (пустую) в конец файла */
int put_num(char *value, int pole);    /** записать значение поля в виде стринга по его номеру */
int put_name(char *value, char *name); /** записать значение поля в виде стринга по его имени */
int db_error(void);                    /** получить ошибку (0- нет ошибки) см. файл db_error.h */
long int db_lastrec(void);             /** получить кол-во записей в базе */
int find_field(char *name);            /** поиск номера поля по его имени */
int find_sort(char *pole, char *isk_stroka);   /** быстрый ??? поиск в упорядоченной базе */
long db_recno(void);                   /** получить текущий номер записи (1...lastrec) */
char db_used(void);                    /** проверить есть ли открытая база */
int db_zap(void);                     /** Запание базы ;) */
int db_pack(void);                    /** Упаковка базы от удаленных записей */
char* db_getname(int num);
char db_gettype(int num);
int db_getlengt(int num);
int db_getdecimal(int num);

//******************************************************************************************
struct date
{
    int     da_year;    /** Year - 1980 */
    char    da_day;     /** Day of the month */
    char    da_mon;     /** Month (1 = Jan) */
};

struct field {                  /** подзапись поля заголовка */
          char name[11];        /** имя поля */
          char type;            /** тип поля */
          unsigned int offset;  /** смещения поля относительно записи (из заголовка) */
          unsigned int lengt;   /** длина поля */
          unsigned int decimal; /** позиция десятич. точки */
          unsigned char flag;   /** системный флаг */
          unsigned int sm;      /** расчитанное смещения поля относительно записи */
         };
//******************************************************************************************


//******************************************************************************
//***  локальные процедуры
void skip_local(long kol);
int delete_local(void);

int db_fputc(int, FILE *);         //------------------------------------
int db_fgetc(FILE *);
int db_fseek(FILE *, long , int ); //-- переопределение системных функций
long int db_ftell(FILE *);         //------------------------------------
char *padl(char *in,char *out, int lengt, char sym);     // По умолчанию sym='\x20'
//******************************************************************************

#endif
