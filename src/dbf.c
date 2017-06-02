/**
* Модуль основных функций взаимодействия с DBF файлом
* @file
*/

#include "dbf.h"
         
static unsigned char hdr_type;      /** тип базы */
static unsigned char hdr_date[3];   /** дата последней модификации */
static unsigned long hdr_lastrec;   /** кол-во записей в базе */
static unsigned int  hdr_begin;     /** позиция начала первой записи */
static unsigned int  hdr_lengt;     /** длина записи */
static unsigned int  hdr_flag;      /** флаг служебный */
static unsigned int  hdr_code;      /** меркер кодовойтаблицы */
static unsigned int  hdr_dlina = 1;		/** расчитаннач длина записи */
static          int hdr_err = '\x00'; 	/** Код ошибки */

static struct field *table;         /** таблица полей */
static FILE *hdr = NULL;            /** хедер файла */
static unsigned long recno = 0;     /** текущий номер записи */
static unsigned long recpos = 0 ;   /** позиция в файле текущей записи (Указывает на флаг удаления) */
static unsigned char flag_delete = '\xff' ;   /** SET DELETE ON/OFF */
static          char buffers[257] = "\x00" ;  /** буффер для знаяения полей */
static          char *buf_rec = NULL ;    /** буффер для записи */
static unsigned int fieldcount = 0;       /** кол-во полей */
static unsigned char flag_eof = '\x00', flag_bof = '\x00';   /** ФЛАГ EOF() BOF() */
static          char file_name[256];    /** имя файла базы данных */
static          char flag_ud;           /** флаг 0-обновдения буффера записи не трубуется */


/**
*   Открыть DBF файл.
*   открыть базу, (указать имя) возврат 0-норма; не ноль - ошибка.
*/
int  db_open(char *name)
{
    unsigned int i, filcount = 0, ofst = 1, tmp[15];

    if (hdr !=NULL) 
        db_close();     // есть открытая база, закрыть ее

    hdr_dlina = 1;
    recno = 0;
    hdr_err = NOT_ERROR;

    if ((hdr = fopen(name, "r+b")) == NULL)
    {  
        table = NULL;
        hdr_err = NOT_OPEN;
        return 1;
    }
    for(i=0; (i < 256) && (name[i] != '\x00'); i++) 
        file_name[i] = name[i];       // сохранить имя файла
        
    tmp[0] = db_fgetc(hdr);   
    hdr_type = (char)(tmp[0]);    // тип базы
    tmp[0] = db_fgetc(hdr);   
    tmp[1] = db_fgetc(hdr);   
    tmp[2] = db_fgetc(hdr);     // дата модификации
    hdr_date[0] = (char)(tmp[0]); 
    hdr_date[1] = (char)(tmp[1]); 
    hdr_date[2] = (char)(tmp[2]);
    tmp[0] = db_fgetc(hdr);   
    tmp[1] = db_fgetc(hdr);   
    tmp[2] = db_fgetc(hdr);   
    tmp[3] = db_fgetc(hdr);
    hdr_lastrec = tmp[0] + 256 * tmp[1] + (256 * 256) * tmp[2] + (256 * 256 * 256) * tmp[3];    // число записей в базе
    tmp[0] = db_fgetc(hdr);
    tmp[1] = db_fgetc(hdr);
    hdr_begin = tmp[0] + 256 * tmp[1];      // адрес начала первой записи
    tmp[0] = db_fgetc(hdr);   
    tmp[1] = db_fgetc(hdr);
    hdr_lengt = tmp[0] + 256 * tmp[1];      // длина одной записи
    db_fseek(hdr, 14, SEEK_CUR);            //RESERVED
    tmp[0] = db_fgetc(hdr); 
    hdr_flag = tmp[0];                      //флаг таблицы
    tmp[0] = db_fgetc(hdr); 
    hdr_code = tmp[0];                      // кодовая страница символов
    db_fseek(hdr, 4, SEEK_CUR);             //RESERVED

    //****************  начало таблицы полей *****************************

    // Цикл подстчета кол-во полей
    while ( (char) db_fgetc(hdr) != 0xd ) 
    {   
        db_fseek(hdr, 31, SEEK_CUR);
        filcount++;
    } 
    fieldcount = filcount;

    db_fseek(hdr, 32, SEEK_SET);    //  начало таблицы полей *****************************
    //table = new field[filcount];
    table = (struct field*) calloc(filcount, sizeof(struct field));
    if (table == NULL) 
    { 
        hdr_err=NOT_MEMORY; 
        return 1;
    }
    
    for(i = 0; i < filcount ; i++) 
    {
        table[i].name[0] = (char) db_fgetc(hdr);
        table[i].name[1] = (char) db_fgetc(hdr);
        table[i].name[2] = (char) db_fgetc(hdr);
        table[i].name[3] = (char) db_fgetc(hdr);
        table[i].name[4] = (char) db_fgetc(hdr);
        table[i].name[5] = (char) db_fgetc(hdr);
        table[i].name[6] = (char) db_fgetc(hdr);
        table[i].name[7] = (char) db_fgetc(hdr);
        table[i].name[8] = (char) db_fgetc(hdr);
        table[i].name[9] = (char) db_fgetc(hdr);
        table[i].name[10] = (char) db_fgetc(hdr);

        table[i].type = (char) db_fgetc(hdr);

        tmp[0] = db_fgetc(hdr);
        tmp[1] = db_fgetc(hdr);
        tmp[2] = db_fgetc(hdr);
        tmp[3] = db_fgetc(hdr);
        table[i].offset = tmp[0];
        table[i].offset += 256 * tmp[1];
        table[i].offset += 256 * 256 * tmp[2];
        table[i].offset += 256 * 256 * 256 * tmp[3];

        table[i].lengt = db_fgetc(hdr);
        table[i].decimal = db_fgetc(hdr);
        table[i].flag = (char) db_fgetc(hdr);
        db_fseek(hdr, 13, SEEK_CUR);    //RESERVED

        table[i].sm = ofst;             // расчитать смещение поля относительно записи
        ofst += table[i].lengt;
        hdr_dlina += table[i].lengt;    // расчитать длину записи
    }
    
    //********** инициализация флагов и переменных ****************
    buf_rec = (char*) malloc(hdr_dlina);    // память под буффер записи
    if (buf_rec == NULL) 
    { 
        hdr_err = NOT_MEMORY; 
        return 1;
    }

    recno = 1;  
    db_fseek(hdr, hdr_begin, SEEK_SET); 
    recpos = hdr_begin;
    flag_eof = '\x00'; 
    flag_bof = '\x00';
    flag_ud = '\xff';   // требуется обновление буффера
    return 0 ;
}

