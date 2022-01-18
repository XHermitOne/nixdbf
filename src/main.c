/**
* Модуль функции MAIN
* @file
*/
#include "main.h"

/**
* Основная функция MAIN. Анализ параметров коммандной строки и 
* установка этих параметров. 
*
* Параметры коммандной строки:
* @param \--help|-h|-? Напечатать строки помощи
* @param \--version|-v  Напечатать версию программы
* @param \--debug|-D Включить режим отладки
* @param \--log|-L Включить режим журналирования
*/

int main (int argc, char **argv)
{
    int result = 0;
    
    open_log(NULL);
    
    if (check_parameters("--help", argc, argv) || check_parameters("-h", argc, argv) || check_parameters("-?", argc, argv))
    {
        print_help();
        return 0;
    }

    if (check_parameters("--version", argc, argv) || check_parameters("-v", argc, argv))
    {
        print_version ();
        return 0;
    }

    if ((check_parameters("--debug", argc, argv) || check_parameters("-D", argc, argv)) || (check_parameters("--log", argc, argv) || check_parameters("-L", argc, argv)))
        DebugMode = TRUE;
    else
        DebugMode = FALSE;

    result = run(argc, argv);

    close_log();
    
    return result;
}

