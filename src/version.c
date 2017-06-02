/**
* Модуль функций всё что связано с версией...
* в этом модуле будет хранится версия программы
*
* version - "vXX.YY [DD.MM.YEAR]
*   XX - старший разряд версии 
*   YY - младший, реально кол-во фиксов, исправленных багов в версии XX
*        смотри файлик changelog.txt
* @file
*/

#include "version.h"

const int XV = 0;  /**< XX */
const int YV = 7;  /**< YY */

static char version[100];
char *getVersion(void)
{
    char build_time[100];
    strcpy(build_time, __TIMESTAMP__);
    sprintf(version, "v%i.%2i [%s]", XV, YV, build_time);
    return version;
}

/**
* Вывести на экран версию программы
*/
void printVersion(void)
{
    printf("NixDBF version: ");
    printf(getVersion());
    printf("\n");
}

static char HelpTxt[]="\n\
Параметры коммандной строки:\n\
\n\
    ./nixdbf [Параметры]\n\
\n\
Параметры:\n\
\n\
    [Помощь и отладка]\n\
        --help|-h|-?        Напечатать строки помощи\n\
        --version|-v        Напечатать версию программы\n\
        --debug|-D          Включить режим отладки\n\
        --log|-L            Включить режим журналирования\n\
\n\
    [Параметры запуска]\n\
        --dbf=              DBF файл \n\
        --cmd=              Комманда DBF: SELECT, DELETE, CLEAR, FIELDS, LENGTH и т.п \n\
        --where=            Параметры фильтрации\n\
        --limit=            Ограничение количества записей\n\
        --fields=           Отображаемые поля таблицы\n\
        --order_by=         Сортировка по полю\n\
        --reverse           Сортировка в обратном порядке\n\
        --start_rec=        Первая обрабатываемая запись\n\        
        --src_codepage=     Кодовая страница DBF таблицы\n\
        --dst_codepage=     Кодовая страница, в которой необходимо вывести данные\n\
\n\
";

/**
* Вывести на экран помощь
*/
void printHelp(void)
{
    printf("NixDBF программа вывода на экран результатов запроса к таблице DBF: \n");
    printf(HelpTxt);
    printf("\n");
}

/**
* Вывести на экран системной информации(для выявления утечек памяти)
*/
void printSysInfo(void)
{
    struct sysinfo info;

    if (sysinfo(&info) != 0)
    {
        printf("sysinfo: error reading system statistics");
        return;
    }

    printf("System information:");
    printf("\tUptime: %ld:%ld:%ld\n", info.uptime/3600, info.uptime%3600/60, info.uptime%60);
    printf("\tTotal RAM: %ld MB\n", info.totalram/1024/1024);
    printf("\tFree RAM: %ld MB\n", (info.totalram-info.freeram)/1024/1024);
    printf("\tProcess count: %d\n", info.procs);
}