//*****************************************************************
static void getdate(struct date *_datep)
{
    time_t timer;
    struct tm *_cur_time_;

    /* получает время суток */
    timer = time(NULL);

    /* переводит дату и время в структуру */
    _cur_time_=localtime(&timer);

    // Получить текущую дату и время
    _datep->da_year=_cur_time_->tm_year;
    _datep->da_mon=_cur_time_->tm_mon;
    _datep->da_day=_cur_time_->tm_mday;
}

/**
*   закрытие базы.
*/
int  db_close(void) 
{   
    struct date d;

    hdr_err = NOT_ERROR;
    if ( hdr != NULL ) 
    {
        getdate(&d);                //
        d.da_year -= 1900;
        db_fseek(hdr, 1, SEEK_SET);
        db_fputc(d.da_year , hdr); //
        db_fputc((int) (d.da_mon), hdr);
        db_fputc((int) (d.da_day), hdr);   // дата модификации

        int buf;
        db_fseek(hdr, -1L , SEEK_END); // если нет символа EOF, добавить его
        buf = db_fgetc(hdr);
        if ( buf != (int) ('\x1a') ) 
        {
            db_fseek(hdr, 0, SEEK_END);
            db_fputc((int) ('\x1a'), hdr);
        }
        free(table);        //delete table;
        free(buf_rec);
        fclose(hdr);
    } 
    else    
        hdr_err = NOT_OPEN;
    hdr = NULL;
    fieldcount = 0;
    recno = 0;      // текущий номер записи
    recpos = 0 ;    // позиция в файле текущей записи (Указывает на флаг удаления)
    return 0;
}

/**
*   Получить кол-во полей.
*   вернуть кол-во полей.
*/
int field_count(void)
{
    hdr_err = NOT_ERROR;
    if ( hdr == NULL )  
    {
        hdr_err = NOT_OPEN;
        return 0;       // база не открытая
    } 
    else 
    {
        return fieldcount;  //вернуть кол-во полей=размеру массива
    }
}


