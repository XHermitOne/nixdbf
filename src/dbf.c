/**
* Модуль основных функций взаимодействия с DBF файлом
* @file
* @version 0.0.0.1
*/

#include "nixdbf.h"
         
static unsigned char DbfType;      /** тип базы */
static unsigned char DbfDate[3];   /** дата последней модификации */
static unsigned long DbfLastRec;   /** кол-во записей в базе */
static unsigned int  DbfBegin;     /** позиция начала первой записи */
static unsigned int  DbfRecLength; /** длина записи */
static unsigned int  DbfFlag;      /** флаг служебный */
static unsigned int  DbfCode;      /** меркер кодовойтаблицы */
static unsigned int  DbfCalcRecLength = 1;	/** расчитаннач длина записи */
static          int  DbfError = '\x00'; 	/** Код ошибки */

static nix_dbf_field_t *Table;               /** таблица полей */
static FILE *Header = NULL;                  /** хедер файла */
static unsigned long RecNo = 0;              /** текущий номер записи */
static unsigned long RecPos = 0 ;            /** позиция в файле текущей записи (Указывает на флаг удаления) */
static unsigned char DeleteFlag = '\xff' ;   /** SET DELETE ON/OFF */
static          char Buffers[257] = "\x00" ; /** буффер для знаяения полей */
static          char *RecBuffer = NULL ;     /** буффер для записи */
static unsigned int FieldCount = 0;          /** кол-во полей */
static unsigned char EOFFlag = '\x00', BOFFlag = '\x00';   /** ФЛАГ EOF() BOF() */
static          char DbfFileName[256];       /** имя файла базы данных */
static          char UpdateRecFlag;          /** флаг 0-обновдения буффера записи не трубуется */

//******************************************************************************
//***  локальные процедуры
static void skip_local_dbf(long kol);
static int delete_local_dbf(void);

static int fputc_dbf(int, FILE *);         //------------------------------------
static int fgetc_dbf(FILE *);
static int fseek_dbf(FILE *, long , int ); //-- переопределение системных функций
static long int ftell_dbf(FILE *);         //------------------------------------
static char *padl_dbf(char *in,char *out, int lengt, char sym);     // По умолчанию sym='\x20'
//******************************************************************************

