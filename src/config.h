/**
* Модуль для работа с конфигурации.
* сохранение и востановление параметров...
* @file
* @version 0.0.0.1
*/
#if !defined( __CONFIG_H )
#define __CONFIG_H

#include "main.h"

#define   STD_STR_LEN   80

/**
* Структура хранения данных в конфиге
*/
typedef struct
{
  const char    *name;        /**< Имя параметра */
  char          *location;    /**< Значение */
} nix_config_t;


/**
*полностью параметры прочитать, записать...
*/
BOOL save_config (char *cfg_filename, nix_config_t *strings, int count);
BOOL load_config(const char *cfg_filename, nix_config_t *strings, int count);

/**
* Путь к папке конфига
*/
char *get_config_path(void);

#endif  /*__CONFIG_H*/