/**
*   Получить значение поля в виде стринга по его номеру.
*/
char* get_num(const int field_num)
{
    unsigned int i, j;

    hdr_err = NOT_ERROR;
    if ( hdr == NULL ) 
    { 
        hdr_err = NOT_OPEN; 
        return NULL;
    }   // база не открыта
    
    if ( (field_num < 0) || (field_num > field_count()) ) 
    {
        hdr_err = NOT_SEEK;
        return NULL;    // Вне диапазона
    }
    
    if ( flag_ud != '\x00' )
    { 
        // требуется обновление буффера
        fread(buf_rec, hdr_dlina, 1, hdr);      // чтения всей записи в буффер
        flag_ud = '\x00';                       // сброс флага обновления
    }

    //--------------------
    //        db_fseek(hdr, recpos+table[field_num].sm, SEEK_SET); // позиция начала поля в записи
    /*    for(i=0; i <table[field_num].lengt ;i++ ) {
            buffers[i]=char(db_fgetc(hdr));
        } buffers[i]='\x00'; */
    //          fread(buffers,table[field_num].lengt,1,hdr); // чтения поля в буффер
    //          buffers[table[field_num].lengt+1]='\x00';
    
    //-----------  Выбор нужного поля из буффера -----------
    j = 0;
    for ( i = table[field_num].sm; i < (table[field_num].sm + table[field_num].lengt); i++ ) 
    {
        buffers[j++] = buf_rec[i];
    }
    buffers[j] = '\x00';

    return &buffers[0];
}


/**
*   Движение по записям.
*/
int  db_skip(long kol) 
{ 
    hdr_err = NOT_ERROR;
    if ( hdr == NULL ) 
    {
        hdr_err = NOT_OPEN; 
        return 1; 
    }       // база не открыта
    
    skip_local(kol);
    
    if ( flag_delete !=0 ) 
    { 
        // надо фильтровать от удаленных записей
        while ( (db_delete(99) != 0) && (db_eof() == 0) && (db_bof() == 0) ) 
        {
            if ( kol > 0 ) 
                skip_local(1); 
            else 
                skip_local(-1);
        }
    }
    flag_ud = '\xff';   // требуется обновление буффера
    return 0;
}


/**
*   Переход на предыдущую запись.
*/
int  db_prev(void) 
{ 
    return db_skip(-1);
}


/**
*   Переход на следующую запись.
*/
int  db_next(void) 
{ 
    return db_skip(1);
}


void skip_local(long kol) 
{
    long sm;

    flag_bof = '\x00'; 
    flag_bof = '\x00';
    recno += kol;
    if ( recno <= 0 ) 
    {
        flag_bof = '\xff';
        recno = 1;
    }
    if ( recno > hdr_lastrec ) 
    {
        flag_eof = '\xff';
        recno = hdr_lastrec;
    }
    recpos = hdr_begin + hdr_lengt * (recno - 1);
    sm = recpos - db_ftell(hdr);
    db_fseek(hdr, sm, SEEK_CUR);
}


/**
*   Проверка на конец файла (1-попытка выйти за конец; 0-норма)
*/
int  db_eof(void) 
{
    hdr_err = NOT_ERROR;
    if ( hdr == NULL ) 
    { 
        hdr_err = NOT_OPEN; 
        return 1;
    }       // база не открыта
    return flag_eof;
}


/**
*   Проверка на начало файла (1-попытка выйти за начало; 0-норма)
*/
int  db_bof(void) 
{
    hdr_err = NOT_ERROR;
    if ( hdr == NULL ) 
    {   
        hdr_err = NOT_OPEN; 
        return 1;
    }       // база не открыта
   return flag_bof;
}


/**
*   Удаление( без парам-проверка; 1-уст.удаление; 0-снять).
*       flag = 0 сброс удаление
*            = 1 уст.  удаление
*       без парам.  проверка удаления
*/
int  db_delete(int flag) 
{
    hdr_err = NOT_ERROR;
    if ( flag == 99 ) 
        return delete_local();      // только проверить на удаление
    if ( flag == 0 ) 
    { 
        // снять маркер удаления
        db_fputc('\x20', hdr);
        db_fseek(hdr, -1, SEEK_CUR);
        return 0;
    }
    
    if ( flag > 0 ) 
    { 
        // поставить маркер удаления
        db_fputc('\x2a', hdr);
        db_fseek(hdr, -1, SEEK_CUR);
        return 1;
    }
    return 0;
}