/**
*   Открыть DBF файл.
*   открыть базу, (указать имя) возврат 0-норма; не ноль - ошибка.
*/
extern int  open_dbf(char *name)
{
    unsigned int i, filcount = 0, ofst = 1, tmp[15];

    if (Header !=NULL) 
        close_dbf();     // есть открытая база, закрыть ее

    DbfCalcRecLength = 1;
    RecNo = 0;
    DbfError = NOT_ERROR;

    if ((Header = fopen(name, "r+b")) == NULL)
    {  
        Table = NULL;
        DbfError = ERR_OPEN;
        return 1;
    }
    for(i=0; (i < 256) && (name[i] != '\x00'); i++) 
        DbfFileName[i] = name[i];       // сохранить имя файла
        
    tmp[0] = fgetc_dbf(Header);   
    DbfType = (char)(tmp[0]);    // тип базы
    tmp[0] = fgetc_dbf(Header);   
    tmp[1] = fgetc_dbf(Header);   
    tmp[2] = fgetc_dbf(Header);     // дата модификации
    DbfDate[0] = (char)(tmp[0]); 
    DbfDate[1] = (char)(tmp[1]); 
    DbfDate[2] = (char)(tmp[2]);
    tmp[0] = fgetc_dbf(Header);   
    tmp[1] = fgetc_dbf(Header);   
    tmp[2] = fgetc_dbf(Header);   
    tmp[3] = fgetc_dbf(Header);
    DbfLastRec = tmp[0] + 256 * tmp[1] + (256 * 256) * tmp[2] + (256 * 256 * 256) * tmp[3];    // число записей в базе
    tmp[0] = fgetc_dbf(Header);
    tmp[1] = fgetc_dbf(Header);
    DbfBegin = tmp[0] + 256 * tmp[1];      // адрес начала первой записи
    tmp[0] = fgetc_dbf(Header);   
    tmp[1] = fgetc_dbf(Header);
    DbfRecLength = tmp[0] + 256 * tmp[1];      // длина одной записи
    fseek_dbf(Header, 14, SEEK_CUR);            //RESERVED
    tmp[0] = fgetc_dbf(Header); 
    DbfFlag = tmp[0];                      //флаг таблицы
    tmp[0] = fgetc_dbf(Header); 
    DbfCode = tmp[0];                      // кодовая страница символов
    fseek_dbf(Header, 4, SEEK_CUR);             //RESERVED

    //****************  начало таблицы полей *****************************

    // Цикл подстчета кол-во полей
    while ( (char) fgetc_dbf(Header) != 0xd ) 
    {   
        fseek_dbf(Header, 31, SEEK_CUR);
        filcount++;
    } 
    FieldCount = filcount;

    fseek_dbf(Header, 32, SEEK_SET);    //  начало таблицы полей *****************************
    //table = new field[filcount];
    Table = (struct nix_dbf_field_t*) calloc(filcount, sizeof(nix_dbf_field_t));
    if (Table == NULL) 
    { 
        DbfError=ERR_MEMORY; 
        return 1;
    }
    
    for(i = 0; i < filcount ; i++) 
    {
        Table[i].name[0] = (char) fgetc_dbf(Header);
        Table[i].name[1] = (char) fgetc_dbf(Header);
        Table[i].name[2] = (char) fgetc_dbf(Header);
        Table[i].name[3] = (char) fgetc_dbf(Header);
        Table[i].name[4] = (char) fgetc_dbf(Header);
        Table[i].name[5] = (char) fgetc_dbf(Header);
        Table[i].name[6] = (char) fgetc_dbf(Header);
        Table[i].name[7] = (char) fgetc_dbf(Header);
        Table[i].name[8] = (char) fgetc_dbf(Header);
        Table[i].name[9] = (char) fgetc_dbf(Header);
        Table[i].name[10] = (char) fgetc_dbf(Header);

        Table[i].type = (char) fgetc_dbf(Header);

        tmp[0] = fgetc_dbf(Header);
        tmp[1] = fgetc_dbf(Header);
        tmp[2] = fgetc_dbf(Header);
        tmp[3] = fgetc_dbf(Header);
        Table[i].offset = tmp[0];
        Table[i].offset += 256 * tmp[1];
        Table[i].offset += 256 * 256 * tmp[2];
        Table[i].offset += 256 * 256 * 256 * tmp[3];

        Table[i].length = fgetc_dbf(Header);
        Table[i].decimal = fgetc_dbf(Header);
        Table[i].flag = (char) fgetc_dbf(Header);
        fseek_dbf(Header, 13, SEEK_CUR);    //RESERVED

        Table[i].sm = ofst;             // расчитать смещение поля относительно записи
        ofst += Table[i].length;
        DbfCalcRecLength += Table[i].length;    // расчитать длину записи
    }
    
    //********** инициализация флагов и переменных ****************
    RecBuffer = (char*) malloc(DbfCalcRecLength);    // память под буффер записи
    if (RecBuffer == NULL) 
    { 
        DbfError = ERR_MEMORY; 
        return 1;
    }

    RecNo = 1;  
    fseek_dbf(Header, DbfBegin, SEEK_SET); 
    RecPos = DbfBegin;
    EOFFlag = '\x00'; 
    BOFFlag = '\x00';
    UpdateRecFlag = '\xff';   // требуется обновление буффера
    return 0 ;
}

//*****************************************************************
static void getdate(nix_dbf_date_t *_datep)
{
    time_t timer;
    struct tm *_cur_time_;

    /* получает время суток */
    timer = time(NULL);

    /* переводит дату и время в структуру */
    _cur_time_ = localtime(&timer);

    // Получить текущую дату и время
    _datep->da_year = _cur_time_->tm_year;
    _datep->da_mon = _cur_time_->tm_mon;
    _datep->da_day = _cur_time_->tm_mday;
}

/**
*   закрытие базы.
*/
extern int  close_dbf(void) 
{   
    nix_dbf_date_t dbf_date;

    DbfError = NOT_ERROR;
    if ( Header != NULL ) 
    {
        getdate(&dbf_date);                //
        dbf_date.da_year -= 1900;
        fseek_dbf(Header, 1, SEEK_SET);
        fputc_dbf(dbf_date.da_year , Header); //
        fputc_dbf((int) (dbf_date.da_mon), Header);
        fputc_dbf((int) (dbf_date.da_day), Header);   // дата модификации

        int buf;
        fseek_dbf(Header, -1L , SEEK_END); // если нет символа EOF, добавить его
        buf = fgetc_dbf(Header);
        if ( buf != (int) ('\x1a') ) 
        {
            fseek_dbf(Header, 0, SEEK_END);
            fputc_dbf((int) ('\x1a'), Header);
        }
        free(Table);        //delete table;
        free(RecBuffer);
        fclose(Header);
    } 
    else    
        DbfError = ERR_OPEN;
    Header = NULL;
    FieldCount = 0;
    RecNo = 0;      // текущий номер записи
    RecPos = 0 ;    // позиция в файле текущей записи (Указывает на флаг удаления)
    return 0;
}

