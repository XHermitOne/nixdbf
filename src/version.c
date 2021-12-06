/**
* Модуль функций всё что связано с версией...
* в этом модуле будет хранится версия программы
*
* version - "MajorVersion.MinorVersion [DD.MM.YEAR]
*   MajorVersion - старший разряд версии
*   MinorVersion - младший, реально кол-во фиксов, исправленных багов в версии XX
*        смотри файлик changelog.txt
* @file
* @version 0.0.0.1
*/


#include "version.h"

const int MajorVersion = 1;  /**< старший разряд версии */
const int MinorVersion = 1;  /**< младший разряд версии */


static char Version[100];
char *get_version(void)
{
    char build_time[100];
    strcpy(build_time, __TIMESTAMP__);
    sprintf(Version, "v%i.%2i [%s]", MajorVersion, MinorVersion, build_time);
    return Version;
}

/**
* Вывести на экран версию программы
*/
void print_version(void)
{
    printf("NixDbf version: ");
    printf(get_version());
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
\n\
";

/**
* Вывести на экран помощь
*/
void print_help(void)
{
    printf("NixDbf: Программа отображения на экране содержимого DBF файла: \n");
    printf(HelpTxt);
    printf("\n");
}

/**
* Вывести на экран системной информации(для выявления утечек памяти)
*/
void print_system_info(void)
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