/**
*   Проверка на удаление.
*/
int delete_local(void) 
{ 
    char del;
    del = (char) db_fgetc(hdr);
    db_fseek(hdr, -1 , SEEK_CUR);
    if ( del == '\x2a' ) 
        return 1;
    else 
        return 0;
}

/**
*   сброс/установка/проверка  фильтрации удаленных записей.
*/
int  set_delete(int flag) 
{
    hdr_err = NOT_ERROR;
    if ( flag == 99 ) 
    { 
        // только проверить SET DELETE
        if ( flag_delete != '\x00' ) 
            return 0; 
        else 
            return 1;
    }

    // Установить SET DELETE ON/OFF
    if ( flag == '\x00' ) 
    {
        flag_delete = '\x00';
        return 0;
    } 
    else 
    {
        flag_delete = '\xff';
        return 1;
    }
}

/**
*   Добавить запись.
*/
int  db_append(void)
{ 
    int unsigned i;
    void *pointer;

    hdr_err = NOT_ERROR;
    if ( hdr == NULL ) 
    {
        hdr_err = NOT_OPEN;
        return 1;
    }

    db_fseek(hdr, 4, SEEK_SET);     // LASTREC()
    pointer = ++hdr_lastrec;     //pointer = &(++hdr_lastrec);
    db_fputc((int) (*(char*)pointer), hdr);  
    pointer = (char*) pointer + 1;
    db_fputc((int) (*(char*)pointer), hdr);  
    pointer = (char*) pointer + 1;
    db_fputc((int) (*(char*)pointer), hdr);  
    pointer = (char *) pointer + 1;
    db_fputc((int) (*(char*)pointer), hdr); // LASTREC()****************

    recno = hdr_lastrec;

    db_fseek(hdr, 0, SEEK_END);     // собственно добавление
    recpos = db_ftell(hdr);
    db_fputc('\x20', hdr);          // марекр удаления
    for (i = 1; hdr_dlina > i; i++)     
        db_fputc('\x20', hdr);
    db_fseek(hdr, recpos, SEEK_SET);
    flag_ud = '\xff';               // требуется обновление буффера
    return 0;
}

/**
*   ЗАПИСЬ В ПОЛЕ текущей записи по номеру поля.
*   установить значение в поле по его номеру.
*/
int put_num(char* value, int pole) 
{
    unsigned int i;
    char *val;

    hdr_err = NOT_ERROR;
    if ( hdr == NULL ) 
    {
        hdr_err = NOT_OPEN;
        return 1;
    }
    //val = new char[table[pole].lengt+1];    //нормализация стринга для записи
    val = (char*) calloc(table[pole].lengt + 1, sizeof(char));
    
    padl(value, val, table[pole].lengt, '\x20');

    db_fseek(hdr, recpos + table[pole].sm, SEEK_SET);
    for (i = 0; (i < table[pole].lengt) && (val[i] != '\x00'); i++)
        db_fputc(val[i], hdr);

    db_fseek(hdr, recpos, SEEK_SET);
    free(val);      //delete val;
    return 0;
}


// ***  переопределение системныъ функций   ***

int db_fputc(int a, FILE *b)
{
    int ret = fputc(a, b);
    if ( ret == EOF ) 
        hdr_err = NOT_PUT;
    return ret;
}


int db_fgetc(FILE *a)
{
    int ret = fgetc(a);
    if ( ret == EOF ) 
        hdr_err = NOT_GET;
    return ret;
}


int db_fseek(FILE *a, long b, int c)
{
    int ret = fseek(a, b, c);
    if ( ret != 0 )  
        hdr_err = NOT_SEEK;
    return ret;
}


long int db_ftell(FILE *a)
{
    long int ret = ftell(a);
    if ( ret < 0 ) 
        hdr_err = NOT_TELL;
    return ftell(a);
}


/**
*   Вернуть код ошибки.
*/
int db_error(void) 
{
    return (int) (hdr_err);
}


/**
*   Получить кол-во записей в базе.
*/
long int db_lastrec(void) 
{
    hdr_err = NOT_ERROR;
    return hdr_lastrec;
}


/**
*   Поиск номера поля по его имени.
*/
int find_field(char *name) 
{ 
    int ret = -1;
    int unsigned i;

    hdr_err = NOT_ERROR;
    for (i = 0 ; i < fieldcount; i++) 
    {
        //сравнение с учетом регистра !!!!!!!!!!!!!!!!!!!!!!!!!
        if ( strcmp((table[i].name), (name)) == 0 ) 
        {
           ret = i;
           i = fieldcount + 1;      // досрочный выход из цикла
        }
    }
    if ( ret < 0 ) 
        hdr_err = NOT_FIELD;
    return ret;
}