/**
*   Получить кол-во полей.
*   вернуть кол-во полей.
*/
extern int get_field_count_dbf(void)
{
    DbfError = NOT_ERROR;
    if ( Header == NULL )  
    {
        DbfError = ERR_OPEN;
        return 0;       // база не открытая
    } 
    else 
    {
        return FieldCount;  //вернуть кол-во полей=размеру массива
    }
}


/**
*   Получить значение поля в виде стринга по его номеру.
*/
extern char* get_value_by_num_dbf(const int field_num)
{
    unsigned int i, j;

    DbfError = NOT_ERROR;
    if ( Header == NULL ) 
    { 
        DbfError = ERR_OPEN; 
        return NULL;
    }   // база не открыта
    
    if ( (field_num < 0) || (field_num > get_field_count_dbf()) ) 
    {
        DbfError = ERR_SEEK;
        return NULL;    // Вне диапазона
    }
    
    if ( UpdateRecFlag != '\x00' )
    { 
        // требуется обновление буффера
        fread(RecBuffer, DbfCalcRecLength, 1, Header);      // чтения всей записи в буффер
        UpdateRecFlag = '\x00';                       // сброс флага обновления
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
    for ( i = Table[field_num].sm; i < (Table[field_num].sm + Table[field_num].length); i++ ) 
    {
        Buffers[j++] = RecBuffer[i];
    }
    Buffers[j] = '\x00';

    return &Buffers[0];
}


/**
*   Движение по записям.
*/
extern int  skip_dbf(long kol) 
{ 
    DbfError = NOT_ERROR;
    if ( Header == NULL ) 
    {
        DbfError = ERR_OPEN; 
        return 1; 
    }       // база не открыта
    
    skip_local_dbf(kol);
    
    if ( DeleteFlag !=0 ) 
    { 
        // надо фильтровать от удаленных записей
        while ( (delete_dbf(99) != 0) && (eof_dbf() == 0) && (bof_dbf() == 0) ) 
        {
            if ( kol > 0 ) 
                skip_local_dbf(1); 
            else 
                skip_local_dbf(-1);
        }
    }
    UpdateRecFlag = '\xff';   // требуется обновление буффера
    return 0;
}


/**
*   Переход на предыдущую запись.
*/
extern int  prev_dbf(void) 
{ 
    return skip_dbf(-1);
}


/**
*   Переход на следующую запись.
*/
extern int  next_dbf(void) 
{ 
    return skip_dbf(1);
}


void skip_local_dbf(long kol) 
{
    long sm;

    BOFFlag = '\x00'; 
    BOFFlag = '\x00';
    RecNo += kol;
    if ( RecNo <= 0 ) 
    {
        BOFFlag = '\xff';
        RecNo = 1;
    }
    if ( RecNo > DbfLastRec ) 
    {
        EOFFlag = '\xff';
        RecNo = DbfLastRec;
    }
    RecPos = DbfBegin + DbfRecLength * (RecNo - 1);
    sm = RecPos - ftell_dbf(Header);
    fseek_dbf(Header, sm, SEEK_CUR);
}


/**
*   Проверка на конец файла (1-попытка выйти за конец; 0-норма)
*/
extern int  eof_dbf(void) 
{
    DbfError = NOT_ERROR;
    if ( Header == NULL ) 
    { 
        DbfError = ERR_OPEN; 
        return 1;
    }       // база не открыта
    return EOFFlag;
}


/**
*   Проверка на начало файла (1-попытка выйти за начало; 0-норма)
*/
extern int  bof_dbf(void) 
{
    DbfError = NOT_ERROR;
    if ( Header == NULL ) 
    {   
        DbfError = ERR_OPEN; 
        return 1;
    }       // база не открыта
   return BOFFlag;
}


/**
*   Удаление( без парам-проверка; 1-уст.удаление; 0-снять).
*       flag = 0 сброс удаление
*            = 1 уст.  удаление
*       без парам.  проверка удаления
*/
extern int  delete_dbf(int flag) 
{
    DbfError = NOT_ERROR;
    if ( flag == 99 ) 
        return delete_local_dbf();      // только проверить на удаление
    if ( flag == 0 ) 
    { 
        // снять маркер удаления
        fputc_dbf('\x20', Header);
        fseek_dbf(Header, -1, SEEK_CUR);
        return 0;
    }
    
    if ( flag > 0 ) 
    { 
        // поставить маркер удаления
        fputc_dbf('\x2a', Header);
        fseek_dbf(Header, -1, SEEK_CUR);
        return 1;
    }
    return 0;
}

/**
*   Проверка на удаление.
*/
int delete_local_dbf(void) 
{ 
    char del;
    del = (char) fgetc_dbf(Header);
    fseek_dbf(Header, -1 , SEEK_CUR);
    if ( del == '\x2a' ) 
        return 1;
    else 
        return 0;
}

/**
*   сброс/установка/проверка  фильтрации удаленных записей.
*/
extern int  set_delete_dbf(int flag) 
{
    DbfError = NOT_ERROR;
    if ( flag == 99 ) 
    { 
        // только проверить SET DELETE
        if ( DeleteFlag != '\x00' ) 
            return 0; 
        else 
            return 1;
    }

    // Установить SET DELETE ON/OFF
    if ( flag == '\x00' ) 
    {
        DeleteFlag = '\x00';
        return 0;
    } 
    else 
    {
        DeleteFlag = '\xff';
        return 1;
    }
}

/**
*   Добавить запись.
*/
extern int  append_dbf(void)
{ 
    int unsigned i;
    void *pointer;

    DbfError = NOT_ERROR;
    if ( Header == NULL ) 
    {
        DbfError = ERR_OPEN;
        return 1;
    }

    fseek_dbf(Header, 4, SEEK_SET);     // LASTREC()
    pointer = ++DbfLastRec;     //pointer = &(++hdr_lastrec);
    fputc_dbf((int) (*(char*)pointer), Header);  
    pointer = (char*) pointer + 1;
    fputc_dbf((int) (*(char*)pointer), Header);  
    pointer = (char*) pointer + 1;
    fputc_dbf((int) (*(char*)pointer), Header);  
    pointer = (char *) pointer + 1;
    fputc_dbf((int) (*(char*)pointer), Header); // LASTREC()****************

    RecNo = DbfLastRec;

    fseek_dbf(Header, 0, SEEK_END);     // собственно добавление
    RecPos = ftell_dbf(Header);
    fputc_dbf('\x20', Header);          // марекр удаления
    for (i = 1; DbfCalcRecLength > i; i++)     
        fputc_dbf('\x20', Header);
    fseek_dbf(Header, RecPos, SEEK_SET);
    UpdateRecFlag = '\xff';               // требуется обновление буффера
    return 0;
}

/**
*   ЗАПИСЬ В ПОЛЕ текущей записи по номеру поля.
*   установить значение в поле по его номеру.
*/
extern int set_value_by_num_dbf(char* value, int pole) 
{
    unsigned int i;
    char *val;

    DbfError = NOT_ERROR;
    if ( Header == NULL ) 
    {
        DbfError = ERR_OPEN;
        return 1;
    }
    //val = new char[table[pole].lengt+1];    //нормализация стринга для записи
    val = (char*) calloc(Table[pole].length + 1, sizeof(char));
    
    padl_dbf(value, val, Table[pole].length, '\x20');

    fseek_dbf(Header, RecPos + Table[pole].sm, SEEK_SET);
    for (i = 0; (i < Table[pole].length) && (val[i] != '\x00'); i++)
        fputc_dbf(val[i], Header);

    fseek_dbf(Header, RecPos, SEEK_SET);
    free(val);      //delete val;
    return 0;
}


// ***  переопределение системныъ функций   ***

int fputc_dbf(int a, FILE *b)
{
    int ret = fputc(a, b);
    if ( ret == EOF ) 
        DbfError = ERR_PUT;
    return ret;
}


int fgetc_dbf(FILE *a)
{
    int ret = fgetc(a);
    if ( ret == EOF ) 
        DbfError = ERR_GET;
    return ret;
}


int fseek_dbf(FILE *a, long b, int c)
{
    int ret = fseek(a, b, c);
    if ( ret != 0 )  
        DbfError = ERR_SEEK;
    return ret;
}


long int ftell_dbf(FILE *a)
{
    long int ret = ftell(a);
    if ( ret < 0 ) 
        DbfError = ERR_TELL;
    return ftell(a);
}


/**
*   Вернуть код ошибки.
*/
extern int get_error_dbf(void) 
{
    return (int) (DbfError);
}


/**
*   Получить кол-во записей в базе.
*/
extern long int get_lastrec_dbf(void) 
{
    DbfError = NOT_ERROR;
    return DbfLastRec;
}


/**
*   Поиск номера поля по его имени.
*/
extern int get_field_idx_dbf(char *name) 
{ 
    int ret = -1;
    int unsigned i;

    DbfError = NOT_ERROR;
    for (i = 0 ; i < FieldCount; i++) 
    {
        //сравнение с учетом регистра !!!!!!!!!!!!!!!!!!!!!!!!!
        if ( strcmp((Table[i].name), (name)) == 0 ) 
        {
           ret = i;
           i = FieldCount + 1;      // досрочный выход из цикла
        }
    }
    if ( ret < 0 ) 
        DbfError = ERR_FIELD;
    return ret;
}


/**
*   Получить значение поля в виде стринга по его имени.
*/
extern char* get_value_by_name_dbf(char *name) 
{
    int pole;

    DbfError = NOT_ERROR;
    pole = get_field_idx_dbf(name) ;
    if ( pole < 0 ) 
        return NULL;
    return get_value_by_num_dbf(pole);
}


/**
*   Записать значение поля в виде стринга по его имени.
*/
extern int set_value_by_name_dbf(char *value, char *name) 
{
    int pole;

    DbfError = NOT_ERROR;
    pole = get_field_idx_dbf(name);
    if ( pole < 0 ) 
        return NULL;
    return set_value_by_num_dbf(value, pole);
}


/**
*   Очистить список полей.
*/
extern int create1_dbf(void) 
{ 
    DbfError = NOT_ERROR;
    close_dbf();
    
    // table = new(field[255]);
    Table = (struct nix_dbf_field_t*) calloc(255, sizeof(nix_dbf_field_t));
    if ( Table == NULL ) 
    {
        DbfError = ERR_MEMORY; 
        return 1;
    }
    return 0;
}


/**
*   Добавить в список полей.
*/
extern int create2_dbf(char *db_name, char db_type, unsigned int lengt,
               unsigned int decimal) 
{
    unsigned int i, kolvo;

    DbfError = NOT_ERROR;
    kolvo = FieldCount;
    if ( Table == NULL ) 
    {
        DbfError = ERR_ALLOC; 
        return 1;
    }

    // db_name=strupr(db_name);
    for (i = 0; i < sizeof(Table[0].name); i++) 
        Table[kolvo].name[i] = '\x00';
    for (i = 0; db_name[i] != '\x00'; i++) 
        Table[kolvo].name[i] = db_name[i];


    if ( (db_type == 'C') ||
         (db_type == 'N') ||
         (db_type == 'F') ||
         (db_type == 'D')
        )    
        Table[kolvo].type = db_type;
    else 
    {
        DbfError = ERR_TYPE;
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
        DbfError = ERR_TYPE;
        return 1;
    }
    if ( (lengt > 0 ) && (lengt <= 255) )  
        Table[kolvo].length = lengt;
    else 
    {
        DbfError = ERR_TYPE;
        return 1;
    }
    
    if ( lengt >= (decimal + 1)) 
        Table[kolvo].decimal = decimal;
    else 
    {
        DbfError = ERR_TYPE;
        return 1;
    }
    
    // поле  определено, идет расчет служебной информации
    Table[kolvo].offset = 1;    // маркер удаления
    for (i = 1; i <= kolvo; i++) 
    {
        Table[kolvo].offset += Table[i-1].length;
    }
    Table[kolvo].sm = Table[kolvo].offset;
    FieldCount = kolvo + 1;
    return 0;
}


extern int create3_dbf(char* name)
{
    FILE  *out;
    int unsigned i, buf, begin, lengt;
    void *pointer;
    nix_dbf_date_t dbf_date;

    DbfError = NOT_ERROR;
    if ((out = fopen(name, "wb")) == NULL) 
    { 
        DbfError = ERR_OPEN;
        return 1;
    }

    fputc_dbf('\x03', out);      // тип базы-1

    getdate(&dbf_date);                //
    if (dbf_date.da_year <= 1999) 
        dbf_date.da_year -= 1900; 
    else 
        dbf_date.da_year -= 1900;
    fputc_dbf(dbf_date.da_year, out); //
    fputc_dbf((int) (dbf_date.da_mon), out);
    fputc_dbf((int) (dbf_date.da_day), out);   // дата модификации

    fputc_dbf('\x00', out);  
    fputc_dbf('\x00', out);
    fputc_dbf('\x00', out);  
    fputc_dbf('\x00', out); // кол-во записей

    // считается что таблица полей заполнена
    begin = 32;     // 31-- длина заголовка до таблицы полей
    lengt = 1;      // 1- флаг удаления
    for (i = 0; i < FieldCount; i++) 
    {
        begin += 32;    // 32-длина записи таблицы полей
        lengt += Table[i].length;
    }
    // begin +=264; // 264-длина заголовка после таблицы полей
    begin += 1;     // для терминатора
    pointer = &begin;
    fputc_dbf((int) (*(char*) pointer), out);  
    pointer = (char*) pointer + 1;
    fputc_dbf((int) (*(char*) pointer), out);   // позиция начала записей
                                            // млад. байт + ст.байт

    pointer = &lengt;
    fputc_dbf((int) (*(char*) pointer), out);  
    pointer = (char*) pointer + 1;
    fputc_dbf((int) (*(char*) pointer), out);   // длина записи

    for (i = 0; i < 16; i++) 
        fputc_dbf('\x00', out);      // RESERVED
    fputc_dbf((int) ('\x00'), out);     // флаг заголовка ????
    fputc_dbf((int) ('\xc9'), out);     // кодовая таблица  Rusian Windows
    fputc_dbf((int) ('\x00'), out);
    fputc_dbf((int) ('\x00'), out);     // RESERVED

    for (i = 0; i < FieldCount; i++) 
    {
        fputc_dbf((int) (Table[i].name[0]), out);
        fputc_dbf((int) (Table[i].name[1]), out);
        fputc_dbf((int) (Table[i].name[2]), out);
        fputc_dbf((int) (Table[i].name[3]), out);
        fputc_dbf((int) (Table[i].name[4]), out);
        fputc_dbf((int) (Table[i].name[5]), out);
        fputc_dbf((int) (Table[i].name[6]), out);
        fputc_dbf((int) (Table[i].name[7]), out);
        fputc_dbf((int) (Table[i].name[8]), out);
        fputc_dbf((int) (Table[i].name[9]), out);
        fputc_dbf((int) (Table[i].name[10]), out);    // имя поля

        fputc_dbf((int) (Table[i].type), out);        // тип поля

        pointer = &(Table[i].offset);
        fputc_dbf((int) (*(char*) pointer), out);  
        pointer = (char*) pointer + 1;
        fputc_dbf((int) (*(char*) pointer), out);  
        pointer=(char*) pointer + 1;
        fputc_dbf((int) (*(char*) pointer), out);  
        pointer=(char*) pointer + 1;
        fputc_dbf((int) (*(char*) pointer), out);   // смещение поля

        fputc_dbf((int) (Table[i].length), out);    // длина поля
        fputc_dbf((int) (Table[i].decimal), out);   // позиция десятич. точки

        fputc_dbf('\x00', out);                  // флаг поля ???????
        for (buf = 0; buf < 13; buf++) 
            fputc_dbf('\x00', out);              // RESERVED
    }
    fputc_dbf('\x0d', out);                      // терминатор таблицы полей
    //   for(i=0; i<263; i++) db_fputc('\x00', out); // RESERVSD
    fclose(out);
    close_dbf();
    return 0;
}


/**
*   Позиционировать по номеру записи.
*/
extern int goto_dbf(long rec) 
{ 
    DbfError = NOT_ERROR;
    BOFFlag = '\x00'; 
    EOFFlag = '\x00';

    if ( Header == NULL ) 
    {
        DbfError = ERR_OPEN; 
        return 1;
    }   // база не открыта
    
    if ( rec < 1 ) 
    { 
        goto_dbf(1); 
        BOFFlag = '\xff'; 
        EOFFlag = '\x00'; 
        return 0;
    }
    
    if ( rec > (long) (DbfLastRec)) 
    {
        goto_dbf(DbfLastRec);
        BOFFlag = '\x00'; 
        EOFFlag = '\xff';
        return 0;
    }
    RecNo = rec;
    RecPos = DbfBegin + DbfRecLength * (RecNo - 1);
    fseek_dbf(Header, RecPos, SEEK_SET);

    if ( DeleteFlag != 0) 
    {
        while ( (delete_dbf(99) != 0) && (eof_dbf() == 0) )   
            skip_dbf(1);      // искать вниз ближ. НЕудаленную запись
    }
    UpdateRecFlag = '\xff';       // требуется обновление буффера
    return 0;
}

/**
*   Быстрый ??? поиск в упорядоченной базе.
*/
extern int find_sort_dbf(char *pole, char *isk_stroka) 
{ 
    long  unsigned pos, pos_begin, pos_end, sm;
    char *buf, *isk;
    int numpole, tmp, ret = 0;
    char find_flag = '\x00';

    numpole = get_field_idx_dbf(pole);
    if ( numpole < 0 ) 
        return 0;       // нет такого поля

    //isk = new char[table[numpole].lengt + 1];
    isk = (char*) calloc(Table[numpole].length + 1, sizeof(char));
    padl_dbf(isk_stroka , isk , Table[numpole].length, '\x20');
    pos_begin = RecNo; 
    pos_end = DbfLastRec;
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
        goto_dbf(pos);
        buf = get_value_by_num_dbf(numpole);
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
        if ( (pos_end < 0) || (pos_begin > DbfLastRec)) 
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

    goto_dbf(pos_end);

    return ret;
}


/**
*   Копирование в новую строку(слева направо) с дополнение слева.
*/
char *padl_dbf(char *in, char *out, int lengt, char sym)
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
extern long get_recno_dbf(void) 
{ 
    return RecNo;
}

/**
*   Проверить есть ли открытая база.
*/
extern char is_used_dbf(void) 
{ 
    if ( Header == NULL ) 
        return '\x00';
    else            
        return '\xff';
}


extern char* get_field_name_dbf(int num) 
{
    if ( is_used_dbf == 0) 
    {
        DbfError = ERR_OPEN; 
        return NULL;
    }
    if ( (num < 0) || (num >= (int) (FieldCount)) ) 
    {
        DbfError = ERR_FIELD; 
        return NULL;
    }

    return Table[num].name;
} 


extern char get_field_type_dbf(int num) 
{
    if ( is_used_dbf == 0) 
    {
        DbfError = ERR_OPEN; 
        return NULL;
    }
    if ( (num < 0) || (num >= (int) (FieldCount)) ) 
    {
        DbfError = ERR_FIELD; 
        return NULL;
    }

    return Table[num].type;
} 


extern int get_field_lengt_dbf(int num) 
{
    if ( is_used_dbf == 0) 
    {
        DbfError = ERR_OPEN; 
        return NULL;
    }
    if ( (num < 0) || (num >= (int) (FieldCount)) ) 
    {
        DbfError = ERR_FIELD; 
        return NULL;
    }

    return Table[num].length;
} 


extern int get_field_decimal_dbf(int num) 
{
    if ( is_used_dbf == 0) 
    {
        DbfError = ERR_OPEN; 
        return NULL;
    }
    
    if ( (num < 0) || (num >= (int) (FieldCount)) ) 
    {
        DbfError = ERR_FIELD; 
        return NULL;
    }

    return Table[num].decimal;
} 


/**
*   УДАЛЕНИЕ всех записей базы.
*/
extern int zap_dbf(void) 
{
    FILE  *out;
    unsigned long i;
    nix_dbf_date_t dbf_date;

    DbfError = NOT_ERROR;
    if( Header == NULL ) 
    {
        DbfError = ERR_OPEN;
        return 1;
    }
    if ( (out = fopen(TMP_NAME, "wb")) == NULL ) 
    { 
        DbfError=ERR_OPEN;
        return 1;
    }

    fseek_dbf(Header, 0, SEEK_SET);
    for (i = 0; i < DbfBegin; i++)     
        fputc_dbf(fgetc_dbf(Header), out);   // перезапись всего заголовка

    fseek_dbf(out, 1, SEEK_SET);
    getdate(&dbf_date);                    // ------------------
    dbf_date.da_year -= 1900;
    fputc_dbf(dbf_date.da_year , out);      //
    fputc_dbf((int) (dbf_date.da_mon), out);
    fputc_dbf((int) (dbf_date.da_day), out);   // дата модификации

    fputc_dbf('\x00', out);    // Число записей в базе =0
    fputc_dbf('\x00', out);
    fputc_dbf('\x00', out);
    fputc_dbf('\x00', out);   //------------------------

    fclose(out);
    close_dbf();

    if ( get_error_dbf() == 0 ) 
    {
        if ( remove(DbfFileName) != 0 ) 
        {
            DbfError = ERR_DEL;
            return 1;
        }
        if ( rename(TMP_NAME, DbfFileName) != 0 ) 
        {
            DbfError = ERR_RENAME;
            return 1;
        }
    }
    open_dbf(DbfFileName);
    UpdateRecFlag = '\xff';       // требуется обновление буффера
    return 0;
}


/**
*   Упаковка записей базы.
*/
extern int pack_dbf(void) 
{
    FILE *out;
    unsigned long i, j, count = 0;
    char buf;
    void *pointer;

    DbfError = NOT_ERROR;
    if( Header == NULL ) 
    {
        DbfError = ERR_OPEN;
        return 1;
    }
    if ( (out = fopen(TMP_NAME, "wb")) == NULL ) 
    { 
        DbfError = ERR_OPEN;
        return 1;
    }

    fseek_dbf(Header, 0, SEEK_SET);
    for(i = 0; i < DbfBegin; i++)     
        fputc_dbf(fgetc_dbf(Header), out);   // перезапись всего заголовка

    fseek_dbf(out, DbfBegin, SEEK_SET); // начинается перебор записей
    for (j = 0; j < DbfLastRec; j++) 
    {
        buf = (char) fgetc_dbf(Header);
        if( buf != '\x2a' ) 
        {
            fputc_dbf(buf, out);
            for (i = 1; i < DbfCalcRecLength; i++) 
            {
                buf = (char) fgetc_dbf(Header);
                fputc_dbf(buf, out);
            }
            count++;
        } 
        else 
        {
            fseek_dbf(Header, DbfCalcRecLength - 1, SEEK_CUR);     // Запись не удаленная, пропуск ее
        }
    }
    fseek_dbf(out, 4, SEEK_SET);     // корректировка числа записей в заголовке
    pointer = &(count);
    fputc_dbf((int) (*(char*) pointer), out);  
    pointer = (char *) pointer + 1;
    fputc_dbf((int) (*(char*) pointer), out);  
    pointer = (char *) pointer + 1;
    fputc_dbf((int) (*(char*) pointer), out);  
    pointer = (char *) pointer + 1;
    fputc_dbf((int) (*(char*) pointer), out);   // Число записей в базе

    fclose(out);
    close_dbf();
    if ( get_error_dbf() == 0 ) 
    {
        if( remove(DbfFileName) != 0 ) 
        {            
            DbfError = ERR_DEL;
            return 1;
        }
        if ( rename(TMP_NAME, DbfFileName) != 0 ) 
        {
            DbfError = ERR_RENAME;
            return 1;
        }
    }

    open_dbf(DbfFileName);
    UpdateRecFlag = '\xff';       // требуется обновление буффера
    return 0;
}


/**
*   Распечатать ошибку БД.
*/
extern void print_error_dbf(int err_code)
{
    if ( err_code == ERR_OPEN  )
        printf("ERROR: Файл не возможно открыть");    
    else if ( err_code == ERR_GET )
        printf("ERROR: Ошибка чтения байта из файла");    
    else if ( err_code == ERR_PUT )
        printf("ERROR: Ошибка записи байта в файл");    
    else if ( err_code == ERR_SEEK )
        printf("ERROR: Ошибка при позиционировании файла");    
    else if ( err_code == ERR_TELL )
        printf("ERROR: Ошибка при определении позиции файла");    
    else if ( err_code == ERR_FIELD )
        printf("ERROR: Ошибка поиска имени поля (поле с таким именем не найдено)");    
    else if ( err_code == ERR_TYPE )
        printf("ERROR: Ошибка. Неверно указанный тип поля при создании базы");    
    else if ( err_code == ERR_MEMORY )
        printf("ERROR: Ошибка выделения памяти (не хватает памяти)");    
    else if ( err_code == ERR_ALLOC )
        printf("ERROR: Ошибка. Память не выделена");    
    else if ( err_code == ERR_LIBRARY )
        printf("ERROR: Ошибка. Библиотека dll_dbf.dll не может быть загружена");    
    else if ( err_code == ERR_DEL )
        printf("ERROR: Ошибка. Не могу удалить файл ( возможно где-то он используется)");    
    else if ( err_code == ERR_RENAME )
        printf("ERROR: Ошибка. Не могу переименовать временный файл");    
    else
        printf("Не обрабатываемый код ошибки <%d>", err_code);        
}
