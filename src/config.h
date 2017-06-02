/**
* Модуль для работа с конфигурации.
* сохранение и востановление параметров...
* @file
*/


#if !defined( __CONFIG_H )
#define __CONFIG_H

#include "main.h"

#define     STD_STR_LEN     80

/**
*полностью параметры прочитать, записать...
*/
//void LoadConfig(PrintParam_t *P);
//void SaveConfig(PrintParam_t *P);

/**
* Структура хранения данных в конфиге
*/
typedef struct
{
    const char	*name;  /**< Имя параметра */
	char    *location;  /**< Значение */
} Config_t;


BOOL saveConfig (char *CFGFileName, Config_t *Strings, int Count);
BOOL loadConfig(const char *CFGFileName, Config_t *Strings, int Count);

/**
* Путь к папке конфига
*/
char *getCfgPath(void);

#endif