/**
*   Получить значение поля в виде стринга по его имени.
*/
char* get_name(char *name) 
{
    int pole;

    hdr_err = NOT_ERROR;
    pole = find_field(name) ;
    if ( pole < 0 ) 
        return NULL;
    return get_num(pole);
}


/**
*   Записать значение поля в виде стринга по его имени.
*/
int put_name(char *value, char *name) 
{
    int pole;

    hdr_err = NOT_ERROR;
    pole = find_field(name);
    if ( pole < 0 ) 
        return NULL;
    return put_num(value, pole);
}


/**
*   Очистить список полей.
*/
int db_create1(void) 
{ 
    hdr_err = NOT_ERROR;
    db_close();
    
    // table = new(field[255]);
    table = (struct field*) calloc(255, sizeof(struct field));
    if ( table == NULL ) 
    {
        hdr_err = NOT_MEMORY; 
        return 1;
    }
    return 0;
}


/**
*   Добавить в список полей.
*/
int db_create2(char *db_name, char db_type, unsigned int lengt,
               unsigned int decimal) 
{
    unsigned int i, kolvo;

    hdr_err = NOT_ERROR;
    kolvo = fieldcount;
    if ( table == NULL ) 
    {
        hdr_err = NOT_ALLOC; 
        return 1;
    }

    // db_name=strupr(db_name);
    for (i = 0; i < sizeof(table[0].name); i++) 
        table[kolvo].name[i] = '\x00';
    for (i = 0; db_name[i] != '\x00'; i++) 
        table[kolvo].name[i] = db_name[i];


    if ( (db_type == 'C') ||
         (db_type == 'N') ||
         (db_type == 'F') ||
         (db_type == 'D')
        )    
        table[kolvo].type = db_type;
    else 
    {
        hdr_err = NOT_TYPE;
        return 1;
    }
      
    if ( db_type == 'D') 
    { 
        lengt = 8; 
        decimal = 0; 
    }
    if ( db_type == 'C') 
    {          
        decimal = 0; 
    }
    if ( (db_type =='F') && (lengt <= (decimal + 1)) ) 
    {
        hdr_err = NOT_TYPE;
        return 1;
    }
    if ( (lengt > 0 ) && (lengt <= 255) )  
        table[kolvo].lengt = lengt;
    else 
    {
        hdr_err = NOT_TYPE;
        return 1;
    }
    
    if ( lengt >= (decimal + 1)) 
        table[kolvo].decimal = decimal;
    else 
    {
        hdr_err = NOT_TYPE;
        return 1;
    }
    
    // поле  определено, идет расчет служебной информации
    table[kolvo].offset = 1;    // маркер удаления
    for (i = 1; i <= kolvo; i++) 
    {
        table[kolvo].offset += table[i-1].lengt;
    }
    table[kolvo].sm = table[kolvo].offset;
    fieldcount = kolvo + 1;
    return 0;
}


int db_create3(char* name)
{
    FILE  *out;
    int unsigned i, buf, begin, lengt;
    void *pointer;
    struct date d;

    hdr_err = NOT_ERROR;
    if ((out = fopen(name, "wb")) == NULL) 
    { 
        hdr_err = NOT_OPEN;
        return 1;
    }

    db_fputc('\x03', out);      // тип базы-1

    getdate(&d);                //
    if ( d.da_year <= 1999 ) 
        d.da_year -= 1900; 
    else 
        d.da_year -= 1900;
    db_fputc(d.da_year, out); //
    db_fputc((int) (d.da_mon), out);
    db_fputc((int) (d.da_day), out);   // дата модификации

    db_fputc('\x00', out);  
    db_fputc('\x00', out);
    db_fputc('\x00', out);  
    db_fputc('\x00', out); // кол-во записей

    // считается что таблица полей заполнена
    begin = 32;     // 31-- длина заголовка до таблицы полей
    lengt = 1;      // 1- флаг удаления
    for (i = 0; i < fieldcount; i++) 
    {
        begin += 32;    // 32-длина записи таблицы полей
        lengt += table[i].lengt;
    }
    // begin +=264; // 264-длина заголовка после таблицы полей
    begin += 1;     // для терминатора
    pointer = &begin;
    db_fputc((int) (*(char*) pointer), out);  
    pointer = (char*) pointer + 1;
    db_fputc((int) (*(char*) pointer), out);   // позиция начала записей
                                            // млад. байт + ст.байт

    pointer = &lengt;
    db_fputc((int) (*(char*) pointer), out);  
    pointer = (char*) pointer + 1;
    db_fputc((int) (*(char*) pointer), out);   // длина записи

    for (i = 0; i < 16; i++) 
        db_fputc('\x00', out);      // RESERVED
    db_fputc((int) ('\x00'), out);     // флаг заголовка ????
    db_fputc((int) ('\xc9'), out);     // кодовая таблица  Rusian Windows
    db_fputc((int) ('\x00'), out);
    db_fputc((int) ('\x00'), out);     // RESERVED

    for (i = 0; i < fieldcount; i++) 
    {
        db_fputc((int) (table[i].name[0]), out);
        db_fputc((int) (table[i].name[1]), out);
        db_fputc((int) (table[i].name[2]), out);
        db_fputc((int) (table[i].name[3]), out);
        db_fputc((int) (table[i].name[4]), out);
        db_fputc((int) (table[i].name[5]), out);
        db_fputc((int) (table[i].name[6]), out);
        db_fputc((int) (table[i].name[7]), out);
        db_fputc((int) (table[i].name[8]), out);
        db_fputc((int) (table[i].name[9]), out);
        db_fputc((int) (table[i].name[10]), out);    // имя поля

        db_fputc((int) (table[i].type), out);        // тип поля

        pointer = &(table[i].offset);
        db_fputc((int) (*(char*) pointer), out);  
        pointer = (char*) pointer + 1;
        db_fputc((int) (*(char*) pointer), out);  
        pointer=(char*) pointer + 1;
        db_fputc((int) (*(char*) pointer), out);  
        pointer=(char*) pointer + 1;
        db_fputc((int) (*(char*) pointer), out);   // смещение поля

        db_fputc((int) (table[i].lengt), out);     // длина поля
        db_fputc((int) (table[i].decimal), out);   // позиция десятич. точки

        db_fputc('\x00', out);                  // флаг поля ???????
        for (buf = 0; buf < 13; buf++) 
            db_fputc('\x00', out);              // RESERVED
    }
    db_fputc('\x0d', out);                      // терминатор таблицы полей
    //   for(i=0; i<263; i++) db_fputc('\x00', out); // RESERVSD
    fclose(out);
    db_close();
    return 0;
}


/**
*   Позиционировать по номеру записи.
*/
int db_goto(long rec) 
{ 
    hdr_err = NOT_ERROR;
    flag_bof = '\x00'; 
    flag_eof = '\x00';

    if ( hdr == NULL ) 
    {
        hdr_err = NOT_OPEN; 
        return 1;
    }   // база не открыта
    
    if ( rec < 1 ) 
    { 
        db_goto(1); 
        flag_bof = '\xff'; 
        flag_eof = '\x00'; 
        return 0;
    }
    
    if ( rec > (long) (hdr_lastrec)) 
    {
        db_goto(hdr_lastrec);
        flag_bof = '\x00'; 
        flag_eof = '\xff';
        return 0;
    }
    recno = rec;
    recpos = hdr_begin + hdr_lengt * (recno - 1);
    db_fseek(hdr, recpos, SEEK_SET);

    if ( flag_delete != 0) 
    {
        while ( (db_delete(99) != 0) && (db_eof() == 0) )   
            db_skip(1);      // искать вниз ближ. НЕудаленную запись
    }
    flag_ud = '\xff';       // требуется обновление буффера
    return 0;
}

/**
*   Быстрый ??? поиск в упорядоченной базе.
*/
int find_sort(char *pole, char *isk_stroka) 
{ 
    long  unsigned pos, pos_begin, pos_end, sm;
    char *buf, *isk;
    int numpole, tmp, ret = 0;
    char find_flag = '\x00';

    numpole = find_field(pole);
    if ( numpole < 0 ) 
        return 0;       // нет такого поля

    //isk = new char[table[numpole].lengt + 1];
    isk = (char*) calloc(table[numpole].lengt + 1, sizeof(char));
    padl(isk_stroka , isk , table[numpole].lengt, '\x20');
    pos_begin = recno; 
    pos_end = hdr_lastrec;
    while ( find_flag == 0 ) 
    { 
        // Он сказал "Поехали!" и махнул рукой :)
        if ( pos_begin == pos_end ) 
        { 
            // перелопатили все
            find_flag = '\xff';        // флаг на выход
        }
        sm = (pos_end - pos_begin) * MEDIA;
        pos = pos_begin + sm ;
        db_goto(pos);
        buf = get_num(numpole);
        tmp = strcmp(buf, isk);
        if ( tmp > 0 ) 
        { 
            pos_end  = pos - 1 ;
        }
        if ( tmp < 0 ) 
        { 
            pos_begin = pos + 1 ;
        }
        if ( tmp == 0 ) 
        { 
            pos_end  = pos ;
        }
        if ( (pos_end < 0) || (pos_begin > hdr_lastrec)) 
        {
            find_flag = '\xff';
            ret = 0;
        }
        if ( (find_flag != 0) && (tmp == 0) ) 
        { 
            // перелопатили все и что-то нашли
            ret = 1;
        }
    }
    free(isk);         //delete isk;

    db_goto(pos_end);

    return ret;
}


/**
*   Копирование в новую строку(слева направо) с дополнение слева.
*/
char *padl(char *in, char *out, int lengt, char sym)
{
    int i, j;

    for (i = strlen(in) - 1, j=lengt - 1; (i >= 0) && (j >= 0); i--, j--)
        out[j] = in[i];
    for( ; j >= 0; j--)
        out[j] = sym;               //дополнение пробелами слева

    out[lengt] = '\x00';
    return out;
}

/**
*   АНАЛОГ ФУНКЦИИ  CLIPPER RECNO()
*   Получить текущий номер записи (1...lastrec).
*/
long db_recno(void) 
{ 
    return recno;
}

/**
*   Проверить есть ли открытая база.
*/
char db_used(void) 
{ 
    if ( hdr == NULL ) 
        return '\x00';
    else            
        return '\xff';
}


char* db_getname(int num) 
{
    if ( db_used == 0) 
    {
        hdr_err = NOT_OPEN; 
        return NULL;
    }
    if ( (num < 0) || (num >= (int) (fieldcount)) ) 
    {
        hdr_err = NOT_FIELD; 
        return NULL;
    }

    return table[num].name;
} 


char db_gettype(int num) 
{
    if ( db_used == 0) 
    {
        hdr_err = NOT_OPEN; 
        return NULL;
    }
    if ( (num < 0) || (num >= (int) (fieldcount)) ) 
    {
        hdr_err = NOT_FIELD; 
        return NULL;
    }

    return table[num].type;
} 


int db_getlengt(int num) 
{
    if ( db_used == 0) 
    {
        hdr_err = NOT_OPEN; 
        return NULL;
    }
    if ( (num < 0) || (num >= (int) (fieldcount)) ) 
    {
        hdr_err = NOT_FIELD; 
        return NULL;
    }

    return table[num].lengt;
} 


int db_getdecimal(int num) 
{
    if ( db_used == 0) 
    {
        hdr_err = NOT_OPEN; 
        return NULL;
    }
    
    if ( (num < 0) || (num >= (int) (fieldcount)) ) 
    {
        hdr_err = NOT_FIELD; 
        return NULL;
    }

    return table[num].decimal;
} 


/**
*   УДАЛЕНИЕ всех записей базы.
*/
int db_zap(void) 
{
    FILE  *out;
    unsigned long i;
    struct date d;

    hdr_err = NOT_ERROR;
    if( hdr == NULL ) 
    {
        hdr_err = NOT_OPEN;
        return 1;
    }
    if ( (out = fopen(TMP_NAME, "wb")) == NULL ) 
    { 
        hdr_err=NOT_OPEN;
        return 1;
    }

    db_fseek(hdr, 0, SEEK_SET);
    for (i = 0; i < hdr_begin; i++)     
        db_fputc(db_fgetc(hdr), out);   // перезапись всего заголовка

    db_fseek(out, 1, SEEK_SET);
    getdate(&d);                    // ------------------
    d.da_year -= 1900;
    db_fputc(d.da_year , out);      //
    db_fputc((int) (d.da_mon), out);
    db_fputc((int) (d.da_day), out);   // дата модификации

    db_fputc('\x00', out);    // Число записей в базе =0
    db_fputc('\x00', out);
    db_fputc('\x00', out);
    db_fputc('\x00', out);   //------------------------

    fclose(out);
    db_close();

    if ( db_error() == 0 ) 
    {
        if ( remove(file_name) != 0 ) 
        {
            hdr_err = NOT_DEL;
            return 1;
        }
        if ( rename(TMP_NAME, file_name) != 0 ) 
        {
            hdr_err = NOT_RENAME;
            return 1;
        }
    }
    db_open(file_name);
    flag_ud = '\xff';       // требуется обновление буффера
    return 0;
}


/**
*   Упаковка записей базы.
*/
int db_pack(void) 
{
    FILE *out;
    unsigned long i, j, count = 0;
    char buf;
    void *pointer;

    hdr_err = NOT_ERROR;
    if( hdr == NULL ) 
    {
        hdr_err = NOT_OPEN;
        return 1;
    }
    if ( (out = fopen(TMP_NAME, "wb")) == NULL ) 
    { 
        hdr_err = NOT_OPEN;
        return 1;
    }

    db_fseek(hdr, 0, SEEK_SET);
    for(i = 0; i < hdr_begin; i++)     
        db_fputc(db_fgetc(hdr), out);   // перезапись всего заголовка

    db_fseek(out, hdr_begin, SEEK_SET); // начинается перебор записей
    for (j = 0; j < hdr_lastrec; j++) 
    {
        buf = (char) db_fgetc(hdr);
        if( buf != '\x2a' ) 
        {
            db_fputc(buf, out);
            for (i = 1; i < hdr_dlina; i++) 
            {
                buf = (char) db_fgetc(hdr);
                db_fputc(buf, out);
            }
            count++;
        } 
        else 
        {
            db_fseek(hdr, hdr_dlina - 1, SEEK_CUR);     // Запись не удаленная, пропуск ее
        }
    }
    db_fseek(out, 4, SEEK_SET);     // корректировка числа записей в заголовке
    pointer = &(count);
    db_fputc((int) (*(char*) pointer), out);  
    pointer = (char *) pointer + 1;
    db_fputc((int) (*(char*) pointer), out);  
    pointer = (char *) pointer + 1;
    db_fputc((int) (*(char*) pointer), out);  
    pointer = (char *) pointer + 1;
    db_fputc((int) (*(char*) pointer), out);   // Число записей в базе

    fclose(out);
    db_close();
    if ( db_error() == 0 ) 
    {
        if( remove(file_name) != 0 ) 
        {            
            hdr_err = NOT_DEL;
            return 1;
        }
        if ( rename(TMP_NAME, file_name) != 0 ) 
        {
            hdr_err = NOT_RENAME;
            return 1;
        }
    }

    db_open(file_name);
    flag_ud = '\xff';       // требуется обновление буффера
    return 0;
}


/**
*   Распечатать ошибку БД.
*/
void print_db_error(int err_code)
{
    if ( err_code == NOT_OPEN  )
        logErr("ERROR: Файл не возможно открыть");    
    else if ( err_code == NOT_GET )
        logErr("ERROR: Ошибка чтения байта из файла");    
    else if ( err_code == NOT_PUT )
        logErr("ERROR: Ошибка записи байта в файл");    
    else if ( err_code == NOT_SEEK )
        logErr("ERROR: Ошибка при позиционировании файла");    
    else if ( err_code == NOT_TELL )
        logErr("ERROR: Ошибка при определении позиции файла");    
    else if ( err_code == NOT_FIELD )
        logErr("ERROR: Ошибка поиска имени поля (поле с таким именем не найдено)");    
    else if ( err_code == NOT_TYPE )
        logErr("ERROR: Ошибка. Неверно указанный тип поля при создании базы");    
    else if ( err_code == NOT_MEMORY )
        logErr("ERROR: Ошибка выделения памяти (не хватает памяти)");    
    else if ( err_code == NOT_ALLOC )
        logErr("ERROR: Ошибка. Память не выделена");    
    else if ( err_code == NOT_LIBRARY )
        logErr("ERROR: Ошибка. Библиотека dll_dbf.dll не может быть загружена");    
    else if ( err_code == NOT_DEL )
        logErr("ERROR: Ошибка. Не могу удалить файл ( возможно где-то он используется)");    
    else if ( err_code == NOT_RENAME )
        logErr("ERROR: Ошибка. Не могу переименовать временный файл");    
    else
        logWarning("Не обрабатываемый код ошибки <%d>", err_code);        
}
